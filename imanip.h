/*
   function prefix is `iman_`

*/
#ifndef IMANIP_H
#define IMANIP_H

typedef struct {
    int id;
    int w;                // image width
    int h;                // image height
    int channel;          // image channels
    unsigned char *data;  // image pixel data
} Iman_Img;

typedef struct Iman_Array {
    int id;
    unsigned char *data;
} Iman_Array;

#define IMAN_ARRAY_SIZE 256

// TODO: Dynamic array
typedef struct Da {
} Da;

void iman_init(void);
void iman_exit(void);

Iman_Img iman_img_new(int w, int h, int channel);  // Create empty image
Iman_Img iman_img_from_pixel_array(
    const int w,
    const int h,
    const int channel,
    const unsigned char *pixels);   // Copy an existing image
void iman_img_free(Iman_Img *img);  // Release memory of the image using `id` from internal structure
Iman_Img iman_img_copy(const Iman_Img src);  // Copy an existing image

Iman_Img iman_threshold(const Iman_Img src, int max_val, int threshold);
Iman_Img iman_grayscale(const Iman_Img src);
// Iman_Img iman_convolution(const Iman_Img src, int kx, int ky, double
// *kernal);
Iman_Img iman_arithmetic_mean_filter(const Iman_Img src, int kernal_size);
double *iman_gaussian_kernal_1d_gen(int kx, double sigx);
Iman_Img iman_gaussian_blur(const Iman_Img src,
                            int kx,
                            int ky,
                            double sigx,
                            double sigy);
Iman_Img iman_sobel(const Iman_Img src, double *direction, int threshold);
Iman_Img iman_canny(const Iman_Img src,
                    const int threshold_weak,
                    const int threshold_strong);
Iman_Img iman_hough_space(const Iman_Img src, double rho_resolution, double theta_resolution);
Da iman_hough_lines(const Iman_Img src, double rho_resolution, double theta_resolution, int threshold);
/*
 */
#endif  // IMANIP_H

#ifdef IMANIP_IMPLEMENTATION

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define IMAN_ASSERT(condition, format, ...)                            \
    do {                                                               \
        if (!(condition)) {                                            \
            fprintf(stderr, "%s:%d: Assertion failed: %s: ", __FILE__, \
                    __LINE__, #condition);                             \
            fprintf(stderr, format "\n", ##__VA_ARGS__);               \
            exit(EXIT_FAILURE);                                        \
        }                                                              \
    } while (0)

unsigned int _index = 0;
Iman_Array _iman_arr[IMAN_ARRAY_SIZE] = {0};

void _iman_arr_append(Iman_Img *img)
{
    img->id = _index;
    _iman_arr[_index].id = _index;
    _iman_arr[_index].data = img->data;
    _index += 1;
}

void _iman_arr_remove(int id)
{
    int i;
    for (i = 0; i < _index; ++i) {
        if (_iman_arr[i].id == id) {
            free(_iman_arr[i].data);
            _iman_arr[i].id = -1;
            _iman_arr[i].data = NULL;
            break;
        }
    }
}

void iman_init(void)
{
    return;
}

void iman_exit(void)
{
    int i;
    for (i = 0; i < _index; ++i) {
        _iman_arr_remove(i);
    }
}

Iman_Img iman_img_new(int w, int h, int channel)
{
    Iman_Img img = {
        .id = 0,  // TODO: A better way to get id
        .w = w,
        .h = h,
        .channel = channel,
        .data = calloc(w * h * channel, sizeof(unsigned char)),
    };
    _iman_arr_append(&img);
    return img;
}

Iman_Img iman_img_from_pixel_array(const int w,
                                   const int h,
                                   const int channel,
                                   const unsigned char *pixels)
{
    Iman_Img img = iman_img_new(w, h, channel);
    int i;
    for (i = 0; i < w * h * channel; ++i) {
        img.data[i] = pixels[i];
    }
    return img;
}

void iman_img_free(Iman_Img *img)
{
    _iman_arr_remove(img->id);
    img->id = -1;
    img->w = -1;
    img->h = -1;
    img->channel = -1;
    img->data = NULL;
}

Iman_Img iman_img_copy(const Iman_Img src)
{
    Iman_Img dst = iman_img_new(src.w, src.h, src.channel);
    int i;
    for (i = 0; i < src.w * src.h * src.channel; ++i) {
        dst.data[i] = src.data[i];
    }
    return dst;
}

Iman_Img iman_threshold(const Iman_Img src, int max_val, int threshold)
{
    int i;
    Iman_Img dst = iman_img_new(src.w, src.h, src.channel);
    for (i = 0; i < src.w * src.h * src.channel; ++i) {
        if (src.data[i] > threshold) {
            dst.data[i] = max_val;
        } else {
            dst.data[i] = 0;
        }
    }
    return dst;
}

Iman_Img iman_grayscale(const Iman_Img src)
{
    Iman_Img dst = iman_img_new(src.w, src.h, 1);
    int i, out_i;
    double gray;
    for (i = 0, out_i = 0; i < src.w * src.h * src.channel; i += src.channel) {
        if (src.channel == 1) {
            // Already grayscale
            dst.data[out_i++] = src.data[i];
        } else if (src.channel == 3) {
            // RGB
            gray = (src.data[i] * 0.299) + (src.data[i + 1] * 0.587) +
                   (src.data[i + 2] * 0.114);
            if (gray < 0)
                gray = 0;
            if (gray > 255)
                gray = 255;
            dst.data[out_i++] = (unsigned char) (gray + 0.5f);
        } else if (src.channel == 4) {
            // RGBA
            gray = (src.data[i] * 0.299) + (src.data[i + 1] * 0.587) +
                   (src.data[i + 2] * 0.114);
            unsigned char alpha = src.data[i + 3];
            dst.data[out_i++] =
                (unsigned char) ((gray * (alpha / 255.0)) + 0.5f);
        } else {
            // Unexpected channel count (e.g. 2)
            // fallback: average all channels
            double sum = 0;
            for (int k = 0; k < src.channel; ++k)
                sum += src.data[i + k];
            gray = sum / src.channel;
            dst.data[out_i++] = (unsigned char) (gray + 0.5f);
        }
    }
    return dst;
}

// Iman_Img iman_convolution(const Iman_Img src, int kx, int ky, double
// *kernal);

Iman_Img iman_arithmetic_mean_filter(const Iman_Img src, int kernal_size)
{
    int w = src.w, h = src.h, channel = src.channel;
    unsigned char *data = src.data;

    Iman_Img dst = iman_img_new(w, h, channel);
    unsigned char *out_pixels = dst.data;

    int half_kernal = (kernal_size - 1) / 2;
    int i, j, idx, column_size = w * channel;
    int fi, fj, fidx, r, g, b;
    int c;
    for (i = 0; i < h; ++i) {
        r = 0;
        g = 0;
        b = 0;
        // Calculate the sum of pixel value in kernal range
        // ie. the r, g, b values, calculate them in the beginning of a row
        for (fi = i - half_kernal; fi <= i + half_kernal; ++fi) {
            for (fj = -(half_kernal * channel); fj <= (half_kernal * channel);
                 fj += channel) {
                if (fi < 0 || fi >= h || fj < 0 || fj >= column_size)
                    continue;
                fidx = fj + fi * column_size;
                r += data[fidx + 0];
                if (channel >= 3) {
                    g += data[fidx + 1];
                    b += data[fidx + 2];
                }
            }
        }

        for (j = 0; j < column_size; j += channel) {
            idx = j + i * column_size;

            // Store into out_pixels
            out_pixels[idx + 0] = r / pow(kernal_size, 2);
            if (channel == 2) {
                out_pixels[idx + 1] = 255;
            } else if (channel >= 3) {
                out_pixels[idx + 1] = g / pow(kernal_size, 2);
                out_pixels[idx + 2] = b / pow(kernal_size, 2);
                if (channel == 4) {
                    out_pixels[idx + 3] = 255;
                }
            }

            // Update the value of r, g, b
            // [x, y, z] --(minus leftest)--> [y, z] --(add rightest+1)--> [y,
            // z, w]
            for (fi = i - half_kernal; fi <= i + half_kernal; ++fi) {
                fidx = j + fi * column_size;
                c = half_kernal * channel;
                if (fi < 0 || fi >= h)
                    continue;
                // Minus leftest column
                if (j - c >= 0) {
                    r -= data[fidx - c];
                    if (channel >= 3) {
                        g -= data[fidx + 1 - c];
                        b -= data[fidx + 2 - c];
                    }
                }
                // Add rightest+1 column
                if (j + channel + c < column_size) {
                    r += data[fidx + channel + c];
                    if (channel >= 3) {
                        g += data[fidx + channel + 1 + c];
                        b += data[fidx + channel + 2 + c];
                    }
                }
            }
        }
    }
    return dst;
}

double *iman_gaussian_kernal_1d_gen(int kx, double sigx)
{
    double *kernal = malloc(sizeof(double) * kx);
    double sum, g, k0;
    int i, origin = kx / 2;
    for (i = 0, sum = 0; i < kx; ++i) {
        // the coeficient will be canceled in the for loop below, thus no need
        // to calculate
        g = exp(-pow(i - origin, 2) / (2 * pow(sigx, 2)));
        sum += g;
        kernal[i] = g;
    }

    // k0 = kernal[0];
    for (i = 0; i < kx; ++i) {
        kernal[i] /= sum;
        // kernal[i] /= k0;
        // printf("%lf ", kernal[i]);
    }
    // printf("\n");
    return kernal;
}

Iman_Img iman_gaussian_blur(const Iman_Img src,
                            int kx,
                            int ky,
                            double sigx,
                            double sigy)
{
    IMAN_ASSERT((kx % 2 == 1 && ky % 2 == 1),
                "kernal size must be odd number, not (%d, %d)", kx, ky);
    IMAN_ASSERT((kx > 1 && ky > 1),
                "kernal size must greater than 1, not (%d, %d)", kx, ky);

    int w = src.w, h = src.h, channel = src.channel;
    unsigned char *data = src.data;

    unsigned char *hori = calloc(w * h * channel, sizeof(unsigned char));
    Iman_Img dst = iman_img_new(w, h, channel);
    unsigned char *vert = dst.data;
    // half gaussian kernal size
    int hx = kx / 2;
    int hy = ky / 2;

    // 1d kernal, horizontal and vertical direction
    double *kernal_hori = iman_gaussian_kernal_1d_gen(kx, sigx);
    double *kernal_vert = iman_gaussian_kernal_1d_gen(ky, sigy);

    int i, j, k, idx;
    int fi, fj, fidx, kidx;
    int col_size = w * channel;

    double sum[channel], norm;

    // Apply gaussian kernal in horizontal direction to the data, store into
    // hori
    for (i = 0; i < h; ++i) {                      // for each row
        for (j = 0; j < col_size; j += channel) {  // for each pixel
            for (k = 0; k < channel; ++k) {
                sum[k] = 0.f;
            }
            norm = 0.0;
            idx = j + i * col_size;
            for (fj = j - hx * channel, kidx = 0; fj <= j + hx * channel;
                 fj += channel,
                ++kidx) {  // apply kernal to each pixel, each channel
                if (fj < 0 || fj >= col_size)
                    continue;
                fidx = fj + i * col_size;
                // for each channel
                for (k = 0; k < channel; ++k) {
                    sum[k] += (double) data[fidx + k] * kernal_hori[kidx];
                }
                norm += kernal_hori[kidx];
            }
            // assign to each channel
            for (k = 0; k < channel; ++k) {
                double val = sum[k] / norm;
                if (val < 0)
                    val = 0;
                if (val > 255)
                    val = 255;
                hori[idx + k] = (unsigned char) (val + 0.5f);
            }
        }
    }

    // Apply gaussian kernal in vertical direction to the hori, store into vert
    for (j = 0; j < col_size; j += channel) {
        for (i = 0; i < h; ++i) {
            for (k = 0; k < channel; ++k) {
                sum[k] = 0;
            }
            norm = 0.f;
            idx = j + i * col_size;
            for (fi = i - hy, kidx = 0; fi <= i + hy; ++fi, ++kidx) {
                if (fi < 0 || fi >= h)
                    continue;
                fidx = j + fi * col_size;
                // for each channel
                for (k = 0; k < channel; ++k) {
                    sum[k] += (double) hori[fidx + k] * kernal_vert[kidx];
                }
                norm += kernal_vert[kidx];
            }
            // assign to each channel
            for (k = 0; k < channel; ++k) {
                double val = sum[k] / norm;
                if (val < 0)
                    val = 0;
                if (val > 255)
                    val = 255;
                vert[idx + k] = (unsigned char) (val + 0.5f);
            }
        }
    }
    free(kernal_hori);
    free(kernal_vert);
    free(hori);

    return dst;
}

Iman_Img iman_sobel(const Iman_Img src, double *direction, int threshold)
{
    Iman_Img gray = iman_grayscale(src);

    int w = gray.w, h = gray.h;
    unsigned char *data = gray.data;
    Iman_Img dst = iman_img_new(w, h, 1);

    int s_half = 1;
    int sx[] = {1, 0, -1, 2, 0, -2, 1, 0, -1};
    int sy[] = {1, 2, 1, 0, 0, 0, -1, -2, -1};
    unsigned char *gx = malloc(sizeof(unsigned char) * w * h);
    unsigned char *gy = dst.data;  // malloc(sizeof(unsigned char) * w * h);

    int i, j, idx;
    int fi, fj, fidx, sum_x, sum_y, c;
    for (i = 0; i < h; ++i) {
        for (j = 0; j < w; ++j) {
            idx = j + i * w;

            sum_x = 0;
            sum_y = 0;
            c = 0;
            for (fi = i - s_half; fi <= i + s_half; ++fi) {
                for (fj = j - s_half; fj <= j + s_half; ++fj) {
                    if (fi < 0 || fi >= h || fj < 0 || fj >= w)
                        continue;
                    fidx = fj + fi * w;
                    sum_x += data[fidx] * sx[c];
                    sum_y += data[fidx] * sy[c];
                    ++c;
                }
            }

            int val = sqrt(pow(sum_x, 2) + pow(sum_y, 2));
            if (threshold < 0 || threshold > 255) {
                if (val > 255)
                    val = 255;
                else if (val < 0)
                    val = 0;
            } else {
                if (val > threshold)
                    val = 255;
                else
                    val = 0;
            }
            gy[idx] = val;
            if (direction != NULL)
                direction[idx] = atan2((double) sum_y, (double) sum_x);
        }
    }
    free(gx);

    return dst;
}

Iman_Img iman_canny(const Iman_Img src,
                    const int threshold_weak,
                    const int threshold_strong)
{
    int i, j, idx;

    // To grayscale
    Iman_Img gray = iman_grayscale(src);

    // Gaussian blur
    Iman_Img gaussian = iman_gaussian_blur(gray, 5, 5, 1.4, 1.4);

    // Apply sobel operator
    double *dir = malloc(sizeof(double) * gaussian.w * gaussian.h);
    Iman_Img sobel = iman_sobel(gaussian, dir, -1);
    // Turn direction into angle
    for (i = 0; i < sobel.h; ++i) {
        for (j = 0; j < sobel.w; ++j) {
            idx = j + i * sobel.w;
            dir[idx] = dir[idx] * 180.f / M_PI;
            if (dir[idx] < 0.f)
                dir[idx] += 180.f;
        }
    }

    // Non-maximum suppression
    Iman_Img non_max = iman_img_new(sobel.w, sobel.h, 1);
    unsigned char q, r, tmp;
    double angle;
    for (i=1; i<sobel.h - 1; ++i) {  // Skip border
        for (j=1; j<sobel.w - 1; ++j) {  // Skip border
            idx = j + i*sobel.w;
            angle = dir[idx];

            // 45 degree, i.e. north-east and south-west direction
            if (dir[idx] >= 22.5 && dir[idx] < 67.5) {
                q = sobel.data[idx - sobel.w + 1];
                r = sobel.data[idx + sobel.w - 1];
            }
            // 90 degree, i.e. north and south direction
            else if (dir[idx] >= 67.5 && dir[idx] < 112.5) {
                q = sobel.data[idx - sobel.w];
                r = sobel.data[idx + sobel.w];
            }
            // 135 degree, i.e. north-west and south-east direction
            else if (dir[idx] >= 112.5 && dir[idx] < 157.5) {
                q = sobel.data[idx-sobel.w-1];
                r = sobel.data[idx+sobel.w+1];
            }
            // 0 degree, i.e. east and west direction
            // [0, 22.5) and [157.5, 180]
            else {
                q = sobel.data[idx+1];
                r = sobel.data[idx-1];
            }

            // Keep pixel if its local maximum
            if (sobel.data[idx] >= q && sobel.data[idx] >= r) {
                non_max.data[idx] = sobel.data[idx];
            } else {
                non_max.data[idx] = 0;
            }
        }
    }

    // Double threshold
    for (i=0; i<non_max.h; ++i) {
        for (j=0; j<non_max.w; ++j) {
            idx = j + i*non_max.w;
            if (non_max.data[idx] > threshold_strong) {
                non_max.data[idx] = 255;
            } else if (non_max.data[idx] < threshold_weak) {
                non_max.data[idx] = 0;
            }
        }
    }

    // Edge tracking by hysteresis
    // By ChatGPT
    // TODO: Check, maybe use queue (BFS)
    typedef struct { int x, y; } Point;
    Point *stack = malloc(sizeof(Point) * non_max.w * non_max.h);
    int top = 0;
    // Push all strong edges initially
    for (i=0; i<non_max.h; ++i)
        for (j=0; j<non_max.w; ++j)
            if (non_max.data[i*non_max.w + j] == 255)
                stack[top++] = (Point){j, i};

    // DFS-style edge tracking
    while (top > 0) {
        Point p = stack[--top];
        for (int dy=-1; dy<=1; ++dy) {
            for (int dx=-1; dx<=1; ++dx) {
                int x = p.x + dx, y = p.y + dy;
                if (x<0 || x>=non_max.w || y<0 || y>=non_max.h) continue;
                int idx2 = y*non_max.w + x;
                if (non_max.data[idx2] >= threshold_weak && non_max.data[idx2] < 255) {
                    non_max.data[idx2] = 255;
                    stack[top++] = (Point){x, y};
                }
            }
        }
    }

    // Cleanup: remove leftover weak pixels
    for (i = 0; i < non_max.h; ++i) {
        for (j = 0; j < non_max.w; ++j) {
            idx = j + i * non_max.w;
            if (non_max.data[idx] != 255) {
                non_max.data[idx] = 0;
            }
        }
    }

    free(dir);
    free(stack);
    iman_img_free(&gray);
    iman_img_free(&gaussian);
    iman_img_free(&sobel);
    // iman_img_free(&non_max);
    return non_max;
}

Iman_Img iman_hough_space(const Iman_Img src, double rho_resolution, double theta_resolution)
{
    // Half length of image diagonal
    double max_rho = sqrt(pow(src.h/2.f, 2) + pow(src.w/2.f, 2));
    // hough space image width and height
    int hs_img_w = M_PI / theta_resolution + 1; // 180 / (theta_resolution * 180.f / M_PI);
    int hs_img_h = (2 * max_rho) / rho_resolution + 1;
    Iman_Img hs_img = iman_img_new(hs_img_w, hs_img_h, 1);
    // Accumulator
    double *hs_acc = calloc(hs_img_w * hs_img_h, sizeof(double));
    int rho_idx;
    int theta_idx;

    // Shifted x and y
    int x, y, idx;
    int sx, sy;

    // Loop through each pixel
    for (y = 0; y < src.h; ++y) {
        sy = -y + src.h / 2;
        for (x = 0; x < src.w; ++x) {
            sx = x - src.w / 2;
            idx = x + y * src.w;

            // If it's an edge pixel
            if (src.data[idx] > 0) {
                for (theta_idx = 0; theta_idx < hs_img_w; ++theta_idx) {
                    double rad = theta_idx * theta_resolution;
                    double original_rho = (sx * cos(rad) + sy * sin(rad));

                    // Shift and scaled (to resolution) rho
                    double r = (original_rho + max_rho) / rho_resolution;

                    if (r >= 0 && r < hs_img_h) {
                        // Vote neighbors
                        int r0 = floor(r);
                        int r1 = r0 + 1;
                        double dr = r - r0;

                        // hs_acc[r0 * hs_img_w + theta_idx] += 1.f;
                        hs_acc[r0 * hs_img_w + theta_idx] += (1 - dr);

                        if (r1 < hs_img_h) {
                            // hs_acc[r1 * hs_img_w + theta_idx] += 1.f;
                            hs_acc[r1 * hs_img_w + theta_idx] += (dr);
                        }
                    }
                }
            }
        }
    }

    // Normalize 0–255
    double maxv = 0;
    for (int i = 0; i < hs_img_w * hs_img_h; ++i)
        if (hs_acc[i] > maxv)
            maxv = hs_acc[i];

    for (int i = 0; i < hs_img_w * hs_img_h; ++i)
        hs_img.data[i] = (unsigned char)(hs_acc[i] / maxv * 255.0);

    free(hs_acc);
    return hs_img;
}

Da iman_hough_lines(const Iman_Img src, double rho_resolution, double theta_resolution, int threshold)
{
    // Half length of image diagonal
    double max_rho = sqrt(pow(src.h/2.f, 2) + pow(src.w/2.f, 2));

    // Transform edge pixels from xy space to hough space
    Iman_Img hs_img = iman_hough_space(src, rho_resolution, theta_resolution);

    // Loop through accumulator
    int rho_idx, theta_idx, hs_img_idx;
    for (rho_idx = 0; rho_idx < max_rho; ++rho_idx) {
        for (theta_idx = 0; theta_idx < 180; ++theta_idx) {
            hs_img_idx = rho_idx * hs_img.w + theta_idx;
            // Find accumulator that greater than threshold
            if (hs_img.data[hs_img_idx] >= threshold) {
                // printf("Line detected: rho=%d, theta=%d\n", rho_idx, theta_idx);

                // Transform to xy space
                double rho = rho_idx - max_rho;
                double theta = theta_idx * M_PI / 180.f;

                double ct = cos(theta);
                double st = sin(theta);

                if (fabs(st) > fabs(ct)) {
                    // use x to get y
                    for (int x = 0; x < src.w; x++) {
                        // Shift x, calc y, shift y back to original src image coordinate
                        double y = -((rho - (x - src.w/2) * ct) / st - src.h/2);
                        if (y >= 0 && y < src.h) {
                            // line_img.data[(int)y * line_img.w + x] = 255;
                            // TODO: Append to Da
                        }
                    }
                } else {
                    // use y to get x
                    for (int y = 0; y < src.h; y++) {
                        // Shift y, calc x, shift x back to original src image coordinate
                        double x = (rho - ((-y + src.h/2) * st)) / ct + src.w/2;
                        if (x >= 0 && x < src.w) {
                            // line_img.data[y * line_img.w + (int)x] = 255;
                            // TODO: Append to Da
                        }
                    }
                }
            }
        }
    }
}

#endif  // IMANIP_IMPLEMENTATION
