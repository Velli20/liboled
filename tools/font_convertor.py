# -*- coding: utf-8 -*-

# MIT License
# Copyright (c) 2019 Velli20
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Imports

import freetype
import sys
import os
import math

# create_font_file
# Writes FreeType font glyph bitmaps to a LibOled compatible c-header file.

def create_font_file(char_data, font_name, width, height, first_char, char_count):

    offset_table= []
    byte_count=   0

    file_name= ("font_%s_%d_x_%d.h" % (font_name, width, height))

    # Create header file.

    out_file= open(file_name, "w+", encoding="utf-8")

    # Write include guard.

    out_file.write(("#ifndef font_%s_h\n"   % (font_name)))
    out_file.write(("#define font_%s_h\n\n" % (font_name)))

    # Write inclusion for liboled header file.

    out_file.write("// Includes\n\n")
    out_file.write("#include \"liboled.h\"\n\n")

    # Byte array constructor.

    out_file.write(("// %s_data_array\n\n" % (font_name)))
    out_file.write(("static const uint8_t %s_data_array[]=\n{\n" % (font_name)))

    for char in char_data:

        # Append index of the glyph data first byte to the offset table.

        offset_table.append(byte_count)

        out_file.write(("    // Data for char '%c'\n\n    " % (chr(char.char_code))))

        # Write font data struct:
        # - width
        # - height
        # - bitmap stride
        # - horizontal advance
        # - horizontal bearing x
        # - horizontal bearing y

        out_file.write("0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,\n    " % (char.width, char.height, char.stride, char.char_advance_x, char.char_bearing_x, char.char_bearing_y))
        byte_count+= 6

        for y in range(char.height):

            for i in range(char.stride):
                # Read the byte that contains the packed pixel data.
                byte_value = byte_swap(char.bit_array[y * char.stride + i])

                byte_count+= 1
                out_file.write("0x%02X," % (byte_value))

            out_file.write("\n    ")

        out_file.write("\n")

    # Close byte array with a semicolon.

    out_file.write("};\n\n")

    # Offset table constructor.

    out_file.write(("// %s_data_array\n\n" % (font_name)))
    out_file.write(("static const uint16_t %s_offset_table[]=\n{" % (font_name)))

    byte_count= 0
    for offset in offset_table:

        if byte_count == 0:
            out_file.write("\n    ")

        byte_count+= 1
        out_file.write("0x%04X," % offset)

        if byte_count == 8:
            byte_count= 0

    # Close offset table.

    out_file.write("\n};\n\n")

    # Create liboled font struct.

    out_file.write("// font_%s\n\n" % (font_name))
    out_file.write("static const LIB_OLED_FONT font_%s=\n{\n" % (font_name))
    out_file.write("    .font=         %s_data_array,\n" % (font_name))
    out_file.write("    .offset_table= %s_offset_table,\n" % (font_name))
    out_file.write("    .width=        %d,\n" % (width))
    out_file.write("    .height=       %d,\n" % (height))
    out_file.write("    .first_char=   %d,\n" % (first_char))
    out_file.write("    .char_count=   %d\n};\n\n" % (char_count))

    # Close inclusion guard and the output file.

    out_file.write("#endif\n")
    out_file.close()

    print("Output file saved as %s" % (file_name))

# byte_swap
# Swaps byte endianness.

def byte_swap(byte):

    byte_out= 0x00

    for i in range(8):
        bit = byte & (1 << (7 - i))

        if bit:
            byte_out|= (1 << (i))

    return(byte_out)

class CharData(object):

    def __init__(self, glyph, char):

        glyph_bitmap= glyph.bitmap

        # Store bitmap pixel data to a bytearray.

        self.char=      char
        self.width=     glyph_bitmap.width
        self.height=    glyph_bitmap.rows
        self.bit_array= glyph_bitmap.buffer
        self.stride=    glyph_bitmap.pitch
        self.advance_x= max(0, int(glyph.advance.x / 64))
        self.bearing_x= max(0, int(glyph.bitmap_left))
        self.bearing_y= max(0, int(glyph.bitmap_top))

    @property
    def char_code(self):
        return(self.char)

    @property
    def char_height(self):
        return(self.height)

    @property
    def char_width(self):
        return(self.width)

    @property
    def char_bit_array(self):
        return(self.bit_array)

    @property
    def char_advance_x(self):
        return(self.advance_x)

    @property
    def char_bearing_x(self):
        return(self.bearing_x)

    @property
    def char_bearing_y(self):
        return(self.bearing_y)

    @property
    def char_stride(self):
        return(self.stride)

class FreetypeFont(object):

    #__init__

    def __init__(self, filename, size):

        # Create a Freetype Face object from given font file.

        self.face= freetype.Face(filename)
        self.face.set_pixel_sizes(size, size)

    # get_glyph

    def get_glyph(self, char):

        # Load glyph in monograph format.

        self.face.load_char(char, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO)
        return(self.face.glyph)


def main():

    # Check arguments.

    if len(sys.argv) < 4:
        print("Font convertor usage: python font_convertor.py <font path> <font name> <font size>.")
        return

    # Get file path, name and size.

    font_path= sys.argv[1]
    font_name= sys.argv[2]
    font_size= sys.argv[3]

    if not font_path:
        print("Error: Font path not provided.")
        return

    if not font_name:
        print("Error: Font name not provided.")
        return

    if not font_size or not font_size.isdigit:
        print("Error:  Font size not provided.")
        return

    # Load font.

    fnt= FreetypeFont(font_path, int(font_size))

    chars=      []
    max_width=  0
    max_height= 0

    # Load bitmaps for glyphs.

    for char in range(32, 132):

        glyph= fnt.get_glyph(chr(char))

        max_height= max(glyph.bitmap.rows, max_height)
        max_width=  max(glyph.bitmap.width, max_width)
        chars.append(CharData(glyph, char))

    print("Width:", max_width, " rows:", max_height)

    create_font_file(chars, font_name, max_width, max_height, 32, 132-32)


if __name__ == '__main__':
    main()