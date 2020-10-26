#ifndef _V4L2_H 
#define _V4L2_H
#include <stddef.h>
#include <stdint.h>
#include <linux/videodev2.h>

enum IO_METHOD
{
        IO_METHOD_READ,
        IO_METHOD_MMAP,
};

struct buffer
{
        void    *start;
        size_t  length;
};
                
typedef struct 
{
        int             *fd;
        char            *dev_name;
        enum IO_METHOD  io_method;
        struct buffer   *buffers;
        uint32_t        n_buffers;
        _Bool           force_format;
        uint32_t        width;
        uint32_t        height;
        uint32_t        pixelformat;
        uint32_t        field;


        _Bool (*process_image)(uint8_t *p, int size);

}V4l2Context;

int xioctl(int fh, int request, void *arg);
int open_device(char * device,V4l2Context *ctx);
int init_device(V4l2Context *ctx);
int init_mmap(V4l2Context *ctx);
int init_read(unsigned int buffer_size,V4l2Context *ctx);
int read_frame(V4l2Context *ctx);
int start_capturing(V4l2Context *ctx);
void main_loop(V4l2Context *ctx);

#endif /* !_V4L2_H */
