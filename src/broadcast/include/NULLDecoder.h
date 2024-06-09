
#pragma once



#include "webrtc/rawVideoFrame.h"
#include  "muxframe.h"

#include  <queue>


#include <functional>


namespace base {
namespace web_rtc {
    

class LiveConnectionContext;

class NULLDecoder
{
public:


    NULLDecoder(bool &recording );
    ~NULLDecoder();
    
    uint64_t startStreaming{0};
    uint64_t vframecount{0};
    
   
     
    void runNULLEnc(unsigned char *buffer, int size,  int & recording, LiveConnectionContext  *ctx); 
    
    void resetTimer();
    
    void delayFrame( );
    
    stFrame *qframe{nullptr};
    
    std::function<void(stFrame* frame) > cb_frame;
    
    std::function<void(web_rtc::BasicFrame* , bool) > cb_mp4;
    
    void WriteTofile( unsigned char *buf , int size,  int& frameCount);
    
    int width{0};
    int height{0};
    int fps;
    
    void recordingTime(LiveConnectionContext  *ctx);
    
private:
    std::vector< uint8_t> m_sps;
    std::vector< uint8_t> m_pps;
    
    uint64_t vdelay{0};
    
    //std::string  &cam;
    //BasicFrame basicframe;
    
    std::string dayDate;
    std::string pathDate;
    FILE *in_file{nullptr};
    base::cnfg::Configuration mf;
    

    
    bool &recording;

        
};



}//ns web_rtc
}//base
