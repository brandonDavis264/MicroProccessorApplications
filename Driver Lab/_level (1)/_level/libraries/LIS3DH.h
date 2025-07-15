#ifndef LIS3DH_H
#define LIS3DH_H

#include <stdint.h>
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"


const int ADDRESS = 0x18;
const uint8_t CTRL_REG_1 = 0x20;

const uint8_t OUT_X_L = 0x28;
const uint8_t OUT_X_H = 0x29;

const uint8_t OUT_Y_L = 0x2A;
const uint8_t OUT_Y_H = 0x2B;

const uint8_t OUT_Z_L = 0x2C;
const uint8_t OUT_Z_H = 0x2D;

class LIS3DH {
public:
    // Class members representing acceleration values in units of g
    float x, y, z;

    // Class constructor
    LIS3DH(void);

    // Initializes the accelerometer
    bool init(void);

    // Set a register on the LIS3DH to the given value
    void set_reg(uint8_t reg, uint8_t val);

    // Reads and returns the byte at address reg on the accelerometer
    uint8_t read_reg(uint8_t reg);

    // Updates the class members x, y, and z with current acceleration values
    void update(void);
};

#endif // LIS3DH_H
