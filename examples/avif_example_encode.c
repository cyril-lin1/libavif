// Copyright 2020 Joe Drago. All rights reserved.
// SPDX-License-Identifier: BSD-2-Clause

#include "avif/avif.h"
#include "psnr.h"
#include "bmp.h"
#include "time-measure.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char * argv[])
{
    if (argc != 6) {
        fprintf(stderr, "avif_example_encode [encodeYUVDirectly:0/1] [output.avif] [input.bmp] [qp:0~63] [output.bmp]\n");
        return 1;
    }
    avifBool encodeYUVDirectly = AVIF_FALSE;
    if (argv[1][0] == '1') {
        encodeYUVDirectly = AVIF_TRUE;
    }
    const char * outputFilename = argv[2];
	const char * outputFilename_bmp = argv[5];

    int returnCode = 1;
    avifEncoder * encoder = NULL;
    avifRWData avifOutput = AVIF_DATA_EMPTY;
    avifRGBImage rgb;
    memset(&rgb, 0, sizeof(rgb));

    /* 读bmp文件 */
    uint8_t *data = NULL;
    BITMAPINFOHEADER bmp_info; 
    returnCode = bmp_get(argv[3], &data, &bmp_info);
    if (returnCode) {
        fprintf(stderr, "bmp_get err\n");
        return 1;
    }
    if (bmp_info.biBitCount == 24) {
        bmp_24to32(&data, &bmp_info);
        //bmp_save("E:/tmp/8.1/out.bmp", data, bmp_info.biHeight, bmp_info.biWidth, 4);
    }

	int qp = atoi(argv[4]);

    avifImage * image = avifImageCreate(bmp_info.biWidth, bmp_info.biHeight, 8, AVIF_PIXEL_FORMAT_YUV444); // these values dictate what goes into the final AVIF
    // Configure image here: (see avif/avif.h)
    // * colorPrimaries
    // * transferCharacteristics
    // * matrixCoefficients
    // * avifImageSetProfileICC()
    // * avifImageSetMetadataExif()
    // * avifImageSetMetadataXMP()
    // * yuvRange
    // * alphaRange
    // * alphaPremultiplied
    // * transforms (transformFlags, pasp, clap, irot, imir)

    if (encodeYUVDirectly) {
        // If you have YUV(A) data you want to encode, use this path
        printf("Encoding raw YUVA data\n");

        avifImageAllocatePlanes(image, AVIF_PLANES_YUV | AVIF_PLANES_A);

        // Fill your YUV(A) data here
        memset(image->yuvPlanes[AVIF_CHAN_Y], 255, image->yuvRowBytes[AVIF_CHAN_Y] * image->height);
        memset(image->yuvPlanes[AVIF_CHAN_U], 128, image->yuvRowBytes[AVIF_CHAN_U] * image->height);
        memset(image->yuvPlanes[AVIF_CHAN_V], 128, image->yuvRowBytes[AVIF_CHAN_V] * image->height);
        memset(image->alphaPlane, 255, image->alphaRowBytes * image->height);
    } else {
        // If you have RGB(A) data you want to encode, use this path
        printf("Encoding from converted RGBA\n");

        avifRGBImageSetDefaults(&rgb, image);
        // Override RGB(A)->YUV(A) defaults here: depth, format, chromaUpsampling, ignoreAlpha, alphaPremultiplied, libYUVUsage, etc

        // Alternative: set rgb.pixels and rgb.rowBytes yourself, which should match your chosen rgb.format
        // Be sure to use uint16_t* instead of uint8_t* for rgb.pixels/rgb.rowBytes if (rgb.depth > 8)
        avifRGBImageAllocatePixels(&rgb);

#if 0
        // Fill your RGB(A) data here
        memset(rgb.pixels, 255, rgb.rowBytes * image->height);
#else
        memcpy(rgb.pixels, data, rgb.rowBytes * image->height);
#endif

        avifResult convertResult = avifImageRGBToYUV(image, &rgb);
        if (convertResult != AVIF_RESULT_OK) {
            fprintf(stderr, "Failed to convert to YUV(A): %s\n", avifResultToString(convertResult));
            goto cleanup;
        }
    }

    encoder = avifEncoderCreate();
    // Configure your encoder here (see avif/avif.h):
    // * maxThreads
    // * minQuantizer
    // * maxQuantizer
    // * minQuantizerAlpha
    // * maxQuantizerAlpha
    // * tileRowsLog2
    // * tileColsLog2
    // * speed
    // * keyframeInterval
    // * timescale
    encoder->maxThreads = 4;
	encoder->minQuantizer = qp;
    encoder->maxQuantizer = qp;
    encoder->minQuantizerAlpha = qp;
    encoder->maxQuantizerAlpha = qp;
    encoder->speed = AVIF_SPEED_FASTEST;

	TM_CTX tm_ctx;
    time_measure_general_start(&tm_ctx); 

    // Call avifEncoderAddImage() for each image in your sequence
    // Only set AVIF_ADD_IMAGE_FLAG_SINGLE if you're not encoding a sequence
    // Use avifEncoderAddImageGrid() instead with an array of avifImage* to make a grid image
    avifResult addImageResult = avifEncoderAddImage(encoder, image, 1, AVIF_ADD_IMAGE_FLAG_SINGLE);
    if (addImageResult != AVIF_RESULT_OK) {
        fprintf(stderr, "Failed to add image to encoder: %s\n", avifResultToString(addImageResult));
        goto cleanup;
    }

    avifResult finishResult = avifEncoderFinish(encoder, &avifOutput);
    if (finishResult != AVIF_RESULT_OK) {
        fprintf(stderr, "Failed to finish encode: %s\n", avifResultToString(finishResult));
        goto cleanup;
    }

	time_measure_general_end(&tm_ctx, 0, "encoder");

    printf("Encode success: %zu kb\n", avifOutput.size * 8 / 1000);

    FILE * f = fopen(outputFilename, "wb");
    size_t bytesWritten = fwrite(avifOutput.data, 1, avifOutput.size, f);
    fclose(f);
    if (bytesWritten != avifOutput.size) {
        fprintf(stderr, "Failed to write %zu bytes\n", avifOutput.size);
        goto cleanup;
    }
    printf("Wrote: %s\n", outputFilename);

	/* 重置rgb */
	avifRGBImageFreePixels(&rgb);
	memset(&rgb, 0, sizeof(rgb));

	/* 解码 */
	avifDecoder * decoder = avifDecoderCreate();
    // Override decoder defaults here (codecChoice, requestedSource, ignoreExif, ignoreXMP, etc)

    avifResult result = avifDecoderSetIOFile(decoder, outputFilename);
    if (result != AVIF_RESULT_OK) {
        fprintf(stderr, "Cannot open file for read: %s\n", outputFilename);
        goto cleanup;
    }

    result = avifDecoderParse(decoder);
    if (result != AVIF_RESULT_OK) {
        fprintf(stderr, "Failed to decode image: %s\n", avifResultToString(result));
        goto cleanup;
    }

    // Now available:
    // * All decoder->image information other than pixel data:
    //   * width, height, depth
    //   * transformations (pasp, clap, irot, imir)
    //   * color profile (icc, CICP)
    //   * metadata (Exif, XMP)
    // * decoder->alphaPresent
    // * number of total images in the AVIF (decoder->imageCount)
    // * overall image sequence timing (including per-frame timing with avifDecoderNthImageTiming())

    printf("Parsed AVIF: %ux%u (%ubpc)\n", decoder->image->width, decoder->image->height, decoder->image->depth);

	time_measure_general_start(&tm_ctx);
    while (avifDecoderNextImage(decoder) == AVIF_RESULT_OK) {
        // Now available (for this frame):
        // * All decoder->image YUV pixel data (yuvFormat, yuvPlanes, yuvRange, yuvChromaSamplePosition, yuvRowBytes)
        // * decoder->image alpha data (alphaRange, alphaPlane, alphaRowBytes)
        // * this frame's sequence timing

        avifRGBImageSetDefaults(&rgb, decoder->image);
        // Override YUV(A)->RGB(A) defaults here: depth, format, chromaUpsampling, ignoreAlpha, alphaPremultiplied, libYUVUsage, etc

        // Alternative: set rgb.pixels and rgb.rowBytes yourself, which should match your chosen rgb.format
        // Be sure to use uint16_t* instead of uint8_t* for rgb.pixels/rgb.rowBytes if (rgb.depth > 8)
        avifRGBImageAllocatePixels(&rgb);

        if (avifImageYUVToRGB(decoder->image, &rgb) != AVIF_RESULT_OK) {
            fprintf(stderr, "Conversion from YUV failed: %s\n", outputFilename);
            goto cleanup;
        }
		time_measure_general_end(&tm_ctx, 0, "decoder");

        // Now available:
        // * RGB(A) pixel data (rgb.pixels, rgb.rowBytes)

        if (rgb.depth > 8) {
            uint16_t * firstPixel = (uint16_t *)rgb.pixels;
            printf(" * First pixel: RGBA(%u,%u,%u,%u)\n", firstPixel[0], firstPixel[1], firstPixel[2], firstPixel[3]);
        } else {
            uint8_t * firstPixel = rgb.pixels;
            printf(" * First pixel: RGBA(%u,%u,%u,%u)\n", firstPixel[0], firstPixel[1], firstPixel[2], firstPixel[3]);

			/* 保存bmp */
			bmp_save(outputFilename_bmp, firstPixel, decoder->image->height, decoder->image->width, 4);
			printf("Wrote: %s\n", outputFilename_bmp);

			/* psnr */
			float psnr = iqa_psnr(firstPixel, data, decoder->image->width, decoder->image->height, decoder->image->width * 4);
			printf("psnr = %.2f\n", psnr);
        }
		
    }

    returnCode = 0;
cleanup:
	if (data) {
		free(data);
	}
    if (image) {
        avifImageDestroy(image);
    }
    if (encoder) {
        avifEncoderDestroy(encoder);
    }
    avifRWDataFree(&avifOutput);
    avifRGBImageFreePixels(&rgb); // Only use in conjunction with avifRGBImageAllocatePixels()
    return returnCode;
}
