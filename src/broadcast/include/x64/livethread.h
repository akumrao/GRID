#ifndef LIVETHREAD_HEADER_GUARD
#define LIVETHREAD_HEADER_GUARD

//#include "live.h"
//#include "liveserver.h"
#include "base/thread.h"
#include "framefilter.h"
#include <list>
#include <set>

namespace base {
namespace web_rtc {

    class LiveThread;

        enum class LiveConnectionType {
        none,
        rtsp,
        sdp
    };


    struct LiveConnectionContext {
        /** Default constructor */
        LiveConnectionContext(LiveConnectionType ct, std::string address, SlotNumber slot, std::string& cam, 
            bool request_tcp, FrameFilter* liveFrame )
            : connection_type(ct)
            , address(address)
            , slot(slot)
            , cam(cam)
            , liveFrame(liveFrame)
            , msreconnect(5000)
            , request_multicast(false)
            , request_tcp(request_tcp)
            , recv_buffer_size(0)
            , reordering_time(0)
            , time_correction(TimeCorrectionType::smart)
        {
           // if(liveFrame)
              //setLiveFrame[trackid] = liveFrame;
        }
        /** Dummy constructor : remember to set member values by hand */
        LiveConnectionContext()
            : connection_type(LiveConnectionType::none)
            , address("")
            , slot(0)
            , fragmp4_muxer(NULL)
            , txt(NULL)
            , msreconnect(5000)
            , request_multicast(false)
            , request_tcp(request_tcp)
            , time_correction(TimeCorrectionType::smart)
        {
           // setLiveFrame.clear();
        }
        LiveConnectionType connection_type; ///< Identifies the connection type
        std::string address; ///< Stream address
        SlotNumber slot; ///< A unique stream slot that identifies this stream
        //std::map< std::string, FrameFilter* >    setLiveFrame; // with trackid <> videosource
        
        FrameFilter*  liveFrame{nullptr}; // with trackid <> videosource
        
        Signaler *signaler{nullptr};
        
        LiveThread *liveThread{nullptr};
        
        DummyFrameFilter *fragmp4_filter{nullptr};
        FrameFilter *fragmp4_muxer{nullptr};
        
        //web_rtc::FrameFilter *info{nullptr};;
        FrameFilter *txt{nullptr};
      
        
        std::string cam;

        long unsigned int msreconnect; ///< If stream has delivered nothing during this many milliseconds, reconnect
        bool request_multicast; ///< Request multicast in the rtsp negotiation or not
        bool request_tcp; ///< Request interleaved rtsp streaming or not
        unsigned recv_buffer_size; ///< Operating system ringbuffer size for incoming socket
        unsigned reordering_time; ///< Live555 packet reordering treshold time (microsecs)
        TimeCorrectionType time_correction; ///< How to perform frame timestamp correction
        
//        void addLiveFrameSource(std::string & trackid, FrameFilter* src)
//        {
//           muLiveFrame.lock();
//           setLiveFrame[trackid] = src;
//           muLiveFrame.unlock();
//        }
//        
//        int removeLiveFrameSource(std::string & trackid)
//        {
//            int ret;
//            muLiveFrame.lock();
//            setLiveFrame.erase(trackid);
//            ret = setLiveFrame.size();
//            muLiveFrame.unlock();
//            
//            return ret;
//        }
        
        //std::mutex muLiveFrame;
        
       // std::mutex muRecFrame;
        
    };


    class LiveThread : public Thread {

    public:

    public:
        /** Default constructor
   * 
   * @param name          Thread name
   * @param n_max_slots   Maximum number of connections (each Connection instance is placed in a slot)
   * 
   */
    
    LiveThread(const char* name, st_track *trackInfo, LiveConnectionContext* ctx);
    
    void onMessage(json &msg );
    
    
    ~LiveThread();
    


    #define H264_INBUF_SIZE 16384  

    uint8_t inbuf[H264_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    
    public: // redefined virtual functions
       
    void run();

    std::atomic<bool> record{false};

  
    private: // internal


    st_track *trackInfo;
    
    LiveConnectionContext *ctx;
     
    BasicFrame basicframe;

    public:
      
    };
}
}
#endif
