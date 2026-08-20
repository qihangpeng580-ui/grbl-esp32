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

#define MACHINE_NAME            "HE_VISION"

#if (boards_config==11)

#define Y_STEP_PIN              GPIO_NUM_27
#define Y_DIRECTION_PIN         GPIO_NUM_14
#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_26
#define X2_STEP_PIN              GPIO_NUM_15
#define X2_DIRECTION_PIN         GPIO_NUM_13

#endif

#if (boards_config==12)

#define X_STEP_PIN              GPIO_NUM_27
#define X_DIRECTION_PIN         GPIO_NUM_14
#define Y_STEP_PIN              GPIO_NUM_12
#define Y_DIRECTION_PIN         GPIO_NUM_26
#define Y2_STEP_PIN              GPIO_NUM_15
#define Y2_DIRECTION_PIN         GPIO_NUM_13

#endif

#if (boards_config==13)

#define CUSTOM_CODE_FILENAME    "../Custom/CoreXY.cpp"

#define Y_STEP_PIN              GPIO_NUM_27
#define Y_DIRECTION_PIN         GPIO_NUM_14
#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_26
#define X2_STEP_PIN              GPIO_NUM_15
#define X2_DIRECTION_PIN         GPIO_NUM_13

#endif

#define Z_STEP_PIN              GPIO_NUM_33
#define Z_DIRECTION_PIN         GPIO_NUM_32

#define X_LIMIT_PIN             GPIO_NUM_4
#define Y_LIMIT_PIN             GPIO_NUM_16
#define Z_LIMIT_PIN             GPIO_NUM_17
// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN    GPIO_NUM_25