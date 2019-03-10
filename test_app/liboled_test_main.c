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

// Includes

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"

#include "liboled.h"
#include "font_roboto_14_x_17.h"
#include "font_digital_14_x_26.h"
#include "weather_bmp.h"
#include "driver/gpio.h"

// UI defines

#define UI_SIGNAL_DISPLAY_TEXT     0x02
#define UI_SIGNAL_START_DRAW_TEST  0x04

// SSD1306 Display Driver defines

#define SDA_PIN                    GPIO_NUM_4
#define SCL_PIN                    GPIO_NUM_15
#define RST_PIN                    GPIO_NUM_16

// draw_test

static void draw_test(void)
{
    LIB_OLED_DRIVER oled;
    int16_t         i;
    int16_t         n;

    oled.sda_pin=     SDA_PIN;
    oled.scl_pin=     SCL_PIN;
    oled.rst_pin=     RST_PIN;
    oled.i2c_port=    I2C_NUM_0;
    oled.i2c_address= DISPLAY_DEFAULT_ADDRESS;

    // Initialize display driver.

    liboled_init(&oled);

    // Draw weather bitmap with text.
    // Clear old content from the screen.

    liboled_buffer_clear(&oled);

    // Draw weather bitmap with text.

    liboled_draw_bitmap(&oled, &weather_bmp, 0, 0);
    liboled_draw_string(&oled, 60, 0, &font_roboto, "20 Deg.");

    // Draw clock.

    liboled_draw_string(&oled, 62, 30, &font_digital, "14:30");

    // Refresh display.

    liboled_buffer_refresh(&oled);

    // Delay for 10 seconds.

    vTaskDelay(10000 / portTICK_PERIOD_MS);

    // Draw/animate text along y-axis.

    for ( i= 0; i >= -30; i-- )
    {
        liboled_buffer_clear(&oled);
        liboled_draw_string(&oled, 0, i, &font_roboto, "Liboled test. Multiline text that is animated along the y-axis.");
        liboled_buffer_refresh(&oled);
        vTaskDelay(70 / portTICK_PERIOD_MS);
    }

    vTaskDelay(200 / portTICK_PERIOD_MS);

    for ( i= -30; i <= 0; i++ )
    {
        liboled_buffer_clear(&oled);
        liboled_draw_string(&oled, 0, i, &font_roboto, "Liboled test. Multiline text that is animated along the y-axis.");
        liboled_buffer_refresh(&oled);
        vTaskDelay(70 / portTICK_PERIOD_MS);
    }

    // Draw circles.

    for ( i= 1; i < DISPLAY_HEIGHT/2; i++ )
    {
        liboled_buffer_clear(&oled);
        liboled_draw_filled_circle(&oled, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, i);
        liboled_buffer_refresh(&oled);
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    for ( i= DISPLAY_HEIGHT/2; i > 2; i-- )
    {
        liboled_buffer_clear(&oled);
        liboled_draw_filled_circle(&oled, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, i);
        liboled_buffer_refresh(&oled);
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // Draw rectangles.

    for ( n= 0; n < DISPLAY_HEIGHT; n+= 10 )
    {
        for ( i= 0; i < (DISPLAY_WIDTH-10); i++ )
        {
            liboled_buffer_clear(&oled);

            // Animate rect from right to left...

            if ( n % 20 == 0 )
                liboled_draw_filled_rect(&oled, i, n, 10, 10);

            // ... and back to left end of the screen.

            else
                liboled_draw_filled_rect(&oled, DISPLAY_WIDTH-i, n, 10, 10);

            liboled_buffer_refresh(&oled);
        }
    }

    liboled_buffer_clear(&oled);

    // Draw lines.

    for ( i= 0; i <= DISPLAY_WIDTH; i+= 8 )
    {
        liboled_draw_line(&oled, i/4, 0, i, DISPLAY_WIDTH-1);
        liboled_buffer_refresh(&oled);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// app_main

void app_main(void)
{
    // Start draw test.

    draw_test();

    // Delay forever.

    vTaskDelay(portMAX_DELAY);
}
