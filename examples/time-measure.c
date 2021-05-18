/*
 * Copyright(C) 2018 Ruijie Network. All rights reserved.
 * time-measure.c
 * Original Author: heyao@ruijie.com.cn
 *
 * History:
 *
 */
#define TM_STREAM_TIME
#include "time-measure.h"
#include <stdio.h>

void time_measure_general_start(TM_CTX *tm_ctx)
{
#ifdef TM_STREAM_TIME
    tm_ctx->start = mbce_get_monotonic_time_ns();
#endif
}
void time_measure_general_end(TM_CTX *tm_ctx, int level, char *func_name)
{
#ifdef TM_STREAM_TIME
    tm_ctx->end =  mbce_get_monotonic_time_ns();

    int64_t delta = tm_ctx->end - tm_ctx->start;

    if (delta >= NSEC_PER_MSEC) {
        char *prefix = "";
        if (level == 1)
            prefix = " ";
        else if (level == 2)
            prefix = "  ";
        else if (level == 3)
            prefix = "   ";

        printf("%s [L%d]%25s expire %9ld (%lldms)\n", prefix, level, func_name, delta, delta / NSEC_PER_MSEC);
    }

    tm_ctx->start = tm_ctx->end;
#endif
}

