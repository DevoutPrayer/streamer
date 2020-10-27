#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <string.h>
#include <errno.h>
#include <rockchip/rk_mpi.h>
#include "rtmp.h"
#include "streamer.h"
#include "v4l2.h"
#include "mpp.h"
#include "global.h"

MpiEncData          mpp_enc_data;


_Bool m_process_image(uint8_t *p, int size)
{
        
        return process_image(p,size,&mpp_enc_data);
}

void stop(V4l2Context*);
int main()
{
        StreamerContext     streamer_ctx;
        V4l2Context         v4l2_ctx;
        memset(&streamer_ctx, 0, sizeof(streamer_ctx));
        memset(&v4l2_ctx, 0, sizeof(v4l2_ctx));
        memset(&mpp_enc_data, 0, sizeof(mpp_enc_data));
        v4l2_ctx.process_image = m_process_image;
        
        mpp_enc_data.fp_outputx = fopen("/tmp/output1.yuv", "wb+");
        mpp_enc_data.fp_output = fopen("/tmp/output1.h264", "wb+");
        mpp_enc_data.write_frame = write_frame;


        open_device("/dev/video0",&v4l2_ctx);
        init_device(&v4l2_ctx);
        init_rtmp_streamer();

        init_mpp(&mpp_enc_data);
        start_capturing(&v4l2_ctx);
  
        main_loop(&v4l2_ctx);

        stop(&v4l2_ctx);
        fclose(mpp_enc_data.fp_outputx);
        return 0;
}

void stop(V4l2Context* ctx)
{
    enum v4l2_buf_type type;

    switch (ctx->io_method)
    {
    case IO_METHOD_READ:
        /* Nothing to do. */
        break;
    case IO_METHOD_MMAP:
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(ctx->fd, VIDIOC_STREAMOFF, &type);
        break;
    }
    MPP_RET ret = MPP_OK;
	ret = mpp_enc_data.mpi->reset(mpp_enc_data.ctx);
	if (ret)
	{
		printf("mpi->reset failed\n");
	}

	if (mpp_enc_data.ctx)
	{
		mpp_destroy(mpp_enc_data.ctx);
		mpp_enc_data.ctx = NULL;
	}

	if (mpp_enc_data.frm_buf)
	{
		mpp_buffer_put(mpp_enc_data.frm_buf);
		mpp_enc_data.frm_buf = NULL;
	}

	fclose(mpp_enc_data.fp_output);

    unsigned int i;

    switch (ctx->io_method)
    {
    case IO_METHOD_READ:
        free(ctx->buffers[0].start);
        break;
    case IO_METHOD_MMAP:
        for (i = 0; i < ctx->n_buffers; ++i)
            munmap(ctx->buffers[i].start, ctx->buffers[i].length);
        break;
    }

    free(ctx->buffers);
    close(ctx->fd);
    
}


