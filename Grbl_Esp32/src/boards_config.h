// \Grbl_Esp32\src\boards_config.h
// \Grbl_Esp32\src\Machines\boards_config.h
// "boards_config"为0开头时代表是小政哥的板子,1为宏荷视觉的板子，2为宏荷常规的板子
//尾帧数字为运动结构，1为双x，2为双y，3为Corexy
//视觉为11、12，13

#define boards_config  43


#define BAUD_RATE 115200

#define ENABLE_BLUETOOTH  // enable bluetooth

#define ENABLE_SD_CARD  // enable use of SD Card to run jobs

//#define ENABLE_WIFI  //enable wifi

#define CUSTOM_CODE_FILENAME    "../Custom/CoreXY.cpp"
#define DEFAULT_KINEMATICS_COREXY 1  // 默认CoreXY模式，0=Cartesian, 1=CoreXY, 2=SwapXY
//De By LBY
#if (boards_config==01||boards_config==02||boards_config==03)

#    include "Machines/LBY.h"

#endif
//De By He VISION
#if (boards_config==11||boards_config==12||boards_config==13)

#    include "Machines/vision.h"

#endif
//De By He V2.1
#if (boards_config==21||boards_config==22||boards_config==23)

#    include "Machines/HE_V2.1.h"

#endif
//De By He V1.8
#if (boards_config==31||boards_config==32||boards_config==33)

#    include "Machines/HE_V1.8.h"

#endif

//De By He 3AXIS
#if (boards_config==41||boards_config==42||boards_config==43)

#    include "Machines/HE_3AXIS.h"

#endif

//De By He 3AXIS
#if (boards_config==51||boards_config==52||boards_config==53)

#    include "Machines/XCJX_6AXIS.h"

#endif



//步进值
#define DEFAULT_X_STEPS_PER_MM 80
#define DEFAULT_Y_STEPS_PER_MM 80
#define DEFAULT_Z_STEPS_PER_MM 60 // This is percent in servo mode

//加速度配置
#define DEFAULT_X_ACCELERATION 5000.0 // mm/sec^2. 200 mm/sec^2 = 720000 mm/min^2
#define DEFAULT_Y_ACCELERATION 5000.0 // mm/sec^2
#define DEFAULT_Z_ACCELERATION 8000.0 // mm/sec^2

//最大行程
#define DEFAULT_X_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.
#define DEFAULT_Y_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.
#define DEFAULT_Z_MAX_TRAVEL 300.0 // This is percent in servo mode

//最大速率
#define DEFAULT_X_MAX_RATE 9000 // mm/min
#define DEFAULT_Y_MAX_RATE 9000 // mm/min
#define DEFAULT_Z_MAX_RATE 9000.0 // mm/min

#define SERVO_PIN2 GPIO_NUM_2

#define DEFAULT_JUNCTION_DEVIATION  0.01 // mm
#define DEFAULT_ARC_TOLERANCE       0.002 // mm
#define DEFAULT_REPORT_INCHES       0 // false

#define DEFAULT_STATUS_REPORT_MASK 1

#define DEFAULT_STEP_PULSE_MICROSECONDS 10
#define DEFAULT_STEPPER_IDLE_LOCK_TIME  255 // stay on（254）

#define DEFAULT_LASER_MODE 1 // false

#define DEFAULT_DIRECTION_INVERT_MASK 4// uint8_t

#define DEFAULT_HOMING_ENABLE 1  // $22 false



