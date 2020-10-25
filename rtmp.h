#ifndef _RTMP_H_
#define _RTMP_H_
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <string.h>
#include <stdlib.h>

int init_rtmp_streamer();
int write_frame(uint8_t*data,int size);

#endif