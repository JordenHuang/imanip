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
    gray.w = src.w;
    gray.h = src.h;
    gray.channel = 1;
    gray.data = malloc(sizeof(unsigned char) * gray.w * gray.h * 1);
    iman_grayscale(gray, src);

    Img out;
    // out.w = src.w; out.h = src.h; out.channel = src.channel;
    out.w = gray.w; out.h = gray.h; out.channel = gray.channel;
    out.data = malloc(sizeof(unsigned char) * out.w * out.h * out.channel);
    iman_sobel(out, gray, -1);

    // arithmetic_mean_filter
    // iman_arithmetic_mean_filter(out, src, 15);

    // gaussian_blur
    Img gau;
    gau.w = out.w; gau.h = out.h; gau.channel = out.channel;
    gau.data = malloc(sizeof(unsigned char) * gau.w * gau.h * gau.channel);
    iman_gaussian_blur(gau, out, 3, 3, 0.7, 0.7);

    // Output blur img
    char *out_filename = "imanip.png";
    if (!stbi_write_png(out_filename, gau.w, gau.h, gau.channel, gau.data, gau.w*gau.channel)) {
    // if (!stbi_write_png(out_filename, out.w, out.h, out.channel, out.data, out.w*out.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", out_filename);
    } else {
        fprintf(stdout, "Writing file [%s] successfully\n", out_filename);
    }

    stbi_image_free(src.data);
    free(gray.data);
    free(out.data);
    free(gau.data);
    return 0;
}
