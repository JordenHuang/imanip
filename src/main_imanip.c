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

    Img out;
    iman_img_new(&out, src.w, src.h, 1);
    iman_canny(out, src, 0, 0);


    char *out_filename = "imanip.png";
    if (!stbi_write_png(out_filename, out.w, out.h, out.channel, out.data, out.w*out.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", out_filename);
    } else {
        fprintf(stdout, "Writing file [%s] successfully\n", out_filename);
    }

    stbi_image_free(src.data);
    iman_img_free(&out);

    return 0;
}