

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


using namespace base;
using namespace base::net;

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

    
    
   //unsigned long  p_width = 0;
   //unsigned long  p_height = 0;
   //size_t  p_output_size = 0;
   //unsigned char *  tmp = rgbaMagic_decode( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_RGB , 0,  &p_width , &p_height , &p_output_size );
   
   //w=640 h=360
           
           
  unsigned long  width = 640;
  unsigned long  height = 360;
  size_t  p_output_size = 0;
  
    
  unsigned char *  bgrBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_BGR , 0, width , height , &p_output_size );
   
           
  write_bmp(bgrBuf, width, height, "arvind.bmp"  );
  
  
  free(bgrBuf) ;
    
    
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



   
    json test = json::object();

    // test["arv"] = 1;

    std::string msg = "Hello, world!";

    std::cout << msg << std::endl;

#if XALIENT_TEST
  

    {

        xa_fi_error_t returnValue;

        const char * path_to_vision_cell = "/mnt/libxailient-fi-vcell.so"; // For shared lib
        returnValue = xa_sdk_initialize(path_to_vision_cell); // For shared lib

        // returnValue = xa_sdk_initialize(); // For static lib

        if (returnValue != XA_ERR_NONE) {


             SError << "xa_sdk_initialize failed ";

        }



    }

#endif



}
