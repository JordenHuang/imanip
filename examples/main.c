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
    int w, h, channel;
    unsigned char *pixels = stbi_load(filename, &w, &h, &channel, 0);
    printf("[%s] width: %d, height: %d, channel: %d\n", filename, w, h, channel);

    iman_init();

    Iman_Img img = iman_img_from_pixel_array(w, h, channel, pixels);

#if 0
    // Grayscale
    Iman_Img grayscale_img = iman_grayscale(img);
    char *grayscale_filename = "grayscale.png";
    if (!stbi_write_png(grayscale_filename, grayscale_img.w, grayscale_img.h, grayscale_img.channel, grayscale_img.data, grayscale_img.w*grayscale_img.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", grayscale_filename);
    }

    // Binary threshold
    Iman_Img binary_img = iman_threshold(grayscale_img, 255, 150);
    char *binary_filename = "binary.png";
    if (!stbi_write_png(binary_filename, binary_img.w, binary_img.h, binary_img.channel, binary_img.data, binary_img.w*binary_img.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", binary_filename);
    }

    // Arithmetic mean filter
    Iman_Img amf_img = iman_arithmetic_mean_filter(img, 7);
    char *amf_filename = "amf.png";
    if (!stbi_write_png(amf_filename, amf_img.w, amf_img.h, amf_img.channel, amf_img.data, amf_img.w*amf_img.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", amf_filename);
    }

    // Gausian blur
    Iman_Img gb_img = iman_gaussian_blur(img, 5, 5, 1.4, 1.4);
    char *gb_filename = "gb.png";
    if (!stbi_write_png(gb_filename, gb_img.w, gb_img.h, gb_img.channel, gb_img.data, gb_img.w*gb_img.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", gb_filename);
    }

    // Sobel
    Iman_Img sobel_img = iman_sobel(img, NULL, -1);
    char *sobel_filename = "sobel.png";
    if (!stbi_write_png(sobel_filename, sobel_img.w, sobel_img.h, sobel_img.channel, sobel_img.data, sobel_img.w*sobel_img.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", sobel_filename);
    }

    // Canny
    Iman_Img canny_img = iman_canny(img, 25, 75);
    char *canny_filename = "canny.png";
    if (!stbi_write_png(canny_filename, canny_img.w, canny_img.h, canny_img.channel, canny_img.data, canny_img.w*canny_img.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", canny_filename);
    }
#endif

#if 0
    double p_rho = 1, p_theta = M_PI/180; // Resolution of rho and theta
    int threshold = 35;

    double rho, theta;
    int x, y, shifted_x, shifted_y, idx, hough_space_img_idx;
    Iman_Img line_img = iman_img_new(img.w, img.h, 1);
    Iman_Img canny_tmp_img = iman_canny(img, 25, 75);

    // // Test image: only one white pixel
    // Iman_Img empty_img = iman_img_new(img.w, img.h, 1);
    // // Draw a line
    // y = img.h / 2;
    // for (x = 0; x < empty_img.w; ++x) {
    //     empty_img.data[y * img.w + x] = 255;
    // }
    // Iman_Img canny_tmp_img = empty_img;
    // char *empty_filename = "empty.png";
    // if (!stbi_write_png(empty_filename, empty_img.w, empty_img.h, empty_img.channel, empty_img.data, empty_img.w*empty_img.channel)) {
    //     fprintf(stderr, "Cannot write file [%s]\n", empty_filename);
    // }

    double max_rho = sqrt(pow(canny_tmp_img.h/2.0, 2) + pow(canny_tmp_img.w/2.0, 2));
    int hough_space_img_w = 180;
    int hough_space_img_h = 2 * max_rho;
    Iman_Img hough_space_img = iman_img_new(hough_space_img_w, hough_space_img_h, 1);

    for (y = 0; y < canny_tmp_img.h; ++y) {
        shifted_y = -y + canny_tmp_img.h / 2;
        // printf("sy = (%d)\n", shifted_y);
        for (x = 0; x < canny_tmp_img.w; ++x) {
            idx = x + y * canny_tmp_img.w;
            shifted_x = x - canny_tmp_img.w / 2;
            // printf("sx, sy = (%d, %d)\n", shifted_x, shifted_y);
            // If it's an edge pixel
            if (canny_tmp_img.data[idx] > 0) {
                for (theta = 0; theta < 180; theta += 1) {
                    double rad = theta * M_PI / 180.0;
                    rho = shifted_x * cos(rad) + shifted_y * sin(rad);
                    // printf("theta: %lf, rho: %lf, %d\n", theta, rho, shifted_x);
                    // hough_space_img_idx = (int)(rho + hough_space_img_h/2.0) * hough_space_img_w + theta;
                    // hough_space_img.data[hough_space_img_idx] += 1;
                    int rho_idx = (int)(rho + max_rho);
                    if (rho_idx >= 0 && rho_idx < hough_space_img_h) {
                        int hough_space_img_idx = rho_idx * hough_space_img_w + theta;
                        hough_space_img.data[hough_space_img_idx] += 1;
                    }
                }
            }
        }
    }

    // // For visualization
    // for (y = 0; y < canny_tmp_img.h; ++y) {
    //     shifted_y = -y + canny_tmp_img.h / 2;
    //     for (x = 0; x < canny_tmp_img.w; ++x) {
    //         shifted_x = x - canny_tmp_img.w / 2;
    //         for (theta = 0; theta < 180; theta += 1) {
    //             double rad = theta * M_PI / 180.0;
    //             rho = shifted_x * cos(rad) + shifted_y * sin(rad);
    //             int rho_idx = (int)(rho + max_rho);
    //             if (rho_idx >= 0 && rho_idx < hough_space_img_h) {
    //                 int hough_space_img_idx = rho_idx * hough_space_img_w + theta;
    //                 if (hough_space_img.data[hough_space_img_idx] != 0)
    //                     hough_space_img.data[hough_space_img_idx] = 255;
    //             }
    //         }
    //     }
    // }

    // Find accumulator that greater than threshold
    struct Hough_Line {
         int rho;
         int theta;
    } hough_lines[30000];
    int line_count = 0;

    int rho_idx, theta_idx;
    for (rho_idx = 0; rho_idx < max_rho; ++rho_idx) {
        for (theta_idx = 0; theta_idx < 180; ++theta_idx) {
            int hough_space_img_idx = rho_idx * hough_space_img_w + theta_idx;
            if (hough_space_img.data[hough_space_img_idx] >= threshold) {
                // printf("Line detected: rho=%d, theta=%d\n", rho_idx, theta_idx);
                hough_lines[line_count].rho = rho_idx;
                hough_lines[line_count].theta = theta_idx;
                line_count++;
            }
        }
    }

    int lc;
    for (lc = 0; lc < line_count; ++lc) {
        double rho = hough_lines[lc].rho - max_rho;
        double theta = hough_lines[lc].theta * M_PI / 180.0;

        double ct = cos(theta);
        double st = sin(theta);

        if (fabs(st) > fabs(ct)) {
            // use x -> y
            for (int x = 0; x < img.w; x++) {
                double y = -((rho - (x - img.w/2) * ct) / st - img.h/2);
                if (y >= 0 && y < img.h) {
                    line_img.data[(int)y * line_img.w + x] = 255;
                }
            }
        } else {
            // use y -> x
            for (int y = 0; y < img.h; y++) {
                double x = (rho - ((-y + img.h/2) * st)) / ct + img.w/2;
                if (x >= 0 && x < img.w) {
                    line_img.data[y * line_img.w + (int)x] = 255;
                }
            }
        }
    }

    char *hough_space_filename = "hough_space.png";
    if (!stbi_write_png(hough_space_filename, hough_space_img.w, hough_space_img.h, hough_space_img.channel, hough_space_img.data, hough_space_img.w*hough_space_img.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", hough_space_filename);
    }

    char *line_filename = "line.png";
    if (!stbi_write_png(line_filename, line_img.w, line_img.h, line_img.channel, line_img.data, line_img.w*line_img.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", line_filename);
    }
#endif



    // Canny
    // Iman_Img canny_img = iman_canny(img, 25, 75);
    Iman_Img canny_img = iman_canny(img, 10, 10);
    char *canny_filename = "canny.png";
    if (!stbi_write_png(canny_filename, canny_img.w, canny_img.h, canny_img.channel, canny_img.data, canny_img.w*canny_img.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", canny_filename);
    }

    Iman_Img hs_img = iman_hough_space(canny_img, 1, 1 * M_PI/180.f);
    // Visualize
    // for (int y = 0; y < hs_img.h; ++y) {
    //     for (int x = 0; x < hs_img.w; ++x) {
    //         if (hs_img.data[y * hs_img.w + x] > 0)
    //             hs_img.data[y * hs_img.w + x] = 255;
    //     }
    // }

    char *hs_filename = "hs.png";
    if (!stbi_write_png(hs_filename, hs_img.w, hs_img.h, hs_img.channel, hs_img.data, hs_img.w*hs_img.channel)) {
        fprintf(stderr, "Cannot write file [%s]\n", hs_filename);
    }

    iman_exit();

    stbi_image_free(pixels);

    return 0;
}
