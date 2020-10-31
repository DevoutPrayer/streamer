#ifndef _STREAMER_H
#define _STREAMER_H
#ifdef __cplusplus
        extern "C"
        {
#endif
#include <stdio.h>
#include "v4l2.h"
#include "mpp.h"
typedef struct
{
        V4l2Context             *v4l2_ctx;
        MppContext              *mpp_enc_data;
}StreamerContext;
#ifdef __cplusplus
        }
#endif
#endif /* ！_STREAMER_H */