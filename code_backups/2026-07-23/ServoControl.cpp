// ServoControl.cpp - M103 舵机控制
// ESP32 LEDC 50Hz PWM, 500-2500us -> 0-180deg
// speed_dps=0 瞬移, >0 渐变转动
// v2: FreeRTOS 异步任务化 - 消除阻塞delay, 支持急停
#include "ServoControl.h"

// ---------- 静态变量 ----------
static TaskHandle_t  srv_task_handle = NULL;
static QueueHandle_t srv_cmd_queue   = NULL;
static bool          srv_ok          = false;
static volatile bool srv_moving      = false;
static volatile bool srv_stop_flag   = false;
static int8_t        srv_chan        = -1;
static float         srv_tpb         = 0.0f;
static float         srv_cur_deg     = 90.0f;

// ---------- 工具函数 ----------
static float deg_to_pulse_us(float deg) {
    return (float)SCTL_MIN_PULSE_US
        + (deg / 180.0f) * (float)(SCTL_MAX_PULSE_US - SCTL_MIN_PULSE_US);
}

static uint32_t pulse_to_duty(float us) {
    return (uint32_t)(us / 1000000.0f / srv_tpb);
}

static void write_pwm(float deg) {
    if (srv_chan >= 0) {
        ledcWrite(srv_chan, pulse_to_duty(deg_to_pulse_us(deg)));
    }
}

// ---------- FreeRTOS 任务 ----------
void servo_task(void* pvParameters) {
    servo_cmd_t cmd;

    while (true) {
        if (xQueueReceive(srv_cmd_queue, &cmd, portMAX_DELAY) == pdTRUE) {
            srv_moving = true;

            float angle     = cmd.angle;
            float speed_dps = cmd.speed_dps;

            if (angle < 0.0f)   angle = 0.0f;
            if (angle > 180.0f) angle = 180.0f;

            if (speed_dps <= 0.0f) {
                write_pwm(angle);
            } else {
                float step_deg = speed_dps
                    * ((float)SCTL_STEP_INTERVAL_MS / 1000.0f);
                if (step_deg < 0.5f) step_deg = 0.5f;

                int dir = (angle > srv_cur_deg) ? 1 : -1;
                while ((dir > 0 && srv_cur_deg < angle)
                    || (dir < 0 && srv_cur_deg > angle)) {
                    if (srv_stop_flag) { srv_stop_flag = false; break; }
                    srv_cur_deg += (float)dir * step_deg;
                    if ((dir > 0 && srv_cur_deg > angle)
                     || (dir < 0 && srv_cur_deg < angle))
                        srv_cur_deg = angle;
                    write_pwm(srv_cur_deg);
                    vTaskDelay(pdMS_TO_TICKS(SCTL_STEP_INTERVAL_MS));
                }
            }
            srv_cur_deg = angle;
            // Only clear srv_moving when no more commands are queued
            if (uxQueueMessagesWaiting(srv_cmd_queue) == 0) {
                srv_moving = false;
            }

            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info,
                "Servo: %.0f deg, %.0f deg/s",
                (double)angle,
                speed_dps > 0 ? (double)speed_dps : (double)SCTL_DEFAULT_SPEED);
        }
    }
}

// ---------- 初始化 ----------
void servo_init() {
#ifdef SERVO_PIN
    srv_chan = sys_get_next_PWM_chan_num();
    if (srv_chan < 0) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error,
            "Servo: No free LEDC channel");
        return;
    }
    srv_tpb = (1.0f / (float)SCTL_PULSE_FREQ)
            / (float)(1 << SCTL_PULSE_RES_BITS);
    ledcSetup(srv_chan, (double)SCTL_PULSE_FREQ, SCTL_PULSE_RES_BITS);
    ledcWrite(srv_chan, pulse_to_duty(deg_to_pulse_us((float)SCTL_RESET_ANGLE)));
    ledcAttachPin((int)SERVO_PIN, srv_chan);
    srv_cur_deg = (float)SCTL_RESET_ANGLE;

    srv_cmd_queue = xQueueCreate(SCTL_QUEUE_LEN, sizeof(servo_cmd_t));
    if (srv_cmd_queue == NULL) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error,
            "Servo: Failed to create queue");
        return;
    }

    BaseType_t ret = xTaskCreatePinnedToCore(
        servo_task, "servo_task", 2048, NULL, 1, &srv_task_handle, tskNO_AFFINITY);
    if (ret != pdPASS) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error,
            "Servo: Failed to create task");
        vQueueDelete(srv_cmd_queue);
        srv_cmd_queue = NULL;
        return;
    }

    srv_ok = true;
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info,
        "Servo Pin:%s ch:%d (0-180deg, %d deg/s max, FreeRTOS async)",
        pinName((int)SERVO_PIN).c_str(), srv_chan, SCTL_DEFAULT_SPEED);
#endif
}

void servo_set_angle(float angle, float speed_dps) {
#ifdef SERVO_PIN
    if (!srv_ok || srv_cmd_queue == NULL) return;

    // Wait for any pending motion in the planner to complete first
    protocol_buffer_synchronize();

    if (angle < 0.0f)   angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;

    servo_cmd_t cmd;
    cmd.angle     = angle;
    cmd.speed_dps = speed_dps;

    // Set moving flag BEFORE sending to queue so the wait loop
    // below correctly blocks until servo_task finishes.
    srv_moving = true;

    if (xQueueSend(srv_cmd_queue, &cmd, pdMS_TO_TICKS(10000)) != pdTRUE) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error,
            "Servo: queue full, command rejected");
        if (uxQueueMessagesWaiting(srv_cmd_queue) == 0) {
            srv_moving = false;
        }
        return;
    }

    // Wait for servo to complete (blocks GCode parser)
    uint32_t start_ms = millis();
    while (srv_moving) {
        if (millis() - start_ms > 120000) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error,
                "Servo: timeout waiting for completion");
            break;
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
#endif
}

void servo_reset() {
#ifdef SERVO_PIN
    if (!srv_ok || srv_cmd_queue == NULL) return;
    servo_set_angle((float)SCTL_RESET_ANGLE, 0);
#endif
}

void servo_stop() {
#ifdef SERVO_PIN
    if (!srv_ok || srv_cmd_queue == NULL) return;
    srv_stop_flag = true;
    xQueueReset(srv_cmd_queue);
    ledcWrite(srv_chan, pulse_to_duty(deg_to_pulse_us(srv_cur_deg)));
    srv_moving = false;
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Servo stopped at %.0f deg", (double)srv_cur_deg);
#endif
}

void servo_disable() {
#ifdef SERVO_PIN
    if (!srv_ok) return;
    ledcWrite(srv_chan, 0);
    srv_moving = false;
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Servo disabled");
#endif
}
