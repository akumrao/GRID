

#include <string>

#include <xailient-fi/sdk_json_interface.h>
#include <iostream>
#include <pthread.h>                                                            


#include "net/netInterface.h"
#include "http/client.h"
#include "base/logger.h"
#include "base/application.h"
#include "base/platform.h"

#include "http/url.h"
#include "base/filesystem.h"
#include "http/HttpClient.h"
#include "http/HttpsClient.h"
#include "base/platform.h"


#include "http/HTTPResponder.h"
#include "base/logger.h"

#include "rgba_bitmap.h"
#include "base/base64.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"



//#define JSON_ASSERT(x) /* value */
//#define assert(x)

#include <json/json.hpp>
using json = nlohmann::json;
#include "json/configuration.h"


using namespace base;
using namespace base::net;
using namespace base::cnfg;


typedef unsigned char uchar;
static const std::string b = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//=
static std::string base64_encode(const std::string &in) {
    std::string out;

    int val=0, valb=-6;
    for (uchar c : in) {
        val = (val<<8) + c;
        valb += 8;
        while (valb>=0) {
            out.push_back(b[(val>>valb)&0x3F]);
            valb-=6;
        }
    }
    if (valb>-6) out.push_back(b[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}


static std::string base64_decode(const std::string &in) {

    std::string out;

    std::vector<int> T(256,-1);
    for (int i=0; i<64; i++) T[b[i]] = i;

    int val=0, valb=-8;
    for (uchar c : in) {
        if (T[c] == -1) break;
        val = (val<<6) + T[c];
        valb += 6;
        if (valb>=0) {
            out.push_back(char((val>>valb)&0xFF));
            valb-=8;
        }
    }
    return out;
}





void RestAPI(std::string method, std::string uri)
{
    
    ClientConnecton *conn = new HttpsClient( "https", "ipcamera.adapptonline.com", 8080, uri);
    //Client *conn = new Client("http://zlib.net/index.html");
    conn->fnComplete = [&](const Response & response) {
        std::string reason = response.getReason();
        StatusCode statuscode = response.getStatus();
     //   std::string body = conn->readStream() ? conn->readStream()->str() : "";
      //  STrace << "Post API reponse" << "Reason: " << reason << " Response: " << body;
    };

    conn->fnConnect = [&](HttpBase * con) {

        std::cout << "fnConnect:";
    };

    conn->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {

        std::cout << "client->fnPayload " << data << std::endl << std::flush;
    };

    conn->fnUpdateProgess = [&](const std::string str) {
        std::cout << "final test " << str << std::endl << std::flush;
    };

    conn->_request.setMethod(method);
    conn->_request.setKeepAlive(false);
    conn->setReadStream(new std::stringstream);
    conn->send();
    
}
#if XALIENT_TEST

int XA_addGallery(std::string jpegBuffBase64 )
{

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

  
  cnfg::Configuration identity;

  identity.load("./identity.json");

          
  //if (< a new gallery manifest exists >) 
  if (identity.loaded()) 
  {
           std::string xaidentity = identity.root.dump();
           
        galleryIdentityManifest = xaidentity.c_str();

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
           
            SInfo << "remaining_identity_image_pairs passed: " << remaining_identity_image_pairs->identity_images[0].identity_id;
      }
      else {
          SError << "remaining_identity_image_pairs fails " << remaining_identity_image_pairs->identity_images[0].identity_id;
          return -1;
      }
  }

  const char * deviceCheckinJson = xa_sdk_get_device_checkin_json();
  
  SInfo << "deviceCheckinJson:" << deviceCheckinJson;


  free(img);

    //< perform a deviceCheckin with the deviceCheckinJson >

}

int XAProcess( uint8_t* buffer_containing_raw_rgb_data , int w, int h  )
{

    xa_fi_image_t image;
    image.width = w;
    image.height = h;

    image.pixel_format =
        XA_FI_COLOR_RGB888;  // signifies the buffer data format

   // uint8_t* buffer_containing_raw_rgb_data = new uint8_t [image.width*3*image.height];

    //memset(buffer_containing_raw_rgb_data, 0, image.width*3*image.height);


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
                    STrace << "json to Face Track Event endpoint: " <<  blob.json;

        
                     json event = json::parse(blob.json); 

                     if(  (event.find("eventType") != event.end())  &&  (event["eventType"].get<std::string>() ==  "IDENTITY_NOT_IN_GALLERY"))
                     {

                        if(  event.find("registrationImage") != event.end()) 
                        XA_addGallery(event["registrationImage"].get<std::string>()) ;
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
#endif


int main(int argc, char** argv) {


    ConsoleChannel *ch = new ConsoleChannel("trace", Level::Trace);
    Logger::instance().add(ch);
    //test::init();


    
     
//    cnfg::Configuration config;
//
//    config.load("./event.json");
//
//    if( config.loaded())
//    {
//        json event = json::parse(config.root.dump(4)); 
//
//        if(  (event.find("eventType") != event.end())  &&  (event["eventType"].get<std::string>() ==  "IDENTITY_NOT_IN_GALLERY"))
//        {
//
//           if(  event.find("registrationImage") != event.end()) 
//          // XA_addGallery(event["registrationImage"].get<std::string>()) ;
//           {
//                std::string jpegBuffBase64 = event["looselyCroppedImage"].get<std::string>();
//                
//                
//                SInfo << "jpegBuffBase64 " << jpegBuffBase64.size() ;
//
//                std::string out;
//              //  base64::Decoder dec;
//                //dec.decode(jpegBuffBase64, out);
//                
//                out = base64_decode(jpegBuffBase64);
//
//                SInfo << "base64 decode " << out.size() ;
//
//                int width, height , channels;
//                
////                if(!stbi_info_from_memory(out.c_str(), out.size(), &width, &height, &channels)) return -1;
////
////               SInfo << "wx: " << width  << " he: " << height <<  " ch: " << channels;
//
//
//                /* exit if the image is larger than ~80MB */
//              //  if(width && height > (80000000 / 4) / height) return -1;
//
//                unsigned char *img = stbi_load_from_memory(out.c_str(), out.size(), &width, &height, &channels, 3);
//                
//                int x = 1;
//                
//                free(img);
//           }
//           else
//            SError << "no registrationImage " <<  event.dump(4);  
//
//        }
//        else
//        {
//          SError << "no eventType " << event.dump(4);
//        }
//     
//    }
    
    
    std::ifstream f("./arvind.rgba"); //taking file as inputstream
    std::string str;
    if(f) {
       std::stringstream ss;
       ss << f.rdbuf(); // reading data
       str = ss.str();
    }
    else
    {
        SError << " rgba file does not exist ";
        return -1;
    }

    
    
   //unsigned long  p_width = 0;
   //unsigned long  p_height = 0;
   //size_t  p_output_size = 0;
   //unsigned char *  tmp = rgbaMagic_decode( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_RGB , 0,  &p_width , &p_height , &p_output_size );
   
   //w=640 h=360
           
           
  unsigned long  width = 640;
  unsigned long  height = 360;
  size_t  p_output_size = 0;
  
    
  //unsigned char *  bgrBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_BGR , 0, width , height , &p_output_size );
  //write_bmp(bgrBuf, width, height, "arvind.bmp"  );
  //free(bgrBuf) ;
  
  unsigned char *  rgbBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_RGB , 0, width , height , &p_output_size );

  
  
  
#if XALIENT_TEST
  

    cnfg::Configuration config;

    config.load("./config.json");

   
    
    if (!config.loaded()) 
    {
        SError << "Could not load config";
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


 

    for( int x = 0; x < 100; ++x)
    {
        XAProcess( rgbBuf , width, height );

        base::sleep(700);
    }
    
  
  
  free(rgbBuf) ;
  #endif  
    
  Application app;

    
    
//    RestAPI("GET", "/"); //GET, POST, PUT, DELETE
//    
//    
//
//   net::ClientConnecton *m_client = new HttpsClient("wss", "192.168.0", 443, "/");
//
//
//   // conn->Complete += sdelegate(&context, &CallbackContext::onClientConnectionComplete);
//   m_client->fnComplete = [&](const Response & response) {
//       std::string reason = response.getReason();
//       StatusCode statuscode = response.getStatus();
//       std::string body = m_client->readStream() ? m_client->readStream()->str() : "";
//       STrace << "SocketIO handshake response:" << "Reason: " << reason << " Response: " << body;
//   };
//
//   m_client->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {
//       STrace << "client->fnPayload " << std::string(data, sz);
//       //m_ping_timeout_timer.Reset();
//       //m_packet_mgr.put_payload(std::string(data,sz));
//   };
//
//   m_client->fnClose = [&](HttpBase * con, std::string str) {
//       STrace << "client->fnClose " << str;
//       //close(0,"exit");
//       //on_close();
//   };
//
//   m_client->fnConnect = [&](HttpBase * con) {
//
//       m_client->send("{\"messageType\": \"createorjoin\", \"room\": \"room11\"}");
//
//       std::cout << "onConnect:";
//   };
//
//
//   //  conn->_request.setKeepAlive(false);
//   m_client->setReadStream(new std::stringstream);
//   m_client->send();
//   LTrace("sendHandshakeRequest over")




    app.waitForShutdown([&](void*) {



    }

    );



   




}
