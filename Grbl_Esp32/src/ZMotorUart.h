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
