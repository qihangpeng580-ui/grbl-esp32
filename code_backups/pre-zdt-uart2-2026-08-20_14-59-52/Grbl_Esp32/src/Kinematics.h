#pragma once

#include <stdint.h>

typedef enum {
    KINEMATICS_CARTESIAN = 0,
    KINEMATICS_COREXY = 1,
    KINEMATICS_SWAPXY = 2,
} kinematics_mode_t;

void kinematics_set_mode(kinematics_mode_t m);
kinematics_mode_t kinematics_get_mode();
