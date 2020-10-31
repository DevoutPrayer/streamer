#include "mpp.h"
static void mpp_close(MppContext* ctx);
static void init_mpp(MppContext *mpp_enc_data);
static _Bool write_header(MppContext *mpp_enc_data);
static _Bool process_image(uint8_t *p, int size,MppContext *mpp_enc_data);

MppContext * alloc_mpp_context()
{
        MppContext *ctx = (MppContext *)malloc(sizeof(MppContext));
        ctx->init_mpp = init_mpp;
        ctx->close = mpp_close;
        ctx->write_header = write_header;
        ctx->process_image = process_image;
        return ctx;
}

static void mpp_close(MppContext* ctx)
{
        MPP_RET ret = MPP_OK;
        ret = ctx->mpi->reset(ctx->ctx);
        if (ret)
        {
                printf("mpi->reset failed\n");
        }

        if (ctx->ctx)
        {
                mpp_destroy(ctx->ctx);
                ctx->ctx = NULL;
        }

        if (ctx->frm_buf)
        {
                mpp_buffer_put(ctx->frm_buf);
                ctx->frm_buf = NULL;
        }
        free(ctx);
    
}

static void init_mpp(MppContext *mpp_enc_data)
{
        MPP_RET ret = MPP_OK;
        mpp_enc_data->width = 640;
	mpp_enc_data->height = 480;
        mpp_enc_data->type = MPP_VIDEO_CodingAVC;
        mpp_enc_data->fmt = MPP_FMT_YUV422_YUYV;
        mpp_enc_data->hor_stride = MPP_ALIGN(mpp_enc_data->width, 16);
        mpp_enc_data->ver_stride = MPP_ALIGN(mpp_enc_data->height, 16);
        mpp_enc_data->frame_size = mpp_enc_data->hor_stride * mpp_enc_data->ver_stride * 2;

        ret = mpp_buffer_get(NULL, &(mpp_enc_data->frm_buf), mpp_enc_data->frame_size);
	if (ret)
	{
		printf("failed to get buffer for input frame ret %d\n", ret);
		goto MPP_INIT_OUT;
	}

	ret = mpp_create(&mpp_enc_data->ctx, &mpp_enc_data->mpi);
	if (ret)
	{
		printf("mpp_create failed ret %d\n", ret);
		goto MPP_INIT_OUT;
	}

	ret = mpp_init(mpp_enc_data->ctx, MPP_CTX_ENC, mpp_enc_data->type);
	if (ret)
	{
		printf("mpp_init failed ret %d\n", ret);
		goto MPP_INIT_OUT;
	}

	mpp_enc_data->fps = 30;
	mpp_enc_data->gop = 60;
	mpp_enc_data->bps = mpp_enc_data->width * mpp_enc_data->height / 8 * mpp_enc_data->fps;

	mpp_enc_data->prep_cfg.change           = MPP_ENC_PREP_CFG_CHANGE_INPUT |
                                                  MPP_ENC_PREP_CFG_CHANGE_ROTATION |
			                          MPP_ENC_PREP_CFG_CHANGE_FORMAT;
	mpp_enc_data->prep_cfg.width            = mpp_enc_data->width;
	mpp_enc_data->prep_cfg.height           = mpp_enc_data->height;
        mpp_enc_data->prep_cfg.hor_stride       = mpp_enc_data->hor_stride;
	mpp_enc_data->prep_cfg.ver_stride       = mpp_enc_data->ver_stride;
	mpp_enc_data->prep_cfg.format           = mpp_enc_data->fmt;
	mpp_enc_data->prep_cfg.rotation         = MPP_ENC_ROT_0;

	ret = mpp_enc_data->mpi->control(mpp_enc_data->ctx, MPP_ENC_SET_PREP_CFG, &mpp_enc_data->prep_cfg);
	if (ret)
	{
		printf("mpi control enc set prep cfg failed ret %d\n", ret);
		goto MPP_INIT_OUT;
	}

	mpp_enc_data->rc_cfg.change  = MPP_ENC_RC_CFG_CHANGE_ALL;
	mpp_enc_data->rc_cfg.rc_mode = MPP_ENC_RC_MODE_CBR;
	mpp_enc_data->rc_cfg.quality = MPP_ENC_RC_QUALITY_MEDIUM;
	if (mpp_enc_data->rc_cfg.rc_mode == MPP_ENC_RC_MODE_CBR)
	{
		/* constant bitrate has very small bps range of 1/16 bps */
		mpp_enc_data->rc_cfg.bps_target   = mpp_enc_data->bps;
		mpp_enc_data->rc_cfg.bps_max      = mpp_enc_data->bps * 17 / 16;
		mpp_enc_data->rc_cfg.bps_min      = mpp_enc_data->bps * 15 / 16;
	}
	else if (mpp_enc_data->rc_cfg.rc_mode ==  MPP_ENC_RC_MODE_VBR)
	{
		if (mpp_enc_data->rc_cfg.quality == MPP_ENC_RC_QUALITY_CQP)
		{
			/* constant QP does not have bps */
			mpp_enc_data->rc_cfg.bps_target   = -1;
			mpp_enc_data->rc_cfg.bps_max      = -1;
			mpp_enc_data->rc_cfg.bps_min      = -1;
		}
		else
		{
			/* variable bitrate has large bps range */
			mpp_enc_data->rc_cfg.bps_target   = mpp_enc_data->bps;
			mpp_enc_data->rc_cfg.bps_max      = mpp_enc_data->bps * 17 / 16;
			mpp_enc_data->rc_cfg.bps_min      = mpp_enc_data->bps * 1 / 16;
		}
	}

	/* fix input / output frame rate */
	mpp_enc_data->rc_cfg.fps_in_flex      = 0;
	mpp_enc_data->rc_cfg.fps_in_num       = mpp_enc_data->fps;
	mpp_enc_data->rc_cfg.fps_in_denorm    = 1;
	mpp_enc_data->rc_cfg.fps_out_flex     = 0;
	mpp_enc_data->rc_cfg.fps_out_num      = mpp_enc_data->fps;
	mpp_enc_data->rc_cfg.fps_out_denorm   = 1;
	mpp_enc_data->rc_cfg.gop              = mpp_enc_data->gop;
	mpp_enc_data->rc_cfg.skip_cnt         = 0;

	ret = mpp_enc_data->mpi->control(mpp_enc_data->ctx, MPP_ENC_SET_RC_CFG, &mpp_enc_data->rc_cfg);
	if (ret)
	{
		printf("mpi control enc set rc cfg failed ret %d\n", ret);
		goto MPP_INIT_OUT;
	}

	mpp_enc_data->codec_cfg.coding = mpp_enc_data->type;
	switch (mpp_enc_data->codec_cfg.coding)
	{
	case MPP_VIDEO_CodingAVC :
	{
		mpp_enc_data->codec_cfg.h264.change   = MPP_ENC_H264_CFG_CHANGE_PROFILE |
				                        MPP_ENC_H264_CFG_CHANGE_ENTROPY |
				                        MPP_ENC_H264_CFG_CHANGE_TRANS_8x8;
		/*
		 * H.264 profile_idc parameter
		 * 66  - Baseline profile
		 * 77  - Main profile
		 * 100 - High profile
		 */
		mpp_enc_data->codec_cfg.h264.profile  = 100;
		/*
		 * H.264 level_idc parameter
		 * 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
		 * 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
		 * 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
		 * 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
		 * 50 / 51 / 52         - 4K@30fps
		 */
		mpp_enc_data->codec_cfg.h264.level    = 31;
		mpp_enc_data->codec_cfg.h264.entropy_coding_mode  = 1;
		mpp_enc_data->codec_cfg.h264.cabac_init_idc  = 0;
		mpp_enc_data->codec_cfg.h264.transform8x8_mode = 1;
	}
	break;
	case MPP_VIDEO_CodingMJPEG :
	{
		mpp_enc_data->codec_cfg.jpeg.change  = MPP_ENC_JPEG_CFG_CHANGE_QP;
		mpp_enc_data->codec_cfg.jpeg.quant   = 10;
	}
	break;
	case MPP_VIDEO_CodingVP8 :
	case MPP_VIDEO_CodingHEVC :
	default :
	{
		printf("support encoder coding type %d\n", mpp_enc_data->codec_cfg.coding);
	}
	break;
	}

	ret = mpp_enc_data->mpi->control(mpp_enc_data->ctx, MPP_ENC_SET_CODEC_CFG, &mpp_enc_data->codec_cfg);
	if (ret)
	{
		printf("mpi control enc set codec cfg failed ret %d\n", ret);
		goto MPP_INIT_OUT;
	}

	/* optional */
	mpp_enc_data->sei_mode = MPP_ENC_SEI_MODE_ONE_FRAME;
	ret = mpp_enc_data->mpi->control(mpp_enc_data->ctx, MPP_ENC_SET_SEI_CFG, &mpp_enc_data->sei_mode);
	if (ret)
	{
		printf("mpi control enc set sei cfg failed ret %d\n", ret);
		goto MPP_INIT_OUT;
	}

	

	return 0;

MPP_INIT_OUT:

        if (mpp_enc_data->ctx)
        {
                mpp_destroy(mpp_enc_data->ctx);
                mpp_enc_data->ctx = NULL;
        }

        if (mpp_enc_data->frm_buf)
        {
                mpp_buffer_put(mpp_enc_data->frm_buf);
                mpp_enc_data->frm_buf = NULL;
        }

        printf("init mpp failed!\n");
}

static _Bool write_header(MppContext *mpp_enc_data)
{
        int ret;
        if (mpp_enc_data->type == MPP_VIDEO_CodingAVC)
	{
		MppPacket packet = NULL;
		ret = mpp_enc_data->mpi->control(mpp_enc_data->ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
		if (ret)
		{
			printf("mpi control enc get extra info failed\n");
			return 1;
		}

		/* get and write sps/pps for H.264 */
		if (packet)
		{

			void *ptr   = mpp_packet_get_pos(packet);
			size_t len  = mpp_packet_get_length(packet);

			if (mpp_enc_data->fp_output)
                        {
                                fwrite(ptr, 1, len, mpp_enc_data->fp_output);
                                return 0;
                        }
                        // if(mpp_enc_data->write_frame)
                        //         if(!(mpp_enc_data->write_frame)(ptr,len))
                        //                         printf("------------sendok!\n");
			packet = NULL;
		}
	}
        return 1;
}

static _Bool process_image(uint8_t *p, int size,MppContext *mpp_enc_data)
{
//     if(mpp_enc_data->fp_outputx)
//     {
//         printf("ok\n");
//         //fwrite(p, 1, size, mpp_enc_data.fp_outputx);
//     }
    
	MPP_RET ret = MPP_OK;
    
     

        MppFrame frame = NULL;
        MppPacket packet = NULL;

        void *buf = mpp_buffer_get_ptr(mpp_enc_data->frm_buf);
        //TODO: improve performance here?
        memcpy(buf, p, size);
        ret = mpp_frame_init(&frame);
        if (ret)
        {
                printf("mpp_frame_init failed\n");
                return 1;
        }

        mpp_frame_set_width(frame, mpp_enc_data->width);
        mpp_frame_set_height(frame, mpp_enc_data->height);
        mpp_frame_set_hor_stride(frame, mpp_enc_data->hor_stride);
        mpp_frame_set_ver_stride(frame, mpp_enc_data->ver_stride);
        mpp_frame_set_fmt(frame, mpp_enc_data->fmt);
        mpp_frame_set_buffer(frame, mpp_enc_data->frm_buf);
        mpp_frame_set_eos(frame, mpp_enc_data->frm_eos);

        ret = mpp_enc_data->mpi->encode_put_frame(mpp_enc_data->ctx, frame);
        if (ret)
        {
                printf("mpp encode put frame failed\n");
                return 1;
        }
  
mdddd:
        ret = mpp_enc_data->mpi->encode_get_packet(mpp_enc_data->ctx, &packet);
        if (ret)
        {
                printf("mpp encode get packet failed\n");
                return 1;
        }
    
        if (packet)
        {
                // write packet to file here
                void *ptr   = mpp_packet_get_pos(packet);
                size_t len  = mpp_packet_get_length(packet);
                mpp_enc_data->pkt_eos = mpp_packet_get_eos(packet);
                if (mpp_enc_data->fp_output)
                {
                	fwrite(ptr, 1, len, mpp_enc_data->fp_output);   
                }
                if(!mpp_enc_data->fp_output&&mpp_enc_data->write_frame)
                        if(!(mpp_enc_data->write_frame)(ptr,len))
                                        printf("------------sendok!\n");

                mpp_packet_deinit(&packet);
                printf("encoded frame %d size %ld\n", mpp_enc_data->frame_count, len);
                mpp_enc_data->stream_size += len;
                mpp_enc_data->frame_count++;

                if (mpp_enc_data->pkt_eos)
                {
                        printf("found last packet\n");
                }
        }
        else
                goto mdddd;
        if (mpp_enc_data->num_frames && mpp_enc_data->frame_count >= mpp_enc_data->num_frames)
        {
        	printf("encode max %d frames", mpp_enc_data->frame_count);
        	return 0;
        }
        if (mpp_enc_data->frm_eos && mpp_enc_data->pkt_eos)
                return 0;
    
        return 1;
}