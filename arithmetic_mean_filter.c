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

unsigned char *arithmetic_mean_filter(int w, int h, int channel, unsigned char *data, int kernal_size)
{
    int half_kernal = (kernal_size-1) / 2;
    unsigned char *out_pixels = malloc(sizeof(unsigned char) * w * h * channel);

    int i, j, idx, column_size = w*channel;
    int fi, fj, fidx, r, g, b;
    for (i=0; i<h; ++i) {
        for (j=0; j<column_size; j+=channel) {
            idx = j + i*column_size;
            r = 0; g = 0; b = 0;
            for (fi=i-half_kernal; fi<=i+half_kernal; ++fi) {
                for (fj=j-(half_kernal*channel); fj<=j+(half_kernal*channel); fj+=channel) {
                    if (fi<0 || fi>=h || fj<0 || fj>=column_size) continue;
                    fidx = fj + fi*column_size;
                    r += data[fidx+0];
                    g += data[fidx+1];
                    b += data[fidx+2];
                }
            }
            out_pixels[idx+0] = r/power(kernal_size);
            out_pixels[idx+1] = g/power(kernal_size);
            out_pixels[idx+2] = b/power(kernal_size);
            if (channel == 4) out_pixels[idx+3] = 255;
        }
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
    unsigned char *out_pixels = arithmetic_mean_filter(img_width, img_height, out_channel, pixels, 19);


    char *out_filename = "arithmetic_mean_filter.png";
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
