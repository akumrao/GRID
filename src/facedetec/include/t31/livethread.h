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
    
/*class LiveThread;

    enum class LiveConnectionType {
    none,
    rtsp,
    sdp
};
*/
class LiveThread;
struct LiveConnectionContext {
    /** Default constructor */
    LiveConnectionContext( std::string& cam, FrameFilter* liveFrame ):
         cam(cam)
        , liveFrame(liveFrame)
    {
       // if(liveFrame)
          //setLiveFrame[trackid] = liveFrame;
    }
    /** Dummy constructor : remember to set member values by hand */
    LiveConnectionContext()
    {
       // setLiveFrame.clear();
    }
   // LiveConnectionType connection_type; ///< Identifies the connection type
   // std::string address; ///< Stream address
  //  SlotNumber slot; ///< A unique stream slot that identifies this stream
    //std::map< std::string, FrameFilter* >    setLiveFrame; // with trackid <> videosource
    
    FrameFilter*  liveFrame{nullptr}; // with trackid <> videosource
    
    FrameFilter*  recFrame{nullptr}; // with trackid <> videosource
    
    LiveThread *liveThread{nullptr};
    
    
    Signaler *signaler{nullptr};
    
   
    
   // DummyFrameFilter *fragmp4_filter{nullptr};
    //FrameFilter *fragmp4_muxer{nullptr};
    
    //web_rtc::FrameFilter *info{nullptr};;
    //FrameFilter *txt{nullptr};
  
    
    std::string cam;

  //  long unsigned int msreconnect; ///< If stream has delivered nothing during this many milliseconds, reconnect
  //  bool request_multicast; ///< Request multicast in the rtsp negotiation or not
  //  bool request_tcp; ///< Request interleaved rtsp streaming or not
 //   unsigned recv_buffer_size; ///< Operating system ringbuffer size for incoming socket
  //  unsigned reordering_time; ///< Live555 packet reordering treshold time (microsecs)
  //  TimeCorrectionType time_correction; ///< How to perform frame timestamp correction

    
};
#endif


class T31H264:public base::Thread
{
    public:

    T31H264(  LiveConnectionContext *ctx, st_track *trackInfo ):ctx(ctx),trackInfo(trackInfo)
    {
    }

    st_track *trackInfo ;
    LiveConnectionContext *ctx;
    
        
    void run();

    int T31H264Init(int ch);
    int T31H264Exit();

    int chnNum{0};

    BasicFrame basicframe; 
    void play( unsigned char *str, int len);
    
    ~T31H264();
};

class T31RGBA:public base::Thread
{
    public:
        
    T31RGBA(  LiveConnectionContext *ctx,  st_track *trackInfo, bool QRCode ):ctx(ctx),trackInfo(trackInfo),QRCode(QRCode)
    {
    }

    st_track *trackInfo ;
    LiveConnectionContext *ctx;
    

    void run();

    int T31RGBAInit();
    int T31RGBAExit();

    int XAProcess( uint8_t* buffer_containing_raw_rgb_data , int w, int h  );
    
    int XA_addGallery(std::string jpegBuffBase64, json & registrationImage);


    int XA_addGallery();

    void onMessage(json &msg );

    std::atomic<int> ready_flag {1};

    std::atomic<bool> record{false};

    bool QRCode;

    ~T31RGBA();
};


class Recording:public base::Thread
{
    public:

    Recording( LiveConnectionContext *ctx, st_track *trackInfo ):ctx(ctx),trackInfo(trackInfo)
    {
    }

    st_track *trackInfo ;
    LiveConnectionContext *ctx;
    
    BasicFrame basicframe; 

    
    #define H264_INBUF_SIZE 30000  
    uint8_t inbuf[H264_INBUF_SIZE];


    void run();


    ~Recording()
    {
    }

};
    

class LiveThread
{
    public:
        
    LiveThread(const char* name, LiveConnectionContext *ctx, st_track *trackInfo, bool &record, bool QRCode);

    st_track *trackInfo ;
    LiveConnectionContext *ctx;


    void onMessage(json &msg )
    {
        if(t31rgba)
        t31rgba->onMessage(msg);
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

    T31H264 *t31h264{nullptr};
    T31RGBA *t31rgba{nullptr};
    
    Recording *recording{nullptr};

    bool record;
    bool QRCode;

};

}}


#endif
