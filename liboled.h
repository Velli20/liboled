/*
MIT License
Copyright (c) 2019 Velli20
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef liboled_h
#define liboled_h

// Inludes

#include <stdint.h>
#include "driver/gpio.h"
#include "driver/i2c.h"

// Defines

#define OLED_RESULT_OK              0
#define OLED_RESULT_ERROR           1

#define DISPLAY_HEIGHT              64
#define DISPLAY_WIDTH               128
#define DISPLAY_BITS_PER_PIXEL      8
#define DISPLAY_DEFAULT_ADDRESS     0x3C

typedef int32_t OLED_ERROR;

// Font Freetype data struct.

typedef struct liboled_font_fft
{
    uint8_t width;
    uint8_t height;
    uint8_t stride;
    uint8_t advance;
    uint8_t bearing_x;
    uint8_t bearing_y;

    const uint8_t glyph[];
}
LIB_OLED_FONT_FFT;

// liboled_font

typedef struct liboled_font
{
    const uint8_t*  font;
    const uint16_t* offset_table;

    // Char width and height.

    uint16_t height;
    uint16_t width;

    // Fisrt and last char unicodes.

    uint16_t first_char;
    uint16_t char_count;
}
LIB_OLED_FONT;

typedef struct liboled_bitmap
{
    const uint8_t* bytes;

    uint16_t stride;
    uint16_t width;
    uint16_t height;
}
LIB_OLED_BITMAP;

// liboled_driver

typedef struct liboled_driver
{
    gpio_num_t sda_pin;
    gpio_num_t scl_pin;
    gpio_num_t rst_pin;

    i2c_port_t i2c_port;
    uint8_t    i2c_address;
    uint8_t    padding;
    uint16_t   driver_status;

    uint8_t*   display_buffer;
}
LIB_OLED_DRIVER;

// liboled_init

OLED_ERROR liboled_init(LIB_OLED_DRIVER* oled);

// liboled_draw_pixel

OLED_ERROR liboled_draw_pixel(LIB_OLED_DRIVER* oled,
                              int16_t          y_position,
                              int16_t          x_position);

// liboled_draw_line

OLED_ERROR liboled_draw_line(LIB_OLED_DRIVER* oled,
                             uint16_t         y1,
                             uint16_t         x1,
                             uint16_t         y2,
                             uint16_t         x2);

// liboled_draw_filled_rect

OLED_ERROR liboled_draw_filled_rect(LIB_OLED_DRIVER* oled,
                                    uint16_t         x_position,
                                    uint16_t         y_position,
                                    uint16_t         width,
                                    uint16_t         height);

OLED_ERROR liboled_draw_bitmap(LIB_OLED_DRIVER*       oled,
                               const LIB_OLED_BITMAP* bitmap,
                               uint16_t               x_position,
                               uint16_t               y_position);

// liboled_draw_string

OLED_ERROR liboled_draw_string(LIB_OLED_DRIVER*     oled,
                               int16_t              x_start,
                               int16_t              y_start,
                               const LIB_OLED_FONT* font,
                               const char*          text);

// liboled_draw_circle

OLED_ERROR liboled_draw_circle(LIB_OLED_DRIVER* oled,
                               uint16_t         x_position,
                               uint16_t         y_position,
                               uint16_t         radius);

// liboled_draw_filled_circle

OLED_ERROR liboled_draw_filled_circle(LIB_OLED_DRIVER* oled,
                                      uint16_t         x_position,
                                      uint16_t         y_position,
                                      uint16_t         radius);

// liboled_buffer_refresh

OLED_ERROR liboled_buffer_refresh(LIB_OLED_DRIVER* oled);

// liboled_clear

OLED_ERROR liboled_buffer_clear(LIB_OLED_DRIVER* oled);

// liboled_buffer_clear_area

OLED_ERROR liboled_buffer_clear_area(LIB_OLED_DRIVER* oled,
                                     uint16_t         x_position,
                                     uint16_t         y_position,
                                     uint16_t         width,
                                     uint16_t         height);

// liboled_display_driver_invert

OLED_ERROR liboled_display_driver_invert(LIB_OLED_DRIVER* oled);

// liboled_display_driver_set_contrast

OLED_ERROR liboled_display_driver_set_contrast(LIB_OLED_DRIVER* oled,
                                               uint8_t          contrast);

#endif
