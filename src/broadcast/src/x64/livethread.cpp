
#include "livethread.h"
//#include "logging.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <iterator>
#include "base/logger.h"
#include "base/platform.h"
#include "Settings.h"

#include "webrtc/signaler.h"

// #define RECONNECT_VERBOSE   // by default, disable
// #define LIVE_SIGNAL_FRAMES // experimental


std::atomic<int>  HDVideo{0} ;

namespace base {
namespace web_rtc {
    
    LiveThread::LiveThread(const char* name, st_track *trackInfo, LiveConnectionContext* ctx):trackInfo(trackInfo), ctx(ctx)
    {
          t31rgba = new T31RGBA(trackInfo, ctx) ;

    }
    
    void LiveThread::onMessage(json &msg )
    {
        delete t31rgba ;
    }
         
    
    LiveThread::~LiveThread(){
        join();
    }

//    void LiveThread::run(){
//        
//    
//        
//        std::string filepath = "/mnt/test.264";
//        
//        FILE *fp = fopen(filepath.c_str(), "rb");
//
//        if(!fp) {
//          printf("Error: cannot open: %s\n", filepath.c_str());
//          return ;
//        }
//
//        while(!stopped() )
//        {
//            int bytes_read = (int)fread(inbuf, 1, H264_INBUF_SIZE, fp);
//
//            if(bytes_read) {
//               basicframe.data = inbuf ;
//               basicframe.sz = bytes_read;
//            }
//            else
//            {
//
//                  if(feof(fp))
//                  {
//                    if (fseek(fp, 0, SEEK_SET))
//                        continue;
//                      
//                    if(ctx->signaler)
//                    {
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
//
//
//                    }
//
//
//                  }
//
//            }
//
//
//
//           
//
//           // ctx.muRecFrame.lock();
//            if(ctx->liveFrame)
//            ctx->liveFrame->run(&basicframe); // starts the frame filter chain
//            //ctx->muRecFrame.unlock(); 
//
//            SInfo << "payload " << bytes_read;
//            basicframe.payload.resize(basicframe.payload.capacity());
//       
//           //  base::sleep(10);   
//        }
//        
//        fclose(fp);
//        
//    }

    void LiveThread::run(){
        
           
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
        
       
        
    }

}
}
