#include "LIS3DH.h"

void lis3dh_calc_value(uint16_t raw_value, float *final_value, bool isAccel);

LIS3DH::LIS3DH(void){
    x = 0;
    y = 0;
    z = 0;
}

bool LIS3DH::init(void){
    //Accell Enable
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    set_reg(CTRL_REG_1, 0x97);

    return true;
}

uint8_t LIS3DH::read_reg(uint8_t reg){
    uint8_t data; 
    i2c_write_blocking(i2c_default, ADDRESS, &reg, 1, true);
    i2c_read_blocking(i2c_default, ADDRESS, &data, 1, false);

    return data;
}

void LIS3DH::set_reg(uint8_t reg, uint8_t val){
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = val;   
    i2c_write_blocking(i2c_default, ADDRESS, buf, 2, false);
}

void LIS3DH::update(void){
    uint16_t raw_accel;

    uint8_t xL = read_reg(OUT_X_L);
    uint8_t xH = read_reg(OUT_X_H);
    raw_accel = (xH << 8) | xL;
    lis3dh_calc_value(raw_accel, &x, true);

    uint8_t yL = read_reg(OUT_Y_L);
    uint8_t yH = read_reg(OUT_Y_H);
    raw_accel = (yH << 8) | yL;
    lis3dh_calc_value(raw_accel, &y, true);

    uint8_t zL = read_reg(OUT_Z_L);
    uint8_t zH = read_reg(OUT_Z_H);
    raw_accel = (zH << 8) | zL;
    lis3dh_calc_value(raw_accel, &z, true);
    
}

void lis3dh_calc_value(uint16_t raw_value, float *final_value, bool isAccel) {
    // Convert with respect to the value being temperature or acceleration reading 
    float scaling;
    float senstivity = 0.004f; // g per unit

    if (isAccel == true) {
        scaling = 64 / senstivity;
    } else {
        scaling = 64;
    }

    // raw_value is signed
    *final_value = (float) ((int16_t) raw_value) / scaling;
}
