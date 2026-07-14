// PaperFeed.h - 独立进纸电机控制
// 不进 Planner、不进轴数组、不参与 Bresenham 插补
// 通过 M100/M101/M102 命令直接控制

#pragma once
#include "Grbl.h"

// 默认参数（可根据实际机械调整）
#define PAPER_FEED_STEPS_PER_MM      3200    // 200步/转 × 16细分 = 3200 steps/rev，按机械传动换算
#define PAPER_FEED_DEFAULT_RATE      500.0f  // 默认进纸速度 mm/min
#define PAPER_FEED_PULSE_US          3       // 步进脉冲宽度 μs
#define PAPER_FEED_DIR_DELAY_US      5       // 方向建立时间 μs

void paper_feed_init();                               // 初始化 GPIO
void paper_feed_sync(float mm, float rate, bool forward);  // 进纸（先同步 Planner）
void paper_feed_stop();                               // 紧急停止
bool paper_feed_is_moving();                          // 查询是否运动中
