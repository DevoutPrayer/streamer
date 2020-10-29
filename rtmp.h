#ifndef _RTMP_H_
#define _RTMP_H_


int init_rtmp_streamer(char *  stream);
int write_frame(uint8_t*data,int size);

#endif