#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <string.h>
#include <stdlib.h>
#include "rtmp.h"
                
static AVPacket pkt;
static char *outfile_name = "rtmp://192.168.2.213:1935/live/room1";
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

// int main(int argc,char* argv[])
// {
//         
//         char *outfile_name = "rtmp://192.168.2.213:1935/live/room1";
//         init_rtmp_streamer();
        
//         if((ret = avformat_open_input(&ifmt_ctx,infile_name,0,0)) < 0)
//         {
//                 fprintf(stderr, "Could not open input file '%s' %d", infile_name,ret);
//                 return -1;
//         }

//         if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
// 		fprintf(stderr, "Failed to retrieve input stream information");
// 		return -1;
//         }
//         av_dump_format(ifmt_ctx, 0, infile_name, 0);


//         start_time=av_gettime();
//         while(1)
//         {
//                 AVStream *in_stream, *out_stream;
// 		//获取一个AVPacket（Get an AVPacket）
// 		ret = av_read_frame(ifmt_ctx, &pkt);
// 		if (ret < 0)
// 			break;
//                 printf("pts=%d,dts=%d,dur=%d\n",pkt.pts,pkt.dts,pkt.duration);
// 		//FIX：No PTS (Example: Raw H.264)
// 		//Simple Write PTS
// 		if(pkt.pts==AV_NOPTS_VALUE){
// 			//Write PTS
// 			AVRational time_base1=ifmt_ctx->streams[videoindex]->time_base;
// 			//Duration between 2 frames (us)
// 			int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(ifmt_ctx->streams[videoindex]->r_frame_rate);
// 			//Parameters
// 			pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
// 			pkt.dts=pkt.pts;
// 			pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
// 		}
// 		//Important:Delay
// 		if(pkt.stream_index==videoindex){
// 			AVRational time_base=ifmt_ctx->streams[videoindex]->time_base;
// 			AVRational time_base_q={1,AV_TIME_BASE};
// 			int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
// 			int64_t now_time = av_gettime() - start_time;
//                         av_usleep(50000);
// 			if (pts_time > now_time)
//                                 av_usleep(50000);
// 				//av_usleep(pts_time - now_time);

// 		}
 
// 		in_stream  = ifmt_ctx->streams[pkt.stream_index];
// 		out_stream = ofmt_ctx->streams[pkt.stream_index];
// 		/* copy packet */
// 		//转换PTS/DTS（Convert PTS/DTS）
// 		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
// 		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
// 		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
// 		pkt.pos = -1;
// 		//Print to Screen
// 		if(pkt.stream_index==videoindex){
// 			printf("Send %8d video frames to output URL\n",frame_index);
// 			frame_index++;
// 		}
// 		//ret = av_write_frame(ofmt_ctx, &pkt);
// 		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
 
// 		if (ret < 0) {
// 			printf( "Error muxing packet\n");
// 			break;
// 		}
		
// 		av_free_packet(&pkt);

//         }
//         av_write_trailer(ofmt_ctx);

// end:
//         avformat_close_input(&ifmt_ctx);
//         if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
// 		avio_close(ofmt_ctx->pb);
// 	avformat_free_context(ofmt_ctx);
// 	if (ret < 0 && ret != AVERROR_EOF) {
// 		printf( "Error occurred.\n");
// 		return -1;
// 	}

//         return 0;
// }

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
enum AVPixelFormat get_format(struct AVCodecContext *s, const enum AVPixelFormat * fmt)
{
        printf("77888\n");
        fmt = &(s->pix_fmt);
        return s->pix_fmt;
}


int init_rtmp_streamer()
{
        int ret;
        av_register_all();

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
        AVCodecContext *codec_ctx;
        codec_ctx = avcodec_alloc_context3(0);
        
        codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
        codec_ctx->bit_rate = 0x1c;
        codec_ctx->width = 640;
        codec_ctx->height = 480;
        //codec_ctx->coded_width = 0x12345678;

        codec_ctx->coded_height = ifmt_ctx->streams[0]->codec->coded_height;
        //avcodec_set_dimensions(codec_ctx,640,480);
        codec_ctx->gop_size = 85;
        codec_ctx->pix_fmt = 35;
        codec_ctx->max_b_frames = 480;
        codec_ctx->get_format = get_format;
        printf("---------size = %d---------\n",ifmt_ctx->streams[0]->codec->coded_width);
        printf("---------size = %d---------\n",ifmt_ctx->streams[0]->codec->coded_height);
        for(int i = 0; i<60;i++)
        {
                printf("\n%3d-%3d:",i*10,i*10+10);
                for(int j=0;j<10;j++)
                {
                        printf("%.2x ",*((char *)(codec_ctx)+10*i+j));
                }
        }
        //00 0b d8 99
        // 
        // codec_ctx->bit_rate = 0x1c;
        // codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
        // codec_ctx->pix_fmt =  AV_PIX_FMT_YUV420P;
        // codec_ctx->codec_id = AV_CODEC_ID_H264;
        // codec_ctx->profile = 100;
        // codec_ctx->level = -99;
        // codec_ctx->debug = FF_DEBUG_BUGS;
        // codec_ctx->colorspace = ifmt_ctx->streams[0]->codec->colorspace;
        // codec_ctx->color_primaries = ifmt_ctx->streams[0]->codec->color_primaries;
        // codec_ctx->color_trc = ifmt_ctx->streams[0]->codec->color_trc;
        // codec_ctx->color_range = ifmt_ctx->streams[0]->codec->color_range;
        // codec_ctx->slices = ifmt_ctx->streams[0]->codec->slices;
        // codec_ctx->extradata = ifmt_ctx->streams[0]->codec->extradata;
        // codec_ctx->extradata_size = ifmt_ctx->streams[0]->codec->extradata_size;
        // codec_ctx->code
        out_stream->codec = codec_ctx;
        
        // ret = avcodec_copy_context(out_stream->codec,ifmt_ctx->streams[0]->codec);
        // if(ret < 0)
        // {
        //         printf("Fail to copy context from out stream to input stream!\n");
        //         goto end;
        // }
        

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
