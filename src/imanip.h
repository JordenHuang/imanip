/*
   function prefix is `iman_`

*/
#ifndef IMANIP_H
#define IMANIP_H

typedef struct {
    int w;              // image width
    int h;              // image height
    int channel;        // image channels
    unsigned char *data;// image pixel data
} Img;

void iman_convolution_i(Img dst, int *kernal, int k_size, int stride);

// Turn src image into grayscale, store to dst
void iman_grayscale(Img dst, Img src);
// Apply kernal_size*kernal_size arithmetic mean filter to src, store to dst
void iman_arithmetic_mean_filter(Img dst, Img src, int kernal_size);
// Generate 1d gaussian kernal
double *iman_gaussian_kernal_1d_gen(int kx, double sigx);
// Apply kx*ky size with sigma x and sigma y gaussian kernal on src, sotre to dst
void iman_gaussian_blur(Img dst, Img src, int kx, int ky, double sigx, double sigy);
// Apply sobel operator on src, store to dst. Kernal size is 3*3
// If threshold is NOT between 0 and 255, no threshold is used
void iman_sobel(Img dst, Img src, int threshold);

#endif // IMANIP_H

#ifdef IMANIP_IMPLEMENTATION

#include <math.h>

void iman_grayscale(Img dst, const Img src)
{
    int w=src.w, h=src.h, channel=src.channel;
    int i, out_i;
    double sum;
    for (i=0, out_i=0; i<w*h*channel; i+=channel) {
        sum = (src.data[i] * 0.299) + (src.data[i+1] * 0.587) + (src.data[i+2] * 0.114);
        dst.data[out_i++] = (int)sum;
    }
}

void iman_arithmetic_mean_filter(Img dst, const Img src, int kernal_size)
{
    int w=src.w, h=src.h, channel=src.channel;
    unsigned char *data = src.data;
    unsigned char *out_pixels = dst.data;

    int half_kernal = (kernal_size-1) / 2;
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
                if (channel >= 3) {
                    g += data[fidx+1];
                    b += data[fidx+2];
                }
            }
        }

        for (j=0; j<column_size; j+=channel) {
            idx = j + i*column_size;

            // Store into out_pixels
            out_pixels[idx+0] = r/pow(kernal_size, 2);
            if (channel == 2) {
                out_pixels[idx+1] = 255;
            }
            else if (channel >= 3) {
                out_pixels[idx+1] = g/pow(kernal_size, 2);
                out_pixels[idx+2] = b/pow(kernal_size, 2);
                if (channel == 4){
                    out_pixels[idx+3] = 255;
                }
            }

            // Update the value of r, g, b
            // [x, y, z] --(minus leftest)--> [y, z] --(add rightest+1)--> [y, z, w]
            for (fi=i-half_kernal; fi<=i+half_kernal; ++fi) {
                fidx = j + fi*column_size;
                c = half_kernal*channel;
                if (fi<0 || fi>=h) continue;
                // Minus leftest column
                if (j-c >= 0) {
                    r -= data[fidx-c];
                    if (channel >= 3) {
                        g -= data[fidx+1-c];
                        b -= data[fidx+2-c];
                    }
                }
                // Add rightest+1 column
                if (j+channel+c < column_size) {
                    r += data[fidx+channel+c];
                    if (channel >= 3) {
                        g += data[fidx+channel+1+c];
                        b += data[fidx+channel+2+c];
                    }
                }
            }
        }
    }
}

double *iman_gaussian_kernal_1d_gen(int kx, double sigx)
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

void iman_gaussian_blur(Img dst, Img src, int kx, int ky, double sigx, double sigy)
{
    if (kx%2 != 1 || ky%2 != 1 ) {
        fprintf(stderr, "kernal size must be odd number, not (%d, %d)", kx, ky);
        return;
    } else if (kx == 1 || ky == 1) {
        fprintf(stderr, "kernal size must greater than 1, not (%d, %d)", kx, ky);
        return;
    }

    int w=src.w, h=src.h, channel=src.channel;
    unsigned char *data = src.data;

    unsigned char *hori = malloc(sizeof(unsigned char) * w * h * channel);
    unsigned char *vert = dst.data;
    // half gaussian kernal size
    int hx = kx/2;
    int hy = ky/2;

    // 1d kernal, horizontal and vertical direction
    double *kernal_hori = iman_gaussian_kernal_1d_gen(kx, sigx);
    double *kernal_vert = iman_gaussian_kernal_1d_gen(ky, sigy);

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
}

void iman_sobel(Img dst, Img src, int threshold)
{
    if (dst.channel != 1){
        fprintf(stderr, "[Error] destination image channel must be 1, i.e. grayscale image");
        return;
    }

    int w=src.w, h=src.h, channel=src.channel;
    unsigned char *data = src.data;

    int s_half = 1;
    int sx[] = {
        1, 0, -1,
        2, 0, -2,
        1, 0, -1
    };
    int sy[] = {
        1, 2, 1,
        0, 0, 0,
        -1, -2, -1
    };
    unsigned char *gx = malloc(sizeof(unsigned char) * w * h);
    unsigned char *gy = dst.data; //malloc(sizeof(unsigned char) * w * h);

    int i, j, idx;
    int fi, fj, fidx, sum_x, sum_y, c;
    for (i=0; i<h; ++i) {
        for (j=0; j<w; ++j) {
            idx = j + i*w;

            sum_x = 0;
            sum_y = 0;
            c = 0;
            for (fi=i-s_half; fi<=i+s_half; ++fi) {
                for (fj=j-s_half; fj<=j+s_half; ++fj) {
                    if (fi<0 || fi>=h || fj<0 || fj>=w) continue;
                    fidx = fj + fi*w;
                    sum_x += data[fidx] * sx[c];
                    sum_y += data[fidx] * sy[c];
                    ++c;
                }
            }

            int val = sqrt(pow(sum_x, 2) + pow(sum_y, 2));
            if (threshold < 0 || threshold > 255) {
                if (val > 255) val = 255;
                else if (val < 0) val = 0;
            } else {
                if (val > threshold) val = 255;
                else val = 0;
            }
            gy[idx] = val;
            // if (val > threshold) gy[idx] = 255;
            // else gy[idx] = 0;
            // printf("%d ", gy[idx]);
        }
        // printf("\n");
    }

    free(gx);
}

#endif // IMANIP_IMPLEMENTATION
