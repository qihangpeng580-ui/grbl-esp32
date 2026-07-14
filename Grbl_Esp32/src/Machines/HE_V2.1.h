#pragma once
// clang-format off

/*
    3axis_v4.h
    Part of Grbl_ESP32

    Pin assignments for the ESP32 Development Controller, v4.1 and later.
    https://github.com/bdring/Grbl_ESP32_Development_Controller
    https://www.tindie.com/products/33366583/grbl_esp32-cnc-development-board-v35/

    2018    - Bart Dring
    2020    - Mitch Bradley

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
*/
#define SPINDLE_TYPE            SpindleType::LASER
#define LASER_OUTPUT_PIN      GPIO_NUM_2

// 注意：N_AXIS 在各自的 #if 块中分别定义

#if (boards_config==21)

#define N_AXIS 4
#define MACHINE_NAME            "External Dual X Driver Board V2"

#define Y_STEP_PIN              GPIO_NUM_27
#define Y_DIRECTION_PIN         GPIO_NUM_14
#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_26
#define X2_STEP_PIN              GPIO_NUM_13
#define X2_DIRECTION_PIN         GPIO_NUM_15

#endif

#if (boards_config==22)

#define N_AXIS 3
#define MACHINE_NAME            "HE_V2.1 Single XY"

#define X_STEP_PIN              GPIO_NUM_27
#define X_DIRECTION_PIN         GPIO_NUM_26
#define Y_STEP_PIN              GPIO_NUM_12
#define Y_DIRECTION_PIN         GPIO_NUM_14
// 单X单Y模式，不定义Y2（避免Y2误触发导致运动错乱）
// 接线：X=27(STEP)+26(DIR)，Y=12(STEP)+14(DIR)，Z=32(STEP)+33(DIR)

#endif

#if (boards_config==23)

#define N_AXIS 4
#define MACHINE_NAME            "External Dual X Driver Board V2 CoreXY"

#define CUSTOM_CODE_FILENAME    "../Custom/CoreXY.cpp"

#define Y_STEP_PIN              GPIO_NUM_27
#define Y_DIRECTION_PIN         GPIO_NUM_14
#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_26
#define X2_STEP_PIN              GPIO_NUM_15
#define X2_DIRECTION_PIN         GPIO_NUM_13

#endif

#define Z_STEP_PIN              GPIO_NUM_32
#define Z_DIRECTION_PIN         GPIO_NUM_33

#define X_LIMIT_PIN             GPIO_NUM_4
#define Y_LIMIT_PIN             GPIO_NUM_16
#define Z_LIMIT_PIN             GPIO_NUM_17
// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN    GPIO_NUM_25

#define SDCARD_SS GPIO_NUM_5
#define SDCARD_MOSI GPIO_NUM_4
#define SDCARD_MISO GPIO_NUM_4
#define SDCARD_SCK GPIO_NUM_18
#define SDCARD_DET_PIN GPIO_NUM_35