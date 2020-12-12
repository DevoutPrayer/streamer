#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <string.h>
#include <stdlib.h>
#include "rtmp.h"
                
static AVPacket pkt;
static char *outfile_name = "rtmp://192.168.1.165:1935/live/room1";
static AVOutputFormat *ofmt = NULL;
static AVFormatContext *ofmt_ctx = NULL;
static AVFormatContext *ifmt_ctx = NULL;
static int ret;
static int i;
static int videoindex=0;
static int frame_index=0;
static int64_t start_time=0;
static char *infile_name = "/tmp/output.h264";
static int ceshi;

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
        ret = av_write_frame(ofmt_ctx, &pkt);
 
        if (ret < 0) {
                printf( "Error muxing packet\n");
                return -1;
        }
        return 0;
}



int init_rtmp_streamer(char* stream)
{
        int ret;
        av_register_all();
outfile_name=stream;
        if((ret = avformat_network_init()) < 0)
        {
                fprintf(stderr, "avformat_network_init failed!");
                return -1;
        }

        avformat_alloc_output_context2(&ofmt_ctx,NULL,"flv",outfile_name);
        if(!ofmt_ctx)
        {
                fprintf(stderr, "Could not create output context\n");
                return -1;
        }
        ofmt = ofmt_ctx->oformat;

        if((ret = avformat_open_input(&ifmt_ctx,infile_name,0,0)) < 0)
        {
                fprintf(stderr, "Could not open input file '%s' %d", infile_name,ret);
                return -1;
        }

        if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		fprintf(stderr, "Failed to retrieve input stream information");
		return -1;
        }
        av_dump_format(ifmt_ctx, 0, infile_name, 0);



        AVStream *out_stream = avformat_new_stream(ofmt_ctx,NULL);
        if(! out_stream)
        {
                printf("Failed allocating output stream!\n");
                goto end;
        }

        ret = avcodec_copy_context(out_stream->codec,ifmt_ctx->streams[0]->codec);
        if(ret < 0)
        {
                printf("Fail to copy context from out stream to input stream!\n");
                goto end;
        }
        

        out_stream->codec->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


           
        av_dump_format(ofmt_ctx,0,outfile_name,1);

        //打开输出URL（Open output URL）
	if (!(ofmt->flags & AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx->pb, outfile_name, AVIO_FLAG_WRITE);
		if (ret < 0) {
			printf( "Could not open output URL '%s'", outfile_name);
			goto end;
		}
	}
        //写文件头（Write file header）
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0) {
		printf( "Error occurred when opening output URL\n");
		goto end;
	}
        return 0;
end:
        if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	if (ret < 0 && ret != AVERROR_EOF) {
		printf( "Error occurred.\n");
		return -1;
	}
}
