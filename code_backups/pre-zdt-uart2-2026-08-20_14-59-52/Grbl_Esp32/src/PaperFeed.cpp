// PaperFeed.cpp - 独立进纸电机控制实现
// 直接 GPIO 翻转产生步进脉冲，不经过 Stepper ISR / Planner
// v2: FreeRTOS 异步任务化 - 阻塞式步进循环跑在独立任务中
//     主循环安全阻塞等待，支持 M102 急停 + TWDT 喂狗
//     使能脚独立控制（不拉全局 STEPPERS_DISABLE_PIN）
#include "PaperFeed.h"
#include "Protocol.h"
#include <driver/gpio.h>

// ---------- 静态变量 ----------
static TaskHandle_t   pf_task_handle  = NULL;
static QueueHandle_t  pf_cmd_queue    = NULL;
static bool           pf_initialized  = false;
static volatile bool  pf_moving       = false;
static volatile bool  pf_stop_flag    = false;
static volatile bool  pf_task_done    = false;
static uint8_t        pf_step_pin;
static uint8_t        pf_dir_pin;
static uint8_t        pf_disable_pin;
static uint32_t       pf_steps_per_mm = PAPER_FEED_STEPS_PER_MM;
static uint32_t       pf_pulse_us     = PAPER_FEED_PULSE_US;
static uint32_t       pf_dir_delay_us = PAPER_FEED_DIR_DELAY_US;

// ---------- FreeRTOS 任务 ----------
void paper_feed_task(void* pvParameters) {
    paper_feed_cmd_t cmd;
    while (true) {
        if (xQueueReceive(pf_cmd_queue, &cmd, portMAX_DELAY) == pdTRUE) {
            pf_stop_flag = false;
            pf_task_done = false;
            float mm = cmd.mm, rate = cmd.rate;
            bool forward = cmd.forward;
            if (mm <= 0.0f || !pf_initialized) { pf_task_done = true; pf_moving = false; continue; }
            if (rate <= 0.0f) rate = PAPER_FEED_DEFAULT_RATE;
            int32_t steps = (int32_t)(mm * pf_steps_per_mm);
            if (steps <= 0) { pf_task_done = true; pf_moving = false; continue; }
            digitalWrite(pf_disable_pin, LOW);
            digitalWrite(pf_dir_pin, forward ? HIGH : LOW);
            delayMicroseconds(pf_dir_delay_us);
            uint32_t interval_us = (uint32_t)(60000000.0f / (rate * pf_steps_per_mm));
            if (interval_us < pf_pulse_us + 2) interval_us = pf_pulse_us + 2;
            uint32_t low_time = interval_us - pf_pulse_us;
            uint32_t wdt_count = 0;
            uint32_t wdt_interval_steps = (rate * pf_steps_per_mm) / (60 * (1000 / PAPER_FEED_WDT_INTERVAL_MS));
            if (wdt_interval_steps < 1) wdt_interval_steps = 1;
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Paper feed: %.1fmm %s at %.0fmm/min (%d steps)", (double)mm, forward ? "forward" : "backward", (double)rate, steps);
            for (int32_t i = 0; i < steps; i++) {
                if (pf_stop_flag) { grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Paper feed stopped at step %d of %d", i, steps); break; }
                digitalWrite(pf_step_pin, HIGH);
                delayMicroseconds(pf_pulse_us);
                digitalWrite(pf_step_pin, LOW);
                delayMicroseconds(low_time);
                if (++wdt_count >= wdt_interval_steps) { wdt_count = 0; esp_task_wdt_reset(); vTaskDelay(0); }
            }
            pf_task_done = true;
            pf_moving    = false;
        }
    }
}

// ---------- 对外接口 ----------
void paper_feed_init() {
    pf_step_pin = PAPER_FEED_STEP_PIN;
    pf_dir_pin = PAPER_FEED_DIR_PIN;
    pf_disable_pin = PAPER_FEED_DISABLE_PIN;
    pinMode(pf_step_pin, OUTPUT);
    pinMode(pf_dir_pin, OUTPUT);
    pinMode(pf_disable_pin, OUTPUT);
    digitalWrite(pf_step_pin, LOW);
    digitalWrite(pf_dir_pin, LOW);
    pf_cmd_queue = xQueueCreate(PAPER_FEED_QUEUE_LEN, sizeof(paper_feed_cmd_t));
    if (!pf_cmd_queue) { grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error, "Paper Feed: Failed to create queue"); return; }
    BaseType_t ret = xTaskCreatePinnedToCore(paper_feed_task, "paper_feed_task", 4096, NULL, 1, &pf_task_handle, tskNO_AFFINITY);
    if (ret != pdPASS) { grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error, "Paper Feed: Failed to create task"); vQueueDelete(pf_cmd_queue); pf_cmd_queue = NULL; return; }
    pf_initialized = true; pf_moving = false;
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Paper Feed Step:%s Dir:%s Disable:%s (FreeRTOS async, GPIO isolated)", pinName(pf_step_pin).c_str(), pinName(pf_dir_pin).c_str(), pinName(pf_disable_pin).c_str());
}

void paper_feed_sync(float mm, float rate, bool forward) {
    if (!pf_initialized || pf_cmd_queue == NULL || mm <= 0.0f) return;
    protocol_buffer_synchronize();
    if (rate <= 0.0f) rate = PAPER_FEED_DEFAULT_RATE;
    pf_moving = true;
    pf_task_done = false;
    paper_feed_cmd_t cmd = { mm, rate, forward };
    if (xQueueSend(pf_cmd_queue, &cmd, pdMS_TO_TICKS(10000)) != pdTRUE) { grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error, "Paper feed: queue full, rejected"); pf_moving = false; return; }
    uint32_t start_ms = millis();
    while (!pf_task_done && pf_moving) {
        if (millis() - start_ms > 300000) { grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error, "Paper feed: timeout"); pf_stop_flag = true; pf_moving = false; break; }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void paper_feed_stop() {
    if (!pf_initialized) return;
    if (pf_cmd_queue) xQueueReset(pf_cmd_queue);
    pf_stop_flag = true; pf_moving = false;
    digitalWrite(pf_step_pin, LOW);
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Paper feed emergency stopped");
}

bool paper_feed_is_moving() { return pf_moving; }
