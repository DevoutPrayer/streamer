#ifndef _RTMP_H_
#define _RTMP_H_
#ifdef __cplusplus
        extern "C"
        {
#endif

int init_rtmp_streamer(char* stream,uint8_t* data,uint32_t size);
int write_frame(uint8_t*data,int size);
#ifdef __cplusplus
        }
#endif
#endif