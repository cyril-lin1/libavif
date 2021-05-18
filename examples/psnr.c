/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 * psnr.h
 * Original Author: xu_lin@ruijie.com.cn
 * 计算PSNR
 * History:
 *
 */

#include "psnr.h"
#include <math.h>

/* MSE(a,b) = 1/N * SUM((a-b)^2) */
float iqa_mse(const unsigned char *ref, const unsigned char *cmp, int w, int h, int stride)
{
    int error, offset;
    unsigned long long sum = 0;
    int ww, hh;
    for (hh = 0; hh<h; ++hh) {
        offset = hh*stride;
        for (ww = 0; ww<w; ++ww, ++offset) {
            error = ref[offset] - cmp[offset];
            sum += error * error;
        }
    }
    return (float)((double)sum / (double)(w*h));
}

/* PSNR(a,b) = 10*log10(L^2 / MSE(a,b)), where L=2^b - 1 (8bit = 255) */
float iqa_psnr(const unsigned char *ref, const unsigned char *cmp, int w, int h, int stride)
{
    const int L_sqd = 255 * 255;
    float mse = iqa_mse(ref, cmp, w, h, stride);
    if (mse == 0.00) {
        return 100.00;        /* 两份相同数据的psnr设置为100 */
    }
    return (float)(10.0 * log10(L_sqd / mse));
}

/* RGBA的psnr, 并且计算的psnr不带A, cmp相对于ref是上下颠倒的 */
float RGBA_mse(const unsigned char* ref, const unsigned char* cmp, int w, int h, int stride)
{
    int error, offset;
    unsigned long long sum = 0;
    int ww, hh;
    /* cmp是上下翻转的 */
    cmp += (h - 1) * stride;
    for (hh = 0; hh < h; ++hh) {
        offset = 0;
        for (ww = 0; ww < w; ++ww) {
            /* R */
            error = ref[offset] - cmp[offset];
            sum += error * error;
            /* G */
            error = ref[offset + 1] - cmp[offset + 1];
            sum += error * error;
            /* B */
            error = ref[offset + 2] - cmp[offset + 2];
            sum += error * error;

            offset += 4;
        }
        ref += stride;
        cmp -= stride;
    }
    return (float)((double)sum / (double)(w * h * 3));
}
float RGBA_psnr(const unsigned char* ref, const unsigned char* cmp, int w, int h, int stride)
{
    const int L_sqd = 255 * 255;
    float mse = RGBA_mse(ref, cmp, w, h, stride);
    if (mse == 0.00) {
        return 100.00;        /* 两份相同数据的psnr设置为100 */
    }
    return (float)(10.0 * log10(L_sqd / mse));
}
