/*
   Example of using xor operator to hide a secret image with noise image as a key

   Compile with:
   gcc xor.c -o xor.out -lm
*/

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_library/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb_library/stb_image_write.h"

#define IMANIP_IMPLEMENTATION
#include "../imanip.h"

int main(void)
{
    // Read the image
    const char *filename = "./baboon.png";
    Img src;
    src.data = stbi_load(filename, &src.w, &src.h, &src.channel, 0);
    printf("width: %d, height: %d, channel: %d\n", src.w, src.h, src.channel);

    // Create key
    Img key;
    iman_img_new(&key, src.w, src.h, src.channel);
    iman_noise(key, -1);

    // Write out the key image
    char *key_filename = "xor-key.png";
    if (!stbi_write_png(key_filename, key.w, key.h, key.channel, key.data, key.w*key.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", key_filename);
    } else {
        fprintf(stdout, "[%s] created successfully\n", key_filename);
    }

    // Encode
    Img secret;
    iman_img_new(&secret, src.w, src.h, src.channel);
    iman_xor(secret, src, key);

    // Write out the encoded image
    char *secret_filename = "xor-secret.png";
    if (!stbi_write_png(secret_filename, secret.w, secret.h, secret.channel, secret.data, secret.w*secret.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", secret_filename);
    } else {
        fprintf(stdout, "[%s] created successfully\n", secret_filename);
    }

    // Decode
    Img origin;
    iman_img_new(&origin, secret.w, secret.h, secret.channel);
    iman_xor(origin, secret, key);

    // Write out the decoded image
    char *origin_filename = "xor-origin.png";
    if (!stbi_write_png(origin_filename, origin.w, origin.h, origin.channel, origin.data, origin.w*origin.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", origin_filename);
    } else {
        fprintf(stdout, "[%s] created successfully\n", origin_filename);
    }

    // Free the arrays
    stbi_image_free(src.data);
    iman_img_free(&key);
    iman_img_free(&secret);
    iman_img_free(&origin);
}
