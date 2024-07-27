
#include "livethread.h"
#include "base/platform.h"
#include "base/base64.hpp"
#include "webrtc/signaler.h"
#include "base/logger.h"
#include "Settings.h"

#include "http/url.h"
#include "base/filesystem.h"
#include "http/HttpClient.h"
#include "http/HttpsClient.h"


using namespace base;
using namespace base::net;
using namespace base::cnfg;



//#define DUMPFILE 1

std::atomic<int>  HDVideo {2};

                                                
namespace base {
namespace web_rtc {
    

void RestAPI(std::string method, std::string ip, std::string uri,json &m)
{
    Application app;

    std::string sendMe = m.dump();
    
    //ClientConnecton *conn = new HttpsClient( "https", "ipcamera.adapptonline.com", 8080, uri);
    ClientConnecton *conn = new HttpsClient("https", ip, 443, uri);
    //Client *conn = new Client("http://zlib.net/index.html");
    conn->fnComplete = [&](const Response & response) {
        std::string reason = response.getReason();
        StatusCode statuscode = response.getStatus();
     //   std::string body = conn->readStream() ? conn->readStream()->str() : "";
      //  STrace << "Post API reponse" << "Reason: " << reason << " Response: " << body;
    };

    conn->fnConnect = [&, sendMe](HttpBase * con) {
        
       // SInfo << sendMe.length();
        
        con->send( sendMe.c_str(), sendMe.length(), false);
        
    };

    conn->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {

        std::cout << "client->fnPayload " << data << std::endl << std::flush;
    };

    conn->fnUpdateProgess = [&](const std::string str) {
        std::cout << "final test " << str << std::endl << std::flush;
    };

    conn->_request.setMethod(method);
    conn->_request.setKeepAlive(false);
    
    conn->_request.setContentLength(sendMe.size());
    conn->_request.setContentType("application/json");
    
    
    conn->setReadStream(new std::stringstream);
    conn->send();
    
    
    app.run();
       
    
}

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
                        cnfg::Configuration identity;

                        identity.load("./event.json");
                       // std::string xaidentity = identity.root.dump();
                        
                        json m;
                        
                        m["messageType"] = "IDENTITY_NOT_IN_GALLERY";
                        m["messagePayload"] =  identity.root;
                        ctx->signaler->postAppMessage( m);


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
        
        if(  ctx && ctx->liveThread->t31rgba->record ==true)
        {
            
            if(ctx->signaler)
            {
                cnfg::Configuration identity;

                identity.load("./event.json");
               // std::string xaidentity = identity.root.dump();

               Timestamp ts;
//                Timestamp::TimeVal time = ts.epochMicroseconds();
//                int milli = int(time % 1000000) / 1000;

                std::time_t time1 = ts.epochTime();
 
               
                ctx->liveThread->t31rgba->m_date = time1;
                
                 struct std::tm* tms = std::localtime(&time1);

                char date[100] = {'\0'}; //"%Y-%m-%d-%H-%M-%S"
                int len = std::strftime(date, sizeof (date), "%Y-%m-%d-%H-%M-%S", tms);
                
                json m;

                m["messageType"] = "IDENTITY_NOT_IN_GALLERY";
                m["messagePayload"] =  identity.root;
                m["ts"] =  date;
                m["camid"] = ctx->cam;
                ctx->signaler->postAppMessage( m);
                
                
                RestAPI("POST",  "backend.adapptonline.com", "/eventsToCloudX", m);  


            }
        }

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
        if(ctx && ctx->liveFrame)
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
     return 0;
}


int LiveThread::XAExit()
{
    return 0;

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



 LiveThread::LiveThread(const char* name, LiveConnectionContext *ctx, st_track *trackInfo, bool &record, bool QRCode ):ctx(ctx),trackInfo(trackInfo), QRCode(QRCode)
{

    if(!record)
    {
        t31h264 =  new  T31H264(ctx, trackInfo);
        t31rgba =  new  T31RGBA(ctx, trackInfo, QRCode);
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
    
    std::string date = trackInfo->start; 
    recDate = date;          
    while(!stopped() )
    {
        if( date != recDate )
        {
            SDebug << " Recordign change  recDate";
            ncount =0;
            date = recDate;
        }
         
        ncount = ncount% Settings::configuration.recordsize;

        sprintf(outPutNameBuffer, "%s%s/frame-%.3d.h264", Settings::configuration.storage.c_str(),  date.c_str(), ++ncount);

        FILE *fp = fopen(outPutNameBuffer, "rb");
        if(!fp) {
             SError << "Error: cannot open: " <<  outPutNameBuffer;
            ncount =0;
            base::sleep(20);   
            continue;
             
        }


        int bytes_read = (int)fread(inbuf, 1, H264_INBUF_SIZE, fp);

        if(bytes_read) {
           basicframe.data = inbuf ;
           basicframe.sz = bytes_read;
        }


       // ctx.muRecFrame.lock();
        if(ctx->recFrame)
        ctx->recFrame->run(&basicframe); // starts the frame filter chain
        //ctx->muRecFrame.unlock(); 

        //SInfo << "payload " << bytes_read;
        basicframe.payload.resize(basicframe.payload.capacity());
        fclose(fp);

       //  base::sleep(10);   
    }
        
      
    SInfo << "Recording::run() over";
        
}


}}
