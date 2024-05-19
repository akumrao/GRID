
#include "livethread.h"
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include "sample-common.h"

#include "base/platform.h"
#include "base/rgba_bitmap.h"
#include "base/base64.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "base/stb_image.h"

#if(DUMPFILE)
#else
#include "webrtc/signaler.h"
#endif

#include <xailient-fi/sdk_json_interface.h>


#include "base/logger.h"
using namespace base;
using namespace base::cnfg;



extern struct chn_conf chn[];

#define LOW_BITSTREAM
#define SHOW_FRM_BITRATE
#ifdef SHOW_FRM_BITRATE
#define FRM_BIT_RATE_TIME 2
#define STREAM_TYPE_NUM 3
static int frmrate_sp[STREAM_TYPE_NUM] = { 0 };
static int statime_sp[STREAM_TYPE_NUM] = { 0 };
static int bitrate_sp[STREAM_TYPE_NUM] = { 0 };
#endif

//#define DUMPFILE 1


                                                
namespace base {
namespace web_rtc {
    


void T31RGBA::run() {
    

    int ret = 0;
    int i = 0;
   
    IMPFrameInfo *frame;

   #if(DUMPFILE)

    FILE *fp;

    fp = fopen("/tmp/snap.rgba", "wb");
    if(fp == NULL) {
        SError<<"file open error ";
        return ;
    }

   #endif

    while (!stopped()) {
        /* Snap RGBA */
        ret = IMP_FrameSource_SetFrameDepth(3, 1);
        if (ret < 0) {
            SError<<"IMP_FrameSource_SetFrameDepth failed";
            return;
        }

        ret = IMP_FrameSource_GetFrame(3, &frame);
        if (ret < 0) {
            SError<<"IMP_FrameSource_GetFrame failed";
            return ;
        }



        STrace << "Frame size "  <<  frame->size << " width"  << frame->width << " height "  <<   frame->height <<  " format "  << frame->pixfmt;
        
       
        if ( ready_flag ) 
        {
            size_t  p_output_size1 = 0;

            unsigned char *  rgbBuf = rgba_to_rgb_brg( (const unsigned char*)frame->virAddr , frame->size,  bitmap_buffer_format_RGB , 0, frame->width , frame->height , &p_output_size1 );


           // for( int x =0; x < 40 ; ++x)
            {

                XAProcess( rgbBuf, frame->width , frame->height) ;
               // base::sleep(100);
             }

             free(rgbBuf) ;


            // fwrite((void *)frame->virAddr, frame->size, 1, fp);
            // fclose(fp);

            
            // unsigned char *  bgrBuf = rgba_to_rgb_brg( (const unsigned char*) frame->virAddr , frame->size, bitmap_buffer_format_BGR , 0, frame->width , frame->height , &p_output_size1 );
            // write_bmp(bgrBuf, frame->width , frame->height, "/tmp/snap.bmp"  );
            // free(bgrBuf) ;


             // STrace << " new  try end";
        

             //            //  #if(DUMPFILE)

             //  std::ifstream f("./arvind.rgba"); //taking file as inputstream
             //  std::string str;
             //  if(f) {
             //     std::stringstream ss;
             //     ss << f.rdbuf(); // reading data
             //     str = ss.str();
             //  }
             //  else
             //  {
             //      SError << " rgba file does not exist ";
             //      return -1;
             //  }

                
                      
            //   unsigned long  width = 640;
            //   unsigned long  height = 360;
            //   size_t  p_output_size = 0;
              
                
            //   //unsigned char *  bgrBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_BGR , 0, width , height , &p_output_size );
            //   //write_bmp(bgrBuf, width, height, "arvind.bmp"  );
            //   //free(bgrBuf) ;
              
            //   unsigned char *  rgbBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_RGB , 0, width , height , &p_output_size );

  
            //  for( int x = 0; x < 100; ++x)
            //  {
            //      XAProcess( rgbBuf , width, height );

            //      base::sleep(100);

            //      STrace << " new  try " << x ; 
            //  }
              

            
            // free(rgbBuf) ;
          
            
        }

      //  ++i;


         //XAProcess( frame->virAddr, frame->width , frame->height) ;

       //  base::sleep(700);

      

        IMP_FrameSource_ReleaseFrame(3, frame);
        if (ret < 0) {
            SError<<"IMP_FrameSource_ReleaseFrame failed";
            return;
        }
        ret = IMP_FrameSource_SetFrameDepth(3, 0);
        if (ret < 0) {
            SError<<"IMP_FrameSource_SetFrameDepth failed";
            return ;
        }
    }

    #if(DUMPFILE)

    fclose(fp);

    #endif

    /* end */
    return ;
}



void T31RGBA::onMessage(json &jsonMsg )
{

   std::string registrationJson  = jsonMsg["messagePayload"].dump();
   std::string  jpegBuffBase64= jsonMsg["registrationImage"].get<std::string>();;

   XA_addGallery(jpegBuffBase64, registrationJson);

}
         
    

int T31RGBA::XA_addGallery(std::string jpegBuffBase64, std::string & registrationJson)
{


  ready_flag = 0;

  SInfo << "jpegBuffBase64 " << jpegBuffBase64.size() ;
            
  std::string out;
 //  base64::Decoder dec;
 //  dec.decode(jpegBuffBase64, out);
                
 out = base64_decode(jpegBuffBase64);
    
 SInfo << "base64 decoded " << out.size() ;
  


 int width, height , channels;

//  if(!stbi_info_from_memory(out, jpegBuffBase64.size(), &width, &height, &channels)) return -1;
//

//  /* exit if the image is larger than ~80MB */
//  if(width && height > (80000000 / 4) / height) return -1;

  unsigned char *img = stbi_load_from_memory(out.c_str(), out.size(), &width, &height, &channels, 3);

  SInfo << "wx: " << width  << " he: " << height <<  " ch: " << channels;

  
  const char * galleryIdentityManifest;
  const xa_sdk_identity_images_t * remaining_identity_image_pairs;
  const char * updated_json_identities;
  xa_fi_error_t returnValue;

  //if (< a new gallery manifest exists >) 
 
  {
        galleryIdentityManifest = registrationJson.c_str();

        // Step 1
        returnValue = xa_sdk_update_identities(galleryIdentityManifest,
        &remaining_identity_image_pairs,
        &updated_json_identities);
        if (returnValue == XA_ERR_NONE) {
            //< persist updated_json_identities >
            SInfo << "xa_sdk_update_identities passed";      
                 
         }
         else {
            SError << "xa_sdk_update_identities fails";
            free(img);
            return -1;
       }
  }

  int totalIdentity = remaining_identity_image_pairs->number_of_remaining_images;
  // Step 2 / Step 4
  while ((returnValue == XA_ERR_NONE) && (remaining_identity_image_pairs->number_of_remaining_images > 0)) 
  {
      
      int index = totalIdentity - remaining_identity_image_pairs->number_of_remaining_images;
      
      SInfo << "IdentityIndex:" << index  << " totalIndenty:" << totalIdentity;
      
      xa_fi_image_t image;
      image.width = width;
      image.height = height;
      image.pixel_format =  XA_FI_COLOR_RGB888;  // signifies the buffer data format
      image.buff = img;

  
      //< acquire the image for remaining_identity_image_pairs->identity_images[0] >
     // xa_fi_image_t image = < convert the acquired image to xa_fi_image_t - see fi_image.h >

      // Step 3
      returnValue = xa_sdk_add_identity_image(remaining_identity_image_pairs->identity_images[index].identity_id,
      remaining_identity_image_pairs->identity_images[index].image_id,
      &image,
      &remaining_identity_image_pairs,
      &updated_json_identities);
      
      // Step 5 - persist after each image to avoid needing to download them again
       // this step could also be placed after the while loop
       if (returnValue == XA_ERR_NONE) {
        //< persist updated_json_identities >
        
        json up_json_identities = json::parse(updated_json_identities); 

        std::string path = "./updated_json_identities.json";

        base::cnfg::saveFile(path, up_json_identities );


        SInfo << "remaining_identity_image_pairs passed: " << remaining_identity_image_pairs->identity_images[0].identity_id;
      }
      else 
      {
          SError << "remaining_identity_image_pairs fails " << remaining_identity_image_pairs->identity_images[0].identity_id;
          
          free(img);
          return -1;
          
      }
  }

  const char * deviceCheckinJson = xa_sdk_get_device_checkin_json();
  
  SInfo << "deviceCheckinJson:" << deviceCheckinJson;

  SInfo << "sleep for 10 secs ";

  base::sleep(10000);
  free(img);



    //XA_addGallery(event["registrationImage"].get<std::string>()) ;
  uint8_t* tmpBuf = new uint8_t [width*3*height];
  memset(tmpBuf, 0, width*3*height);
  XAProcess( tmpBuf , width, height );
  delete [] tmpBuf;

   ready_flag = 1;

    //< perform a deviceCheckin with the deviceCheckinJson >

}

int T31RGBA::XAProcess( uint8_t* buffer_containing_raw_rgb_data , int w, int h  )
{
    
    xa_fi_image_t image;
    image.width = w;
    image.height = h;

    image.pixel_format =   XA_FI_COLOR_RGB888;  // signifies the buffer data format


    image.buff =  buffer_containing_raw_rgb_data;  // note this is in RGB order, otherwise
                                         // colors will be swapped

    xa_fi_error_t returnValue;

    xa_sdk_process_image_outputs* process_image_outputs;

    if (1) {
        returnValue = xa_sdk_process_image(&image, &process_image_outputs);

        if (returnValue == XA_ERR_NONE) {
            for (int index = 0;
                 index < process_image_outputs->number_of_json_blobs; ++index) {
                xa_sdk_json_blob_t blob = process_image_outputs->blobs[index];

                if (blob.blob_descriptor == XA_FACE_TRACK_EVENT) {
                    //<
                    // send blob -> json to Face Track Event endpoint >

                    json event = json::parse(blob.json); 

                    SInfo << "json to Face Track Event endpoint";

                    //std::string path = "./event.json";
                    //base::cnfg::saveFile(path, event );

                    if(  (event.find("eventType") != event.end()) )
                    {

                        if(  event.find("registrationImage") != event.end()) 
                        {

                            if(event["eventType"].get<std::string>() ==  "IDENTITY_NOT_IN_GALLERY")
                            {

                              SInfo << "IDENTITY_NOT_IN_GALLERY";

                          
                              #if DUMPFILE

                                cnfg::Configuration identity;

                                identity.load("./identity.json");
                                std::string xaidentity = identity.root.dump();

                                 if (identity.loaded()) 
                                 {
                                  
                                   XA_addGallery(event["registrationImage"].get<std::string>() ,xaidentity ) ;
                                   // uint8_t* tmpBuf = new uint8_t [image.width*3*image.height];
                                   // memset(tmpBuf, 0, image.width*3*image.height);
                                   // XAProcess( tmpBuf , image.width, image.height );
                                   // delete [] tmpBuf;
                                   
                                 }
                                 else
                                  SError << "./identity.json missing";

                               #else
                              //cnfg::Configuration identity;

                              //identity.load("./event.json");
                              // std::string xaidentity = identity.root.dump();
                              
                                  std::string str = event["registrationImage"].get<std::string>();

                                  json m;
                                  
                                  m["messageType"] = "IDENTITY_NOT_IN_GALLERY";
                                  m["messagePayload"] =  event;
                                  ctx->signaler->postAppMessage( m);
                              #endif
                            }
                            else if(event["eventType"].get<std::string>() ==  "IDENTITY_RECOGNIZED")
                            {
                                 SInfo << "IDENTITY_RECOGNIZED";

                                  #if DUMPFILE

                                  #else  
                                  json m;
                                  
                                 
                                  m["messageType"] = "IDENTITY_RECOGNIZED";
                                  m["messagePayload"] =  event;
                                  ctx->signaler->postAppMessage( m);
                                  #endif  
                            }


                        }
                        else
                         SError << "no registrationImage " <<  event.dump(4);  

                     }
                     else
                     {
                       SError << "no eventType " << event.dump(4);
                     }



                } else if (blob.blob_descriptor == XA_ACCURACY_MONITOR) {
                    //<
                    // send blob -> json to Accuracy Monitor endpoint >
                    STrace << "send blob -> json to Accuracy Monitor endpoint: " <<  blob.json;
                } else {
                    SError << "Not a possible state";
                }
            }
        } else {
            SError << "Error at process_image_outputs";
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

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
}


/*******************************************************************************************
  T31H264 start
*******************************************************************************************/


void T31H264::play( unsigned char *str, int len)
{


#if(DUMPFILE)

#else

  basicframe.data = str ;
  basicframe.sz = len;
            

  if(ctx->liveFrame)
  ctx->liveFrame->run(&basicframe); // starts the frame filter chain
  //ctx->muRecFrame.unlock(); 
  basicframe.payload.resize(basicframe.payload.capacity());

 #endif 
}

void T31H264::run()
{
 
  IMPEncoderEncType encType;



  int ret = IMP_Encoder_StartRecvPic(chnNum);
  if (ret < 0) {
    SError <<  "IMP_Encoder_StartRecvPic failed\n" <<  chnNum;
    return ;
  }

  SInfo << "Streaming h264 channel" << chnNum;


  #if(DUMPFILE)
//  IMP_LOG_DBG(TAG, "Video ChnNum=%d Open Stream file %s ", chnNum, stream_path);
   FILE *stream_fd = fopen("/tmp/test.264", "wb"); 
   if (stream_fd < 0) {
      SError <<  "could not open file /tmp/test.264" <<  chnNum;
     return ;
   }
   #endif

   //IMP_LOG_DBG(TAG, "OK\n");
  
  while (!stopped())
  {
    ret = IMP_Encoder_PollingStream(chnNum, 1000);
    if (ret < 0) {
      SError <<  "IMP_Encoder_PollingStream timeout" <<  chnNum;
      continue;
    }

    IMPEncoderStream stream;
    /* Get H264 or H265 Stream */
    ret = IMP_Encoder_GetStream(chnNum, &stream, 1);
#ifdef SHOW_FRM_BITRATE
    int i, len = 0;
    for (i = 0; i < stream.packCount; i++) {
      len += stream.pack[i].length;
    }
    bitrate_sp[chnNum] += len;
    frmrate_sp[chnNum]++;

    int64_t now = IMP_System_GetTimeStamp() / 1000;
    if(((int)(now - statime_sp[chnNum]) / 1000) >= FRM_BIT_RATE_TIME){
      double fps = (double)frmrate_sp[chnNum] / ((double)(now - statime_sp[chnNum]) / 1000);
      double kbr = (double)bitrate_sp[chnNum] * 8 / (double)(now - statime_sp[chnNum]);

      printf("streamNum[%d]:FPS: %0.2f,Bitrate: %0.2f(kbps)\n", chnNum, fps, kbr);
      //fflush(stdout);

      frmrate_sp[chnNum] = 0;
      bitrate_sp[chnNum] = 0;
      statime_sp[chnNum] = now;
    }
#endif
    if (ret < 0) {
     SError <<  "IMP_Encoder_GetStream failed " <<  chnNum;
      return ;
    }

//    ret = save_stream(stream_fd, &stream);
    int ret,  nr_pack = stream.packCount;

//IMP_LOG_DBG(TAG, "----------packCount=%d, stream->seq=%u start----------\n", stream->packCount, stream->seq);
    for (i = 0; i < nr_pack; i++) {
//IMP_LOG_DBG(TAG, "[%d]:%10u,%10lld,%10u,%10u,%10u\n", i, stream->pack[i].length, stream->pack[i].timestamp, stream->pack[i].frameEnd, *((uint32_t *)(&stream->pack[i].nalType)), stream->pack[i].sliceType);
            IMPEncoderPack *pack = &stream.pack[i];
            if(pack->length){
                    uint32_t remSize = stream.streamSize - pack->offset;
                    if(remSize < pack->length){
                            #if(DUMPFILE)
                            fwrite( (void *)(stream.virAddr + pack->offset), remSize,1,stream_fd);
                            fwrite( (void *)stream.virAddr, pack->length - remSize, 1, stream_fd);
                            #else
                            play( (stream.virAddr + pack->offset), remSize );
                            play( stream.virAddr, pack->length - remSize );
                            
                            #endif
                    }else {
                            #if(DUMPFILE)
                            fwrite( (void *)(stream.virAddr + pack->offset), pack->length, 1, stream_fd);
                            #else
                             play( (stream.virAddr + pack->offset), pack->length );
                            #endif
                    }
            }
    }

    
    #if(DUMPFILE)
    if (ret < 0) {
      fclose(stream_fd);
      return ;
    }
    #endif

    IMP_Encoder_ReleaseStream(chnNum, &stream);
  }




  return ;
    
    
}

int T31H264::T31H264Init()
{

    unsigned int i;
    int ret;
    pthread_t tid[FS_CHN_NUM];

    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            chnNum = chn[i].index;
            break;
        }
    }


    return 0;
}

int T31H264::T31H264Exit()
{

  int ret = IMP_Encoder_StopRecvPic(chnNum);
  if (ret < 0) {
    SError << "IMP_Encoder_StopRecvPic failed" <<  chnNum;
    return -1;
  }
      
    return ret;
}

 T31H264::~T31H264() {

  stop();
  join();
  
  T31H264Exit();

}



/*******************************************************************************************
  T31H264 end
*******************************************************************************************/





/*******************************************************************************************
  LiveThread start
*******************************************************************************************/


int LiveThread::T31Init()
{
    int i, ret;

    /* Step.1 System init */
    ret = sample_system_init();
    if (ret < 0) {
        SError<<"IMP_System_Init() failed";
        return -1;
    }

    chn[3].enable = 0;
    chn[2].enable = 1;
    ret = sample_framesource_init();
    if (ret < 0) {
        SError<<"FrameSource init failed";
        return -1;
    }

    ret = sample_framesource_ext_rgba_init();
    if (ret < 0) {
        SError<<"FrameSource init ext rgba failed";
        return -1;
    }

    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_Encoder_CreateGroup(chn[i].index);
            if (ret < 0) {
                SError<<"IMP_Encoder_CreateGroup(%d) error: " <<  i ;
                return -1;
            }
        }
    }

    /* Step.3 Encoder init */
    ret = sample_encoder_init();
    if (ret < 0) {
        SError<<"Encoder init failed";
        return -1;
    }

    /* Step.4 Bind */
    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
            if (ret < 0) {
                SError<<"Bind FrameSource channel%d and Encoder failed: " << i ;
                return -1;
            }
        }
    }

    /* Step.5 Stream On */
    ret = sample_framesource_streamon();
    if (ret < 0) {
        SError<<"ImpStreamOn failed";
        return -1;
    }
    ret = sample_framesource_ext_rgba_streamon();
    if (ret < 0) {
        SError<<"ImpStreamOn ext rgba failed";
        return -1;
    }


    /* Step.6 Get stream */



    // ret = sample_get_video_stream();  // For hh264 stream
    // if (ret < 0) {
    //     SError<<"Get H264 stream failed";
    //     return -1;
    // }


    return 0;
}


int LiveThread::T31Exit()
{
     int ret;
    /* Exit sequence as follow */

    /* Step.a Stream Off */
    ret = sample_framesource_streamoff();
    if (ret < 0) {
        SError<<"FrameSource StreamOff failed";
        return -1;
    }

    ret = sample_framesource_ext_rgba_streamoff();
    if (ret < 0) {
        SError<<"FrameSource StreamOff  ext rgba failed";
        return -1;
    }

    /* Step.b UnBind */
    for (int i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
            if (ret < 0) {
                SError<<"UnBind FrameSource channel%d and Encoder failed: "  << i ;
                return -1;
            }
        }
    }

    /* Step.c Encoder exit */
    ret = sample_encoder_exit();
    if (ret < 0) {
        SError<<"Encoder exit failed";
        return -1;
    }

    /* Step.d FrameSource exit */
    ret = sample_framesource_exit();
    if (ret < 0) {
        SError<<"FrameSource exit failed";
        return -1;
    }
    ret = sample_framesource_ext_rgba_exit();
    if (ret < 0) {
        SError<<"FrameSource exit rgba exit failed";
        return -1;
    }

    /* Step.e System exit */
    ret = sample_system_exit();
    if (ret < 0) {
        SError<<"sample_system_exit() failed";
        return -1;
    }
}




int LiveThread::XAInit()
{


  //  #if(DUMPFILE)

  //   std::ifstream f("./arvind.rgba"); //taking file as inputstream
  //   std::string str;
  //   if(f) {
  //      std::stringstream ss;
  //      ss << f.rdbuf(); // reading data
  //      str = ss.str();
  //   }
  //   else
  //   {
  //       SError << " rgba file does not exist ";
  //       return -1;
  //   }

    
    
  //  //unsigned long  p_width = 0;
  //  //unsigned long  p_height = 0;
  //  //size_t  p_output_size = 0;
  //  //unsigned char *  tmp = rgbaMagic_decode( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_RGB , 0,  &p_width , &p_height , &p_output_size );
   
  //  //w=640 h=360
           
           
  // unsigned long  width = 640;
  // unsigned long  height = 360;
  // size_t  p_output_size = 0;
  
    
  // //unsigned char *  bgrBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_BGR , 0, width , height , &p_output_size );
  // //write_bmp(bgrBuf, width, height, "arvind.bmp"  );
  // //free(bgrBuf) ;
  
  // unsigned char *  rgbBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_RGB , 0, width , height , &p_output_size );

  
  // #endif
  

  

    cnfg::Configuration config;

    config.load("./configXA.json");

   
    
    if (!config.loaded()) 
    {
        SError << "Could not load config";

        return -1;
    }
     std::string xaconfig = config.root.dump();
     
    
    xa_fi_error_t returnValue;

    const char* path_to_vision_cell =
        "/mnt/libxailient-fi-vcell.so";                    // For shared lib
    returnValue = xa_sdk_initialize(path_to_vision_cell);  // For shared lib

    // returnValue = xa_sdk_initialize(); // For static lib

    if (returnValue != XA_ERR_NONE) {
        SError << "Error at xa_sdk_initialize";

        return -1;
    }

    const char* configuration = xaconfig.c_str();

    STrace << "config json: " << configuration;

    // xa_sdk_update_identities
    // xa_sdk_update_identity_image
    returnValue = xa_sdk_configure(configuration);
    if (returnValue != XA_ERR_NONE) {
        SError << "Error at xa_sdk_configure";

        return -1;
    }

    returnValue = xa_sdk_is_face_recognition_enabled();
    if (returnValue != XA_ERR_NONE) {
        SError << "Error at xa_sdk_configure";

        return -1;
    }


    /*
    cnfg::Configuration event;
    event.load("./event.json");

    if( event.root.find("registrationImage") == event.root.end()) 
    {
       SError  << " no registrationImage found in event" ;

       return -1;
    }

    XA_addGallery(event.root["registrationImage"].get<std::string>()) ;
    */

//    for( int x = 0; x < 100; ++x)
//    {
//        XAProcess( rgbBuf , width, height );
//
//        base::sleep(700);
//    }
    
  // #if(DUMPFILE)
  
  // free(rgbBuf) ;
  
  // #endif




}



 LiveThread::~LiveThread() {


  T31Exit();

}


void LiveThread:: stop()
{
    t31rgba.stop();
    t31h264.stop();
}


void LiveThread::start()
{
     XAInit();
     T31Init();

    //T31H264 t31h264;
    //T31RGBA t31rgba;
    //t31h264.T31H264Init();
    t31rgba.T31RGBAInit();

    t31h264.start();
    t31rgba.start();
}


}}
