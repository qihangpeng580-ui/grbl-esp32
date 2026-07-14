#pragma once
// clang-format off

/*
    6_pack_external_XYZ.h

    Covers all V1 versions V1p0, V1p1, etc

    Part of Grbl_ESP32
    Pin assignments for the ESP32 I2S 6-axis board

    2021    - Bart Dring
    
    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    Grbl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.


    6 Pack Jumpers for External Drivers
    The only jumper you set is the Vcc on 5V
    Stallguard jumpers must not be connected
    MS/SPI Do not need to be installed. It is OK to put them oonm the MS side
    TMC5160 Is does not matter if this is installed or not on any side.


*/
#define MACHINE_NAME            "XCJX_8AXIS"
//轴数（最大为6）
#define N_AXIS 6

// === Special Features

// I2S (steppers & other output-only pins)
#define USE_I2S_OUT
#define USE_I2S_STEPS
//#define DEFAULT_STEPPER ST_I2S_STATIC

#define I2S_OUT_BCK             GPIO_NUM_26
#define I2S_OUT_WS              GPIO_NUM_25
#define I2S_OUT_DATA            GPIO_NUM_27

//激光主轴（BLTOUCH接口）
#define SPINDLE_TYPE            SpindleType::LASER
#define LASER_OUTPUT_PIN     GPIO_NUM_2

//PWM主轴（加热头接口）电源电压
//#define SPINDLE_TYPE SpindleType::PWM
//#define SPINDLE_OUTPUT_PIN      GPIO_NUM_2
//#define SPINDLE_ENABLE_PIN      I2SO(23)//方向（热床口）电源电压

//DAC主轴（BLTOUCH接口）
//#define SPINDLE_TYPE SpindleType::DAC
//#define SPINDLE_OUTPUT_PIN      GPIO_NUM_2

//继电器主轴（BLTOUCH接口）
//#define SPINDLE_TYPE SpindleType::RELAY
//#define SPINDLE_OUTPUT_PIN      I2SO(22)


//X轴
#define X_DISABLE_PIN           I2SO(18)
#define X_DIRECTION_PIN         I2SO(16)
#define X_STEP_PIN              I2SO(17)


//Y轴
#define Y_DIRECTION_PIN         I2SO(13)
#define Y_STEP_PIN              I2SO(14)
#define Y_DISABLE_PIN           I2SO(15)



// Z轴
#define Z_DISABLE_PIN           I2SO(12)
#define Z_DIRECTION_PIN         I2SO(10)
#define Z_STEP_PIN              I2SO(11)


// A轴
#define A_DISABLE_PIN           I2SO(9)
#define A_DIRECTION_PIN         I2SO(7)
#define A_STEP_PIN              I2SO(8)
// B轴
#define B_DISABLE_PIN           I2SO(6)
#define B_DIRECTION_PIN         I2SO(4)
#define B_STEP_PIN              I2SO(5)

// C轴
#define C_DISABLE_PIN           I2SO(3)
#define C_DIRECTION_PIN         I2SO(1)
#define C_STEP_PIN              I2SO(2)

#define COOLANT_FLOOD_PIN                 I2SO(23)      // 推荐使用未占用的 I2SO(0)
#define COOLANT_MIST_PIN                  I2SO(22)  // labeled "Mist"

//限位
#define X_LIMIT_PIN     GPIO_NUM_33
#define Y_LIMIT_PIN     GPIO_NUM_32
#define Z_LIMIT_PIN     GPIO_NUM_35
#define A_LIMIT_PIN     GPIO_NUM_34
#define B_LIMIT_PIN     GPIO_NUM_36
#define C_LIMIT_PIN     GPIO_NUM_39


#define SDCARD_SS GPIO_NUM_5
#define SDCARD_MOSI GPIO_NUM_23
#define SDCARD_MISO GPIO_NUM_19
#define SDCARD_SCK GPIO_NUM_18
#define SDCARD_DET_PIN GPIO_NUM_22