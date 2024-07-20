#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_library/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb_library/stb_image_write.h"


// param: image width, height ,channel and pixel data
// return: grayscale pixel data
unsigned char *to_grayscale(int w, int h, int channel, unsigned char *data)
{
    int i, out_i;
    double sum;
    unsigned char *out_pixels = malloc(sizeof(unsigned char) * w * h * 1);
    for (i=0, out_i=0; i<w*h*channel; i+=channel) {
        sum = (data[i] * 0.299) + (data[i+1] * 0.587) + (data[i+2] * 0.114);
        out_pixels[out_i++] = (unsigned char)sum;
    }
    return out_pixels;
}


// int *gaussian_kernal_gen(int kx, int ky, double sigx, double sigy)
// {
//     if (kx%2 != 1 || ky%2 != 1 ) {
//         fprintf(stderr, "kernal size must be odd number, not (%d, %d)", kx, ky);
//         return NULL;
//     } else if (kx == 1 || ky == 1) {
//         fprintf(stderr, "kernal size must greater than 1, not (%d, %d)", kx, ky);
//         return NULL;
//     }
//
//     double *kernal = malloc(sizeof(double) * kx * ky);
//     int hx = kx/2, hy = ky/2;
//     double coef = 1.f / 2.f*M_PI * sigx * sigy;
//     double x, y, sum;
//
//     int i, j, idx;
//     for (i=0, sum=0; i<ky; ++i) {
//         y = pow(i-hy, 2);
//         for (j=0; j<kx; ++j) {
//          x = pow(j-hx, 2);
//             idx = j + i*kx;
//             kernal[idx] = coef * exp(-(y+x) / (2*sigy*sigx));
//             // sum += kernal[idx];
//         }
//     }
//     printf("%lf\n", coef);
//     printf("result:\n");
//     int n = kernal[0];
//     for (i=0; i<ky; ++i) {
//         for (j=0; j<kx; ++j) {
//             idx = j + i*kx;
//             kernal[idx] = kernal[idx] / n;
//             // kernal[idx] /= n;
//             printf("%d ", kernal[idx]);
//         }
//         printf("\n");
//     }
//     free(kernal);
//     return NULL;
// }

double *gaussian_kernal_1d_gen(int kx, double sigx)
{
    double *kernal = malloc(sizeof(double) * kx);
    double sum, g, k0;
    int i, origin = kx/2;
    for (i=0, sum=0; i<kx; ++i) {
        // the coeficient will be canceled in the for loop below, thus no need to calculate
        g = exp(-pow(i-origin, 2) / (2 * pow(sigx, 2)));
        sum += g;
        kernal[i] = g;
    }

    k0 = kernal[0];
    for (i=0; i < kx; ++i) {
        kernal[i] /= sum;
        // kernal[i] /= k0;
        // printf("%lf ", kernal[i]);
    }
    // printf("\n");
    return kernal;
}

unsigned char *gaussian_blur(int w, int h, int channel, unsigned char *data, int kx, int ky, double sigx, double sigy)
{
    if (kx%2 != 1 || ky%2 != 1 ) {
        fprintf(stderr, "kernal size must be odd number, not (%d, %d)", kx, ky);
        return NULL;
    } else if (kx == 1 || ky == 1) {
        fprintf(stderr, "kernal size must greater than 1, not (%d, %d)", kx, ky);
        return NULL;
    }

    unsigned char *hori = malloc(sizeof(unsigned char) * w * h * channel);
    unsigned char *vert = malloc(sizeof(unsigned char) * w * h * channel);
    // half gaussian kernal size
    int hx = kx/2;
    int hy = ky/2;

    // 1d kernal, horizontal and vertical direction
    double *kernal_hori = gaussian_kernal_1d_gen(kx, sigx);
    double *kernal_vert = gaussian_kernal_1d_gen(ky, sigy);

    int i, j, k, idx;
    int fi, fj, fidx, kidx;
    int col_size = w * channel;

    int sum_size;
    if (channel <= 2) sum_size = 1;
    else sum_size = 3;
    double sum[sum_size];

    // Apply gaussian kernal in horizontal direction to the data, store into hori
    for (i=0; i<h; ++i) {
        for (j=0; j<col_size; ++j) {
            for (k=0; k<sum_size; ++k) {
                sum[k] = 0;
            }
            idx = j + i*col_size;
            for (fj = j-hx*channel, kidx=0; fj <= j+hx*channel; fj+=channel, ++kidx) {
                if (fj < 0 || fj >= col_size) continue;
                fidx = fj + i*col_size;
                // for each channel
                for (k=0; k<sum_size; ++k) {
                    sum[k] += (double)data[fidx + k] * kernal_hori[kidx];
                }
            }
            // assign to each channel
            for (k=0; k<sum_size; ++k) {
                hori[idx + k] = (unsigned char)sum[k];
            }
        }
    }
    // Apply gaussian kernal in vertical direction to the hori, store into vert
    for (j=0; j<col_size; ++j) {
        for (i=0; i<h; ++i) {
            for (k=0; k<sum_size; ++k) {
                sum[k] = 0;
            }
            idx = j + i*col_size;
            for (fi = i-hy, kidx=0; fi <= i+hy; ++fi, ++kidx) {
                if (fi < 0 || fi >= h) continue;
                fidx = j + fi*col_size;
                // for each channel
                for (k=0; k<sum_size; ++k) {
                    sum[k] += (double)hori[fidx + k] * kernal_vert[kidx];
                }
            }
            // assign to each channel
            for (k=0; k<sum_size; ++k) {
                vert[idx + k] = (unsigned char)sum[k];
            }
        }
    }
    free(kernal_hori);
    free(kernal_vert);
    free(hori);
    return vert;
}


int main(void)
{
    // gaussian_kernal_gen(5, 5, 0.8, 0.8);
    // free(gaussian_kernal_1d_gen(5, 0.8));
    // return 0;

    // const char *filename = "../images/girl_car.png";
    const char *filename = "../images/baboon.png";
    int img_width, img_height, channel;
    unsigned char *pixels = stbi_load(filename, &img_width, &img_height, &channel, 0);
    printf("width: %d, height: %d, channel: %d\n", img_width, img_height, channel);

    // To grayscale
    // int out_width = img_width;
    // int out_height = img_height;
    // int out_channel = 1;
    // unsigned char *gray_pixels = to_grayscale(img_width, img_height, channel, pixels);
    // char *gray_filename = "out_gray.png";
    // if (!stbi_write_png(gray_filename, img_width, img_height, 1, gray_pixels, img_width*1)) {
    //     fprintf(stderr, "Cannot write file [%s]\n", gray_filename);
    // } else {
    //     fprintf(stdout, "Writing file [%s] successfully\n", gray_filename);
    // }

    // Apply gaussian filter
    int out_width = img_width;
    int out_height = img_height;
    int out_channel = channel;
    unsigned char *out_pixels = gaussian_blur(img_width, img_height, channel, pixels, 7, 7, 2.5, 2.5);


    char *out_filename = "out_gaussian.png";
    if (!stbi_write_png(out_filename, out_width, out_height, out_channel, out_pixels, out_width*out_channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", out_filename);
    } else {
        fprintf(stdout, "Writing file [%s] successfully\n", out_filename);
    }

    stbi_image_free(pixels);
    // free(gray_pixels);
    free(out_pixels);

    return 0;
}
