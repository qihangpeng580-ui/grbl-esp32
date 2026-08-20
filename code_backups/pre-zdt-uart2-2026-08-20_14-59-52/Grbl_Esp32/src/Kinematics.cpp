#include "Kinematics.h"

static volatile kinematics_mode_t current_mode = KINEMATICS_COREXY;

void kinematics_set_mode(kinematics_mode_t m) {
    current_mode = m;
}

kinematics_mode_t kinematics_get_mode() {
    return current_mode;
}
