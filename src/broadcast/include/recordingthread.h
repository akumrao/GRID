#ifndef RECTHREAD_HEADER_GUARD
#define RECTHREAD_HEADER_GUARD

//#include "live.h"
#include "livethread.h"
#include "base/thread.h"
#include "framefilter.h"
#include <list>
#include <set>

namespace base {
namespace web_rtc {

    class RecordThread : public Thread {

    public:

    public:
        /** Default constructor
   * 
   * @param name          Thread name
   * @param n_max_slots   Maximum number of connections (each Connection instance is placed in a slot)
   * 
   */
    
    RecordThread(const char* name, st_track *trackInfo, LiveConnectionContext* ctx);
    
    //void onMessage(json &msg );
    
    
    ~RecordThread();
    


    #define H264_INBUF_SIZE 30000  
    uint8_t inbuf[H264_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    
    public: // redefined virtual functions
       
    void run();

    private: // internal


    st_track *trackInfo;
    
    LiveConnectionContext *ctx;
     
    BasicFrame basicframe;



    public:
      
    };
}
}
#endif
