

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

#include "base/rgba_bitmap.h"
#include "base/base64.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "base/stb_image.h"
#include "t31Video.h"
#include "Settings.h"

#include <json/json.hpp>
using json = nlohmann::json;
#include "json/configuration.h"


using namespace base;
using namespace base::net;
using namespace base::cnfg;




void RestAPI(std::string method, std::string uri)
{
    json m;
    
    m["xailientDeviceID"]= "surya";

    
   //  json m;

   //  m["messageType"] = "IDENTITY_NOT_IN_GALLERY";
                
    std::string sendMe = m.dump();
    
    SInfo <<  "sendit " << sendMe;

    
      //ClientConnecton *conn = new HttpsClient( "https", "ipcamera.adapptonline.com", 8080, uri);
    ClientConnecton *conn = new HttpsClient( "https", "backend.adapptonline.com/eventsToCloudX", 443, uri);
    //Client *conn = new Client("http://zlib.net/index.html");
    conn->fnComplete = [&](const Response & response) {
        std::string reason = response.getReason();
        StatusCode statuscode = response.getStatus();
     //   std::string body = conn->readStream() ? conn->readStream()->str() : "";
      //  STrace << "Post API reponse" << "Reason: " << reason << " Response: " << body;
    };

    conn->fnConnect = [&, sendMe](HttpBase * con) {
        
         SInfo <<  "sendit " << sendMe;
        
        con->send( sendMe.c_str(), sendMe.length());
        
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
    conn->_request.setContentType("application/x-www-form-urlencoded");
    
    
    conn->setReadStream(new std::stringstream);
    conn->send();
    
}




int main(int argc, char** argv) {


    ConsoleChannel *ch = new ConsoleChannel("trace", Level::Trace);
    Logger::instance().add(ch);
    //test::init();


 
    Settings::configuration.dtlsCertificateFile = "/var/tmp/key/certificate.crt";

    
  Application app;

    
    
   // RestAPI("GET", "/"); //GET, POST, PUT, DELETE
    
    RestAPI("POST", "/"); //GET, POST, PUT, DELETE
    
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


    std::cout << "done:" << std::endl << std::flush;
    sleep(500);

    app.waitForShutdown([&](void*) {



    }

    );



   




}
