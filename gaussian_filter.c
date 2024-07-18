#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int power(int x)
{
    return x * x;
}

// param: image width, height ,channel and pixel data
// return: grayscale pixel data
unsigned char *to_grayscale(int w, int h, int channel, unsigned char *data)
{
    int i, out_i;
    float sum;
    unsigned char *out_pixels = malloc(sizeof(unsigned char) * w * h * 1);
    for (i=0, out_i=0; i<w*h*channel; i+=channel) {
        sum = (data[i] * 0.299) + (data[i+1] * 0.587) + (data[i+2] * 0.114);
        out_pixels[out_i++] = (int)sum;
    }
    return out_pixels;
}

int main(void)
{
    // const char *filename = "./wallhaven-o587o9.jpg";
    const char *filename = "./wallhaven-vqlr98.png";
    int img_width, img_height, channel;
    unsigned char *pixels = stbi_load(filename, &img_width, &img_height, &channel, 0);
    printf("width: %d, height: %d, channel: %d\n", img_width, img_height, channel);

    // To grayscale
    // int out_width = img_width;
    // int out_height = img_height;
    // int out_channel = 1;
    // unsigned char *gray_pixels = to_grayscale(img_width, img_height, channel, pixels);

    // Apply arithmetic mean filter
    int out_width = img_width;
    int out_height = img_height;
    int out_channel = channel;


    char *out_filename = "out.png";
    if (!stbi_write_png(out_filename, out_width, out_height, out_channel, out_pixels, out_width*out_channel)) {
        fprintf(stderr, "Cannot write file [%s]", out_filename);
    } else {
        fprintf(stdout, "Writing file [%s] successfully", out_filename);
    }

    stbi_image_free(pixels);
    // free(gray_pixels);
    free(out_pixels);

    return 0;
}
