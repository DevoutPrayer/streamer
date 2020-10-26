#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include "v4l2.h"

int xioctl(int fh, int request, void *arg)
{
        int r;
        do
        {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}

int open_device(char * device,V4l2Context *ctx)
{ 
        ctx->dev_name = device;
        int ret;
        struct stat st;
        if (stat(device, &st) == -1)
        {
                fprintf(stderr, "Cannot identify '%s': %d, %s\n",device, errno, strerror(errno));
                return -1;
        }

        if (!S_ISCHR(st.st_mode))
        {
                fprintf(stderr, "%s is not device", ctx->dev_name);
                return -1;
        }

        ctx->fd = open(device, O_RDWR /* required */ | O_NONBLOCK, 0);
        if (ctx->fd == -1)
        {
                fprintf(stderr, "Cannot open '%s': %d, %s\\n",device, errno, strerror(errno));
                return -1;
        }
        return 0;
}

int init_device(V4l2Context *ctx)
{

        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;
        unsigned int min;

        if(xioctl(ctx->fd, VIDIOC_QUERYCAP, &cap) == -1)
        {
                fprintf(stderr, "get VIDIOC_QUERYCAP error: %d, %s\n", errno, strerror(errno));
                return -1;
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        {
                fprintf(stderr, "%s is not video capture device\n",ctx->dev_name);
                return -1;
        }

        if (!(cap.capabilities & V4L2_CAP_STREAMING))
        {
                fprintf(stderr, "%s does not support streaming i/o\n", ctx->dev_name);

                if (!(cap.capabilities & V4L2_CAP_READWRITE))
                {
                        fprintf(stderr, "%s does not support read i/o\n", ctx->dev_name);
                        return -1;
                }
                ctx->io_method = IO_METHOD_READ;
        }
        else
        {
                ctx->io_method = IO_METHOD_MMAP;
        }
        /* Select video input, video standard and tune here. */
        memset(&cropcap, 0, sizeof(cropcap));
        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (xioctl(ctx->fd, VIDIOC_CROPCAP, &cropcap) == 0)
        {
                memset(&crop, 0, sizeof(crop));
                crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                crop.c = cropcap.defrect; /* reset to default */

                if (xioctl(ctx->fd, VIDIOC_S_CROP, &crop) == -1)
                {
                        fprintf(stderr, "set VIDIOC_S_CROP failed: %d, %s\n", errno, strerror(errno));
                }
        }
        else
        {
                fprintf(stderr, "get VIDIOC_CROPCAP failed: %d, %s\n", errno, strerror(errno));
        }

        /* Enum pixel format */
        for(int i=0; i<20; i++)
        {
                struct v4l2_fmtdesc fmtdesc;
                fmtdesc.index = i;
                fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if(xioctl(ctx->fd, VIDIOC_ENUM_FMT, &fmtdesc) == -1)
                        break;
                printf("%d: %s\n", i, fmtdesc.description);
        }


        memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ctx->force_format)
        {
                fmt.fmt.pix.width       = ctx->width;
                fmt.fmt.pix.height      = ctx->height;
                fmt.fmt.pix.pixelformat = ctx->pixelformat;
                fmt.fmt.pix.field       = ctx->field;

                if (xioctl(ctx->fd, VIDIOC_S_FMT, &fmt) == -1)
                {
                        fprintf(stderr, "get VIDIOC_S_FMT failed: %d, %s\n", errno, strerror(errno));
                        return -1;
                }

                /* Note VIDIOC_S_FMT may change width and height. */
        }
        else
        {
                /* Preserve original settings as set by v4l2-ctl for example */
                if (xioctl(ctx->fd, VIDIOC_G_FMT, &fmt) == -1)
                {
                        fprintf(stderr, "get VIDIOC_G_FMT failed: %d, %s\n", errno, strerror(errno));
                        return -1;
                }
        }
        printf("fmt.w=%d,fmt.h=%d\n",fmt.fmt.pix.width,fmt.fmt.pix.height);
        printf("fmt.pixfmt=0x%x\n",fmt.fmt.pix.pixelformat);

        /* Buggy driver paranoia. */
        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
                fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
                fmt.fmt.pix.sizeimage = min;

        if(ctx->io_method == IO_METHOD_MMAP)
                return init_mmap(ctx);
        else
                return init_read(fmt.fmt.pix.sizeimage,ctx);
}

void main_loop(V4l2Context *ctx)
{
        int fd = ctx->fd;
        int r;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        for (;;)
        {
                struct timeval tv;
                tv.tv_sec = 2;
                tv.tv_usec = 0;

                fd_set rdset = fds;
                r = select(fd + 1, &rdset, NULL, NULL, &tv);

                if(r > 0)
                {   
                        if(read_frame(ctx) == -2)
                                break;
                }
                else if(r == 0)
                {
                        fprintf(stderr, "select timeout\\n");
                }
                else
                {
                        if (EINTR == errno || EAGAIN == errno)
                                continue;
                        fprintf(stderr, "select failed: %d, %s\n", errno, strerror(errno));
                                break;
                }
                /* EAGAIN - continue select loop. */
        }
}



int init_mmap(V4l2Context *ctx)
{        
        struct v4l2_requestbuffers req;
        memset(&req, 0, sizeof(req));

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if(xioctl(ctx->fd, VIDIOC_REQBUFS, &req) == -1)
        {
                fprintf(stderr, "set VIDIOC_REQBUFS failed: %d, %s\n", errno, strerror(errno));
                return -1;
        }

        if (req.count < 2)
        {
                fprintf(stderr, "Insufficient buffer memory on %s\n",ctx->dev_name);
                return -1;
        }

        ctx->buffers = (struct buffer*)calloc(req.count, sizeof(struct buffer));

        if (!ctx->buffers)
        {
                fprintf(stderr, "Out of memory\n");
                return -1;
        }

        for (ctx->n_buffers = 0; ctx->n_buffers < req.count; ++ctx->n_buffers)
        {
                struct v4l2_buffer buf;
                memset(&buf, 0, sizeof(buf));
                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = ctx->n_buffers;

                if (xioctl(ctx->fd, VIDIOC_QUERYBUF, &buf) == -1)
                {
                        fprintf(stderr, "set VIDIOC_QUERYBUF %u failed: %d, %s\n", ctx->n_buffers, errno, strerror(errno));
                        return -1;
                }

                ctx->buffers[ctx->n_buffers].length = buf.length;
                ctx->buffers[ctx->n_buffers].start =
                        mmap(NULL /* start anywhere */,
                        buf.length,
                        PROT_READ | PROT_WRITE /* required */,
                        MAP_SHARED /* recommended */,
                        ctx->fd, buf.m.offset);

                if (MAP_FAILED == ctx->buffers[ctx->n_buffers].start)
                {
                        fprintf(stderr, "mmap %u failed: %d, %s\n", ctx->n_buffers, errno, strerror(errno));
                        return -1;
                }
        }

        return 0;
}

int init_read(unsigned int buffer_size,V4l2Context *ctx)
{
        ctx->buffers = (struct buffer*)calloc(1, sizeof(struct buffer));

        if (!ctx->buffers)
        {
                fprintf(stderr, "Out of memory\n");
                return -1;
        }

        ctx->buffers[0].length = buffer_size;
        ctx->buffers[0].start = malloc(buffer_size);

        if (!ctx->buffers[0].start)
        {
                fprintf(stderr, "Out of memory\n");
                return -1;
        }

        return 0;
}


int start_capturing(V4l2Context *ctx)
{
        unsigned int i;
        enum v4l2_buf_type type;

        switch (ctx->io_method)
        {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;
        case IO_METHOD_MMAP:
                for (i = 0; i < ctx->n_buffers; ++i)
                {
                        struct v4l2_buffer buf;
                        memset(&buf, 0, sizeof(buf));

                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_MMAP;
                        buf.index = i;

                        if (xioctl(ctx->fd, VIDIOC_QBUF, &buf) == -1)
                        {
                                fprintf(stderr, "set VIDIOC_QBUF failed: %d, %s\n", errno, strerror(errno));
                                return -1;
                        }
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (xioctl(ctx->fd, VIDIOC_STREAMON, &type) == -1)
                {
                        fprintf(stderr, "set VIDIOC_STREAMON failed: %d, %s\n", errno, strerror(errno));
                        return -1;
                }
                break;
        }
        return 0;
}

int read_frame(V4l2Context *ctx)
{
        struct v4l2_buffer buf;
        switch (ctx->io_method)
        {
        case IO_METHOD_READ:
                if (read(ctx->fd, ctx->buffers[0].start, ctx->buffers[0].length) == -1)
                {
                        if(errno == EAGAIN || errno == EINTR)
                        {
                                return 0;
                        }
                        else
                        {
                                fprintf(stderr, "read failed: %d, %s\n", errno, strerror(errno));
                                return -1;
                        }
                }
                if(!ctx->process_image((uint8_t*)ctx->buffers[0].start, ctx->buffers[0].length))
                {
                        return -2;
                }
                break;

        case IO_METHOD_MMAP:
                memset(&buf, 0, sizeof(buf));

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;

                if (xioctl(ctx->fd, VIDIOC_DQBUF, &buf) == -1)
                {
                        if(errno == EAGAIN || errno == EINTR)
                        {
                                return 0;
                        }
                else
                {
                        fprintf(stderr, "set VIDIOC_DQBUF failed: %d, %s\n", errno, strerror(errno));
                        return -1;
                }
                }
                if(buf.index < ctx->n_buffers)
                {
                        if(!process_image((uint8_t*)ctx->buffers[buf.index].start, buf.bytesused))
                        {
                                return -2;
                        }
                        if (-1 == xioctl(ctx->fd, VIDIOC_QBUF, &buf))
                        {
                                fprintf(stderr, "set VIDIOC_QBUF failed: %d, %s\n", errno, strerror(errno));
                                return -1;
                        }
                }
                break;
        }
        return 0;
}