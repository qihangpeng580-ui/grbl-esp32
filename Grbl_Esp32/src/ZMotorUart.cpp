#include "ZMotorUart.h"
#include "ZMotorConfig.h"
#include "Grbl.h"

namespace {
    HardwareSerial z_motor_serial(Z_MOTOR_UART);
    bool z_motor_initialized = false;
}

namespace ZMotorUart {
    void init() {
        if (z_motor_initialized) {
            return;
        }

        z_motor_serial.begin(Z_MOTOR_UART_BAUD,
                             SERIAL_8N1,
                             Z_MOTOR_UART_RX,
                             Z_MOTOR_UART_TX);
        z_motor_initialized = true;
        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "Z motor UART%d TX:%d RX:%d baud:%d",
                       Z_MOTOR_UART,
                       Z_MOTOR_UART_TX,
                       Z_MOTOR_UART_RX,
                       Z_MOTOR_UART_BAUD);
    }

    bool available() {
        return z_motor_initialized && z_motor_serial.available() > 0;
    }

    int read() {
        return z_motor_initialized ? z_motor_serial.read() : -1;
    }

    size_t write(const uint8_t* data, size_t length) {
        if (!z_motor_initialized || data == nullptr || length == 0) {
            return 0;
        }
        return z_motor_serial.write(data, length);
    }

    void flush_input() {
        if (!z_motor_initialized) {
            return;
        }
        while (z_motor_serial.available() > 0) {
            z_motor_serial.read();
        }
    }
}
