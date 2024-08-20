/*
 * sample-common.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>

//#include "logodata_100x100_bgra.h"

#include "sample-common.h"

#define TAG "Sample-Common"

static const IMPEncoderRcMode S_RC_METHOD = IMP_ENC_RC_MODE_CAPPED_QUALITY;





#define LOW_BITSTREAM
//#define SHOW_FRM_BITRATE
#ifdef SHOW_FRM_BITRATE
#define FRM_BIT_RATE_TIME 2
#define STREAM_TYPE_NUM 3
static int frmrate_sp[STREAM_TYPE_NUM] = { 0 };
static int statime_sp[STREAM_TYPE_NUM] = { 0 };
static int bitrate_sp[STREAM_TYPE_NUM] = { 0 };
#endif

struct chn_conf chn[FS_CHN_NUM] = {
	{
		.index = CH0_INDEX,
		.enable = CHN0_EN,
    .payloadType = IMP_ENC_PROFILE_AVC_MAIN,
		.fs_chn_attr = {
			.picWidth = SENSOR_WIDTH,
			.picHeight = SENSOR_HEIGHT,
			.pixFmt = PIX_FMT_NV12,
			.crop={ 
			.enable = CROP_EN,
			.left = 0,
			.top = 0,
			.width = SENSOR_WIDTH,
			.height = SENSOR_HEIGHT
		     },
			.scaler= 
			 {
			 	.enable = 0
			 },
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL
		

		
		   },
		.framesource_chn =	{ DEV_ID_FS, CH0_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH0_INDEX, 0},
	},
	{
		.index = CH1_INDEX,
		.enable = CHN1_EN,
    .payloadType = IMP_ENC_PROFILE_AVC_MAIN,
		.fs_chn_attr = {
			.picWidth = SENSOR_WIDTH_THIRD,
			.picHeight = SENSOR_HEIGHT_THIRD,
			.pixFmt = PIX_FMT_NV12,

			.crop={ 
			.enable = 0,
			.left = 0,
			.top = 0,
			.width = SENSOR_WIDTH_THIRD,
			.height = SENSOR_HEIGHT_THIRD
		     },
		     .scaler= 
			 {
			 	.enable = 1,
			 	.outwidth = SENSOR_WIDTH_THIRD,
				.outheight = SENSOR_HEIGHT_THIRD

			 },

			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			
		   },
		.framesource_chn =	{ DEV_ID_FS, CH1_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH1_INDEX, 0},
	},
	{
		.index = CH2_INDEX,
		.enable = CHN2_EN,
    .payloadType = IMP_ENC_PROFILE_AVC_MAIN,
		.fs_chn_attr = {
			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
			.pixFmt = PIX_FMT_NV12,
			.crop={ 
			.enable = 0,
			.left = 0,
			.top = 0,
			.width = SENSOR_WIDTH_SECOND,
			.height = SENSOR_HEIGHT_SECOND
		     },

		     .scaler= 
			 {
			 	.enable = 1,
			 	.outwidth = SENSOR_WIDTH_SECOND,
				.outheight = SENSOR_HEIGHT_SECOND

			 },

			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL

		
		   },
		.framesource_chn =	{ DEV_ID_FS, CH2_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH2_INDEX, 0},
	},
	{
		.index = CH3_INDEX,
		.enable = CHN3_EN,
    .payloadType = IMP_ENC_PROFILE_AVC_MAIN,
		.fs_chn_attr = {
			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
			.pixFmt = PIX_FMT_NV12,

			.crop={ 
			.enable = 0,
			.left = 0,
			.top = 0,
			.width = SENSOR_WIDTH_SECOND,
			.height = SENSOR_HEIGHT_SECOND
		     },

		     .scaler= 
			 {
			 	.enable = 1,
			 	.outwidth = SENSOR_WIDTH_SECOND,
				.outheight = SENSOR_HEIGHT_SECOND

			 },


			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_EXT_CHANNEL

			
		   },
		.framesource_chn =	{ DEV_ID_FS, CH3_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH3_INDEX, 0},
	},
};

struct chn_conf chn_ext_hsv[1] = {
	{
		.index = 0,
 		.enable = 0,
        .payloadType = 0,
		.fs_chn_attr = {
			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
			.pixFmt = PIX_FMT_HSV,

			.crop={ 
			.enable = 0,
			.left = 0,
			.top = 0,
			.width = SENSOR_WIDTH_SECOND,
			.height = SENSOR_HEIGHT_SECOND
		     },

		     .scaler= 
			 {
			 	.enable = 1,
			 	.outwidth = SENSOR_WIDTH_SECOND,
				.outheight = SENSOR_HEIGHT_SECOND

			 },


			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 3,
			.type = FS_EXT_CHANNEL

		
		},
	},
};



struct chn_conf chn_ext_rgba[1] = {
	{
		.index = 0,
 		.enable = 0,
        .payloadType = 0,

		.fs_chn_attr = {
			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,

			.pixFmt = PIX_FMT_RGBA,

			.crop={ 
			.enable = 0,
			.left = 0,
			.top = 0,
			.width = SENSOR_WIDTH_SECOND,
			.height = SENSOR_HEIGHT_SECOND
		     },

		     .scaler= 
			 {
			 	.enable = 1,
			 	.outwidth = SENSOR_WIDTH_SECOND,
				.outheight = SENSOR_HEIGHT_SECOND

			 },

			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 3,
			.type = FS_EXT_CHANNEL,

		},

	},
};

//extern int IMP_OSD_SetPoolSize(int size);

IMPSensorInfo sensor_info;
int sample_system_init()
{
	int ret = 0;

	IMP_OSD_SetPoolSize(512*1024);

/*
 *    IMP_System_MemPoolRequest(0, 12 * ( 1 << 20 ), "mempool0");
 *    IMP_System_MemPoolRequest(1, 10 * ( 1 << 20 ), "mempool1");
 *
 *    IMP_Encoder_SetPool(0, 0);
 *    IMP_Encoder_SetPool(1, 1);
 *
 *    IMP_FrameSource_SetPool(0, 0);
 *    IMP_FrameSource_SetPool(1, 1);
 *
 *    IMP_System_MemPoolFree(1);
 *    IMP_System_MemPoolFree(0);
 */

	memset(&sensor_info, 0, sizeof(IMPSensorInfo));
	memcpy(sensor_info.name, SENSOR_NAME, sizeof(SENSOR_NAME));
	sensor_info.cbus_type = SENSOR_CUBS_TYPE;
	memcpy(sensor_info.i2c.type, SENSOR_NAME, sizeof(SENSOR_NAME));
	sensor_info.i2c.addr = SENSOR_I2C_ADDR;

	IMP_LOG_DBG(TAG, "sample_system_init start\n");

	ret = IMP_ISP_Open();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	ret = IMP_ISP_AddSensor(&sensor_info);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

	ret = IMP_ISP_EnableSensor();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	ret = IMP_System_Init();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_System_Init failed\n");
		return -1;
	}

	/* enable turning, to debug graphics */
	ret = IMP_ISP_EnableTuning();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
		return -1;
	}
    IMP_ISP_Tuning_SetContrast(128);
    IMP_ISP_Tuning_SetSharpness(128);
    IMP_ISP_Tuning_SetSaturation(128);
    IMP_ISP_Tuning_SetBrightness(128);
#if 1
    ret = IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
    if (ret < 0){
        IMP_LOG_ERR(TAG, "failed to set running mode\n");
        return -1;
    }
#endif
#if 0
    ret = IMP_ISP_Tuning_SetSensorFPS(SENSOR_FRAME_RATE_NUM, SENSOR_FRAME_RATE_DEN);
    if (ret < 0){
        IMP_LOG_ERR(TAG, "failed to set sensor fps\n");
        return -1;
    }
#endif
	IMP_LOG_DBG(TAG, "ImpSystemInit success\n");

	return 0;
}

int sample_system_exit()
{
	int ret = 0;

	IMP_LOG_DBG(TAG, "sample_system_exit start\n");


	IMP_System_Exit();

	ret = IMP_ISP_DisableSensor();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	ret = IMP_ISP_DelSensor(&sensor_info);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

	ret = IMP_ISP_DisableTuning();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_ISP_DisableTuning failed\n");
		return -1;
	}

	if(IMP_ISP_Close()){
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	IMP_LOG_DBG(TAG, " sample_system_exit success\n");

	return 0;
}

int sample_framesource_streamon()
{
	int ret = 0, i = 0;
	/* Enable channels */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_FrameSource_EnableChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[i].index);
				return -1;
			}
		}
	}
	return 0;
}



int sample_framesource_ext_rgba_streamon()
{
	int ret = 0;
	/* Enable channels */
	ret = IMP_FrameSource_EnableChn(3);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, 3);
		return -1;
	}
	return 0;
}

int sample_framesource_streamoff()
{
	int ret = 0, i = 0;
	/* Enable channels */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable){
			ret = IMP_FrameSource_DisableChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[i].index);
				return -1;
			}
		}
	}
	return 0;
}



int sample_framesource_ext_rgba_streamoff()
{
	int ret = 0;
	/* Enable channels */
	ret = IMP_FrameSource_DisableChn(3);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, 3);
		return -1;
	}
	return 0;
}



int sample_framesource_init()
{
	int i, ret;

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr);
			if(ret < 0){
				IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", chn[i].index);
				return -1;
			}

			ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n",  chn[i].index);
				return -1;
			}
		}
	}

	return 0;
}



int sample_framesource_ext_rgba_init()
{
	int ret;
    chn[3].enable = 0;

	ret = IMP_FrameSource_CreateChn(3, &chn_ext_rgba[0].fs_chn_attr);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", 3);
		return -1;
	}

	ret = IMP_FrameSource_SetSource(3, 2);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_FrameSource_SetSource(chn%d) error !\n", 3);
		return -1;
	}

	ret = IMP_FrameSource_SetChnAttr(3, &chn_ext_rgba[0].fs_chn_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n", 3);
		return -1;
	}
	return 0;
}

int sample_framesource_exit()
{
	int ret,i;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			/*Destroy channel */
			ret = IMP_FrameSource_DestroyChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn[i].index, ret);
				return -1;
			}
		}
	}
	return 0;
}



int sample_framesource_ext_rgba_exit()
{
	int ret;

	ret = IMP_FrameSource_DestroyChn(3);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn() error: %d\n", ret);
		return -1;
	}
	return 0;
}

int sample_jpeg_init()
{
	int i, ret;
	IMPEncoderChnAttr channel_attr;
	IMPFSChnAttr *imp_chn_attr_tmp;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			imp_chn_attr_tmp = &chn[i].fs_chn_attr;
			memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
			ret = IMP_Encoder_SetDefaultParam(&channel_attr, IMP_ENC_PROFILE_JPEG, IMP_ENC_RC_MODE_FIXQP,
					imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight,
					imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen, 0, 0, 25, 0);

			/* Create Channel */
			ret = IMP_Encoder_CreateChn(4 + chn[i].index, &channel_attr);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error: %d\n",
							chn[i].index, ret);
				return -1;
			}

			/* Resigter Channel */
			ret = IMP_Encoder_RegisterChn(i, 4 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(0, %d) error: %d\n",
							chn[i].index, ret);
				return -1;
			}
		}
	}

	return 0;
}

int sample_encoder_init()
{
	int i, ret, chnNum = 0;
	IMPFSChnAttr *imp_chn_attr_tmp;
	IMPEncoderChnAttr channel_attr;

    for (i = 0; i <  FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            imp_chn_attr_tmp = &chn[i].fs_chn_attr;
            chnNum = chn[i].index;

            memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));

			float ratio = 1;
			if (((uint64_t)imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) > (1280 * 720)) {
				ratio = log10f(((uint64_t)imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720.0)) + 1;
			} else {
				ratio = 1.0 / (log10f((1280 * 720.0) / ((uint64_t)imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight)) + 1);
			}
			ratio = ratio > 0.1 ? ratio : 0.1;
			unsigned int uTargetBitRate = BITRATE_720P_Kbs * ratio;

            ret = IMP_Encoder_SetDefaultParam(&channel_attr, chn[i].payloadType, S_RC_METHOD,
                    imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight,
                    imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen,
                    imp_chn_attr_tmp->outFrmRateNum * 2 / imp_chn_attr_tmp->outFrmRateDen, 2,
                    (S_RC_METHOD == IMP_ENC_RC_MODE_FIXQP) ? 35 : -1,
                    uTargetBitRate);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_SetDefaultParam(%d) error !\n", chnNum);
                return -1;
            }
#ifdef LOW_BITSTREAM
			IMPEncoderRcAttr *rcAttr = &channel_attr.rcAttr;
			uTargetBitRate /= 2;

			switch (rcAttr->attrRcMode.rcMode) {
				case IMP_ENC_RC_MODE_FIXQP:
					rcAttr->attrRcMode.attrFixQp.iInitialQP = 38;
					break;
				case IMP_ENC_RC_MODE_CBR:
					rcAttr->attrRcMode.attrCbr.uTargetBitRate = uTargetBitRate;
					rcAttr->attrRcMode.attrCbr.iInitialQP = -1;
					rcAttr->attrRcMode.attrCbr.iMinQP = 34;
					rcAttr->attrRcMode.attrCbr.iMaxQP = 51;
					rcAttr->attrRcMode.attrCbr.iIPDelta = -1;
					rcAttr->attrRcMode.attrCbr.iPBDelta = -1;
					rcAttr->attrRcMode.attrCbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
					rcAttr->attrRcMode.attrCbr.uMaxPictureSize = uTargetBitRate * 4 / 3;
					break;
				case IMP_ENC_RC_MODE_VBR:
					rcAttr->attrRcMode.attrVbr.uTargetBitRate = uTargetBitRate;
					rcAttr->attrRcMode.attrVbr.uMaxBitRate = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrVbr.iInitialQP = -1;
					rcAttr->attrRcMode.attrVbr.iMinQP = 34;
					rcAttr->attrRcMode.attrVbr.iMaxQP = 51;
					rcAttr->attrRcMode.attrVbr.iIPDelta = -1;
					rcAttr->attrRcMode.attrVbr.iPBDelta = -1;
					rcAttr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
					rcAttr->attrRcMode.attrVbr.uMaxPictureSize = uTargetBitRate * 4 / 3;
					break;
				case IMP_ENC_RC_MODE_CAPPED_VBR:
					rcAttr->attrRcMode.attrCappedVbr.uTargetBitRate = uTargetBitRate;
					rcAttr->attrRcMode.attrCappedVbr.uMaxBitRate = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrCappedVbr.iInitialQP = -1;
					rcAttr->attrRcMode.attrCappedVbr.iMinQP = 34;
					rcAttr->attrRcMode.attrCappedVbr.iMaxQP = 51;
					rcAttr->attrRcMode.attrCappedVbr.iIPDelta = -1;
					rcAttr->attrRcMode.attrCappedVbr.iPBDelta = -1;
					rcAttr->attrRcMode.attrCappedVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
					rcAttr->attrRcMode.attrCappedVbr.uMaxPictureSize = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrCappedVbr.uMaxPSNR = 42;
					break;
				case IMP_ENC_RC_MODE_CAPPED_QUALITY:
					rcAttr->attrRcMode.attrCappedQuality.uTargetBitRate = uTargetBitRate;
					rcAttr->attrRcMode.attrCappedQuality.uMaxBitRate = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrCappedQuality.iInitialQP = -1;
					rcAttr->attrRcMode.attrCappedQuality.iMinQP = 34;
					rcAttr->attrRcMode.attrCappedQuality.iMaxQP = 51;
					rcAttr->attrRcMode.attrCappedQuality.iIPDelta = -1;
					rcAttr->attrRcMode.attrCappedQuality.iPBDelta = -1;
					rcAttr->attrRcMode.attrCappedQuality.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
					rcAttr->attrRcMode.attrCappedQuality.uMaxPictureSize = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrCappedQuality.uMaxPSNR = 42;
					break;
				case IMP_ENC_RC_MODE_INVALID:
					IMP_LOG_ERR(TAG, "unsupported rcmode:%d, we only support fixqp, cbr vbr and capped vbr\n", rcAttr->attrRcMode.rcMode);
					return -1;
			}
#endif

            ret = IMP_Encoder_CreateChn(chnNum, &channel_attr);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", chnNum);
                return -1;
            }

			ret = IMP_Encoder_RegisterChn(chn[i].index, chnNum);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", chn[i].index, chnNum, ret);
				return -1;
			}
		}
	}

	return 0;
}

int sample_jpeg_exit(void)
{
	int ret = 0, i = 0, chnNum = 0;
	IMPEncoderChnStat chn_stat;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			chnNum = 4 + chn[i].index;
			memset(&chn_stat, 0, sizeof(IMPEncoderChnStat));
			ret = IMP_Encoder_Query(chnNum, &chn_stat);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_Query(%d) error: %d\n", chnNum, ret);
				return -1;
			}

			if (chn_stat.registered) {
				ret = IMP_Encoder_UnRegisterChn(chnNum);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "IMP_Encoder_UnRegisterChn(%d) error: %d\n", chnNum, ret);
					return -1;
				}

				ret = IMP_Encoder_DestroyChn(chnNum);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyChn(%d) error: %d\n", chnNum, ret);
					return -1;
				}
			}
		}
	}

	return 0;
}


int sample_encoder_exit(void)
{
    int ret = 0, i = 0, chnNum = 0;
    IMPEncoderChnStat chn_stat;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
            chnNum = chn[i].index;
            memset(&chn_stat, 0, sizeof(IMPEncoderChnStat));
            ret = IMP_Encoder_Query(chnNum, &chn_stat);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_Query(%d) error: %d\n", chnNum, ret);
                return -1;
            }

            if (chn_stat.registered) {
                ret = IMP_Encoder_UnRegisterChn(chnNum);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_Encoder_UnRegisterChn(%d) error: %d\n", chnNum, ret);
                    return -1;
                }

                ret = IMP_Encoder_DestroyChn(chnNum);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyChn(%d) error: %d\n", chnNum, ret);
                    return -1;
                }

                ret = IMP_Encoder_DestroyGroup(chnNum);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyGroup(%d) error: %d\n", chnNum, ret);
                    return -1;
                }
            }
        }
    }

    return 0;
}





static int save_stream(int fd, IMPEncoderStream *stream)
{
	int ret, i, nr_pack = stream->packCount;

  //IMP_LOG_DBG(TAG, "----------packCount=%d, stream->seq=%u start----------\n", stream->packCount, stream->seq);
	for (i = 0; i < nr_pack; i++) {
    //IMP_LOG_DBG(TAG, "[%d]:%10u,%10lld,%10u,%10u,%10u\n", i, stream->pack[i].length, stream->pack[i].timestamp, stream->pack[i].frameEnd, *((uint32_t *)(&stream->pack[i].nalType)), stream->pack[i].sliceType);
		IMPEncoderPack *pack = &stream->pack[i];
		if(pack->length){
			uint32_t remSize = stream->streamSize - pack->offset;
			if(remSize < pack->length){
				ret = write(fd, (void *)(stream->virAddr + pack->offset), remSize);
				if (ret != remSize) {
					IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].remSize(%d) error:%s\n", ret, i, remSize, strerror(errno));
					return -1;
				}
				ret = write(fd, (void *)stream->virAddr, pack->length - remSize);
				if (ret != (pack->length - remSize)) {
					IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].(length-remSize)(%d) error:%s\n", ret, i, (pack->length - remSize), strerror(errno));
					return -1;
				}
			}else {
				ret = write(fd, (void *)(stream->virAddr + pack->offset), pack->length);
				if (ret != pack->length) {
					IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].length(%d) error:%s\n", ret, i, pack->length, strerror(errno));
					return -1;
				}
			}
		}
	}
  //IMP_LOG_DBG(TAG, "----------packCount=%d, stream->seq=%u end----------\n", stream->packCount, stream->seq);
	return 0;
}



static void *get_video_stream(void *args)
{
  int val, i, chnNum, ret;
  char stream_path[64];
  IMPEncoderEncType encType;
  int stream_fd = -1, totalSaveCnt = 0;

  val = (int)args;
  chnNum = val & 0xffff;
  encType = (val >> 16) & 0xffff;

  ret = IMP_Encoder_StartRecvPic(chnNum);
  if (ret < 0) {
    IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", chnNum);
    return ((void *)-1);
  }

  sprintf(stream_path, "%s/stream-%d.%s", STREAM_FILE_PATH_PREFIX, chnNum,
      (encType == IMP_ENC_TYPE_AVC) ? "h264" : ((encType == IMP_ENC_TYPE_HEVC) ? "h265" : "jpeg"));

  if (encType == IMP_ENC_TYPE_JPEG) {
    totalSaveCnt = ((NR_FRAMES_TO_SAVE / 50) > 0) ? (NR_FRAMES_TO_SAVE / 50) : 1;
  } else {
    IMP_LOG_DBG(TAG, "Video ChnNum=%d Open Stream file %s ", chnNum, stream_path);
    stream_fd = open(stream_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
    if (stream_fd < 0) {
      IMP_LOG_ERR(TAG, "failed: %s\n", strerror(errno));
      return ((void *)-1);
    }
    IMP_LOG_DBG(TAG, "OK\n");
    totalSaveCnt = NR_FRAMES_TO_SAVE;
  }

  for (i = 0; i < totalSaveCnt; i++) {
    ret = IMP_Encoder_PollingStream(chnNum, 1000);
    if (ret < 0) {
      IMP_LOG_ERR(TAG, "IMP_Encoder_PollingStream(%d) timeout\n", chnNum);
      continue;
    }

    IMPEncoderStream stream;
    /* Get H264 or H265 Stream */
    ret = IMP_Encoder_GetStream(chnNum, &stream, 1);
#ifdef SHOW_FRM_BITRATE
    int i, len = 0;
    for (i = 0; i < stream.packCount; i++) {
      len += stream.pack[i].length;
    }
    bitrate_sp[chnNum] += len;
    frmrate_sp[chnNum]++;

    int64_t now = IMP_System_GetTimeStamp() / 1000;
    if(((int)(now - statime_sp[chnNum]) / 1000) >= FRM_BIT_RATE_TIME){
      double fps = (double)frmrate_sp[chnNum] / ((double)(now - statime_sp[chnNum]) / 1000);
      double kbr = (double)bitrate_sp[chnNum] * 8 / (double)(now - statime_sp[chnNum]);

      printf("streamNum[%d]:FPS: %0.2f,Bitrate: %0.2f(kbps)\n", chnNum, fps, kbr);
      //fflush(stdout);

      frmrate_sp[chnNum] = 0;
      bitrate_sp[chnNum] = 0;
      statime_sp[chnNum] = now;
    }
#endif
    if (ret < 0) {
      IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream(%d) failed\n", chnNum);
      return ((void *)-1);
    }

   
#if 1
    {
      ret = save_stream(stream_fd, &stream);
      if (ret < 0) {
        close(stream_fd);
        return ((void *)ret);
      }
    }
#endif
    IMP_Encoder_ReleaseStream(chnNum, &stream);
  }

  close(stream_fd);

  ret = IMP_Encoder_StopRecvPic(chnNum);
  if (ret < 0) {
    IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic(%d) failed\n", chnNum);
    return ((void *)-1);
  }

  return ((void *)0);
}

int sample_get_video_stream()
{
	unsigned int i;
	int ret;
	pthread_t tid[FS_CHN_NUM];

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
            int arg = 0;
            if (chn[i].payloadType == IMP_ENC_PROFILE_JPEG) {
                arg = (((chn[i].payloadType >> 24) << 16) | (4 + chn[i].index));
            } else {
                arg = (((chn[i].payloadType >> 24) << 16) | chn[i].index);
            }
			ret = pthread_create(&tid[i], NULL, get_video_stream, (void *)arg);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Create ChnNum%d get_video_stream failed\n", (chn[i].payloadType == IMP_ENC_PROFILE_JPEG) ? (4 + chn[i].index) : chn[i].index);
			}
		}
	}

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			pthread_join(tid[i],NULL);
		}
	}

	return 0;
}


int sample_get_jpeg_snap()
{
	int i, ret;
	char snap_path[64];

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_Encoder_StartRecvPic(4 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", 3 + chn[i].index);
				return -1;
			}

			sprintf(snap_path, "%s/snap-%d.jpg",
					SNAP_FILE_PATH_PREFIX, chn[i].index);

			IMP_LOG_ERR(TAG, "Open Snap file %s ", snap_path);
			int snap_fd = open(snap_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
			if (snap_fd < 0) {
				IMP_LOG_ERR(TAG, "failed: %s\n", strerror(errno));
				return -1;
			}
			IMP_LOG_DBG(TAG, "OK\n");

			/* Polling JPEG Snap, set timeout as 1000msec */
			ret = IMP_Encoder_PollingStream(4 + chn[i].index, 10000);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Polling stream timeout\n");
				continue;
			}

			IMPEncoderStream stream;
			/* Get JPEG Snap */
			ret = IMP_Encoder_GetStream(chn[i].index + 4, &stream, 1);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream() failed\n");
				return -1;
			}

			ret = save_stream(snap_fd, &stream);
			if (ret < 0) {
				close(snap_fd);
				return ret;
			}

			IMP_Encoder_ReleaseStream(4 + chn[i].index, &stream);

			close(snap_fd);

			ret = IMP_Encoder_StopRecvPic(4 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic() failed\n");
				return -1;
			}
		}
	}
	return 0;
}


int sample_SetIRCUT(int enable)
{
	int fd, fd52, fd53;
	char on[4], off[4];

	// if (!access("/tmp/setir",0)) {
	// 	if (enable) {
	// 		system("/tmp/setir 0 1");
	// 	} else {
	// 		system("/tmp/setir 1 0");
	// 	}
	// 	return 0;
	// }

	// fd = open("/sys/class/gpio/export", O_WRONLY);
	// if(fd < 0) {
	// 	IMP_LOG_DBG(TAG, "open /sys/class/gpio/export error !");
	// 	return -1;
	// }

	// write(fd, "52", 2);
	// write(fd, "53", 2);

	// close(fd);

	// fd52 = open("/sys/class/gpio/gpio52/direction", O_RDWR);
	// if(fd52 < 0) {
	// 	IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio52/direction error !");
	// 	return -1;
	// }

	// fd53 = open("/sys/class/gpio/gpio53/direction", O_RDWR);
	// if(fd53 < 0) {
	// 	IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio53/direction error !");
	// 	return -1;
	// }

	// write(fd52, "out", 3);
	// write(fd53, "out", 3);

	// close(fd52);
	// close(fd53);

	// fd52 = open("/sys/class/gpio/gpio52/active_low", O_RDWR);
	// if(fd52 < 0) {
	// 	IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio52/active_low error !");
	// 	return -1;
	// }

	// fd53 = open("/sys/class/gpio/gpio53/active_low", O_RDWR);
	// if(fd53 < 0) {
	// 	IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio53/active_low error !");
	// 	return -1;
	// }

	// write(fd52, "0", 1);
	// write(fd53, "0", 1);

	// close(fd52);
	// close(fd53);

	fd52 = open("/sys/class/gpio/gpio52/value", O_RDWR);
	if(fd52 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio52/value error !");
		return -1;
	}

	fd53 = open("/sys/class/gpio/gpio53/value", O_RDWR);
	if(fd53 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio53/value error !");
		return -1;
	}

	sprintf(on, "%d", enable);
	sprintf(off, "%d", !enable);

	write(fd52, "0", 1);
	usleep(10*1000);

	write(fd52, on, strlen(on));
	write(fd53, off, strlen(off));

	if (!enable) {
		usleep(10*1000);
		write(fd52, off, strlen(off));
	}

	close(fd52);
	close(fd53);

	return 0;
}

static int  g_soft_ps_running = 1;
void *sample_soft_photosensitive_ctrl(void *p)
{
	int i = 0;
	float gb_gain,gr_gain;
	float iso_buf;
	bool ircut_status = true;
	g_soft_ps_running = 1;
	int night_count = 0;
	int day_count = 0;
	//int day_oth_count = 0;
	//(g/b) statistic in bayer region
	float gb_gain_record = 200;
	float gr_gain_record = 200;
	float gb_gain_buf = 200, gr_gain_buf = 200;
	IMPISPRunningMode pmode;
	IMPISPEVAttr ExpAttr;
	IMPISPWB wb;
	IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
	sample_SetIRCUT(1);

	while (g_soft_ps_running) {
		//obtain exposure AE information
		int ret = IMP_ISP_Tuning_GetEVAttr(&ExpAttr);
		if (ret ==0) {
			printf("u32ExposureTime: %d\n", ExpAttr.ev);
			printf("u32AnalogGain: %d\n", ExpAttr.again);
			printf("u32DGain: %d\n", ExpAttr.dgain);
		} else {
			return NULL;
        }
		iso_buf = ExpAttr.ev;
		printf(" iso buf ==%f\n",iso_buf);
		ret = IMP_ISP_Tuning_GetWB_Statis(&wb);
		if (ret == 0) {
			gr_gain =wb.rgain;
			gb_gain =wb.bgain;
			// printf("gb_gain: %f\n", gb_gain);
			// printf("gr_gain: %f\n", gr_gain);
			// printf("gr_gain_record: %f\n", gr_gain_record);
		} else {
			return NULL;
        }

		//If the average brightness is less than 20, switches to night vision mode
		if (iso_buf >1900000) {
			night_count++;
			printf("night_count==%d\n",night_count);
			if (night_count>5) {
				IMP_ISP_Tuning_GetISPRunningMode(&pmode);
				if (pmode!=IMPISP_RUNNING_MODE_NIGHT) {
					printf("### entry night mode ###\n");
					IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_NIGHT);
					sample_SetIRCUT(0);
					ircut_status = true;
				}
				//After switching to night vision, take the minimum value of 20 gb_gain as the reference value for switching to day gb_gain_record，the gb_gain is bayer's G/B
				for (i=0; i<20; i++) {
					IMP_ISP_Tuning_GetWB_GOL_Statis(&wb);
					gr_gain =wb.rgain;
					gb_gain =wb.bgain;
					if (i==0) {
						gb_gain_buf = gb_gain;
						gr_gain_buf = gr_gain;
					}
					gb_gain_buf = ((gb_gain_buf>gb_gain)?gb_gain:gb_gain_buf);
					gr_gain_buf = ((gr_gain_buf>gr_gain)?gr_gain:gr_gain_buf);
					usleep(300000);
					gb_gain_record = gb_gain_buf;
					gr_gain_record = gr_gain_buf;
					// printf("gb_gain == %f,iso_buf=%f",gb_gain,iso_buf);
					// printf("gr_gain_record == %f\n ",gr_gain_record);
				}
			}
		} else {
			night_count = 0;
        }
		//Meeting these three conditions, enter the daytime switching judgment condition
		if (((int)iso_buf < 479832) &&(ircut_status == true) &&(gb_gain>gb_gain_record+15)) {
			if ((iso_buf<361880)||(gb_gain >145)) {
				day_count++;
            } else {
				day_count=0;
            }
			// printf("gr_gain_record == %f gr_gain =%f line=%d\n",gr_gain_record,gr_gain,__LINE__);
			// printf("day_count == %d\n",day_count);
			if (day_count>3) {
				printf("### entry day mode ###\n");
				IMP_ISP_Tuning_GetISPRunningMode(&pmode);
				if (pmode!=IMPISP_RUNNING_MODE_DAY) {
					IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
					sample_SetIRCUT(1);
					ircut_status = false;
				}
			}
		} else {
            day_count = 0;
        }
		sleep(1);
	}
	return NULL;
}

