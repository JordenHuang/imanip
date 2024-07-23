#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_library/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb_library/stb_image_write.h"

#define IMANIP_IMPLEMENTATION
#include "./imanip.h"

int main(void)
{
    const char *filename = "../images/engine.png";
    // const char *filename = "../images/baboon.png";
    Img src;
    src.data = stbi_load(filename, &src.w, &src.h, &src.channel, 0);
    printf("width: %d, height: %d, channel: %d\n", src.w, src.h, src.channel);

    // To grayscale
    Img gray;
    iman_img_new(&gray, src.w, src.h, 1);
    iman_grayscale(gray, src);

    Img out;
    iman_img_new(&out, gray.w, gray.h, gray.channel);
    // iman_img_new(&out, src.w, src.h, src.channel);
    // double kernal[] = {
    //     1.f/9, 1.f/9, 1.f/9,
    //     1.f/9, 1.f/9, 1.f/9,
    //     1.f/9, 1.f/9, 1.f/9
    // };
    double kernal[] = {
        -1, -1, -1,
        -1, 8, -1,
        -1, -1, -1
    };
    // double kernal[] = {
    //     -2, -1, 0,
    //     -1, 1, 1,
    //     0, 1, 2
    // };
    iman_convolution_lf(out, gray, 3, 3, kernal);
    iman_threshold(gray, out, 255, 225);

    Img gau;
    iman_img_new(&gau, gray.w, gray.h, gray.channel);
    iman_gaussian_blur(gau, gray, 3, 3, 0.5, 0.5);

    iman_img_copy(&out, gau);
    // iman_convolution_lf(out, src, 3, 3, kernal);
    // iman_sobel(out, gray, -1);

    // arithmetic_mean_filter
    // iman_arithmetic_mean_filter(out, src, 15);

    // gaussian_blur
    // Img gau;
    // iman_img_new(&gau, out.w, out.h, out.channel);
    // iman_gaussian_blur(gau, out, 3, 3, 0.7, 0.7);

    // Output blur img
    char *out_filename = "imanip.png";
    // char *out_filename = "emboss.png";
    // if (!stbi_write_png(out_filename, gau.w, gau.h, gau.channel, gau.data, gau.w*gau.channel)) {
    if (!stbi_write_png(out_filename, out.w, out.h, out.channel, out.data, out.w*out.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", out_filename);
    } else {
        fprintf(stdout, "Writing file [%s] successfully\n", out_filename);
    }

    stbi_image_free(src.data);
    iman_img_free(&gray);
    iman_img_free(&out);
    iman_img_free(&gau);

    return 0;
}
