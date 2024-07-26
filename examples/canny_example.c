/*
   Canny edge detection example

   Compile with:
   gcc canny_example.c -o canny_example.out -lm
*/
#include <stdio.h>

// Reading and writing images
#define STB_IMAGE_IMPLEMENTATION
#include "./stb_library/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb_library/stb_image_write.h"

// Image process functionality
#define IMANIP_IMPLEMENTATION
#include "../imanip.h"

int main(void)
{
    // Read the image
    const char *filename = "./baboon.png";
    Img src;
    src.data = stbi_load(filename, &src.w, &src.h, &src.channel, 0);
    printf("width: %d, height: %d, channel: %d\n", src.w, src.h, src.channel);

    // Create an image and apply canny edge detection
    Img out;
    iman_img_new(&out, src.w, src.h, 1);
    iman_canny(out, src, 25, 75);

    // Write out the image
    char *out_filename = "canny.png";
    if (!stbi_write_png(out_filename, out.w, out.h, out.channel, out.data, out.w*out.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", out_filename);
    } else {
        fprintf(stdout, "[%s] created successfully\n", out_filename);
    }

    // Free the arrays
    stbi_image_free(src.data);
    iman_img_free(&out);

    return 0;
}
