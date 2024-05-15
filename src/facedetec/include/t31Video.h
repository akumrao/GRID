#include "base/thread.h"
#include  <string>


  struct st_track 
  {
      
  };
  

  struct LiveConnectionContext{
      
  };
    

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
    
    int XA_addGallery(std::string jpegBuffBase64 );


     ~T31RGBA();
};

class T31Video
{
    public:
        
    T31Video( st_track *trackInfo,  LiveConnectionContext *ctx):trackInfo(trackInfo),ctx(ctx),t31h264( trackInfo, ctx),t31rgba( trackInfo, ctx)
    {
    }

    st_track *trackInfo ;
    LiveConnectionContext *ctx;

    void start();

    int T31Init();
    int T31Exit();

    int XAInit();  //XA_SDK
    int XAExit();
    //int XAProcess( uint8_t* buffer_containing_raw_rgb_data , int w, int h  );

     ~T31Video();
    
private:

    T31H264 t31h264;
    T31RGBA t31rgba;

};