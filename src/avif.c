// Copyright 2019 Joe Drago. All rights reserved.
// SPDX-License-Identifier: BSD-2-Clause

#include "avif/internal.h"

#include <string.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define AVIF_VERSION_STRING (STR(AVIF_VERSION_MAJOR) "." STR(AVIF_VERSION_MINOR) "." STR(AVIF_VERSION_PATCH))

const char * avifVersion(void)
{
    return AVIF_VERSION_STRING;
}

const char * avifPixelFormatToString(avifPixelFormat format)
{
    switch (format) {
        case AVIF_PIXEL_FORMAT_YUV444:
            return "YUV444";
        case AVIF_PIXEL_FORMAT_YUV420:
            return "YUV420";
        case AVIF_PIXEL_FORMAT_YUV422:
            return "YUV422";
        case AVIF_PIXEL_FORMAT_YUV400:
            return "YUV400";
        case AVIF_PIXEL_FORMAT_NONE:
        default:
            break;
    }
    return "Unknown";
}

void avifGetPixelFormatInfo(avifPixelFormat format, avifPixelFormatInfo * info)
{
    memset(info, 0, sizeof(avifPixelFormatInfo));

    switch (format) {
        case AVIF_PIXEL_FORMAT_YUV444:
            info->chromaShiftX = 0;
            info->chromaShiftY = 0;
            break;

        case AVIF_PIXEL_FORMAT_YUV422:
            info->chromaShiftX = 1;
            info->chromaShiftY = 0;
            break;

        case AVIF_PIXEL_FORMAT_YUV420:
            info->chromaShiftX = 1;
            info->chromaShiftY = 1;
            break;

        case AVIF_PIXEL_FORMAT_YUV400:
            info->chromaShiftX = 1;
            info->chromaShiftY = 1;
            info->monochrome = AVIF_TRUE;
            break;

        case AVIF_PIXEL_FORMAT_NONE:
        default:
            break;
    }
}

const char * avifResultToString(avifResult result)
{
    // clang-format off
    switch (result) {
        case AVIF_RESULT_OK:                            return "OK";
        case AVIF_RESULT_INVALID_FTYP:                  return "Invalid ftyp";
        case AVIF_RESULT_NO_CONTENT:                    return "No content";
        case AVIF_RESULT_NO_YUV_FORMAT_SELECTED:        return "No YUV format selected";
        case AVIF_RESULT_REFORMAT_FAILED:               return "Reformat failed";
        case AVIF_RESULT_UNSUPPORTED_DEPTH:             return "Unsupported depth";
        case AVIF_RESULT_ENCODE_COLOR_FAILED:           return "Encoding of color planes failed";
        case AVIF_RESULT_ENCODE_ALPHA_FAILED:           return "Encoding of alpha plane failed";
        case AVIF_RESULT_BMFF_PARSE_FAILED:             return "BMFF parsing failed";
        case AVIF_RESULT_NO_AV1_ITEMS_FOUND:            return "No AV1 items found";
        case AVIF_RESULT_DECODE_COLOR_FAILED:           return "Decoding of color planes failed";
        case AVIF_RESULT_DECODE_ALPHA_FAILED:           return "Decoding of alpha plane failed";
        case AVIF_RESULT_COLOR_ALPHA_SIZE_MISMATCH:     return "Color and alpha planes size mismatch";
        case AVIF_RESULT_ISPE_SIZE_MISMATCH:            return "Plane sizes don't match ispe values";
        case AVIF_RESULT_NO_CODEC_AVAILABLE:            return "No codec available";
        case AVIF_RESULT_NO_IMAGES_REMAINING:           return "No images remaining";
        case AVIF_RESULT_INVALID_EXIF_PAYLOAD:          return "Invalid Exif payload";
        case AVIF_RESULT_INVALID_IMAGE_GRID:            return "Invalid image grid";
        case AVIF_RESULT_INVALID_CODEC_SPECIFIC_OPTION: return "Invalid codec-specific option";
        case AVIF_RESULT_TRUNCATED_DATA:                return "Truncated data";
        case AVIF_RESULT_IO_NOT_SET:                    return "IO not set";
        case AVIF_RESULT_IO_ERROR:                      return "IO Error";
        case AVIF_RESULT_WAITING_ON_IO:                 return "Waiting on IO";
        case AVIF_RESULT_INVALID_ARGUMENT:              return "Invalid argument";
        case AVIF_RESULT_NOT_IMPLEMENTED:               return "Not implemented";
        case AVIF_RESULT_UNKNOWN_ERROR:
        default:
            break;
    }
    // clang-format on
    return "Unknown Error";
}

// This function assumes nothing in this struct needs to be freed; use avifImageClear() externally
static void avifImageSetDefaults(avifImage * image)
{
    memset(image, 0, sizeof(avifImage));
    image->yuvRange = AVIF_RANGE_FULL;
    image->alphaRange = AVIF_RANGE_FULL;
    image->colorPrimaries = AVIF_COLOR_PRIMARIES_UNSPECIFIED;
    image->transferCharacteristics = AVIF_TRANSFER_CHARACTERISTICS_UNSPECIFIED;
    image->matrixCoefficients = AVIF_MATRIX_COEFFICIENTS_UNSPECIFIED;
}

avifImage * avifImageCreate(int width, int height, int depth, avifPixelFormat yuvFormat)
{
    avifImage * image = (avifImage *)avifAlloc(sizeof(avifImage));
    avifImageSetDefaults(image);
    image->width = width;
    image->height = height;
    image->depth = depth;
    image->yuvFormat = yuvFormat;
    return image;
}

avifImage * avifImageCreateEmpty(void)
{
    return avifImageCreate(0, 0, 0, AVIF_PIXEL_FORMAT_NONE);
}

void avifImageCopy(avifImage * dstImage, const avifImage * srcImage, avifPlanesFlags planes)
{
    avifImageFreePlanes(dstImage, AVIF_PLANES_ALL);

    dstImage->width = srcImage->width;
    dstImage->height = srcImage->height;
    dstImage->depth = srcImage->depth;
    dstImage->yuvFormat = srcImage->yuvFormat;
    dstImage->yuvRange = srcImage->yuvRange;
    dstImage->yuvChromaSamplePosition = srcImage->yuvChromaSamplePosition;
    dstImage->alphaRange = srcImage->alphaRange;
    dstImage->alphaPremultiplied = srcImage->alphaPremultiplied;

    dstImage->colorPrimaries = srcImage->colorPrimaries;
    dstImage->transferCharacteristics = srcImage->transferCharacteristics;
    dstImage->matrixCoefficients = srcImage->matrixCoefficients;

    dstImage->transformFlags = srcImage->transformFlags;
    memcpy(&dstImage->pasp, &srcImage->pasp, sizeof(dstImage->pasp));
    memcpy(&dstImage->clap, &srcImage->clap, sizeof(dstImage->clap));
    memcpy(&dstImage->irot, &srcImage->irot, sizeof(dstImage->irot));
    memcpy(&dstImage->imir, &srcImage->imir, sizeof(dstImage->imir));

    avifImageSetProfileICC(dstImage, srcImage->icc.data, srcImage->icc.size);

    avifImageSetMetadataExif(dstImage, srcImage->exif.data, srcImage->exif.size);
    avifImageSetMetadataXMP(dstImage, srcImage->xmp.data, srcImage->xmp.size);

    if ((planes & AVIF_PLANES_YUV) && srcImage->yuvPlanes[AVIF_CHAN_Y]) {
        avifImageAllocatePlanes(dstImage, AVIF_PLANES_YUV);

        avifPixelFormatInfo formatInfo;
        avifGetPixelFormatInfo(srcImage->yuvFormat, &formatInfo);
        uint32_t uvHeight = (dstImage->height + formatInfo.chromaShiftY) >> formatInfo.chromaShiftY;
        for (int yuvPlane = 0; yuvPlane < 3; ++yuvPlane) {
            uint32_t planeHeight = (yuvPlane == AVIF_CHAN_Y) ? dstImage->height : uvHeight;

            if (!srcImage->yuvRowBytes[yuvPlane]) {
                // plane is absent. If we're copying from a source without
                // them, mimic the source image's state by removing our copy.
                avifFree(dstImage->yuvPlanes[yuvPlane]);
                dstImage->yuvPlanes[yuvPlane] = NULL;
                dstImage->yuvRowBytes[yuvPlane] = 0;
                continue;
            }

            for (uint32_t j = 0; j < planeHeight; ++j) {
                uint8_t * srcRow = &srcImage->yuvPlanes[yuvPlane][j * srcImage->yuvRowBytes[yuvPlane]];
                uint8_t * dstRow = &dstImage->yuvPlanes[yuvPlane][j * dstImage->yuvRowBytes[yuvPlane]];
                memcpy(dstRow, srcRow, dstImage->yuvRowBytes[yuvPlane]);
            }
        }
    }

    if ((planes & AVIF_PLANES_A) && srcImage->alphaPlane) {
        avifImageAllocatePlanes(dstImage, AVIF_PLANES_A);
        for (uint32_t j = 0; j < dstImage->height; ++j) {
            uint8_t * srcAlphaRow = &srcImage->alphaPlane[j * srcImage->alphaRowBytes];
            uint8_t * dstAlphaRow = &dstImage->alphaPlane[j * dstImage->alphaRowBytes];
            memcpy(dstAlphaRow, srcAlphaRow, dstImage->alphaRowBytes);
        }
    }
}

void avifImageDestroy(avifImage * image)
{
    avifImageFreePlanes(image, AVIF_PLANES_ALL);
    avifRWDataFree(&image->icc);
    avifRWDataFree(&image->exif);
    avifRWDataFree(&image->xmp);
    avifFree(image);
}

void avifImageSetProfileICC(avifImage * image, const uint8_t * icc, size_t iccSize)
{
    avifRWDataSet(&image->icc, icc, iccSize);
}

void avifImageSetMetadataExif(avifImage * image, const uint8_t * exif, size_t exifSize)
{
    avifRWDataSet(&image->exif, exif, exifSize);
}

void avifImageSetMetadataXMP(avifImage * image, const uint8_t * xmp, size_t xmpSize)
{
    avifRWDataSet(&image->xmp, xmp, xmpSize);
}

void avifImageAllocatePlanes(avifImage * image, avifPlanesFlags planes)
{
    int channelSize = avifImageUsesU16(image) ? 2 : 1;
    int fullRowBytes = channelSize * image->width;
    int fullSize = fullRowBytes * image->height;
    if ((planes & AVIF_PLANES_YUV) && (image->yuvFormat != AVIF_PIXEL_FORMAT_NONE)) {
        avifPixelFormatInfo info;
        avifGetPixelFormatInfo(image->yuvFormat, &info);

        int shiftedW = (image->width + info.chromaShiftX) >> info.chromaShiftX;
        int shiftedH = (image->height + info.chromaShiftY) >> info.chromaShiftY;

        int uvRowBytes = channelSize * shiftedW;
        int uvSize = uvRowBytes * shiftedH;

        if (!image->yuvPlanes[AVIF_CHAN_Y]) {
            image->yuvRowBytes[AVIF_CHAN_Y] = fullRowBytes;
            image->yuvPlanes[AVIF_CHAN_Y] = avifAlloc(fullSize);
        }

        if (image->yuvFormat != AVIF_PIXEL_FORMAT_YUV400) {
            if (!image->yuvPlanes[AVIF_CHAN_U]) {
                image->yuvRowBytes[AVIF_CHAN_U] = uvRowBytes;
                image->yuvPlanes[AVIF_CHAN_U] = avifAlloc(uvSize);
            }
            if (!image->yuvPlanes[AVIF_CHAN_V]) {
                image->yuvRowBytes[AVIF_CHAN_V] = uvRowBytes;
                image->yuvPlanes[AVIF_CHAN_V] = avifAlloc(uvSize);
            }
        }
        image->imageOwnsYUVPlanes = AVIF_TRUE;
    }
    if (planes & AVIF_PLANES_A) {
        if (!image->alphaPlane) {
            image->alphaRowBytes = fullRowBytes;
            image->alphaPlane = avifAlloc(fullSize);
        }
        image->imageOwnsAlphaPlane = AVIF_TRUE;
    }
}

void avifImageFreePlanes(avifImage * image, avifPlanesFlags planes)
{
    if ((planes & AVIF_PLANES_YUV) && (image->yuvFormat != AVIF_PIXEL_FORMAT_NONE)) {
        if (image->imageOwnsYUVPlanes) {
            avifFree(image->yuvPlanes[AVIF_CHAN_Y]);
            avifFree(image->yuvPlanes[AVIF_CHAN_U]);
            avifFree(image->yuvPlanes[AVIF_CHAN_V]);
        }
        image->yuvPlanes[AVIF_CHAN_Y] = NULL;
        image->yuvRowBytes[AVIF_CHAN_Y] = 0;
        image->yuvPlanes[AVIF_CHAN_U] = NULL;
        image->yuvRowBytes[AVIF_CHAN_U] = 0;
        image->yuvPlanes[AVIF_CHAN_V] = NULL;
        image->yuvRowBytes[AVIF_CHAN_V] = 0;
        image->imageOwnsYUVPlanes = AVIF_FALSE;
    }
    if (planes & AVIF_PLANES_A) {
        if (image->imageOwnsAlphaPlane) {
            avifFree(image->alphaPlane);
        }
        image->alphaPlane = NULL;
        image->alphaRowBytes = 0;
        image->imageOwnsAlphaPlane = AVIF_FALSE;
    }
}

void avifImageStealPlanes(avifImage * dstImage, avifImage * srcImage, avifPlanesFlags planes)
{
    avifImageFreePlanes(dstImage, planes);

    if (planes & AVIF_PLANES_YUV) {
        dstImage->yuvPlanes[AVIF_CHAN_Y] = srcImage->yuvPlanes[AVIF_CHAN_Y];
        dstImage->yuvRowBytes[AVIF_CHAN_Y] = srcImage->yuvRowBytes[AVIF_CHAN_Y];
        dstImage->yuvPlanes[AVIF_CHAN_U] = srcImage->yuvPlanes[AVIF_CHAN_U];
        dstImage->yuvRowBytes[AVIF_CHAN_U] = srcImage->yuvRowBytes[AVIF_CHAN_U];
        dstImage->yuvPlanes[AVIF_CHAN_V] = srcImage->yuvPlanes[AVIF_CHAN_V];
        dstImage->yuvRowBytes[AVIF_CHAN_V] = srcImage->yuvRowBytes[AVIF_CHAN_V];

        srcImage->yuvPlanes[AVIF_CHAN_Y] = NULL;
        srcImage->yuvRowBytes[AVIF_CHAN_Y] = 0;
        srcImage->yuvPlanes[AVIF_CHAN_U] = NULL;
        srcImage->yuvRowBytes[AVIF_CHAN_U] = 0;
        srcImage->yuvPlanes[AVIF_CHAN_V] = NULL;
        srcImage->yuvRowBytes[AVIF_CHAN_V] = 0;

        dstImage->yuvFormat = srcImage->yuvFormat;
        dstImage->imageOwnsYUVPlanes = srcImage->imageOwnsYUVPlanes;
        srcImage->imageOwnsYUVPlanes = AVIF_FALSE;
    }
    if (planes & AVIF_PLANES_A) {
        dstImage->alphaPlane = srcImage->alphaPlane;
        dstImage->alphaRowBytes = srcImage->alphaRowBytes;

        srcImage->alphaPlane = NULL;
        srcImage->alphaRowBytes = 0;

        dstImage->imageOwnsAlphaPlane = srcImage->imageOwnsAlphaPlane;
        srcImage->imageOwnsAlphaPlane = AVIF_FALSE;
    }
}

avifBool avifImageUsesU16(const avifImage * image)
{
    return (image->depth > 8);
}

// avifCodecCreate*() functions are in their respective codec_*.c files

void avifCodecDestroy(avifCodec * codec)
{
    if (codec && codec->destroyInternal) {
        codec->destroyInternal(codec);
    }
    avifFree(codec);
}

// ---------------------------------------------------------------------------
// avifRGBImage

avifBool avifRGBFormatHasAlpha(avifRGBFormat format)
{
    return (format != AVIF_RGB_FORMAT_RGB) && (format != AVIF_RGB_FORMAT_BGR);
}

uint32_t avifRGBFormatChannelCount(avifRGBFormat format)
{
    return avifRGBFormatHasAlpha(format) ? 4 : 3;
}

uint32_t avifRGBImagePixelSize(const avifRGBImage * rgb)
{
    return avifRGBFormatChannelCount(rgb->format) * ((rgb->depth > 8) ? 2 : 1);
}

void avifRGBImageSetDefaults(avifRGBImage * rgb, const avifImage * image)
{
    rgb->width = image->width;
    rgb->height = image->height;
    rgb->depth = image->depth;
    rgb->format = AVIF_RGB_FORMAT_RGBA;
    rgb->chromaUpsampling = AVIF_CHROMA_UPSAMPLING_AUTOMATIC;
    rgb->ignoreAlpha = AVIF_FALSE;
    rgb->pixels = NULL;
    rgb->rowBytes = 0;
    rgb->alphaPremultiplied = AVIF_FALSE; // Most expect RGBA output to *not* be premultiplied. Those that do can opt-in by
                                          // setting this to match image->alphaPremultiplied or forcing this to true
                                          // after calling avifRGBImageSetDefaults(),
}

void avifRGBImageAllocatePixels(avifRGBImage * rgb)
{
    if (rgb->pixels) {
        avifFree(rgb->pixels);
    }

    rgb->rowBytes = rgb->width * avifRGBImagePixelSize(rgb);
    rgb->pixels = avifAlloc((size_t)rgb->rowBytes * rgb->height);
}

void avifRGBImageFreePixels(avifRGBImage * rgb)
{
    if (rgb->pixels) {
        avifFree(rgb->pixels);
    }

    rgb->pixels = NULL;
    rgb->rowBytes = 0;
}

// ---------------------------------------------------------------------------
// avifCropRect

typedef struct clapFraction
{
    int32_t n;
    int32_t d;
} clapFraction;

static clapFraction calcCenter(int32_t dim)
{
    clapFraction f;
    f.n = dim >> 1;
    f.d = 1;
    if ((dim % 2) == 1) {
        f.n = (f.n * 2) + 1;
        f.d = 2;
    }
    return f;
}

static int32_t calcGCD(int32_t a, int32_t b)
{
    if (a < 0) {
        a *= -1;
    }
    if (b < 0) {
        b *= -1;
    }
    while (a > 0) {
        if (a < b) {
            int32_t t = a;
            a = b;
            b = t;
        }
        a = a - b;
    }
    return b;
}

static void clapFractionSimplify(clapFraction * f)
{
    int32_t gcd = calcGCD(f->n, f->d);
    if (gcd > 1) {
        f->n /= gcd;
        f->d /= gcd;
    }
}

// Make the fractions have a common denominator
static void clapFractionCD(clapFraction * a, clapFraction * b)
{
    clapFractionSimplify(a);
    clapFractionSimplify(b);
    if ((a->d != b->d)) {
        const int32_t ad = a->d;
        const int32_t bd = b->d;
        a->n = a->n * bd;
        a->d *= bd;
        b->n = b->n * ad;
        b->d *= ad;
    }
}

static clapFraction clapFractionAdd(clapFraction a, clapFraction b)
{
    clapFractionCD(&a, &b);

    clapFraction result;
    result.n = a.n + b.n;
    result.d = a.d;

    clapFractionSimplify(&result);
    return result;
}

static clapFraction clapFractionSub(clapFraction a, clapFraction b)
{
    clapFractionCD(&a, &b);

    clapFraction result;
    result.n = a.n - b.n;
    result.d = a.d;

    clapFractionSimplify(&result);
    return result;
}

static avifBool avifCropRectIsValid(const avifCropRect * cropRect,
                                    const uint32_t imageW,
                                    const uint32_t imageH,
                                    const avifPixelFormat yuvFormat,
                                    avifDiagnostics * diag)

{
    // ISO/IEC 23000-22:2019/DAM 2:2021, Section 7.3.6.7:
    //   The clean aperture property is restricted according to the chroma
    //   sampling format of the input image (4:4:4, 4:2:2:, 4:2:0, or 4:0:0) as
    //   follows:
    //   - when the image is 4:0:0 (monochrome) or 4:4:4, the horizontal and
    //     vertical cropped offsets and widths shall be integers;
    //   - when the image is 4:2:2 the horizontal cropped offset and width
    //     shall be even numbers and the vertical values shall be integers;
    //   - when the image is 4:2:0 both the horizontal and vertical cropped
    //     offsets and widths shall be even numbers.

    if ((cropRect->width == 0) || (cropRect->height == 0)) {
        avifDiagnosticsPrintf(diag, "[Strict] crop rect width and height must be nonzero");
        return AVIF_FALSE;
    }
    if (((cropRect->x + cropRect->width) > imageW) || ((cropRect->y + cropRect->height) > imageH)) {
        avifDiagnosticsPrintf(diag, "[Strict] crop rect is out of the image's bounds");
        return AVIF_FALSE;
    }

    if ((yuvFormat == AVIF_PIXEL_FORMAT_YUV420) || (yuvFormat == AVIF_PIXEL_FORMAT_YUV422)) {
        if (((cropRect->x % 2) != 0) || ((cropRect->width % 2) != 0)) {
            avifDiagnosticsPrintf(diag, "[Strict] crop rect X offset and width must both be even due to this image's YUV subsampling");
            return AVIF_FALSE;
        }
    }
    if (yuvFormat == AVIF_PIXEL_FORMAT_YUV420) {
        if (((cropRect->y % 2) != 0) || ((cropRect->height % 2) != 0)) {
            avifDiagnosticsPrintf(diag, "[Strict] crop rect Y offset and height must both be even due to this image's YUV subsampling");
            return AVIF_FALSE;
        }
    }
    return AVIF_TRUE;
}

avifBool avifCropRectConvertCleanApertureBox(avifCropRect * cropRect,
                                             const avifCleanApertureBox * clap,
                                             const uint32_t imageW,
                                             const uint32_t imageH,
                                             const avifPixelFormat yuvFormat,
                                             avifDiagnostics * diag)
{
    // ISO/IEC 14496-12:2020, Section 12.1.4.1:
    //   For horizOff and vertOff, D shall be strictly positive and N may be
    //   positive or negative. For cleanApertureWidth and cleanApertureHeight,
    //   N shall be positive and D shall be strictly positive.

    const int32_t widthN = (int32_t)clap->widthN;
    const int32_t widthD = (int32_t)clap->widthD;
    const int32_t heightN = (int32_t)clap->heightN;
    const int32_t heightD = (int32_t)clap->heightD;
    const int32_t horizOffN = (int32_t)clap->horizOffN;
    const int32_t horizOffD = (int32_t)clap->horizOffD;
    const int32_t vertOffN = (int32_t)clap->vertOffN;
    const int32_t vertOffD = (int32_t)clap->vertOffD;
    if ((widthD <= 0) || (heightD <= 0) || (horizOffD <= 0) || (vertOffD <= 0)) {
        avifDiagnosticsPrintf(diag, "[Strict] clap contains a denominator that is not strictly positive");
        return AVIF_FALSE;
    }

    if ((widthN % widthD) != 0) {
        avifDiagnosticsPrintf(diag, "[Strict] clap width is not an integer");
        return AVIF_FALSE;
    }
    if ((heightN % heightD) != 0) {
        avifDiagnosticsPrintf(diag, "[Strict] clap height is not an integer");
        return AVIF_FALSE;
    }

    clapFraction uncroppedCenterX = calcCenter((int32_t)imageW);
    clapFraction uncroppedCenterY = calcCenter((int32_t)imageH);

    clapFraction horizOff;
    horizOff.n = horizOffN;
    horizOff.d = horizOffD;
    clapFraction croppedCenterX = clapFractionAdd(uncroppedCenterX, horizOff);

    clapFraction vertOff;
    vertOff.n = vertOffN;
    vertOff.d = vertOffD;
    clapFraction croppedCenterY = clapFractionAdd(uncroppedCenterY, vertOff);

    clapFraction halfW;
    halfW.n = widthN;
    halfW.d = widthD * 2;
    clapFraction cropX = clapFractionSub(croppedCenterX, halfW);
    if ((cropX.n % cropX.d) != 0) {
        avifDiagnosticsPrintf(diag, "[Strict] calculated crop X offset is not an integer");
        return AVIF_FALSE;
    }

    clapFraction halfH;
    halfH.n = heightN;
    halfH.d = heightD * 2;
    clapFraction cropY = clapFractionSub(croppedCenterY, halfH);
    if (((int32_t)cropY.n % (int32_t)cropY.d) != 0) {
        avifDiagnosticsPrintf(diag, "[Strict] calculated crop Y offset is not an integer");
        return AVIF_FALSE;
    }

    if ((cropX.n < 0) || (cropY.n < 0)) {
        avifDiagnosticsPrintf(diag, "[Strict] at least one crop offset is not positive");
        return AVIF_FALSE;
    }

    cropRect->x = (uint32_t)(cropX.n / cropX.d);
    cropRect->y = (uint32_t)(cropY.n / cropY.d);
    cropRect->width = (uint32_t)(clap->widthN / clap->widthD);
    cropRect->height = (uint32_t)(clap->heightN / clap->heightD);
    return avifCropRectIsValid(cropRect, imageW, imageH, yuvFormat, diag);
}

avifBool avifCleanApertureBoxConvertCropRect(avifCleanApertureBox * clap,
                                             const avifCropRect * cropRect,
                                             const uint32_t imageW,
                                             const uint32_t imageH,
                                             const avifPixelFormat yuvFormat,
                                             avifDiagnostics * diag)
{
    if (!avifCropRectIsValid(cropRect, imageW, imageH, yuvFormat, diag)) {
        return AVIF_FALSE;
    }

    clapFraction uncroppedCenterX = calcCenter(imageW);
    clapFraction uncroppedCenterY = calcCenter(imageH);

    clapFraction croppedCenterX = calcCenter(cropRect->width);
    croppedCenterX.n += cropRect->x * croppedCenterX.d;
    clapFraction croppedCenterY = calcCenter(cropRect->height);
    croppedCenterY.n += cropRect->y * croppedCenterY.d;

    clapFraction horizOff = clapFractionSub(croppedCenterX, uncroppedCenterX);
    clapFraction vertOff = clapFractionSub(croppedCenterY, uncroppedCenterY);

    clap->widthN = cropRect->width;
    clap->widthD = 1;
    clap->heightN = cropRect->height;
    clap->heightD = 1;
    clap->horizOffN = horizOff.n;
    clap->horizOffD = horizOff.d;
    clap->vertOffN = vertOff.n;
    clap->vertOffD = vertOff.d;
    return AVIF_TRUE;
}

// ---------------------------------------------------------------------------
// avifCodecSpecificOption

static char * avifStrdup(const char * str)
{
    size_t len = strlen(str);
    char * dup = avifAlloc(len + 1);
    memcpy(dup, str, len + 1);
    return dup;
}

avifCodecSpecificOptions * avifCodecSpecificOptionsCreate(void)
{
    avifCodecSpecificOptions * ava = avifAlloc(sizeof(avifCodecSpecificOptions));
    avifArrayCreate(ava, sizeof(avifCodecSpecificOption), 4);
    return ava;
}

void avifCodecSpecificOptionsDestroy(avifCodecSpecificOptions * csOptions)
{
    if (!csOptions) {
        return;
    }

    for (uint32_t i = 0; i < csOptions->count; ++i) {
        avifCodecSpecificOption * entry = &csOptions->entries[i];
        avifFree(entry->key);
        avifFree(entry->value);
    }
    avifArrayDestroy(csOptions);
    avifFree(csOptions);
}

void avifCodecSpecificOptionsSet(avifCodecSpecificOptions * csOptions, const char * key, const char * value)
{
    // Check to see if a key must be replaced
    for (uint32_t i = 0; i < csOptions->count; ++i) {
        avifCodecSpecificOption * entry = &csOptions->entries[i];
        if (!strcmp(entry->key, key)) {
            if (value) {
                // Update the value
                avifFree(entry->value);
                entry->value = avifStrdup(value);
            } else {
                // Delete the value
                avifFree(entry->key);
                avifFree(entry->value);
                --csOptions->count;
                if (csOptions->count > 0) {
                    memmove(&csOptions->entries[i], &csOptions->entries[i + 1], (csOptions->count - i) * (size_t)csOptions->elementSize);
                }
            }
            return;
        }
    }

    // Add a new key
    avifCodecSpecificOption * entry = (avifCodecSpecificOption *)avifArrayPushPtr(csOptions);
    entry->key = avifStrdup(key);
    entry->value = avifStrdup(value);
}

// ---------------------------------------------------------------------------
// Codec availability and versions

typedef const char * (*versionFunc)(void);
typedef avifCodec * (*avifCodecCreateFunc)(void);

struct AvailableCodec
{
    avifCodecChoice choice;
    const char * name;
    versionFunc version;
    avifCodecCreateFunc create;
    uint32_t flags;
};

// This is the main codec table; it determines all usage/availability in libavif.

static struct AvailableCodec availableCodecs[] = {
// Ordered by preference (for AUTO)

#if defined(AVIF_CODEC_DAV1D)
    { AVIF_CODEC_CHOICE_DAV1D, "dav1d", avifCodecVersionDav1d, avifCodecCreateDav1d, AVIF_CODEC_FLAG_CAN_DECODE },
#endif
#if defined(AVIF_CODEC_LIBGAV1)
    { AVIF_CODEC_CHOICE_LIBGAV1, "libgav1", avifCodecVersionGav1, avifCodecCreateGav1, AVIF_CODEC_FLAG_CAN_DECODE },
#endif
#if defined(AVIF_CODEC_AOM)
    { AVIF_CODEC_CHOICE_AOM,
      "aom",
      avifCodecVersionAOM,
      avifCodecCreateAOM,
#if defined(AVIF_CODEC_AOM_DECODE) && defined(AVIF_CODEC_AOM_ENCODE)
      AVIF_CODEC_FLAG_CAN_DECODE | AVIF_CODEC_FLAG_CAN_ENCODE
#elif defined(AVIF_CODEC_AOM_DECODE)
      AVIF_CODEC_FLAG_CAN_DECODE
#elif defined(AVIF_CODEC_AOM_ENCODE)
      AVIF_CODEC_FLAG_CAN_ENCODE
#else
#error AVIF_CODEC_AOM_DECODE or AVIF_CODEC_AOM_ENCODE must be defined
#endif
    },
#endif
#if defined(AVIF_CODEC_RAV1E)
    { AVIF_CODEC_CHOICE_RAV1E, "rav1e", avifCodecVersionRav1e, avifCodecCreateRav1e, AVIF_CODEC_FLAG_CAN_ENCODE },
#endif
#if defined(AVIF_CODEC_SVT)
    { AVIF_CODEC_CHOICE_SVT, "svt", avifCodecVersionSvt, avifCodecCreateSvt, AVIF_CODEC_FLAG_CAN_ENCODE },
#endif
    { AVIF_CODEC_CHOICE_AUTO, NULL, NULL, NULL, 0 }
};

static const int availableCodecsCount = (sizeof(availableCodecs) / sizeof(availableCodecs[0])) - 1;

static struct AvailableCodec * findAvailableCodec(avifCodecChoice choice, avifCodecFlags requiredFlags)
{
    for (int i = 0; i < availableCodecsCount; ++i) {
        if ((choice != AVIF_CODEC_CHOICE_AUTO) && (availableCodecs[i].choice != choice)) {
            continue;
        }
        if (requiredFlags && ((availableCodecs[i].flags & requiredFlags) != requiredFlags)) {
            continue;
        }
        return &availableCodecs[i];
    }
    return NULL;
}

const char * avifCodecName(avifCodecChoice choice, avifCodecFlags requiredFlags)
{
    struct AvailableCodec * availableCodec = findAvailableCodec(choice, requiredFlags);
    if (availableCodec) {
        return availableCodec->name;
    }
    return NULL;
}

avifCodecChoice avifCodecChoiceFromName(const char * name)
{
    for (int i = 0; i < availableCodecsCount; ++i) {
        if (!strcmp(availableCodecs[i].name, name)) {
            return availableCodecs[i].choice;
        }
    }
    return AVIF_CODEC_CHOICE_AUTO;
}

avifCodec * avifCodecCreate(avifCodecChoice choice, avifCodecFlags requiredFlags)
{
    struct AvailableCodec * availableCodec = findAvailableCodec(choice, requiredFlags);
    if (availableCodec) {
        return availableCodec->create();
    }
    return NULL;
}

static void append(char ** writePos, size_t * remainingLen, const char * appendStr)
{
    size_t appendLen = strlen(appendStr);
    if (appendLen > *remainingLen) {
        appendLen = *remainingLen;
    }

    memcpy(*writePos, appendStr, appendLen);
    *remainingLen -= appendLen;
    *writePos += appendLen;
    *(*writePos) = 0;
}

void avifCodecVersions(char outBuffer[256])
{
    size_t remainingLen = 255;
    char * writePos = outBuffer;
    *writePos = 0;

    for (int i = 0; i < availableCodecsCount; ++i) {
        if (i > 0) {
            append(&writePos, &remainingLen, ", ");
        }
        append(&writePos, &remainingLen, availableCodecs[i].name);
        if ((availableCodecs[i].flags & (AVIF_CODEC_FLAG_CAN_ENCODE | AVIF_CODEC_FLAG_CAN_DECODE)) ==
            (AVIF_CODEC_FLAG_CAN_ENCODE | AVIF_CODEC_FLAG_CAN_DECODE)) {
            append(&writePos, &remainingLen, " [enc/dec]");
        } else if (availableCodecs[i].flags & AVIF_CODEC_FLAG_CAN_ENCODE) {
            append(&writePos, &remainingLen, " [enc]");
        } else if (availableCodecs[i].flags & AVIF_CODEC_FLAG_CAN_DECODE) {
            append(&writePos, &remainingLen, " [dec]");
        }
        append(&writePos, &remainingLen, ":");
        append(&writePos, &remainingLen, availableCodecs[i].version());
    }
}
