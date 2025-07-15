#include "neomatrix.h"

    // Constructor to instantiate the object
    NeoMatrix::NeoMatrix(uint8_t width, uint8_t height){
        this->width = width;
        this->height = height;

        pixelBuffer = new uint32_t*[height];
        for(int i = 0; i < height; i++)
            pixelBuffer[i] = new uint32_t[width];
    }

    // Initialize the object, returning true on success or false on failure
    bool NeoMatrix::init(void){
        //gpio pin 10 = power (once in the init)
        
        uint offset = pio_add_program(pio, &ws2812_program);
        ws2812_program_init(pio, sm, offset, WS2812_DIN, 800000, false);

        gpio_init(WS2812_POWER);
        gpio_set_dir(WS2812_POWER, GPIO_OUT);
        gpio_put(WS2812_POWER, 1);
    
        clear_pixels();
        write();
        //gpio pin 7 Din (Once in the init)
        return true;
    }

    // Set the pixel at row and column (zero indexed) to color
    void NeoMatrix::set_pixel(uint8_t row, uint8_t col, uint32_t color){
        if(row < height && col < width)
            pixelBuffer[row][col] = color;
    }

    // Set all elements of the pixel buffer to 0x00
    void NeoMatrix::clear_pixels(void){
        for(int i = 0; i < height; i++)
            for(int j = 0; j < width; j++)
                set_pixel(i, j, 0x0);
    }

    // Write the pixel buffer to the NeoMatrix
    void NeoMatrix::write(void){
        for(int i = 0; i < height; i++)
            for(int j = 0; j < width; j++)
                pio_sm_put_blocking(pio, sm, pixelBuffer[i][j] << 8u);
    }