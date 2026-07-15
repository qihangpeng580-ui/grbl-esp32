// ServoControl.cpp - 独立舵机控制实现
// ESP32 LEDC 50Hz PWM, 500-2500us -> 0-180deg
// speed_dps=0 瞬移, >0 逐步渐变
// 完全独立, 与 PaperFeed / CoreXY 零耦合

#include "ServoControl.h"

static bool     srv_ok      = false;
static int8_t   srv_chan    = -1;
static float    srv_tpb     = 0.0f;
static float    srv_cur_deg = 90.0f;

static float deg_to_pulse_us(float deg) {
    return (float)SCTL_MIN_PULSE_US
        + (deg / 180.0f) * (float)(SCTL_MAX_PULSE_US - SCTL_MIN_PULSE_US);
}
static uint32_t pulse_to_duty(float us) {
    return (uint32_t)(us / 1000000.0f / srv_tpb);
}
static void write_pwm(float deg) {
    ledcWrite(srv_chan, pulse_to_duty(deg_to_pulse_us(deg)));
}

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
    ledcAttachPin((int)SERVO_PIN, srv_chan);
    write_pwm((float)SCTL_DEFAULT_ANGLE);
    srv_ok = true;
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info,
        "Servo Pin:%s ch:%d (0-180deg, %d deg/s max)",
        pinName((int)SERVO_PIN).c_str(), srv_chan, SCTL_DEFAULT_SPEED);
#endif
}

void servo_set_angle(float angle, float speed_dps) {
#ifdef SERVO_PIN
    if (!srv_ok) return;
    if (angle < 0.0f)    angle = 0.0f;
    if (angle > 180.0f)  angle = 180.0f;

    if (speed_dps <= 0.0f) {
        // 瞬移
        write_pwm(angle);
    } else {
        // 逐步渐变
        float step_deg = speed_dps
            * ((float)SCTL_STEP_INTERVAL_MS / 1000.0f);
        if (step_deg < 0.5f) step_deg = 0.5f;
        int dir = (angle > srv_cur_deg) ? 1 : -1;
        while ((dir > 0 && srv_cur_deg < angle)
            || (dir < 0 && srv_cur_deg > angle)) {
            srv_cur_deg += (float)dir * step_deg;
            if ((dir > 0 && srv_cur_deg > angle)
             || (dir < 0 && srv_cur_deg < angle))
                srv_cur_deg = angle;
            write_pwm(srv_cur_deg);
            delay(SCTL_STEP_INTERVAL_MS);
        }
    }
    srv_cur_deg = angle;
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info,
        "Servo: %.0f deg, %.0f deg/s",
        (double)angle,
        speed_dps > 0 ? (double)speed_dps : (double)SCTL_DEFAULT_SPEED);
#endif
}

void servo_disable() {
#ifdef SERVO_PIN
    if (!srv_ok) return;
    ledcWrite(srv_chan, 0);
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info,
        "Servo disabled");
#endif
}