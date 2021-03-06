#ifndef _weather_h
#define _weather_h

// Includes

#include "liboled.h"

// weather_bmb_array

static const uint8_t weather_bmb_array[]=
{
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x70,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,
    0x00,0xE0,0x00,0x00,0xFF,0x00,0x00,0x00,
    0x00,0xE0,0x0F,0x80,0xFF,0x00,0x00,0x00,
    0x00,0xE0,0x1F,0x00,0xFC,0x00,0x00,0x00,
    0x00,0xF0,0x0F,0x00,0xF0,0x01,0x00,0x00,
    0x00,0xF0,0x03,0x00,0xC0,0x01,0x00,0x00,
    0x00,0xF0,0x00,0x00,0x00,0x01,0x00,0x00,
    0x00,0x70,0x00,0xFF,0x01,0x00,0x00,0x00,
    0x00,0x30,0xE0,0xFF,0x07,0x00,0x00,0x00,
    0x00,0x00,0xF8,0xFF,0x1F,0x00,0x00,0x00,
    0x00,0x00,0xFC,0xFF,0x3F,0x00,0x00,0x00,
    0x00,0x00,0xFE,0xFF,0x7F,0x00,0x00,0x00,
    0x00,0x00,0xFF,0xFF,0xFF,0x00,0x00,0x00,
    0x00,0x80,0xFF,0xC3,0xFF,0x01,0x00,0x00,
    0x00,0x80,0xFF,0x00,0xFF,0x01,0x00,0x00,
    0x00,0xC0,0x3F,0x00,0xFC,0xC3,0x00,0x00,
    0x00,0xC0,0x1F,0x00,0xF8,0x83,0x03,0x00,
    0x00,0xE0,0x1F,0x00,0xF8,0x87,0x0F,0x00,
    0x00,0xE1,0x0F,0x00,0xF0,0x87,0x1F,0x00,
    0x80,0xE1,0x0F,0x00,0xF0,0x87,0x3F,0x00,
    0xC0,0xE1,0x07,0xE0,0xE7,0x07,0x1F,0x00,
    0xE0,0xE1,0x07,0xFE,0xFF,0x07,0x0F,0x00,
    0xF0,0xE1,0x87,0xFF,0xFF,0x07,0x07,0x00,
    0xF8,0xE1,0xC7,0xFF,0xFF,0x87,0x03,0x00,
    0xFC,0xE1,0xEF,0xFF,0xFF,0x87,0x01,0x00,
    0xF8,0xE1,0xFF,0xFF,0xFF,0x8F,0x00,0x00,
    0xE0,0xE1,0xFF,0xFF,0xFF,0x1F,0x00,0x00,
    0xC0,0xC1,0xFF,0x0F,0xF0,0x3F,0x00,0x00,
    0x00,0xC3,0xFF,0x03,0xC0,0x3F,0x00,0x00,
    0x00,0x80,0xFF,0x01,0x80,0x7F,0x00,0x00,
    0x00,0x80,0xFF,0x00,0x00,0x7F,0x00,0x00,
    0x00,0xF0,0x7F,0x00,0x00,0xFE,0x00,0x00,
    0x00,0xFC,0x7F,0x00,0x00,0xFE,0x00,0x00,
    0x00,0xFE,0x3F,0x00,0x00,0xFC,0x00,0x00,
    0x00,0xFF,0x3F,0x00,0x00,0xFC,0x00,0x00,
    0x80,0xFF,0x3F,0x00,0x00,0xFC,0x00,0x00,
    0xC0,0xFF,0x3F,0x00,0x00,0xFC,0x1F,0x00,
    0xC0,0x7F,0x00,0x00,0x00,0xFC,0x7F,0x00,
    0xE0,0x1F,0x00,0x00,0x00,0xFC,0xFF,0x01,
    0xE0,0x0F,0x00,0x00,0x00,0xFC,0xFF,0x03,
    0xE0,0x0F,0x00,0x00,0x00,0xFC,0xFF,0x03,
    0xE0,0x07,0x00,0x00,0x00,0xFC,0xFF,0x07,
    0xE0,0x07,0x00,0x00,0x00,0x00,0xFC,0x07,
    0xE0,0x0F,0x00,0x00,0x00,0x00,0xF0,0x07,
    0xE0,0x0F,0x00,0x00,0x00,0x00,0xE0,0x07,
    0xE0,0x1F,0x00,0x00,0x00,0x00,0xF0,0x07,
    0xC0,0x7F,0x00,0x00,0x00,0x00,0xF8,0x07,
    0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,
    0x80,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,
    0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,
    0x00,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,
    0x00,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
    0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0x3F,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

// weather_bmp

static const LIB_OLED_BITMAP weather_bmp=
{
    .bytes=  weather_bmb_array,
    .stride= 8,
    .width=  64,
    .height= 64
};

#endif
