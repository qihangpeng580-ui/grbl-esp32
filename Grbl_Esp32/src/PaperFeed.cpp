// PaperFeed.cpp - 独立进纸电机控制实现
// 直接 GPIO 翻转产生步进脉冲，不经过 Stepper ISR / Planner
// 阻塞式执行，在 Planner 清空后调用
// 使能引脚与轴共用 STEPPERS_DISABLE_PIN，通过 motors_set_disable() 统一管理

#include "PaperFeed.h"
#include <driver/gpio.h>

static bool     pf_initialized = false;
static bool     pf_moving      = false;
static uint8_t  pf_step_pin;
static uint8_t  pf_dir_pin;
static uint32_t pf_steps_per_mm = PAPER_FEED_STEPS_PER_MM;
static uint32_t pf_pulse_us     = PAPER_FEED_PULSE_US;
static uint32_t pf_dir_delay_us = PAPER_FEED_DIR_DELAY_US;

void paper_feed_init() {
    pf_step_pin = PAPER_FEED_STEP_PIN;
    pf_dir_pin  = PAPER_FEED_DIR_PIN;

    pinMode(pf_step_pin, OUTPUT);
    pinMode(pf_dir_pin, OUTPUT);

    // 使能脚由 motors_set_disable() 统一管理，不单独初始化

    digitalWrite(pf_step_pin, LOW);
    digitalWrite(pf_dir_pin, LOW);

    pf_initialized = true;
    pf_moving      = false;

    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info,
        "Paper Feed Step:%s Dir:%s (Disable shared with axes)",
        pinName(pf_step_pin).c_str(),
        pinName(pf_dir_pin).c_str());
}

void paper_feed_sync(float mm, float rate, bool forward) {
    if (!pf_initialized || mm <= 0.0f) return;

    protocol_buffer_synchronize();

    if (rate <= 0.0f) rate = PAPER_FEED_DEFAULT_RATE;

    int32_t steps = (int32_t)(mm * pf_steps_per_mm);
    if (steps <= 0) return;

    // 通过 motors 模块使能所有电机（共享使能线）
    motors_set_disable(false);

    digitalWrite(pf_dir_pin, forward ? HIGH : LOW);
    delayMicroseconds(pf_dir_delay_us);

    uint32_t interval_us = (uint32_t)(60000000.0f / (rate * pf_steps_per_mm));
    if (interval_us < pf_pulse_us + 2) {
        interval_us = pf_pulse_us + 2;
    }
    uint32_t low_time = interval_us - pf_pulse_us;

    pf_moving = true;
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info,
        "Paper feed: %.1fmm %s at %.0fmm/min (%d steps)",
        mm, forward ? "forward" : "backward", rate, steps);

    for (int32_t i = 0; i < steps; i++) {
        digitalWrite(pf_step_pin, HIGH);
        delayMicroseconds(pf_pulse_us);
        digitalWrite(pf_step_pin, LOW);
        delayMicroseconds(low_time);
    }

    // 通过 motors 模块释放所有电机
    motors_set_disable(true);
    pf_moving = false;
}

void paper_feed_stop() {
    pf_moving = false;
    digitalWrite(pf_step_pin, LOW);
    motors_set_disable(true);
}

bool paper_feed_is_moving() {
    return pf_moving;
}
