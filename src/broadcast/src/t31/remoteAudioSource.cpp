

#include <imp/imp_audio.h>
#include "base/logger.h"

 

#include <sys/ioctl.h>

#define AEC_SAMPLE_RATE 8000



namespace base
{
namespace web_rtc
{
int devID = 0;
int adChn = 0;


int audioPlayerAcquireStream()
{
    

    if (IMP_AO_Enable(devID)) {
        SError <<"IMP_AO_Enable failed";
        return -EAGAIN;
    }

    if (IMP_AO_EnableChn(devID, adChn)) {
        SError <<"IMP_AO_EnableChn failed";
        return -EAGAIN;
    }

    if (IMP_AO_SetVol(devID, adChn, 90)) {
        SError <<"IMP_AO_SetVol failed";
        return -EAGAIN;
    }

    return 0;
}


int audioPlayerSetFormat( )
{
	

	int speakCtlFd = open("/dev/speakerctl", O_RDWR);

	if (-1 == speakCtlFd)
	{
		//printf("err: open pa fail\n");

		SError << "opening /dev/speakerctl failed";

		return -1;
	}

	unsigned long flag = 0;
	speakCtlFd = ioctl(speakCtlFd, 1,  &flag);


	IMPAudioDecChnAttr chnAttr;
	chnAttr.type = PT_G711A;
	chnAttr.bufSize = 20;
	chnAttr.mode = ADEC_MODE_PACK;


	
	IMPAudioIOAttr attr;
	attr.samplerate = AUDIO_SAMPLE_RATE_8000;
	attr.bitwidth = AUDIO_BIT_WIDTH_16;
	attr.soundmode = AUDIO_SOUND_MODE_MONO;
	attr.frmNum = 20;
	attr.numPerFrm = 400;
	attr.chnCnt = 1;
        
	int ret = IMP_AO_SetPubAttr(devID, &attr);
	if(ret != 0) {
		SError <<  "set ao  attr err devid " <<   devID << " ret " <<  ret;
		return -1;
	}

	memset(&attr, 0x0, sizeof(attr));
	ret = IMP_AO_GetPubAttr(devID, &attr);
	if(ret != 0) {
		SError << "get ao attr err: devid " <<   devID << " ret " <<  ret;
		return -1;
	}

	SInfo <<  "samplerate:" <<  attr.samplerate;
	SInfo <<  "bitwidth:" <<  attr.bitwidth;
	SInfo <<  "soundmode:" << attr.soundmode;
	SInfo <<  "frmNum:" << attr.frmNum;
	SInfo <<  "numPerFrm:" <<  attr.numPerFrm;
	SInfo <<  "chnCnt:" << attr.chnCnt;

	/* Step 2: enable AO device. */
	ret = IMP_AO_Enable(devID);
	if(ret != 0) {
		SError << "enable ao devid " <<   devID << " ret " <<  ret;
		return -1;
	}

	/* Step 3: enable AI channel. */
	int chnID = 0;
	ret = IMP_AO_EnableChn(devID, chnID);
	if(ret != 0) {
		SError << "Audio play enable channel failed";
		return -1;
	}

	/* Step 4: Set audio channel volume. */
	int chnVol = 90;
	ret = IMP_AO_SetVol(devID, chnID, chnVol);
	if(ret != 0) {
		SError << "Audio Play set volume failed";
		return -1;
	}

	ret = IMP_AO_GetVol(devID, chnID, &chnVol);
	if(ret != 0) {
		SError << "Audio Play get volume failed";
		return -1;
	}

	


	IMP_ADEC_DestroyChn(adChn);

	ret = IMP_ADEC_CreateChn(adChn, &chnAttr);
	if(ret != 0) {
		SError << "imp audio decoder create channel failed" ;
		return -1;
	}

	ret = IMP_ADEC_ClearChnBuf(adChn);
	if(ret != 0) {
		SError << "IMP_ADEC_ClearChnBuf failed";
		return -1;
	}

        audioPlayerAcquireStream();

}






/*
static int playFrame( IMPAudioFrame* frame)
{
    

    if (IMP_AO_SendFrame(devID, adChn, frame, BLOCK)) {
        SError <<"IMP_AO_SendFrame failed";
        return -EINVAL;
    }

    return 0;
}
*/

int audioPlayerWriteFrame( void* pData, const size_t size)
{

  
    IMPAudioFrame frame;


    // Need to decode
    IMPAudioStream streamIn;
    streamIn.stream = (uint8_t*) pData;
    streamIn.len = size;

    if (IMP_ADEC_SendStream(adChn, &streamIn, BLOCK)) {
        SError <<"IMP_ADEC_SendStream failed";
        return -EAGAIN;
    }

    IMPAudioStream streamOut;
    if (IMP_ADEC_PollingStream(adChn, 1000)) {
        SError <<"IMP_ADEC_PollingStream failed";
        return -EAGAIN;
    }

    if (IMP_ADEC_GetStream(adChn, &streamOut, BLOCK)) {
        SError <<"IMP_ADEC_GetStream failed";
        return -EAGAIN;
    }

    frame.virAddr = (uint32_t*) streamOut.stream;
    frame.len = streamOut.len;
    //playFrame( &frame);
    
    if (IMP_AO_SendFrame(devID, adChn, &frame, BLOCK)) {
        SError <<"IMP_AO_SendFrame failed";
        return -EINVAL;
    }

//        printf("arvind %d \n", frame.len ); 

    if (IMP_ADEC_ReleaseStream(adChn, &streamOut)) {
        SError <<"IMP_ADEC_ReleaseStream failed";
        return -EAGAIN;
    }
 

    return 0;
}

int audioPlayerReleaseStream()
{
  

    if (IMP_AO_DisableChn(devID, adChn)) {
        SError <<  "IMP_AO_DisableChn failed";
        return -EAGAIN;
    }

    if (IMP_AO_Disable(devID)) {
         SError << "IMP_AO_Disable failed";
        return -EAGAIN;
    }

    return 0;
}



void audioPlayerDestroy()
{
 
    
    audioPlayerReleaseStream();


    if (IMP_ADEC_DestroyChn(adChn)) {
        SInfo <<  "IMP_ADEC_DestroyChn failed";
    }


}


#if 0
void main ()
{

	
	char *buf_g711 = NULL;
	int ret = -1;

	buf_g711 = (char *)malloc(IMP_AUDIO_BUF_SIZE);
	if(buf_g711 == NULL) {
		SError << "[ERROR] %s: malloc audio g711 buf error\n", __func__);
		return -1;
	}

	
	FILE *file_g711 = fopen("/tmp/", "rb");
	if(file_g711 == NULL) {
		SError << "[ERROR] %s: fopen %s failed\n", __func__, IMP_AUDIO_ENCODE_FILE);
		return -1;
	}


	while(1) 
	{
		ret = fread(buf_g711, 1, IMP_AUDIO_BUF_SIZE/2, file_g711);
		if(ret < IMP_AUDIO_BUF_SIZE/2)
			break;



		audioPlayerWriteFrame( );


	}

    fclose(file_g711);

	free(buf_g711);

}

#endif



}}