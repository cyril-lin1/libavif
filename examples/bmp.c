/*
 * Copyright(C) 2019 Ruijie Network. All rights reserved.
 * bmp.h
 * Original Author: xu_lin@ruijie.com.cn
 * 读取保存BMP文件
 * History:
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

/**
 * 功能描述：bmp数据的获取
 * 输入：    @img_path   输入图片的地址
 *        @data       bmp数据要存放的内存地址
 *        @bmp_info   bmp头信息
 * 输出： 返回0表示成功，否则失败
 */
int bmp_get(const char *img_path, uint8_t **data, BITMAPINFOHEADER *bmp_info)
{
    BITMAPFILEHEADER bfh;
    //BITMAPINFOHEADER bih;
    FILE *fp;
    int size;
    fp = fopen(img_path, "rb");
    if(fp == NULL){
        fprintf(stderr, "Open file %s failed!\n", img_path);
        return -1;
    }
    fread( &bfh, 1, sizeof(BITMAPFILEHEADER), fp );
    fread( bmp_info, 1, sizeof(BITMAPINFOHEADER), fp );
    if (bmp_info->biSizeImage == 0) {
        bmp_info->biSizeImage = bfh.bfSize - 54;
    }
    size = bmp_info->biSizeImage;
    if( data != NULL ){
        *data = (uint8_t *)malloc(size);
        fread(*data, 1, size, fp );
    }
    fclose( fp );
    return 0;
}

/**
 * 功能描述：保存bmp图像数据
 * 输入：    @img_path   输出图片的地址
 *        @data       bmp数据
 *        @height     图片的高
 *        @width      图片的宽
 *        @byte_count 每个像素占多少字节
 * 输出： 返回0表示成功，否则失败
 */
void bmp_save(const char *img_path, uint8_t *data, uint32_t height, uint32_t width, uint32_t byte_count)
{
    int size = width*height*byte_count;

    BITMAPFILEHEADER bfh;
    // 位图第一部分，文件信息
    bfh.bfType = (uint16_t)0x4d42;  //bm
    bfh.bfSize = size  // data size
        + sizeof(BITMAPFILEHEADER) // first section size
        + sizeof(BITMAPINFOHEADER) // second section size
        ;
    bfh.bfReserved1 = 0; // reserved
    bfh.bfReserved2 = 0; // reserved
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//真正的数据的位置

    // 位图第二部分，数据信息
    BITMAPINFOHEADER bih;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;      //BMP图片从最后一个点开始扫描，显示时图片是倒着的，所以用-height，这样图片就正了
    bih.biPlanes = 1;
    bih.biBitCount = byte_count*8;
    bih.biCompression = 0;      //不压缩
    bih.biSizeImage = size;
    bih.biXPelsPerMeter = 2835;//像素每米
    bih.biYPelsPerMeter = 2835;
    bih.biClrUsed = 0;//已用过的颜色，24位的为0
    bih.biClrImportant = 0;//每个像素都重要
    FILE * fp = fopen(img_path, "wb");
    if (!fp) return;

    //由于linux上4字节对齐，而信息头大小为54字节，第一部分14字节，第二部分40字节，所以会将第一部分补齐为16自己，直接用sizeof，打开图片时就会遇到premature end-of-file encountered错误
    fwrite(&bfh, 8, 1, fp);
    fwrite(&bfh.bfReserved2, sizeof(bfh.bfReserved2), 1, fp);
    fwrite(&bfh.bfOffBits, sizeof(bfh.bfOffBits), 1, fp);
    fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, fp);
    fwrite(data, size, 1, fp);
    fclose(fp);
}

/* 把bmp24转成bmp32 */
int bmp_24to32(uint8_t **data, BITMAPINFOHEADER* bmp_info)
{
    if (bmp_info->biBitCount == 32) {
        return 0;
    }
    if (bmp_info->biBitCount != 24 || data == NULL || *data == NULL) {
        return -1;
    }

    uint32_t height = bmp_info->biHeight;
    uint32_t width = bmp_info->biWidth;
    uint32_t src_line = width * 3;
    uint32_t dst_line = width * 4;
    uint32_t size = height * dst_line;
    uint8_t* src = *data;
    uint8_t* dst = (uint8_t*)malloc(size);
    memset(dst, 0, size);

    for (uint32_t y = 0;y < height;++y) {
        for (uint32_t x = 0;x < width;++x) {
            memcpy(dst + y * dst_line + x * 4, src + y * src_line + x * 3, 3);
        }
    }

    free(src);
    *data = dst;
    bmp_info->biSizeImage = size;
    bmp_info->biBitCount = 32;
    return 0;
}



