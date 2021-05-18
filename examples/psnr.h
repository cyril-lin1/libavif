/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 * psnr.h
 * Original Author: xu_lin@ruijie.com.cn
 * 计算PSNR
 * History:
 *
 */
#ifndef PSNR_H
#define PSNR_H

#if defined (__cplusplus)
extern "C" {
#endif

float iqa_mse(const unsigned char *ref, const unsigned char *cmp, int w, int h, int stride);
float iqa_psnr(const unsigned char *ref, const unsigned char *cmp, int w, int h, int stride);
float RGBA_psnr(const unsigned char* ref, const unsigned char* cmp, int w, int h, int stride);

#if defined (__cplusplus)
}
#endif

#endif  /* PSNR_H */