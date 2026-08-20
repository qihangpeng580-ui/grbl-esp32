#pragma once

// Z-axis serial motor transport. UART0 remains reserved for USB/G-code.
#define Z_MOTOR_UART       UART_NUM_2
#define Z_MOTOR_UART_TX    GPIO_NUM_18
#define Z_MOTOR_UART_RX    GPIO_NUM_19
#define Z_MOTOR_UART_BAUD  115200
