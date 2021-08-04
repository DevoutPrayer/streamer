#ifndef _MPP_H
#define _MPP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <rockchip/rk_mpi.h>
#ifdef __cplusplus
        extern "C"
        {
#endif

#define MPP_ALIGN(x, a)         (((x)+(a)-1)&~((a)-1))

typedef struct {
        uint8_t *data;
        uint32_t size;
} SpsHeader;

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
        //call back function
        int (*write_frame)(uint8_t*data,int size);
        //function pointer
        void (*init_mpp)(void *mpp_enc_data);
        _Bool (*process_image)(uint8_t *p, int size,void *mpp_enc_data);
        _Bool (*write_header)(void *mpp_enc_data,SpsHeader *sps_header);
        void (*close)(void* ctx);

} MppContext;

MppContext* alloc_mpp_context();

#ifdef __cplusplus
        }
#endif


#endif /* !_MPP_H */