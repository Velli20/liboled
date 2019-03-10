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

import os
import sys
import struct

# rgb_to_luminance
# Performs RGB to Luma converion.

def rgb_to_luminance(red, blue, green):
    return((0.2125 * red) + (0.7154 * blue) + (0.0721 * green))

# create_byte_array
# Creates a header file that contains bitmap pixel data in byte array.

def create_byte_array(colors, output_file_name, width, height):

    pixel_count= width * height * 3
    height_pos=  0
    width_pos=   0
    byte=        0

    # Create header file for pixel data.

    out_file= open(("%s_bmp.h" % (output_file_name)), "w+")
    rnd_file= open(("rnd_test.txt" ), "w+")

    # Write include guard.

    out_file.write(("#ifndef _%s_h\n"   % (output_file_name)))
    out_file.write(("#define _%s_h\n\n" % (output_file_name)))

    # Write inclusion for liboled header file.

    out_file.write("// Includes\n\n")
    out_file.write("#include \"liboled.h\"\n\n")

    # Byte array constructor.

    out_file.write(("// %s_bmb_array\n\n" % (output_file_name)))
    out_file.write(("static const uint8_t %s_bmb_array[]=\n{\n    " % (output_file_name)))

    # Calculate bitmap stride.

    stride= int(width / 8)
    if width % 8 != 0:
        stride+= 1

    bmp_stride= ((((width * 24) + 31) & ~31) >> 3)

    # Write pixel data. Each pixel consits of 3 bytes, 24 bits. Itearate 3 bytes at the time.

    for i in range(0, pixel_count, 3):

        # Get pixel luminance.

        luminance= rgb_to_luminance(colors[i], colors[i+1], colors[i+2])

        # Set bit high if luminace threshold level is exceeded.

        if luminance > 0:
            rnd_file.write("*")
            byte|= (1 << (width_pos % 8))
        else:
            rnd_file.write(" ")

        width_pos+= 1
        if width_pos % 8 == 0 or width_pos == width:
            out_file.write("0x%02X," % byte)
            byte= 0

        # Switch to next row.

        if width_pos == width:
            width_pos=   0
            height_pos+= 1
            rnd_file.write("\n")
            # Append line feed with a padding after each row.

            if height_pos < height:
                out_file.write("\n    ")
            else:
                out_file.write("\n")

    # Close byte array with a semicolon.

    out_file.write("};\n\n")

    # Create liboled bitmap struct.

    out_file.write("// %s_bmp\n\n" % (output_file_name))
    out_file.write("static const LIB_OLED_BITMAP %s_bmp=\n{\n" % (output_file_name))
    out_file.write("    .bytes=  %s_bmb_array,\n" % (output_file_name))
    out_file.write("    .stride= %d,\n" % (stride))
    out_file.write("    .width=  %d,\n" % (width))
    out_file.write("    .height= %d\n};\n\n" % (height))

    # Close inclusion guard and file.

    out_file.write("#endif\n")
    out_file.close()
    rnd_file.close()
    print("Output file saved as %s_bmp.h" % (output_file_name))

# bitmap_to_bytes

def bitmap_to_bytes(image, width, height, pixel_count):

    byte_list= []
    byte_row=  []

    for i in range(0, width * height * 3, 3):

        byte_row.append(ord(image.read(1))) # Blue
        byte_row.append(ord(image.read(1))) # Green
        byte_row.append(ord(image.read(1))) # Red

        # Pixels are stored bottom-up in the Bitmap format.
        # Because of that we have to inverted row before appending it to the byte list.

        if (i + 3) % (width * 3) == 0:
            byte_list.extend(byte_row[::-1])
            byte_row= []

    return(byte_list[::-1])

# main

def main():

    # Check arguments.

    if len(sys.argv) < 2:
        print("Error: Path to file to convert is required.")
        return

    file_path=   sys.argv[1]
    output_name= os.path.split(file_path)[1].split('.')[0]

    # Try to open input bitmap.

    try:
        image_file= open(sys.argv[1], "rb")

    except IOError:
        print("Error: file %s does not appear to exist." % (file_path))
        return

    # Size of the bitmap file header is 54 bytes. Seek 18 bytes to find out width and height.

    image_file.seek(0x0A)
    offset=  int(struct.unpack('I', image_file.read(4))[0])
    image_file.seek(0x12)
    width=  int(struct.unpack('I', image_file.read(4))[0])
    height= int(struct.unpack('I', image_file.read(4))[0])
    image_file.seek(0x22)
    size= int(struct.unpack('I', image_file.read(4))[0])

    print("Converting %s. Bitmap size appears to be %d by %d pixels." % (file_path, width, height))
    print("Pixel count:", size, " offset:", offset)

    # Seek rest of the header to the beginning of the pixel data.

    image_file.seek(offset)

    # Read bitmap pixels to list.

    byte_list= bitmap_to_bytes(image_file, width, height, size)

    image_file.close()

    # Write pixel data to header file.

    create_byte_array(byte_list, output_name, width, height)


if __name__ == "__main__":
    main()