// PaperFeed.h - 独立进纸电机控制
// 不进 Planner、不进轴数组、不参与 Bresenham 插补
// 通过 M100/M101/M102 命令直接控制
// v2: FreeRTOS 异步任务化 - 消除阻塞, 支持 M102 急停

#pragma once
#include "Grbl.h"

// 命令结构体，用于 FreeRTOS 队列
typedef struct {
    float    mm;       // 进纸距离 mm
    float    rate;     // 进纸速度 mm/min
    bool     forward;  // true=前进, false=后退
} paper_feed_cmd_t;

// 默认参数（可根据实际机械调整）
#define PAPER_FEED_STEPS_PER_MM      3200    // 200步/转 × 16细分 = 3200 steps/rev，按机械传动换算
#define PAPER_FEED_DEFAULT_RATE      500.0f  // 默认进纸速度 mm/min
#define PAPER_FEED_PULSE_US          3       // 步进脉冲宽度 μs
#define PAPER_FEED_DIR_DELAY_US      5       // 方向建立时间 μs
#define PAPER_FEED_QUEUE_LEN         4       // 命令队列长度
#define PAPER_FEED_WDT_INTERVAL_MS   500     // 喂狗间隔 ms

// FreeRTOS 任务
void paper_feed_task(void* pvParameters);

// 对外接口
void paper_feed_init();                                     // 初始化 GPIO + 创建 FreeRTOS 任务
void paper_feed_sync(float mm, float rate, bool forward);   // 进纸（异步投递，等待完成）
void paper_feed_stop();                                     // 紧急停止
bool paper_feed_is_moving();                                // 查询是否运动中