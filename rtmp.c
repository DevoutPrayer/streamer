#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <string.h>
#include <stdlib.h>
#include "rtmp.h"
                
static AVPacket pkt;
static AVFormatContext *ofmt_ctx = NULL;
static int frame_index=0;

int write_frame(uint8_t*data,int size)
{
        pkt.size = size;
        pkt.data = data;
        pkt.flags = 0x01;
        pkt.stream_index = 0;
        // pkt.duration = AV_TIME_BASE*1/20000;
        // pkt.dts = frame_index*pkt.duration;
        // pkt.pts = pkt.dts;
        // frame_index ++;

        if (av_write_frame(ofmt_ctx, &pkt) < 0) {
                printf( "Error muxing packet\n");
                return -1;
        }
        return 0;
}

int init_rtmp_streamer(char* stream,uint8_t *data,uint32_t size)
{
        int ret;
        av_register_all();
        if((ret = avformat_network_init()) < 0)
        {
                fprintf(stderr, "avformat_network_init failed!");
                return -1;
        }

        avformat_alloc_output_context2(&ofmt_ctx,NULL,"flv",stream);
        if(!ofmt_ctx)
        {
                fprintf(stderr, "Could not create output context\n");
                return -1;
        }
        

        AVStream *out_stream = avformat_new_stream(ofmt_ctx,NULL);
        if(! out_stream)
        {
                printf("Failed allocating output stream!\n");
                goto end;
        }

        AVCodecContext *o_codec_ctx;
        o_codec_ctx = out_stream->codec;
        o_codec_ctx->codec_id = AV_CODEC_ID_H264;
        o_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
        o_codec_ctx->codec_tag = 0;
        o_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
        o_codec_ctx->width = 640;
        o_codec_ctx->height = 480;
        o_codec_ctx->extradata = data;
        o_codec_ctx->extradata_size = size;
                
        av_dump_format(ofmt_ctx,0,stream,1);

        // 打开输出URL（Open output URL）
        if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
                ret = avio_open(&ofmt_ctx->pb, stream, AVIO_FLAG_WRITE);
                if (ret < 0) {
                        printf( "Could not open output URL '%s'", stream);
                        goto end;
                }
        }
        //写文件头（Write file header）
        ret = avformat_write_header(ofmt_ctx, NULL);
        if (ret < 0) {
                printf( "Error occurred when opening output URL\n");
                goto end;
        }
        // free(data);
        return 0;
end:
        if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
                avio_close(ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
        if (ret < 0 && ret != AVERROR_EOF) {
                printf( "Error occurred.\n");
                return -1;
        }
}
