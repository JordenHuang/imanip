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

// param: image width, height, channel and pixel data
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

// param image width, height, channel, pixel data and kernal size (eg. 5 for 5x5)
// return: pixel data after applying arithmetic mean filter
unsigned char *arithmetic_mean_filter_longer(int w, int h, int channel, unsigned char *data, int kernal_size)
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
                    if (channel == 3) {
                        g += data[fidx+1];
                        b += data[fidx+2];
                    }
                }
            }
            out_pixels[idx+0] = r/power(kernal_size);
            if (channel == 2) {
                out_pixels[idx+1] = 255;
            }
            else if (channel == 3) {
                out_pixels[idx+1] = g/power(kernal_size);
                out_pixels[idx+2] = b/power(kernal_size);
            }
            else if (channel == 4) out_pixels[idx+3] = 255;
        }
    }
    return out_pixels;
}

// param image width, height, channel, pixel data and kernal size (eg. 5 for 5x5)
// return: pixel data after applying arithmetic mean filter
unsigned char *arithmetic_mean_filter(int w, int h, int channel, unsigned char *data, int kernal_size)
{
    int half_kernal = (kernal_size-1) / 2;
    unsigned char *out_pixels = malloc(sizeof(unsigned char) * w * h * channel);

    int i, j, idx, column_size = w*channel;
    int fi, fj, fidx, r, g, b;
    int c;
    for (i=0; i<h; ++i) {
        r = 0; g = 0; b = 0;
        // Calculate the sum of pixel value in kernal range
        // ie. the r, g, b values, calculate them in the beginning of a row
        for (fi=i-half_kernal; fi<=i+half_kernal; ++fi) {
            for (fj=-(half_kernal*channel); fj<=(half_kernal*channel); fj+=channel) {
                if (fi<0 || fi>=h || fj<0 || fj>=column_size) continue;
                fidx = fj + fi*column_size;
                r += data[fidx+0];
                if (channel == 3) {
                    g += data[fidx+1];
                    b += data[fidx+2];
                }
            }
        }
        for (j=0; j<column_size; j+=channel) {
            idx = j + i*column_size;
            // Update the value of r, g, b
            // [x, y, z] --(minus leftest)--> [y, z] --(add rightest)--> [y, z, w]
            for (fi=i-half_kernal; fi<=i+half_kernal; ++fi) {
                fidx = j + fi*column_size;
                c = half_kernal*channel;
                if (fi<0 || fi>=h) continue;
                // Minus leftest column
                if (j-c >= 0) {
                    r -= data[fidx-c];
                    if (channel == 3) {
                        g -= data[fidx+1-c];
                        b -= data[fidx+2-c];
                    }
                }
                // Add rightest column
                if (j+c < column_size) {
                    r += data[fidx+c];
                    if (channel == 3) {
                        g += data[fidx+1+c];
                        b += data[fidx+2+c];
                    }
                }
            }

            // Store into out_pixels
            out_pixels[idx+0] = r/power(kernal_size);
            if (channel == 2) {
                out_pixels[idx+1] = 255;
            }
            else if (channel == 3) {
                out_pixels[idx+1] = g/power(kernal_size);
                out_pixels[idx+2] = b/power(kernal_size);
            }
            else if (channel == 4) out_pixels[idx+3] = 255;
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
    unsigned char *gray_pixels = to_grayscale(img_width, img_height, channel, pixels);

    // Apply arithmetic mean filter
    int out_width = img_width;
    int out_height = img_height;
    // int out_channel = channel;
    int out_channel = 1;
    unsigned char *out_pixels = arithmetic_mean_filter(img_width, img_height, out_channel, gray_pixels, 19);


    // Output grayscale img
    char *gray_filename = "gray.png";
    if (!stbi_write_png(gray_filename, out_width, out_height, out_channel, gray_pixels, out_width*out_channel)) {
        fprintf(stderr, "Cannot write file [%s]", gray_filename);
    } else {
        fprintf(stdout, "Writing file [%s] successfully", gray_filename);
    }

    // Output blur img
    char *out_filename = "arithmetic_mean_filter.png";
    if (!stbi_write_png(out_filename, out_width, out_height, out_channel, out_pixels, out_width*out_channel)) {
        fprintf(stderr, "Cannot write file [%s]", out_filename);
    } else {
        fprintf(stdout, "Writing file [%s] successfully", out_filename);
    }

    stbi_image_free(pixels);
    free(gray_pixels);
    // free(out_pixels);

    return 0;
}
