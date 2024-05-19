#ifndef LIVETHREAD_HEADER_GUARD
#define LIVETHREAD_HEADER_GUARD


#include "base/thread.h"
#include  <string>

#include <json/json.hpp>
using json = nlohmann::json;
#include "json/configuration.h"

#if(DUMPFILE)
namespace base {
namespace web_rtc {
struct FrameFilter
{
};
struct st_track
{
};
struct LiveConnectionContext
{
};
struct BasicFrame
{
};

}}
#else

#include "framefilter.h"

#endif

                                                
namespace base {
namespace web_rtc {

#if(DUMPFILE)
#else
    
class LiveThread;

    enum class LiveConnectionType {
    none,
    rtsp,
    sdp
};
class LiveThread;
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

    
};
#endif


class T31H264:public base::Thread
{
    public:

    T31H264( st_track *trackInfo,  LiveConnectionContext *ctx):trackInfo(trackInfo),ctx(ctx)
    {
    }

    st_track *trackInfo ;
    LiveConnectionContext *ctx;
    
        
    void run();

    int T31H264Init();
    int T31H264Exit();

    int chnNum{0};

    BasicFrame basicframe; 
    void play( unsigned char *str, int len);
    
    ~T31H264();
};

class T31RGBA:public base::Thread
{
    public:
        
    T31RGBA( st_track *trackInfo,  LiveConnectionContext *ctx):trackInfo(trackInfo),ctx(ctx)
    {
    }

    st_track *trackInfo ;
    LiveConnectionContext *ctx;
    

    void run();

    int T31RGBAInit();
    int T31RGBAExit();

    int XAProcess( uint8_t* buffer_containing_raw_rgb_data , int w, int h  );
    
    int XA_addGallery(std::string jpegBuffBase64, std::string & registrationImage);

    void onMessage(json &msg );

    std::atomic<int> ready_flag {1};


    ~T31RGBA();
};

class LiveThread
{
    public:
        
    LiveThread(const char* name, st_track *trackInfo,  LiveConnectionContext *ctx):trackInfo(trackInfo),ctx(ctx),t31h264( trackInfo, ctx),t31rgba( trackInfo, ctx)
    {
    }

    st_track *trackInfo ;
    LiveConnectionContext *ctx;


    void onMessage(json &msg )
    {
        t31rgba.onMessage(msg);
    }

    void start();

    void stop();
  
 


    int T31Init();
    int T31Exit();

    int XAInit();  //XA_SDK
    int XAExit();
    //int XAProcess( uint8_t* buffer_containing_raw_rgb_data , int w, int h  );

     ~LiveThread();
    
public:

    T31H264 t31h264;
    T31RGBA t31rgba;

};

}}


#endif
