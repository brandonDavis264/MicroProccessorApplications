#ifndef NEOMATRIX_H
#define NEOMATRIX_H

#include <cstdint>
#include "ws2812.pio.h"

#define WS2812_DIN 7
#define WS2812_POWER 10

class NeoMatrix {
public:
    uint8_t width;          // Width of the matrix
    uint8_t height;         // Height of the matrix
    uint32_t** pixelBuffer;  // Pointer to the pixel buffer

    PIO pio = pio0;
    int sm = 0;

    // Constructor to instantiate the object
    NeoMatrix(uint8_t width, uint8_t height);

    // Initialize the object, returning true on success or false on failure
    bool init(void);

    // Set the pixel at row and column (zero indexed) to color
    void set_pixel(uint8_t row, uint8_t col, uint32_t color);

    // Set all elements of the pixel buffer to 0x00
    void clear_pixels(void);

    // Write the pixel buffer to the NeoMatrix
    void write(void);
};

#endif // NEOMATRIX_H
