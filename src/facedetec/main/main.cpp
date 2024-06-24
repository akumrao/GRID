
//#include "base/test.h"
#include "base/logger.h"
#include "base/filesystem.h"
#include "base/platform.h"



#include "net/netInterface.h"
#include "http/client.h"
#include "base/logger.h"
#include "base/application.h"
#include "base/platform.h"

#include "http/url.h"
#include "base/filesystem.h"
#include "http/HttpClient.h"
#include "http/HttpsClient.h"
#include "http/HTTPResponder.h"


//#include "json/json.hpp"
#include <fstream>   // std::ifstream
#include <iostream>  // std::cout

#include <chrono>
#include <thread>


#include "livethread.h"

using std::cerr;
using std::cout;
using std::endl;

using namespace base;
using namespace base::net;
using namespace base::web_rtc;

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
    // Logger::instance().add(new RotatingFileChannel("test", "/tmp/test",
    // Level::Trace, "log", 10));

    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    // =========================================================================

    Application app;


    bool recording = false;

     LiveThread livethread("live", nullptr,nullptr, recording );
     livethread.start();

    
   // RestAPI("GET", "/"); //GET, POST, PUT, DELETE  // Rest API TEst
    
    

   // net::ClientConnecton *m_client = new HttpsClient("wss", "ipcamera.adapptonline.com", 443, "/");  //Websocket test


   // // conn->Complete += sdelegate(&context, &CallbackContext::onClientConnectionComplete);
   // m_client->fnComplete = [&](const Response & response) {
   //     std::string reason = response.getReason();
   //     StatusCode statuscode = response.getStatus();
   //     std::string body = m_client->readStream() ? m_client->readStream()->str() : "";
   //     STrace << "SocketIO handshake response:" << "Reason: " << reason << " Response: " << body;
   // };

   // m_client->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {
   //     STrace << "client->fnPayload " << std::string(data, sz);
   //     //m_ping_timeout_timer.Reset();
   //     //m_packet_mgr.put_payload(std::string(data,sz));
   // };

   // m_client->fnClose = [&](HttpBase * con, std::string str) {
   //     STrace << "client->fnClose " << str;
   //     //close(0,"exit");
   //     //on_close();
   // };

   // m_client->fnConnect = [&](HttpBase * con) {

   //     m_client->send("{\"messageType\": \"createorjoin\", \"room\": \"room11\"}");

   //     std::cout << "onConnect:";
   // };


   // //  conn->_request.setKeepAlive(false);
   // m_client->setReadStream(new std::stringstream);
   // m_client->send();
   // LTrace("sendHandshakeRequest over")


    app.waitForShutdown([&](void*) {


        SInfo << "about to exit application" ;

        livethread.stop();

    }

    );




}
