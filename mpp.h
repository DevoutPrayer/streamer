#ifndef _MPP_H
#define _MPP_H

#include <stdio.h>
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
#ifdef __cplusplus
        extern "C"
        {
#endif

#define MPP_ALIGN(x, a)         (((x)+(a)-1)&~((a)-1))
#define CAM_W 640
#define CAM_H 480
#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480
#define FORCE_FORMAT 1

typedef struct {
    // global flow control flag
    RK_U32 frm_eos;
    RK_U32 pkt_eos;
    RK_U32 frame_count;
    RK_U64 stream_size;

    // base flow context
    MppCtx ctx;
    MppApi *mpi;
    MppEncPrepCfg prep_cfg;
    MppEncRcCfg rc_cfg;
    MppEncCodecCfg codec_cfg;

    // input / output
    MppBuffer frm_buf;
    MppEncSeiMode sei_mode;

    // paramter for resource malloc
    RK_U32 width;
    RK_U32 height;
    RK_U32 hor_stride;
    RK_U32 ver_stride;
    MppFrameFormat fmt;
    MppCodingType type;
    RK_U32 num_frames;

    // resources
    size_t frame_size;
    /* NOTE: packet buffer may overflow */
    size_t packet_size;

    // rate control runtime parameter
    RK_S32 gop;
    RK_S32 fps;
    RK_S32 bps;
    FILE *fp_output;
    FILE *fp_outputx;

    int (*write_frame)(uint8_t*data,int size);
} MpiEncData;


void init_mpp(MpiEncData *mpp_enc_data);
_Bool process_image(uint8_t *p, int size,MpiEncData *mpp_enc_data);
_Bool write_header(MpiEncData *mpp_enc_data);
extern int write_frame(uint8_t*data,int size);


#ifdef __cplusplus
        }
#endif


#endif /* !_MPP_H */