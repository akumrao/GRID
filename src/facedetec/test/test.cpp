

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



//#define JSON_ASSERT(x) /* value */
//#define assert(x)

#include <json/json.hpp>
using json = nlohmann::json;
#include "json/configuration.h"


using namespace base;
using namespace base::net;
using namespace base::cnfg;
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


    ConsoleChannel *ch = new ConsoleChannel("debug", Level::Info);
    Logger::instance().add(ch);
    //test::init();


    
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
  
    
  unsigned char *  bgrBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_BGR , 0, width , height , &p_output_size );
   
  //write_bmp(bgrBuf, width, height, "arvind.bmp"  );
  
    // test["arv"] = 1;

#if XALIENT_TEST
  

    cnfg::Configuration config;

    config.load("./config.json");

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
        XAProcess( bgrBuf , width, height );

        base::sleep(700);
    }
    
  
  
  free(bgrBuf) ;
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
