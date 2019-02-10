
#ifndef liboled_h
#define liboled_h

// Inludes

#include <stdint.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "liboled_config.h"

// Defines

#define OLED_RESULT_OK    0
#define OLED_RESULT_ERROR 1

typedef int32_t OLED_ERROR;

// liboled_font

typedef struct liboled_font
{
    const uint8_t* font;

    // Char width and height.

    uint16_t height;
    uint16_t width;

    // Fisrt and last char unicodes.

    uint16_t first_char;
    uint16_t char_count;
}
LIB_OLED_FONT;

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

// liboled_draw_string

OLED_ERROR liboled_draw_string(LIB_OLED_DRIVER*     oled,
                               int16_t              x_start,
                               int16_t              y_start,
                               const LIB_OLED_FONT* font,
                               const char*          text);

// liboled_draw_rect

OLED_ERROR liboled_draw_rect(LIB_OLED_DRIVER* oled,
                             uint16_t         x_position,
                             uint16_t         y_position,
                             uint16_t         width,
                             uint16_t         height);

// liboled_draw_circle

OLED_ERROR liboled_draw_circle(LIB_OLED_DRIVER* oled,
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
