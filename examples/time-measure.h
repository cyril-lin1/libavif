/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 * time-measure.h
 * Original Author: heyao@ruijie.com.cn
 *
 * History:
 *
 */
#ifndef TIME_MEASURE_H_
#define TIME_MEASURE_H_

//#define TM_STREAM_TIME
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define NSEC_PER_SEC    1000000000LL
#define NSEC_PER_MSEC   1000000LL

#if defined(_MSC_VER)
#include <winsock2.h>
#include <Windows.h>
#endif

//#define HIGH_ACCURACY_TIME

static inline int64_t mbce_get_monotonic_time_ns(void)
{
#if defined(_MSC_VER)

#ifdef HIGH_ACCURACY_TIME
    LARGE_INTEGER litmp;
    QueryPerformanceFrequency(&litmp);  //获取CPU时钟频率
    double dfFreq = (double)litmp.QuadPart;

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    double QPart = (double)counter.QuadPart;
    double time = QPart / dfFreq * NSEC_PER_SEC;    //以ms为单位
    return (int64_t)time;
#else
    return (int64_t)GetTickCount() * NSEC_PER_MSEC;
#endif

#else
    struct timespec time;

    clock_gettime(CLOCK_MONOTONIC, &time);
    return NSEC_PER_SEC * time.tv_sec + time.tv_nsec;
#endif
}

typedef struct {
    int64_t start;
    int64_t end;
} TM_CTX;

void time_measure_general_start(TM_CTX *tm_ctx);
void time_measure_general_end(TM_CTX *tm_ctx, int level, char *func_name);

#endif  /* TIME_MEASURE_H_ */
