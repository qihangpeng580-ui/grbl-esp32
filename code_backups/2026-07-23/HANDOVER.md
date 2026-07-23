# Grbl_Esp32 项目交接文档

---

## 1. 项目概述

| 项目 | 说明 |
|------|------|
| **名称** | Grbl_Esp32（宏荷 CNC 固件定制版） |
| **版本** | 1.3a (Build 20211103) |
| **平台** | ESP32 (Arduino 框架 + PlatformIO 构建) |
| **许可证** | GPL v3 |
| **上游项目** | [Grbl_Esp32](https://github.com/bdring/Grbl_Esp32)（原作者 Bart Dring），已进入维护模式 |
| **下一代** | [FluidNC](https://github.com/bdring/FluidNC)（与现有硬件兼容） |
| **当前用途** | 三轴 CoreXY 激光雕刻 CNC 控制器 |

这是一个中文定制版 Grbl_Esp32，在原始固件基础上增加了**宏荷视觉（HE）**、**小政哥（LBY）** 等多款国产 CNC 驱动板的硬件适配，并集成了 CoreXY 运动学、AI 代理实验代码等功能。

---

## 2. 仓库目录结构

```
Grbl_Esp32/
├── Grbl_Esp32.ino              # Arduino 入口 sketch (setup/loop -> grbl_init/run_once)
├── Grbl_Esp32/                 # 主工程目录
│   ├── Grbl_Esp32.ino          #   (同根目录同名文件，PlatformIO 以此为准)
│   ├── src/                    #   核心源代码 (~27,000 行 C++)
│   ├── src/Machines/           #   33 种机器引脚配置文件
│   ├── src/Motors/             #   电机驱动层
│   ├── src/Spindles/           #   主轴驱动层
│   ├── src/WebUI/              #   Web 控制界面 (27 个文件)
│   ├── src/tests/              #   单元测试
│   ├── Custom/                 #   用户自定义代码 (CoreXY 运动学等)
│   ├── data/                   #   SPIFFS Web UI 静态资源
│   └── doc/                    #   内部文档
├── libraries/                  # 第三方库 (arduinoWebSockets, ESP32SSDP)
├── doc/                        # 项目文档 & 脚本
├── AI/                         # DeepSeek AI 代理实验代码
├── embedded/                   # 嵌入式附加文件
├── pp/                         # PowerPoint 文档
├── platformio.ini              # PlatformIO 构建配置
├── build-all.*                 # 批量构建脚本
└── configure-features.py       # 功能配置工具
```

---

## 3. 核心架构

### 3.1 启动与主循环

```
Grbl_Esp32.ino
  setup() -> grbl_init()            // 一次性初始化
    +-- i2s_out_init()              // I2S GPIO 扩展（从 Marlin 固件移植）
    +-- WiFi 关闭                   // 默认关闭以节省功耗
    +-- client_init()               // 串口初始化
    +-- settings_init()             // 加载 NVS 持久化设置
    +-- stepper_init()              // 步进脉冲定时器
    +-- system_ini()                // GPIO 引脚配置
    +-- init_motors()               // 电机驱动初始化
    +-- machine_init()              // 用户自定义初始化 (weak)

  loop() -> run_once()              // 主循环
    +-- Protocol.cpp                //   串口协议处理
    +-- GCode.cpp                   //   G 代码解析执行
    +-- MotionControl.cpp           //   运动控制
```

### 3.2 核心模块一览

| 模块 | 源文件 | 大小 | 职责 |
|------|--------|------|------|
| **G 代码解析** | `GCode.cpp/h` | 92K | RS274/NGC 解析器，支持 G0-G3/G38/G43/G54-G59/M3-M9 等 |
| **步进生成** | `Stepper.cpp/h` | 52K | 基于 I2S/RMT 的硬件脉冲生成，支持 120KHz 步进 |
| **运动规划** | `MotionControl.cpp/h` | 27K | 点动、回零、直线/弧线运动 |
| **前瞻规划** | `Planner.cpp/h` | 25K | 梯形加速 + 连接偏差控制 |
| **协议处理** | `Protocol.cpp/h` | 40K | 串口命令解析、$ 设置、实时命令 |
| **I2S 输出** | `I2SOut.cpp/h` | 40K | 从 Marlin 固件移植的 GPIO 扩展方案 |
| **设置管理** | `Settings.cpp/h` | 23K | 运行时设置 (NVS/Preferences) |
| **设置定义** | `SettingsDefinitions.cpp/h` | 20K | 约 100 个 $ 命令定义 |
| **状态报告** | `Report.cpp/h` | 32K | 机器状态、位置、设置查询 |
| **限位/回零** | `Limits.cpp/h` | 19K | 硬/软限位、回零序列 |
| **探针** | `Probe.cpp/h` | 2K | G38.x 探针测量 |
| **点动** | `Jog.cpp/h` | 2K | 手动点动控制 |
| **冷却液** | `CoolantControl.cpp/h` | 4K | M7/M8/M9 |
| **串口通信** | `Serial.cpp/h` | 14K | 多通道输入缓冲（USB/蓝牙/WiFi） |
| **SD 卡** | `SDCard.cpp/h` | 5K | SD 卡 G 代码流式读取 |
| **系统管理** | `System.cpp/h` | 14K | 状态机、任务调度、FreeRTOS 集成 |
| **错误处理** | `Error.cpp/h` | 4K | 告警码、错误消息 |

### 3.3 状态机

```
Idle(0) -> Alarm(1) -> CheckMode(2) -> Homing(3) -> Cycle(4) -> Hold(5) -> Jog(6) -> SafetyDoor(7) -> Sleep(8)
```

定义见 `Grbl_Esp32/src/System.h` 中 `enum class State : uint8_t`。

### 3.4 设置系统

- 约 100 个 `$` 运行时参数（步数/mm、加速度、最大速率、反向、限位、回零方向等）
- 存储在 ESP32 **NVS (Preferences)** 中，非易失
- 定义文件：`SettingsDefinitions.cpp`（出厂默认值在 `Defaults.h`）

---

## 4. 电机与主轴驱动

### 4.1 电机类型

| 驱动 | 文件 | 说明 |
|------|------|------|
| `StandardStepper` | `Motors/StandardStepper.cpp` | 标准 STEP/DIR 步进 |
| `TrinamicDriver` | `Motors/TrinamicDriver.cpp` | TMC SPI 驱动（StealthChop/CoolStep/StallGuard） |
| `TrinamicUartDriver` | `Motors/TrinamicUartDriverClass.cpp` | TMC UART 驱动 |
| `UnipolarMotor` | `Motors/UnipolarMotor.cpp` | 单极步进 |
| `RcServo` | `Motors/RcServo.cpp` | RC 舵机（联动轴） |
| `Dynamixel2` | `Motors/Dynamixel2.cpp` | Dynamixel 智能舵机 |
| `NullMotor` | `Motors/NullMotor.cpp` | 虚拟电机（用于测试） |

### 4.2 主轴类型

| 驱动 | 文件 | 说明 |
|------|------|------|
| `Laser` | `Spindles/Laser.cpp` | **当前使用** - PWM 激光，带功率/速度补偿 |
| `PWMSpindle` | `Spindles/PWMSpindle.cpp` | PWM 调速主轴 |
| `VFDSpindle` | `Spindles/VFDSpindle.cpp` | RS485 变频器（Huanyang/Teco/YL620） |
| `DacSpindle` | `Spindles/DacSpindle.cpp` | 0-10V 模拟电压 |
| `BESCSpindle` | `Spindles/BESCSpindle.cpp` | RC 无刷电机 |
| `RelaySpindle` | `Spindles/RelaySpindle.cpp` | 继电器开关 |
| `NullSpindle` | `Spindles/NullSpindle.cpp` | 虚拟主轴（测试用） |

---

## 5. 当前硬件配置（boards_config）

### 5.1 配置系统说明

`Grbl_Esp32/src/boards_config.h` 定义了一套编号规则来区分不同的国产 CNC 驱动板：

| 编号前缀 | 品牌/系列 | 运动结构后缀 |
|----------|-----------|-------------|
| `0x` | 小政哥 (LBY) | 1=双X, 2=双Y, 3=CoreXY |
| `1x` | 宏荷视觉 (VISION) | 1=双X, 2=双Y, 3=CoreXY |
| `2x` | 宏荷 V2.1 (HE_V2.1) | 1=双X, 2=双Y, 3=CoreXY |
| `3x` | 宏荷 V1.8 (HE_V1.8) | 1=双X, 2=双Y, 3=CoreXY |
| `4x` | 宏荷 3轴 (HE_3AXIS) | 1=双X, 2=双Y, 3=CoreXY |
| `5x` | 星辰机械 6轴 (XCJX_6AXIS) | 1=双X, 2=双Y, 3=CoreXY |

### 5.2 当前配置

```c
#define boards_config  43       // HE_3AXIS + CoreXY 运动结构
#define SPINDLE_TYPE   SpindleType::LASER
#define BAUD_RATE      115200
#define ENABLE_BLUETOOTH         // 蓝牙已启用
// #define ENABLE_WIFI           // WiFi 已注释（关闭）
#define ENABLE_SD_CARD           // SD 卡已启用
#define CUSTOM_CODE_FILENAME  "../Custom/CoreXY.cpp"
#define DEFAULT_KINEMATICS_COREXY  1  // 默认 CoreXY 模式
```

### 5.3 HE_3AXIS 引脚分配

| 功能 | GPIO |
|------|------|
| X 步进 (STEP) | 27 |
| X 方向 (DIR) | 26 |
| Y 步进 (STEP) | 12 |
| Y 方向 (DIR) | 14 |
| Z 步进 (STEP) | 32 |
| Z 方向 (DIR) | 33 |
| 激光 PWM | 2 |
| X 限位 | 4 |
| Y 限位 | 16 |
| Z 限位 | 17 |

详见 `Grbl_Esp32/src/Machines/HE_3AXIS.h`

---

## 6. 运动学 (CoreXY)

自定义运动学代码位于 `Grbl_Esp32/Custom/CoreXY.cpp`，通过以下方式加载：
```c
#define CUSTOM_CODE_FILENAME    "../Custom/CoreXY.cpp"
#define DEFAULT_KINEMATICS_COREXY  1  // 0=笛卡尔, 1=CoreXY, 2=SwapXY
```

参考模板：`Grbl_Esp32/Custom/custom_code_template.cpp` 提供了完整的自定义函数接口说明。

---

## 7. 构建与编译

### 7.1 PlatformIO 构建

```bash
# 发布版本
platformio run -e release

# 调试版本
platformio run -e debug

# 指定机器配置（另一种方式）
$env:PLATFORMIO_BUILD_FLAGS='-DMACHINE_FILENAME=HE_3AXIS.h'; platformio run
```

### 7.2 关键构建参数

| 参数 | 值 |
|------|-----|
| 框架 | Arduino |
| 芯片平台 | espressif32@3.0.0 |
| 开发板 | esp32dev |
| CPU 频率 | 240MHz |
| Flash 模式 | QIO 80MHz |
| 分区表 | min_spiffs.csv |
| 上传波特率 | 921600 |
| 监控波特率 | 115200 |
| 输出文件名 | mks_eagle.bin |

### 7.3 外部依赖（release 版本）

- `TMCStepper@>=0.7.0,<1.0.0` — Trinamic 步进驱动库
- `ESP8266 and ESP32 OLED driver for SSD1306 displays@^4.2.0` — OLED 显示

---

## 8. AI 实验代码（AI/ 目录）

| 文件 | 语言 | 用途 |
|------|------|------|
| `deepseek_agent_py.py` | Python | DeepSeek API 最小接入示例 |
| `deepseek_agent_node.js` | Node.js | DeepSeek API 最小接入示例 |
| `deepseek_agent_cs.cs` | C# | DeepSeek API 最小接入示例 |
| `deepseek_examples_README.txt` | 文档 | 使用说明与下一步建议 |

这些文件提供了 DeepSeek API 的调用封装，可能用于 AI 辅助 CNC 路径规划或故障诊断。需要设置环境变量 `DEEPSEEK_API_KEY`。

---

## 9. 关键设计决策与注意事项

1. **I2S 步进方案**：本项目使用从 Marlin 3D 打印机固件移植的 I2S 外设来生成步进脉冲（`I2SOut.cpp`），这是 ESP32 上最精确的 GPIO 控制方式之一。

2. **多板兼容**：`boards_config.h` 通过编译期宏选择不同的机器定义文件，无需修改核心代码即可适配多款国产驱动板。

3. **CoreXY 默认**：当前配置默认使用 CoreXY 运动学（而非标准笛卡尔坐标系），通过 `Custom/CoreXY.cpp` 实现坐标变换。

4. **激光模式**：主轴类型固定为 `SpindleType::LASER`，启用了激光功率/速度自动补偿（`$32=1`）。

5. **WiFi 关闭**：当前配置中 WiFi 被注释掉了，可能为了减少功耗或避免信号干扰。蓝牙和 SD 卡保持开启。

6. **上游已冻结**：上游 Grbl_Esp32 已进入维护模式（仅修 bug），新功能开发全部在 FluidNC。如需升级应考虑迁移至 FluidNC。

7. **宏荷/小政哥生态**：代码中存在大量中文注释和国产板厂定制内容，这些修改未合并回上游，属于独立分支。

---

## 10. 常用操作速查

### 10.1 切换机器配置

编辑 `Grbl_Esp32/src/boards_config.h`，修改 `#define boards_config` 的值。例如：
- `43` -> HE_3AXIS + CoreXY（当前）
- `01` -> LBY + 双X
- `11` -> VISION + 双X

### 10.2 切换运动学模式

在 `boards_config.h` 中修改：
```c
#define DEFAULT_KINEMATICS_COREXY  0  // 改回笛卡尔
#define DEFAULT_KINEMATICS_COREXY  2  // SwapXY
```

### 10.3 启用 WiFi

取消 `boards_config.h` 中的注释：
```c
#define ENABLE_WIFI
```

### 10.4 修改引脚映射

编辑 `Grbl_Esp32/src/Machines/HE_3AXIS.h` 中对应的 `#define X_STEP_PIN` 等宏。

### 10.5 串口监控

```bash
platformio device monitor --baud 115200
```

---

## 11. 参考资料

- [Grbl_Esp32 Wiki](https://github.com/bdring/Grbl_Esp32/wiki/Compiling-the-firmware)
- [FluidNC 项目](https://github.com/bdring/FluidNC)
- [Grbl v1.1 命令参考](https://github.com/gnea/grbl/wiki/Grbl-v1.1-Commands)
- [PlatformIO 文档](https://docs.platformio.org/)

---

## 12. 进纸电机 (Paper Feed) 独立控制

### 12.1 概述

单独控制的进纸电机，使用 HE4988E 驱动（与 X/Y/Z 轴同型号），不进 Bresenham 运动插补器、不进 Planner、不进 `myMotor[]` 轴数组。完全独立于 CoreXY 写字系统。

### 12.2 相关文件

| 文件 | 说明 |
|------|------|
| `Grbl_Esp32/src/PaperFeed.h` | 进纸电机头文件，参数 + FreeRTOS 命令结构体定义 |
| `Grbl_Esp32/src/PaperFeed.cpp` | FreeRTOS 异步任务实现，独立 GPIO 控制 |
| `Grbl_Esp32/src/Machines/HE_3AXIS.h` | 引脚定义（末尾新增） |

集成修改：`src/Grbl.h`、`src/GCode.h`、`src/GCode.cpp`、`src/Grbl.cpp`（均用 `#ifdef PAPER_FEED_STEP_PIN` 条件编译包裹）。

### 12.3 引脚分配

| 功能 | GPIO | 说明 |
|------|------|------|
| 进纸 STEP | 13 | |
| 进纸 DIR | 15 | |
| 使能 | 25 | 独立控制（硬件与 STEPPERS_DISABLE_PIN 同线，但软件隔离，不再拉全局轴使能） |

### 12.4 G 代码命令

| 命令 | 参数 | 示例 | 说明 |
|------|------|------|------|
| `M100` | `P<mm>` `F<rate>` | `M100 P10 F500` | 前进进纸 P 毫米，速度 F mm/min |
| `M101` | `P<mm>` `F<rate>` | `M101 P5 F200` | 后退进纸 |
| `M102` | — | `M102` | 急停释放电机 |

默认参数（`PaperFeed.h`）：
- `PAPER_FEED_STEPS_PER_MM = 3200`
- `PAPER_FEED_DEFAULT_RATE = 500`
- `PAPER_FEED_PULSE_US = 3`

### 12.5 控制逻辑（v2 FreeRTOS 异步）

```
上位机发 M100 P10 F500 → GCode 解析 → paper_feed_sync()
  ├─ protocol_buffer_synchronize()     // 等 CoreXY 静止
  ├─ xQueueSend(cmd)                   // 投递到 paper_feed_task
  └─ 轮询等待 pf_task_done             // WDT 喂狗 + 实时命令处理
       │
       └→ paper_feed_task (FreeRTOS, prio=1)
            ├─ GPIO 单独使能进纸电机（不拉全局 STEPPERS_DISABLE_PIN）
            ├─ set_dir → for N 步: pulse STEP pin
            │    ├─ pf_stop_flag 检查（M102 急停）
            │    ├─ esp_task_wdt_reset() 定期喂狗
            │    └─ vTaskDelay(0) 定期 yield（防 IDLE WDT 超时）
            └─ 释放电机 + pf_task_done = true
```

不进 Stepper ISR，不进 Planner，与 CoreXY 零耦合。

### 12.6 v2 vs v1 改进

| 问题 | v1 (阻塞式) | v2 (FreeRTOS 异步) |
|------|------------|-------------------|
| 长进纸 (>5s) | TWDT 超时 → ESP32 重启 | 任务内喂狗 → 安全 |
| M102 急停 | 无效（循环不检查标志） | pf_stop_flag 每步检测 |
| 轴使能 | motors_set_disable(false) 拉全局 | 仅 GPIO25 拉低进纸电机 |
| 进纸期间串口 | 无响应 | protocol_execute_realtime() 处理 ! ? Ctrl-X |
| 舵机渐变 | 阻塞 delay(20ms) | vTaskDelay(20ms) 让出 CPU |

---


---

## 13. 版本管理与发布

### 13.1 版本命名规则

```
v<主版本号>.<次版本号>_<日期>
例如: v1.0_2026-07-14
```

### 13.2 一键发布脚本

```powershell
.\publish.ps1 -Version "v1.1" -Message "修复进纸电机丢步"
```

脚本自动完成：`git add` → `git commit` → `git tag` → `git push`

### 13.3 GitHub 仓库

https://github.com/qihangpeng580-ui/grbl-esp32

每个版本以 Git Tag 标记，可在 GitHub Releases 页面下载任意历史版本。

### 13.4 版本历史

| 版本 | 日期 | 说明 | 主要改动 |
|------|------|------|---------|
| v1.0 | 2026-07-14 | **初始版本** | Grbl_Esp32 定制版（HE_3AXIS 宏荷板 + CoreXY）；新增进纸电机 PaperFeed 模块（M100/M101/M102，阻塞式直接 GPIO） |
| v1.1 | 2026-07-15 | **新增舵机控制** | 新增 ServoControl 模块（M103，GPIO2 LEDC 50Hz PWM，纸张展平）；阻塞式 delay 渐变 |
| v1.2 | 2026-07-15 | **FreeRTOS 异步化** | PaperFeed + ServoControl 改为 FreeRTOS 独立任务；修复 WDT 超时/急停无效/轴使能过热；新增 M104 舵机复位、上电自动归 0° |

### 13.4.1 各版本功能矩阵

| 功能 | v1.0 | v1.1 | v1.2 |
|------|:----:|:----:|:----:|
| CoreXY 写字 (G0/G1/G2/G3) | ✅ | ✅ | ✅ |
| 进纸电机 M100/M101 | ✅ 阻塞 | ✅ 阻塞 | ✅ FreeRTOS 异步 |
| 进纸急停 M102 | ❌ 无效 | ❌ 无效 | ✅ 每步检测 |
| 舵机 M103 | — | ✅ 阻塞 delay | ✅ vTaskDelay 异步 |
| 舵机复位 M104 | — | — | ✅ 瞬移归 SCTL_RESET_ANGLE |
| 长进纸 (>5s) | ❌ WDT 重启 | ❌ WDT 重启 | ✅ 定时喂狗 |
| 进纸时轴使能 | ❌ 全轴拉低 | ❌ 全轴拉低 | ✅ 仅进纸电机 |
| 进纸时 RT 命令 (!/?) | ❌ 无响应 | ❌ 无响应 | ✅ protocol_execute_realtime |

### 13.5 代理配置

Git 已配置代理 `127.0.0.1:7890`，推送需要 VPN 开启。

---

## 14. 舵机控制 (Servo Control)

### 14.1 概述

独立舵机控制模块，用于纸张展平辅助。不进 Planner、不进轴数组，与 CoreXY/PaperFeed 零耦合。

### 14.2 相关文件

| 文件 | 说明 |
|------|------|
| Grbl_Esp32/src/ServoControl.h | 舵机控制头文件 + FreeRTOS 命令结构体 |
| Grbl_Esp32/src/ServoControl.cpp | LEDC 50Hz PWM + FreeRTOS 异步任务实现 |

集成修改：src/Grbl.h、src/Grbl.cpp、src/GCode.h、src/GCode.cpp（均用 #ifdef SERVO_PIN 包裹）。

### 14.3 引脚

| 功能 | GPIO |
|------|------|
| 舵机 PWM | 4（原 X_LIMIT_PIN，X 限位已改 GPIO2） |

### 14.4 G 代码命令

| 命令 | 参数 | 示例 | 说明 |
|------|------|------|------|
| M103 | P角度 F转速 | M103 P90 F30 | 舵机转到 P 度，转速 F 度/秒 |
| M104 | — | M104 | 舵机复位到安全角度（瞬移，无参数） |

默认参数（ServoControl.h）：
- SCTL_PULSE_FREQ = 50（Hz）
- SCTL_MIN_PULSE_US = 500
- SCTL_MAX_PULSE_US = 2500
- SCTL_RESET_ANGLE = 0（上电 / M104 复位目标角度）
- SCTL_DEFAULT_SPEED = 180（度/秒）

### 14.5 控制逻辑（v2 FreeRTOS 异步）

```
M103 P90 F30 → servo_set_angle(90, 30)
  ├─ xQueueSend(cmd)                // 投递到 servo_task
  └─ 轮询等待 srv_moving（WDT 喂狗）
       │
       └→ servo_task (FreeRTOS, prio=1)
            ├─ F=0: 瞬移 ledcWrite
            └─ F>0:
                 step_deg = 30 * 0.02 = 0.6 度/步
                 while 未到目标:
                    ledcWrite → vTaskDelay(20ms)  // 非阻塞让出 CPU
```

LEDC 硬件 PWM 50Hz，不受 CPU 中断影响。

### 14.6 FreeRTOS 架构

PaperFeed 和 ServoControl 各有一个独立 FreeRTOS 任务（优先级 1，同 Arduino loop），用 `xQueueHandle` 接收命令：

- **paper_feed_task**：栈 4096B，从队列取 `{mm, rate, forward}` 执行步进
- **servo_task**：栈 2048B，从队列取 `{angle, speed_dps}` 控舵机

三个电机（CoreXY / 进纸 / 舵机）不会同时运转，任务间无竞态。两个独立任务的存在仅为了异步解耦 + 各自喂狗安全。


---

*文档更新日期: 2026-07-15 (v1.2 FreeRTOS 异步化 + M104 舵机复位)*
---

*文档生成日期: 2026-07-14*
*工作区路径: `D:\Esp32\Grbl_Esp32修改 - 副本 - 副本\Grbl_Esp32`*


