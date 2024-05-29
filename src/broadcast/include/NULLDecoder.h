
#pragma once



#include "webrtc/rawVideoFrame.h"
#include  "muxframe.h"

#include  <queue>

extern "C" {
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
};

#include <functional>


namespace base {
namespace web_rtc {
    


class NULLDecoder
{
public:


    NULLDecoder(std::string  &cam);
    ~NULLDecoder();
    
    uint64_t startStreaming{0};
    uint64_t vframecount{0};
    
   
     
    void runNULLEnc(unsigned char *buffer, int size, AVPictureType pict_type, int & recording); 
    
    void resetTimer();
    
    void delayFrame( );
    
    stFrame *qframe{nullptr};
    
    std::function<void(stFrame* frame) > cb_frame;
    
    std::function<void(web_rtc::BasicFrame* , bool) > cb_mp4;
    
    void WriteTofile( unsigned char *buf , int size);
    
    int  width{0};
    int height{0};
    int fps;
    
    
private:
    std::vector< uint8_t> m_sps;
    std::vector< uint8_t> m_pps;
    
    uint64_t vdelay{0};
    
    std::string  &cam;
    
    //BasicFrame basicframe;
    
    std::string dayDate;
    std::string pathDate;
    FILE *in_file{nullptr};
    base::cnfg::Configuration mf;
    
    int frameCount{0};
    

        
};



}//ns web_rtc
}//base
