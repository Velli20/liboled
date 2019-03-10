
# Library for SSD1306 OLED Driver

This is ESP32 IDF compatible library for driving SSD1306 OLED Driver. This library has support for basic drawing
functions including: drawing circles, lines rectangles, text and bitmap images.

This library supports True Type Fonts. Use python script under ```/tools``` directory. to convert font to a c-header file.
Example usage of font_convertor.py:

```
python font_convertor.py Roboto/Roboto-Regular.ttf roboto 20
```

Output message should look like:

```
Width: 16  rows: 21
Output file saved as font_roboto_16_x_21.h
```

Converting bitmaps by using bitmap_convertor.py:

```
python bitmap_converter.py test.bmp
```

Output message should look like:
```
Converting test.bmp. Bitmap size appears to be 24 by 20 pixels.
Pixel count: 1440  offset: 54
Output file saved as test_bmp.h
```
## Examples

Example program can be found at the ```/test_app ``` directory.

<img src="test_app/test.jpg?raw=true?" width="480">

## API functions

```
liboled_init(LIB_OLED_DRIVER* oled);
```
- Initializes the display driver. Must be called before using any othe API-function.

```
liboled_buffer_refresh(LIB_OLED_DRIVER* oled);
```
- This function is used send content on the frame buffer to display driver. Use this after all drawing actions are done.

```
OLED_ERROR liboled_buffer_clear(LIB_OLED_DRIVER* oled);
```
- Clears the display frame buffer from any old content.

```
liboled_draw_pixel(LIB_OLED_DRIVER* oled, int16_t y_position, int16_t x_position);
```
- Draws a single pixel at given y and x postion.

```
liboled_draw_line(LIB_OLED_DRIVER* oled, uint16_t y1, uint16_t x1, uint16_t y2, uint16_t x2);
```
- Draws a line at given position.

```
liboled_draw_filled_rect(LIB_OLED_DRIVER* oled, uint16_t x_position, uint16_t y_position, uint16_t width, uint16_t height);
```
- Draws solid rectangle.

```
liboled_draw_bitmap(LIB_OLED_DRIVER* oled, const LIB_OLED_BITMAP* bitmap, uint16_t x_position, uint16_t y_position);
```
- Draws bitmap at given position.

```
liboled_draw_string(LIB_OLED_DRIVER* oled, int16_t x_start, int16_t y_start, const LIB_OLED_FONT* font, const char* text);
```
- Draws text string starting from given x and y position.

```
liboled_draw_circle(LIB_OLED_DRIVER* oled, uint16_t x_position, uint16_t y_position, uint16_t radius);
liboled_draw_filled_circle(LIB_OLED_DRIVER* oled, uint16_t x_position, uint16_t y_position, uint16_t radius);
```
- Draws circle at given cy and cx position.

