// ServoControl.h - 独立舵机控制
// 不进 Planner、不进轴数组、不进运动插补
// M103 P<角度> F<转速deg/s>  例: M103 P90 F30
// 使用 ESP32 LEDC 外设输出 50Hz PWM

#pragma once
#include "Grbl.h"

#define SCTL_PULSE_FREQ         50
#define SCTL_PULSE_RES_BITS     16
#define SCTL_MIN_PULSE_US       500
#define SCTL_MAX_PULSE_US       2500
#define SCTL_DEFAULT_ANGLE      90
#define SCTL_DEFAULT_SPEED      180
#define SCTL_STEP_INTERVAL_MS   20

void servo_init();
void servo_set_angle(float angle, float speed_dps = 0);
void servo_disable();