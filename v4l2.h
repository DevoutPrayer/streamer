#ifndef _V4L2_H 
#define _V4L2_H
#ifdef __cplusplus
        extern "C"
        {
#endif
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
        int64_t         *fd;
        char            *dev_name;
        enum IO_METHOD  io_method;
        struct buffer   *buffers;
        uint32_t        n_buffers;
        _Bool           force_format;
        uint32_t        width;
        uint32_t        height;
        uint32_t        pixelformat;
        uint32_t        field;

        /*call back function*/
        _Bool (*process_image)(uint8_t *p, int size);
        /*function pointer*/
        int (*open_device)(char * device,void *ctx);
        int (*init_device)(void *ctx);
        int (*start_capturing)(void *ctx);
        void (*main_loop)(void *ctx);
        int (*close)(void *ctx);

}V4l2Context;

V4l2Context * alloc_v4l2_context(); 
#ifdef __cplusplus
        }
#endif
#endif /* !_V4L2_H */
