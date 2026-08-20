#pragma once

#include <Arduino.h>

// Transport layer for the external Z-axis serial motor/driver.
// The device protocol is intentionally kept out of this layer until the
// actual driver-board command specification is confirmed.
namespace ZMotorUart {
    void init();
    bool available();
    int read();
    size_t write(const uint8_t* data, size_t length);
    void flush_input();
}

namespace ZMotor {
    bool enable(bool on);
    bool stop();
    bool move_pulses(int32_t pulses, uint16_t rpm, uint8_t acceleration = 0, uint32_t timeout_ms = 30000);
    bool home(uint8_t mode = 0, uint32_t timeout_ms = 30000);
    bool has_alarm();
}
