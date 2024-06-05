
#include "webrtc/localAudioSouce.h"
#include <chrono>
#include <thread>

#include "base/logger.h"
#define tcprequest true

#include "system_wrappers/include/sleep.h"

#include "Settings.h"





#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/time.h>

#include <imp/imp_audio.h>
#include <imp/imp_log.h>
#define TAG "Sample-AI"


//#define AI_BASIC_TEST_RECORD_FILE "ai_record.pcm"
#define AI_BASIC_TEST_RECORD_NUM 500



namespace base {
namespace web_rtc {

  static const int kSamplesPerSecond = 8000;
    
//void LocalAudioSource::stop()
//{
//     SInfo << "LocalAudioSource::stop()";
//
//     base::Thread::stop();
//     join();
//}

// void LocalAudioSource::start()
// {
//     base::Thread::start();
// }







void LocalAudioSource::onAnswer()
{
              
    ffparser->start();

    //     ffparser->registerStreamCall(*ctx); // arvind
    // ffparser->playStreamCall(*ctx);
  
}


void LocalAudioSource::run()
{
       
    static const uint8_t kNumberOfChannels = 1;
    
    static const size_t kNumberSamples = 80;
   // static const size_t kBytesPerSample = sizeof (AudioPacketModule::Sample) * kNumberOfChannels;
    static const size_t kBufferBytes = 160;//kNumberSamples * kBytesPerSample;


    //uint8_t test[kBufferBytes];

  

    
    uint8_t encoded[kBufferBytes];
    
//    
//    memset( encoded, 'a',  kBufferBytes);
//    
//    encoded[0]  = '6';
//    encoded[81]  = '7';
//    encoded[82]  = '8';
//    encoded[159] = '9';
    
   
//    while(!stopped())
//    {
//       int64_t currentTime = rtc::TimeMillis();
//          
//        mutexbuf.lock();
//    
//        if( buffer.size() > 160)
//        {
//            this->OnData(&buffer[0], 16, kSamplesPerSecond,1, 80);
//
//            buffer.erase(buffer.begin(), buffer.begin() + 160);
//         }
//
//        mutexbuf.unlock();
//         
//         
//        int64_t deltaTimeMillis = rtc::TimeMillis() - currentTime;
//        
//        
//        if (deltaTimeMillis < 20)
//        {
//                webrtc::SleepMs(20 - deltaTimeMillis);
//        }
//        
//        
//    }
  
    
//   char outPutNameBuffer[256]={'\0'};
//   int ncount = 0;
//        
//   
//    int sz= 0;
//    while(!stopped())
//    {
//        
//             
//        ncount = ncount%999;
//
//        sprintf(outPutNameBuffer, "%s/frame-%.3d.alaw",    "./frames/g711a", ++ncount);
//
//        FILE *in_file = fopen(outPutNameBuffer, "rb");
//        
//      //  SInfo <<  outPutNameBuffer;
//        
//        while(!stopped())
//        {
//            int64_t currentTime = rtc::TimeMillis();
//            if (( sz = fread(encoded, 1, kBufferBytes, in_file)) <= 0)
//            {
//               // SError << "Failed to read " << outPutNameBuffer ;
//                
//                break;
//
//            }
//
//
//            if(sz != kBufferBytes)
//            {
//                 std::cout << " second bad end " << std::endl << std::flush;
//            }
//    
//            
//            
//            this->OnData(encoded, 16, kSamplesPerSecond,1, 80);
//          // std::this_thread::sleep_for(std::chrono::milliseconds(20));
//            
//            int64_t deltaTimeMillis = rtc::TimeMillis() - currentTime;
//        
//            if (deltaTimeMillis < 20)
//            {
//                    webrtc::SleepMs(20 - deltaTimeMillis);
//            }
//        
//        }
//        
//        fclose(in_file);
//        
//    }

    
        int ret = -1;
	int record_num = 0;

//	FILE *record_file = fopen(AI_BASIC_TEST_RECORD_FILE, "wb");
//	if(record_file == NULL) {
//		IMP_LOG_ERR(TAG, "fopen %s failed\n", AI_BASIC_TEST_RECORD_FILE);
//		return NULL;
//	}

	/* Step 1: set public attribute of AI device. */
	int devID = 1;
	IMPAudioIOAttr attr;
	attr.samplerate = AUDIO_SAMPLE_RATE_8000;
	attr.bitwidth = AUDIO_BIT_WIDTH_16;
	attr.soundmode = AUDIO_SOUND_MODE_MONO;
	attr.frmNum = 20;
	attr.numPerFrm = 400;
	attr.chnCnt = 1;
	ret = IMP_AI_SetPubAttr(devID, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "set ai %d attr err: %d\n", devID, ret);
		return NULL;
	}

	memset(&attr, 0x0, sizeof(attr));
	ret = IMP_AI_GetPubAttr(devID, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "get ai %d attr err: %d\n", devID, ret);
		return NULL;
	}

	IMP_LOG_INFO(TAG, "Audio In GetPubAttr samplerate : %d\n", attr.samplerate);
	IMP_LOG_INFO(TAG, "Audio In GetPubAttr   bitwidth : %d\n", attr.bitwidth);
	IMP_LOG_INFO(TAG, "Audio In GetPubAttr  soundmode : %d\n", attr.soundmode);
	IMP_LOG_INFO(TAG, "Audio In GetPubAttr     frmNum : %d\n", attr.frmNum);
	IMP_LOG_INFO(TAG, "Audio In GetPubAttr  numPerFrm : %d\n", attr.numPerFrm);
	IMP_LOG_INFO(TAG, "Audio In GetPubAttr     chnCnt : %d\n", attr.chnCnt);

	/* Step 2: enable AI device. */
	ret = IMP_AI_Enable(devID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "enable ai %d err\n", devID);
		return NULL;
	}

	/* Step 3: set audio channel attribute of AI device. */
	int chnID = 0;
	IMPAudioIChnParam chnParam;
	chnParam.usrFrmDepth = 20;
	ret = IMP_AI_SetChnParam(devID, chnID, &chnParam);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "set ai %d channel %d attr err: %d\n", devID, chnID, ret);
		return NULL;
	}

	memset(&chnParam, 0x0, sizeof(chnParam));
	ret = IMP_AI_GetChnParam(devID, chnID, &chnParam);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "get ai %d channel %d attr err: %d\n", devID, chnID, ret);
		return NULL;
	}

	IMP_LOG_INFO(TAG, "Audio In GetChnParam usrFrmDepth : %d\n", chnParam.usrFrmDepth);

	/* Step 4: enable AI channel. */
	ret = IMP_AI_EnableChn(devID, chnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record enable channel failed\n");
		return NULL;
	}

	/* Step 5: Set audio channel volume. */
	int chnVol = 100;
	ret = IMP_AI_SetVol(devID, chnID, chnVol);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record set volume failed\n");
		return NULL;
	}

	ret = IMP_AI_GetVol(devID, chnID, &chnVol);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record get volume failed\n");
		return NULL;
	}
	IMP_LOG_INFO(TAG, "Audio In GetVol    vol : %d\n", chnVol);

	int aigain = 28;
	ret = IMP_AI_SetGain(devID, chnID, aigain);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record Set Gain failed\n");
		return NULL;
	}

	ret = IMP_AI_GetGain(devID, chnID, &aigain);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record Get Gain failed\n");
		return NULL;
	}
	IMP_LOG_INFO(TAG, "Audio In GetGain    gain : %d\n", aigain);



	/////////////////////////////////////////////////////////////////////////////////

	int AeChn = 0;
	IMPAudioEncChnAttr encattr;
	encattr.type = PT_G711A; /* Use the My method to encoder. if use the system method is attr.type = PT_G711A; */
	encattr.bufSize = 20;
	ret = IMP_AENC_CreateChn(AeChn, &encattr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "imp audio encode create channel failed\n");
		return -1;
	}

    /////////////////////////////////////////////////////////////////////////////////////


	while(!stopped())
        {
		/* Step 6: get audio record frame. */

		ret = IMP_AI_PollingFrame(devID, chnID, 1000);
		if (ret != 0 ) {
			IMP_LOG_ERR(TAG, "Audio Polling Frame Data error\n");
		}
		IMPAudioFrame frm;
		ret = IMP_AI_GetFrame(devID, chnID, &frm, BLOCK);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "Audio Get Frame Data error\n");
			return NULL;
		}
		/* Step 7: Save the recording data to a file. */
		//fwrite(frm.virAddr, 1, frm.len, record_file);





/////////////////////////////////////////////////////////////////////////////////////////////////
		/* Send a frame to encode. */
		//IMPAudioFrame frm;
		//frm.virAddr = (uint32_t *)buf_pcm;
		//frm.len = ret;
		ret = IMP_AENC_SendFrame(AeChn, &frm);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "imp audio encode send frame failed\n");
			return -1;
		}

		/* get audio encode frame. */
		IMPAudioStream stream;
		ret = IMP_AENC_PollingStream(AeChn, 1000);
		if (ret != 0) {
			IMP_LOG_ERR(TAG, "imp audio encode polling stream failed\n");
		}

		ret = IMP_AENC_GetStream(AeChn, &stream, BLOCK);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "imp audio encode get stream failed\n");
			return -1;
		}

		/* save the encode data to the file. */
		//fwrite(stream.stream, 1, stream.len, record_file);

		/* release stream. */
		ret = IMP_AENC_ReleaseStream(AeChn, &stream);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "imp audio encode release stream failed\n");
			return -1;
		}




//////////////////////////////////////////////////////////////////////////////////////////////////






		/* Step 8: release the audio record frame. */
		ret = IMP_AI_ReleaseFrame(devID, chnID, &frm);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "Audio release frame data error\n");
			return NULL;
		}

		if(++record_num >= AI_BASIC_TEST_RECORD_NUM)
			break;
	}
	sleep(3);


	///////////////////////////////////////////////////////////////

	ret = IMP_AENC_DestroyChn(AeChn);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "imp audio encode destroy channel failed\n");
		return -1;
	}


	//////////////////////////////////////////////////////////////
	/* Step 9: disable the audio channel. */
	ret = IMP_AI_DisableChn(devID, chnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio channel disable error\n");
		return NULL;
	}

	/* Step 10: disable the audio devices. */
	ret = IMP_AI_Disable(devID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio device disable error\n");
		return NULL;
	}

  


	//fclose(record_file);

    
    
  

}

LocalAudioSource::LocalAudioSource(const char *name, LiveConnectionContext  *ctx , st_track *trackInfo, bool recording,  base::web_rtc::FrameFilter *next) :   base::web_rtc::FrameFilter(name, next)
{
    
    start();
}


LocalAudioSource::~LocalAudioSource() 
{
    sink_lock_.lock();
     sinks_.clear();
    sink_lock_.unlock();
    

}    
   

void LocalAudioSource::myAddRef(  std::string peerid)  {
   
    mutexAudioSource.lock();
    
    setPeerid.insert(peerid);
    
    mutexAudioSource.unlock();
 // const int count =   rtc::AtomicOps::Increment(&ref_count_);  //arvind
 // SInfo << "VideoPacketSource::AddRef()" << count;
  
}

rtc::RefCountReleaseStatus LocalAudioSource::myRelease(  std::string peerid )  {
    
    std::set< std::string> ::iterator itr;
    int count =1;
    
    mutexAudioSource.lock();
    itr = setPeerid.find(peerid);
    
    if( itr != setPeerid.end())
    {
        setPeerid.erase(itr);
    }
    
    count = setPeerid.size();
    mutexAudioSource.unlock();
  
  
    SInfo << "AudioPacketSource::Release()" << count;

    if (count == 0) {

      return rtc::RefCountReleaseStatus::kDroppedLastRef;
    }
    return rtc::RefCountReleaseStatus::kOtherRefsRemained;
}


 void LocalAudioSource::reset(  std::set< std::string> & peeerids )  {
    
    std::set< std::string> tmp;
    mutexAudioSource.lock();
   
    peeerids =    setPeerid;
    
    setPeerid.clear();
    
    mutexAudioSource.unlock();
    
    //mutexAudioSource.lock();
    
    //setPeerid.insert(peerid);
    
    //mutexAudioSource.unlock();
    
    
}
 
 
 

 
void  LocalAudioSource::run( base::web_rtc::Frame *frame){

     
    web_rtc::BasicFrame *basic_frame = static_cast<web_rtc::BasicFrame *> (frame);
     
    // mutexbuf.lock();
     
    std::copy(basic_frame->data, basic_frame->data + basic_frame->sz, std::back_inserter(buffer));
     
     
    while( buffer.size() >= 160)
    {
          this->OnData(&buffer[0], 16, kSamplesPerSecond,1, 80);

          buffer.erase(buffer.begin(), buffer.begin() + 160);
    }
     
   //  mutexbuf.unlock();
     
        
  //   SInfo <<  "LocalAudioSource::run  " <<   basic_frame->sz;
             
             
    
//    if (!codec) {
//        StartParser(basic_frame->codec_id);
//    }
//
//    uint8_t* data = NULL;
//    int size = 0;
//
//    std::copy(basic_frame->data, basic_frame->data + basic_frame->sz, std::back_inserter(buffer));

     
     //this->OnData(basic_frame->data, 16, 8000,1, 80);
 }


 
 

 void LocalAudioSource::oncommand( std::string & cmd , int first,  int second)
 {
      //  ffparser->pausedAudio(first);  //arvind
    
 }

 


}}



