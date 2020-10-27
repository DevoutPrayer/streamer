#ifndef _STREAMER_H
#define _STREAMER_H
#include <stdio.h>
#include "v4l2.h"
#include "mpp.h"
typedef struct
{
        V4l2Context             *v4l2_ctx;
        MpiEncData              *mpp_enc_data;
}StreamerContext;
#endif /* ！_STREAMER_H */