
// Includes

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "liboled.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

// Defines

#define SSD1306_SEGMENT_VERTICAL_ADRESSING_MODE   0xA1
#define SSD1306_SEGMENT_HORIZONTAL_ADRESSING_MODE 0xA0

// In normal mode scan from COM0 to COM8. In reversed mode
// scan mode is done from COM8 to COM0.

#define SSD1306_SCAN_MODE_REMAPPED                0xC8
#define SSD1306_SCAN_MODE_NORMAL                  0xC0

#define SSD1306_DISPLAY_OFF                       0xAE
#define SSD1306_DISPLAY_ON                        0xAF

#define SSD1306_SET_CHARGE_PUMP                   0x8D
#define SSD1306_CHAGE_PUMP_ENABLE                 0x14

#define SSD1306_NORMAL_DISPLAY                    0xA6
#define SSD1306_INVERTED_DISPLAY                  0xA7
#define SSD1306_PAGE_ADDRESS_START                0xB0
#define SSD1306_CONTRAST                          0x81

#define OLED_DRIVER_STATUS_INITIALIZED            0x01
#define OLED_DRIVER_STATUS_DISPLAY_INVERTED       0x02
#define OLED_DRIVER_STATUS_DISPLAY_NORMAL         0x04

// Static data

static uint8_t       frame_buffer[DISPLAY_HEIGHT*DISPLAY_WIDTH];
static const uint8_t startup_sequence[]=
{
    // Enable charge pump regulator.

    SSD1306_SET_CHARGE_PUMP, SSD1306_CHAGE_PUMP_ENABLE,

    // Set segment re-map.

    SSD1306_SEGMENT_VERTICAL_ADRESSING_MODE,

    // Set COM Output Scan direction.

    SSD1306_SCAN_MODE_REMAPPED,

    // Set display to mode.

    SSD1306_NORMAL_DISPLAY,

    // Display on.

    SSD1306_DISPLAY_ON,
};

// Macros

#define ESP32_DELAY(MILLIS)(vTaskDelay(MILLIS / portTICK_PERIOD_MS))
#define ABS(X)((X) > 0 ? (X) : -(X))

#define BUFFER_PIXEL(BUFFER, X_POSITION, Y_POSITION)(BUFFER[((((Y_POSITION) / DISPLAY_BITS_PER_PIXEL) * DISPLAY_WIDTH) + X_POSITION)])
#define DRAW_PIXEL(BUFFER, X_POSITION, Y_POSITION)                                     \
if ( (X_POSITION) < DISPLAY_WIDTH  &&                                                  \
     (Y_POSITION) < DISPLAY_HEIGHT )                                                   \
     {                                                                                 \
        BUFFER_PIXEL(BUFFER, (X_POSITION), (Y_POSITION))|= (1 << ((Y_POSITION) % 8));  \
     }
#define CLEAR_PIXEL(BUFFER, X_POSITION, Y_POSITION)                                    \
if ( (X_POSITION) < DISPLAY_WIDTH  &&                                                  \
     (Y_POSITION) < DISPLAY_HEIGHT )                                                   \
     {                                                                                 \
        BUFFER_PIXEL(BUFFER, (X_POSITION), (Y_POSITION))&= ~(1 << ((Y_POSITION) % 8)); \
     }

#define ASSERT(CONDITION)                                                              \
if ( !(CONDITION) )                                                                    \
{                                                                                      \
    printf("Assert failed in function: %s at line %d: condition: %s",                  \
    __FUNCTION__, __LINE__, #CONDITION);                                               \
    return(OLED_RESULT_ERROR);                                                         \
}

// ssd1306_reset

static OLED_ERROR ssd1306_reset(LIB_OLED_DRIVER* oled)
{
    ASSERT(oled);

    gpio_pad_select_gpio(oled->rst_pin);
    gpio_set_direction(oled->rst_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(oled->rst_pin, 0);

    ESP32_DELAY(500);

    gpio_pad_select_gpio(oled->rst_pin);
    gpio_set_direction(oled->rst_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(oled->rst_pin, 1);

    return(0);
}

// ssd1306_write_bytes

static OLED_ERROR ssd1306_write_bytes(LIB_OLED_DRIVER* oled,
                                      const uint8_t*   bytes,
                                      size_t           byte_count)
{
    i2c_cmd_handle_t esp_i2c_handle;
    esp_err_t        rc;

    ASSERT(oled);
    ASSERT(bytes);
    ASSERT(byte_count > 0);

    // Allocate ESP32 I2C handle.

    esp_i2c_handle= i2c_cmd_link_create();
    if ( !esp_i2c_handle )
        return(OLED_RESULT_ERROR);

    // Generate start signal.

    rc= i2c_master_start(esp_i2c_handle);
    if ( rc )
        goto end;

    // Queue command bytes to be written on I2C bus. Begin with a display slave address.

    rc= i2c_master_write_byte(esp_i2c_handle, (oled->i2c_address << 1) | I2C_MASTER_WRITE, true);
    if ( rc )
        goto write_error;

    // Queue control byte.

    rc= i2c_master_write_byte(esp_i2c_handle, 0x00, true);
    if ( rc )
        goto write_error;

    // Queue data byte(s).

    i2c_master_write(esp_i2c_handle, (uint8_t*)bytes, byte_count, true);

    write_error:

    // Queue stop command.

    rc= i2c_master_stop(esp_i2c_handle);
    if ( rc )
        goto end;

    // Call blocking call to send queued commands.

    rc= i2c_master_cmd_begin(oled->i2c_port, esp_i2c_handle, 10 / portTICK_PERIOD_MS);

    end:

    // Free ESP32 I2C handle.

    i2c_cmd_link_delete(esp_i2c_handle);

    return(rc);
}

// ssd1306_refresh

static OLED_ERROR ssd1306_refresh(LIB_OLED_DRIVER* oled,
                                  uint16_t         y1,
                                  uint16_t         x1,
                                  uint16_t         y2,
                                  uint16_t         x2)
{
    i2c_cmd_handle_t esp_i2c_handle;
    uint8_t          page;

    ASSERT(oled);
    ASSERT(oled->display_buffer);

    // Return if refresh area is out of diplay bounds.

    if ( x1 > DISPLAY_WIDTH  || x2 > DISPLAY_WIDTH )
        return(0);

    if ( y2 > DISPLAY_HEIGHT )
        y2= DISPLAY_HEIGHT;

    if ( x2 > DISPLAY_WIDTH )
        x2= DISPLAY_WIDTH;

    for ( page= (y1 / DISPLAY_BITS_PER_PIXEL); page < (y2 / DISPLAY_BITS_PER_PIXEL); page++ )
    {
        esp_i2c_handle= i2c_cmd_link_create();
        if ( !esp_i2c_handle )
            return(OLED_RESULT_ERROR);

        i2c_master_start(esp_i2c_handle);
        i2c_master_write_byte(esp_i2c_handle, (oled->i2c_address << 1) | I2C_MASTER_WRITE, true);

        // Sets display column end address.

        i2c_master_write_byte(esp_i2c_handle, ((uint8_t)0x80), true);

        // Set SSD1306 GDDRAM page start address.

        i2c_master_write_byte(esp_i2c_handle, (SSD1306_PAGE_ADDRESS_START | page), true);

        // Sets display RAM start line to 0.

        i2c_master_write_byte(esp_i2c_handle, ((uint8_t)0x40), true);
        i2c_master_write(esp_i2c_handle, &oled->display_buffer[(page * DISPLAY_WIDTH)], DISPLAY_WIDTH, true);

        i2c_master_stop(esp_i2c_handle);
        i2c_master_cmd_begin(oled->i2c_port, esp_i2c_handle, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(esp_i2c_handle);
    }

    return(0);
}

// ssd1306_init

static OLED_ERROR ssd1306_init(LIB_OLED_DRIVER* oled)
{
    i2c_config_t esp_i2c_struct;
    OLED_ERROR   error;

    ASSERT(oled);

    // Initialize ESP32 I2C port.

    esp_i2c_struct.mode=             I2C_MODE_MASTER;
    esp_i2c_struct.sda_io_num=       oled->sda_pin;
    esp_i2c_struct.scl_io_num=       oled->scl_pin;
    esp_i2c_struct.sda_pullup_en=    GPIO_PULLUP_ENABLE;
    esp_i2c_struct.scl_pullup_en=    GPIO_PULLUP_ENABLE;
    esp_i2c_struct.master.clk_speed= 1000000;

    i2c_param_config(oled->i2c_port, &esp_i2c_struct);
    i2c_driver_install(oled->i2c_port, I2C_MODE_MASTER, 0, 0, 0);

    // Reset OLED

    error= ssd1306_reset(oled);
    ASSERT(!error);

    // Delay 100 ms

    ESP32_DELAY(100);

    // See SSD1306 software initialization App Note for the Flow Chart.

    error= ssd1306_write_bytes(oled, startup_sequence, sizeof(startup_sequence));
    ASSERT(!error);

    return(0);
}

// liboled_wordlen

static inline uint32_t liboled_wordlen(const char* word_start)
{
    uint32_t len;

    if ( !word_start )
        return(0);

    // Return char count until next white space.

    for ( len= 0; *word_start != ' ' && *word_start != '\0'; word_start++, len++ );

    return(len);
}

// liboled_draw_pixel

OLED_ERROR liboled_draw_pixel(LIB_OLED_DRIVER* oled,
                              int16_t          y_position,
                              int16_t          x_position)
{
    // Check display bounds.

    if ( x_position > DISPLAY_WIDTH || y_position > DISPLAY_HEIGHT || x_position < 0 || y_position < 0 )
        return(0);

    else if ( !oled || !oled->display_buffer )
        return(OLED_RESULT_ERROR);

    DRAW_PIXEL(oled->display_buffer, x_position, y_position);

    return(0);
}

// liboled_draw_line

OLED_ERROR liboled_draw_line(LIB_OLED_DRIVER* oled,
                             uint16_t         y1,
                             uint16_t         x1,
                             uint16_t         y2,
                             uint16_t         x2)
{
    int16_t deltax;
    int16_t deltay;
    int16_t x;
    int16_t y;
    int16_t xinc1;
    int16_t xinc2;
    int16_t yinc1;
    int16_t yinc2;
    int16_t den;
    int16_t num;
    int16_t num_add;
    int16_t num_pixels;
    int16_t curpixel;

#if defined(DEPRECATED)
    uint16_t position;
    uint16_t i;
#endif

    ASSERT(oled);
    ASSERT(oled->display_buffer);

    if ( x1 > DISPLAY_WIDTH || y1 > DISPLAY_HEIGHT )
        return(0);

    xinc1= 0;
    xinc2= 0;
    yinc1= 0;
    yinc2= 0;

    deltax= ABS(x2 - x1);
    deltay= ABS(y2 - y1);
    x= x1;
    y= y1;

    if ( x2 >= x1 )
    {
        xinc1= 1;
        xinc2= 1;
    }

    else
    {
        xinc1= -1;
        xinc2= -1;
    }

    if ( y2 >= y1 )
    {
        yinc1= 1;
        yinc2= 1;
    }

    else
    {
        yinc1= -1;
        yinc2= -1;
    }

    if ( deltax >= deltay )
    {
        xinc1=      0;
        yinc2=      0;
        den=        deltax;
        num=        deltax / 2;
        num_add=    deltay;
        num_pixels= deltax;
    }

    else
    {
        xinc2=      0;
        yinc1=      0;
        den=        deltay;
        num=        deltay / 2;
        num_add=    deltax;
        num_pixels= deltay;
    }

    for ( curpixel = 0; curpixel <= num_pixels; curpixel++ )
    {
        DRAW_PIXEL(oled->display_buffer, x, y);

        num+= num_add;
        if ( num >= den )
        {
            num-= den;
            x+=   xinc1;
            y+=   yinc1;
        }

        x+= xinc2;
        y+= yinc2;
    }

#if defined(DEPRECATED)
    if ( (y2 - y1) > (x2 - x1) )
    {
        for ( i= y1; i <= y2; i++ )
        {
            position= x1 + (uint16_t)(((float)i / (float)y2) * (float)(x2 - x1));
            DRAW_PIXEL(oled->display_buffer, position, i);
        }
    }

    else
    {
        for ( i= x1; i <= x2; i++ )
        {
            position= y1 + (uint16_t)(((float)i / (float)x2) * (float)(y2 - y1));
            DRAW_PIXEL(oled->display_buffer, i, position);
        }
    }
#endif

    return(0);
}

// liboled_draw_circle

OLED_ERROR liboled_draw_circle(LIB_OLED_DRIVER* oled,
                               uint16_t         x_position,
                               uint16_t         y_position,
                               uint16_t         radius)
{
    int32_t  decision;
    uint16_t current_x;
    uint16_t current_y;

    ASSERT(oled);
    ASSERT(oled->display_buffer);

    decision=  3 - (radius << 1);
    current_x= 0;
    current_y= radius;

    while ( current_x <= current_y )
    {
        DRAW_PIXEL(oled->display_buffer, (x_position + current_x), (y_position - current_y));
        DRAW_PIXEL(oled->display_buffer, (x_position - current_x), (y_position - current_y));
        DRAW_PIXEL(oled->display_buffer, (x_position + current_y), (y_position - current_x));
        DRAW_PIXEL(oled->display_buffer, (x_position - current_y), (y_position - current_x));
        DRAW_PIXEL(oled->display_buffer, (x_position + current_x), (y_position + current_y));
        DRAW_PIXEL(oled->display_buffer, (x_position - current_x), (y_position + current_y));
        DRAW_PIXEL(oled->display_buffer, (x_position + current_y), (y_position + current_x));
        DRAW_PIXEL(oled->display_buffer, (x_position - current_y), (y_position + current_x));

        if ( decision < 0 )
        {
            decision+= (current_x << 2) + 6;
        }

        else
        {
            decision+= ((current_x - current_y) << 2) + 10;
            current_y--;
        }

        current_x++;
    }

    return(0);
}

// liboled_draw_rect

OLED_ERROR liboled_draw_rect(LIB_OLED_DRIVER* oled,
                             uint16_t         x_position,
                             uint16_t         y_position,
                             uint16_t         width,
                             uint16_t         height)
{
    uint16_t i;
    uint16_t n;

    if ( x_position > DISPLAY_WIDTH || y_position > DISPLAY_HEIGHT )
        return(0);

    else if ( !oled || !oled->display_buffer )
        return(OLED_RESULT_ERROR);

    for ( n= 0; n < height; n++ )
        for ( i= 0; i < width; i++ )
            DRAW_PIXEL(oled->display_buffer, x_position + i, y_position + n);

    return(0);
}

// liboled_draw_string

OLED_ERROR liboled_draw_string(LIB_OLED_DRIVER*     oled,
                               int16_t              x_start,
                               int16_t              y_start,
                               const LIB_OLED_FONT* font,
                               const char*          text)
{
    int16_t  x_position;
    int16_t  y_position;
    uint32_t width;
    int32_t  i;
    int32_t  n;

    // Check parameters.

    ASSERT(oled);
    ASSERT(oled->display_buffer);
    ASSERT(font);
    ASSERT(text);

    x_position= x_start;
    y_position= y_start;

    while ( *text != '\0' )
    {
        // Handle new line or text overflow.

        if ( x_position > (DISPLAY_WIDTH - font->width) || *text == '\n' )
        {
            x_position=  x_start;
            y_position+= DISPLAY_BITS_PER_PIXEL;

            // Skip any spaces after new line.

            if ( *text == '\n' || *text == ' ' )
            {
                text++;
                continue;
            }
        }

        // Check that next word fits on the display.

        else if ( *text == ' ' )
        {
            width= liboled_wordlen(&text[1]) * font->width;
            if ( (x_position + font->width + width) > DISPLAY_WIDTH && width < DISPLAY_WIDTH )
            {
                text++;

                // Switch to new line to fit word on the screen without splitting it
                // in two parts.

                x_position=  x_start;
                y_position+= DISPLAY_BITS_PER_PIXEL;
            }
        }

        // Draw char pixel by pixel.

        for ( i= 0; i < font->height && (y_position + font->height) >= 0; i++ )
        {
            for ( n= 0; n < font->width; n++ )
            {
                if ( (y_position + n) >= 0 && font->font[((text[0] - 32) * font->width) + i] & (1 << n) )
                    DRAW_PIXEL(oled->display_buffer, x_position + i, y_position + n);
            }
        }

        text++;
        x_position+= font->width;
    }

    return(0);
}

// liboled_buffer_refresh

OLED_ERROR liboled_buffer_refresh(LIB_OLED_DRIVER* oled)
{
    OLED_ERROR rc;

    ASSERT(oled);
    ASSERT(oled->display_buffer);

    rc= ssd1306_refresh(oled, 0, 0, DISPLAY_HEIGHT, DISPLAY_WIDTH);

    return(rc);
}

// liboled_clear

OLED_ERROR liboled_buffer_clear_area(LIB_OLED_DRIVER* oled,
                                     uint16_t         x_position,
                                     uint16_t         y_position,
                                     uint16_t         width,
                                     uint16_t         height)
{
    uint16_t i;
    uint16_t n;

    if ( !oled || !oled->display_buffer )
        return(OLED_RESULT_ERROR);

    else if ( x_position > DISPLAY_WIDTH || y_position > DISPLAY_HEIGHT )
        return(0);

    // Needs some optimization :D

    for ( n= 0; n < height; n++ )
        for ( i= 0; i < width; i++ )
            CLEAR_PIXEL(oled->display_buffer, x_position + i, y_position + n);

    return(0);
}

// liboled_clear

OLED_ERROR liboled_buffer_clear(LIB_OLED_DRIVER* oled)
{
    if ( !oled || !oled->display_buffer )
        return(OLED_RESULT_ERROR);

    // Clear display buffer.

    memset(oled->display_buffer, 0, DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint8_t));

    return(0);
}

// liboled_init

OLED_ERROR liboled_init(LIB_OLED_DRIVER* oled)
{
    ASSERT(oled);

    oled->display_buffer= frame_buffer;
    oled->driver_status=  (OLED_DRIVER_STATUS_INITIALIZED | OLED_DRIVER_STATUS_DISPLAY_NORMAL);

    return(ssd1306_init(oled));
}

// liboled_display_driver_invert

OLED_ERROR liboled_display_driver_invert(LIB_OLED_DRIVER* oled)
{
    OLED_ERROR error;
    uint8_t    byte;

    ASSERT(oled);

    // Set display inverted.

    if ( oled->driver_status & OLED_DRIVER_STATUS_DISPLAY_NORMAL )
    {
        oled->driver_status&= ~OLED_DRIVER_STATUS_DISPLAY_NORMAL;
        oled->driver_status|=  OLED_DRIVER_STATUS_DISPLAY_INVERTED;

        byte= SSD1306_INVERTED_DISPLAY;
    }

    // Set display mode to normal.

    else if ( oled->driver_status & OLED_DRIVER_STATUS_DISPLAY_INVERTED )
    {
        oled->driver_status&= ~OLED_DRIVER_STATUS_DISPLAY_INVERTED;
        oled->driver_status|=  OLED_DRIVER_STATUS_DISPLAY_NORMAL;

        byte= SSD1306_NORMAL_DISPLAY;
    }

    // Return error if mode is unknown or display is unitialized.

    else
    {
        return(OLED_RESULT_ERROR);
    }

    // Write display mode command.

    error= ssd1306_write_bytes(oled, (const uint8_t*)&byte, 1);
    ASSERT(!error);

    return(0);
}

// liboled_display_driver_set_contrast

OLED_ERROR liboled_display_driver_set_contrast(LIB_OLED_DRIVER* oled,
                                               uint8_t          contrast)
{
    OLED_ERROR error;
    uint16_t   data;

    ASSERT(oled);

    if ( !(oled->driver_status & OLED_DRIVER_STATUS_INITIALIZED) )
        return(OLED_RESULT_ERROR);

    data= ((contrast << 8) | (SSD1306_CONTRAST));

    // Write display contrast command followed by contrast byte.

    error= ssd1306_write_bytes(oled, (const uint8_t*)&data, 2);
    ASSERT(!error);

    return(0);
}
