#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "../libraries/LIS3DH.h"
#include "../libraries/neomatrix.h"
#include "pico/binary_info.h"
#include <cmath>

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9


// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 4
#define UART_RX_PIN 5

#define GREEN  0x00ff0000
#define RED    0x0000ff00

int main()
{
    stdio_init_all();

    LIS3DH obj;
    obj.init();
    
    NeoMatrix leds(8,8);
    leds.init();

    while (1) {
        obj.update();
        leds.clear_pixels();
        // Acceleration is read as a multiple of g (gravitational acceleration on the Earth's surface)
        printf("ACCELERATION VALUES: \n");
        printf("X acceleration: %.3fg\n", obj.x);
        printf("Y acceleration: %.3fg\n", obj.y);
        printf("Z acceleration: %.3fg\n", obj.z);


        if((obj.x < 0.1 && obj.x > -0.1) && (obj.y < 0.1 && obj.y > -0.1)){
            leds.set_pixel(4,4, GREEN);
            leds.set_pixel(3,4, GREEN);
            leds.set_pixel(4,3, GREEN);
            leds.set_pixel(3,3, GREEN);
        }else{
            //If x tilt is more significant
            uint ledX = ((obj.x + 0.9)/1.8)*7;

            uint ledY = ((-obj.y + 0.9)/1.8)*7;

            leds.set_pixel(ledX, ledY, RED);
        }

        leds.write();

        sleep_ms(100);

        // Clear terminal 
        printf("\033[1;1H\033[2J");
    }
}
