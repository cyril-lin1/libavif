/*
 * Copyright(C) 2019 Ruijie Network. All rights reserved.
 * bmp.h
 * Original Author: xu_lin@ruijie.com.cn
 * 读取保存BMP文件
 * History:
 *
 */
#ifndef _BMP_H_
#define _BMP_H_

#include <stdint.h>
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <windows.h>
#else
#pragma pack(push)
#pragma pack(2)
 //下面两个结构是位图的结构
typedef struct BITMAPFILEHEADER
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
}BITMAPFILEHEADER;

typedef struct BITMAPINFOHEADER
{
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
}BITMAPINFOHEADER;
#pragma pack(pop)
#endif

/**
 * 功能描述：bmp数据的获取
 * 输入：    @img_path   输入图片的地址
 *        @data       bmp数据要存放的内存地址
 *        @bmp_info   bmp头信息
 * 输出： 返回0表示成功，否则失败
 */
int bmp_get(const char *img_path, uint8_t **data, BITMAPINFOHEADER *bmp_info);
/**
 * 功能描述：保存bmp图像数据
 * 输入：    @img_path   输出图片的地址
 *        @data       bmp数据
 *        @height     图片的高
 *        @width      图片的宽
 *        @byte_count 每个像素占多少字节
 * 输出： 返回0表示成功，否则失败
 */
void bmp_save(const char *img_path, uint8_t *data, uint32_t height, uint32_t width, uint32_t byte_count);
/* 把bmp24转成bmp32 */
int bmp_24to32(uint8_t **data, BITMAPINFOHEADER* bmp_info);


#endif /* _BMP_H_ */