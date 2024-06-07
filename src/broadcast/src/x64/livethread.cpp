
#include "livethread.h"
#include "base/platform.h"
#include "base/base64.hpp"
#include "webrtc/signaler.h"
#include "base/logger.h"
#include "Settings.h"


using namespace base;
using namespace base::cnfg;



//#define DUMPFILE 1

std::atomic<int>  HDVideo {2};

                                                
namespace base {
namespace web_rtc {
    


void T31RGBA::run() {
    

  

    SInfo << "31RGBA::run()" ;

    /* end */
    return ;
}



void T31RGBA::onMessage(json &jsonMsg )
{

   json & rjson  = jsonMsg["messagePayload"];
 
   SInfo << " onMessage "  << rjson.dump();

   std::string  jpegBuffBase64= jsonMsg["registrationImage"].get<std::string>();;



}
         


/*******************************************************************************************
  T31RGBA end
*******************************************************************************************/

int T31RGBA::T31RGBAInit()
{
    return 0;
}
int T31RGBA::T31RGBAExit()
{
    return 0;
}


T31RGBA::~T31RGBA() 
{
  stop();
  join();
  T31RGBAExit();

  SInfo << "~t31rgba->()";

}


/*******************************************************************************************
  T31H264 start
*******************************************************************************************/


void T31H264::play( unsigned char *str, int len)
{




  basicframe.data = str ;
  basicframe.sz = len;
            

  if(ctx->liveFrame)
  ctx->liveFrame->run(&basicframe); // starts the frame filter chain
  //ctx->muRecFrame.unlock(); 
  basicframe.payload.resize(basicframe.payload.capacity());


}



void T31H264::run()
{
 
  //IMP_LOG_DBG(TAG, "OK\n");

    T31H264Init(2);
#if 0 
    while (!stopped())
    {



        if( HDVideo !=  chnNum)
        {
          T31H264Exit();
          T31H264Init(HDVideo);
        }

        std::string filepath = "/mnt/test.264";
        
        FILE *fp = fopen(filepath.c_str(), "rb");

        if(!fp) {
          printf("Error: cannot open: %s\n", filepath.c_str());
          return ;
        }

        while(!stopped() )
        {
            int bytes_read = (int)fread(inbuf, 1, H264_INBUF_SIZE, fp);

            if(bytes_read) {
               basicframe.data = inbuf ;
               basicframe.sz = bytes_read;
            }
            else
            {

                  if(feof(fp))
                  {
                    if (fseek(fp, 0, SEEK_SET))
                        continue;
                      
                    if(ctx->signaler)
                    {
//                        cnfg::Configuration identity;
//
//                        identity.load("./event.json");
//                       // std::string xaidentity = identity.root.dump();
//                        
//                        json m;
//                        
//                        m["messageType"] = "IDENTITY_NOT_IN_GALLERY";
//                        m["messagePayload"] =  identity.root;
//                        ctx->signaler->postAppMessage( m);


                    }


                  }

            }



           

           // ctx.muRecFrame.lock();
            if(ctx->liveFrame)
            ctx->liveFrame->run(&basicframe); // starts the frame filter chain
            //ctx->muRecFrame.unlock(); 

           // SInfo << "payload " << bytes_read;
            basicframe.payload.resize(basicframe.payload.capacity());
       
           //  base::sleep(10);   
        }
        
        fclose(fp);
        
    }
#else 
           
      //  std::string filepath = "/mnt/test.264";
        
    char outPutNameBuffer[256]={'\0'};

    int ncount = 0;

    while(!stopped() )
    {

        ncount = ncount%240;

        sprintf(outPutNameBuffer, "%s/frame-%.3d.h264",    "./frames/h264", ++ncount);

        FILE *fp = fopen(outPutNameBuffer, "rb");
        if(!fp) {
             SError << "Error: cannot open: " <<  outPutNameBuffer;
             return ;
        }


        int bytes_read = (int)fread(inbuf, 1, H264_INBUF_SIZE, fp);

        if(bytes_read) {
           basicframe.data = inbuf ;
           basicframe.sz = bytes_read;
        }


       // ctx.muRecFrame.lock();
        if(ctx->liveFrame)
        ctx->liveFrame->run(&basicframe); // starts the frame filter chain
        //ctx->muRecFrame.unlock(); 

        //SInfo << "payload " << bytes_read;
        basicframe.payload.resize(basicframe.payload.capacity());
        fclose(fp);

       //  base::sleep(10);   
    }
        
       
        
    
#endif
    
    


  }

int T31H264::T31H264Exit()
{
      
    return 1;
}

int T31H264::T31H264Init( int ch)
{


  
    return 0;
}



 T31H264::~T31H264() {

  stop();
  join();
  


  SInfo << "~T31H264()";

}



/*******************************************************************************************
  T31H264 end
*******************************************************************************************/





/*******************************************************************************************
  LiveThread start
*******************************************************************************************/


int LiveThread::T31Init()
{
    return 0;
}


int LiveThread::T31Exit()
{
    return 0; 
}



int LiveThread::XAInit()
{

}


int LiveThread::XAExit()
{


}


 LiveThread::~LiveThread() {


  T31Exit();
  XAExit();

  SInfo << "~LiveThread";

}


void LiveThread::stop()
{
    SInfo << "LiveThread:: stop";

     if(recording)
    {
        recording->stop();
        
        
        recording->join();


        delete recording ;
        recording = nullptr;
        
    }
    else
    {
        t31rgba->stop();

        t31h264->stop();

        t31rgba->join();

        t31h264->join();


        delete t31h264 ;
        t31h264 = nullptr;

       // delete t31rgba ;
       // t31rgba = nullptr;

        SInfo << "LiveThread:: stop over";
    }
   
}



 LiveThread::LiveThread(const char* name, LiveConnectionContext *ctx, st_track *trackInfo, bool &record):ctx(ctx),trackInfo(trackInfo)
{

    if(!record)
    {
        t31h264 =  new  T31H264(ctx, trackInfo);
        t31rgba =  new  T31RGBA(ctx, trackInfo);
    }
    else
    {
       recording =  new  Recording(ctx, trackInfo); 
    }
        
    
}



void LiveThread::start()
{
    if(recording)
    {
        recording->start();
    }
    else
    {
        XAInit();
        T31Init();
        t31rgba->T31RGBAInit();
        t31h264->start();
        t31rgba->start();
    }
}



void Recording::run()
{
 

    char outPutNameBuffer[256]={'\0'};

    int ncount = 0;
    
    std::string date = Settings::configuration.storage + trackInfo->start; 
             
    while(!stopped() )
    {

        ncount = ncount%250;

        sprintf(outPutNameBuffer, "%s/frame-%.4d.h264",date.c_str(), ++ncount);

        FILE *fp = fopen(outPutNameBuffer, "rb");
        if(!fp) {
             SError << "Error: cannot open: " <<  outPutNameBuffer;
             return ;
        }


        int bytes_read = (int)fread(inbuf, 1, H264_INBUF_SIZE, fp);

        if(bytes_read) {
           basicframe.data = inbuf ;
           basicframe.sz = bytes_read;
        }


       // ctx.muRecFrame.lock();
        if(ctx->liveFrame)
        ctx->liveFrame->run(&basicframe); // starts the frame filter chain
        //ctx->muRecFrame.unlock(); 

        //SInfo << "payload " << bytes_read;
        basicframe.payload.resize(basicframe.payload.capacity());
        fclose(fp);

       //  base::sleep(10);   
    }
        
       
        
}


}}
