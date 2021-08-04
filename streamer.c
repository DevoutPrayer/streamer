#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include "rtmp.h"
#include "streamer.h"
#include "v4l2.h"
#include "mpp.h"

MppContext      *mpp_ctx;


_Bool m_process_image(uint8_t *p, int size)
{ 
        return mpp_ctx->process_image(p,size,mpp_ctx);
}

int main(int argc,char* argv[])
{
        StreamerContext                 streamer_ctx;
        V4l2Context                     *v4l2_ctx;
        v4l2_ctx                        = alloc_v4l2_context();
        mpp_ctx                         = alloc_mpp_context();
        streamer_ctx.v4l2_ctx           = v4l2_ctx;
        streamer_ctx.mpp_enc_data       = mpp_ctx;
        /*Configure v4l2Context*/
        v4l2_ctx->process_image         = m_process_image;
        v4l2_ctx->force_format          = 1;
        v4l2_ctx->width                 = 640;
        v4l2_ctx->height                = 480;
        v4l2_ctx->pixelformat           = V4L2_PIX_FMT_YUYV;
        v4l2_ctx->field                 = V4L2_FIELD_INTERLACED;
        /*Configure MpiEncData*/
        mpp_ctx->width                  = v4l2_ctx->width;
	mpp_ctx->height                 = v4l2_ctx->height;
        mpp_ctx->fps                    = 30;
	mpp_ctx->gop                    = 60;
	mpp_ctx->bps                    = mpp_ctx->width * mpp_ctx->height / 8 * mpp_ctx->fps*2;
        mpp_ctx->write_frame            = write_frame;

        SpsHeader sps_header;
        /*Begin*/
        v4l2_ctx->open_device(argv[1],v4l2_ctx);
        v4l2_ctx->init_device(v4l2_ctx);  
        mpp_ctx->init_mpp(mpp_ctx);
        v4l2_ctx->start_capturing(v4l2_ctx);
        mpp_ctx->write_header(mpp_ctx,&sps_header);
        init_rtmp_streamer(argv[2],sps_header.data,sps_header.size);
  
        v4l2_ctx->main_loop(v4l2_ctx);

        mpp_ctx->close(mpp_ctx);
        v4l2_ctx->close(v4l2_ctx);
        /*End*/
        return 0;
}




