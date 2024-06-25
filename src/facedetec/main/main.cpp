
//#include "base/test.h"
#include "base/logger.h"
#include "base/filesystem.h"
#include "base/platform.h"
#include "base/application.h"
#include "livethread.h"


// #include <fstream>   // std::ifstream
// #include <iostream>  // std::cout

// #include <chrono>
// #include <thread>


using namespace base;


int main(int argc, char** argv) {
    // Logger::instance().add(new RotatingFileChannel("test", "/tmp/test",
    // Level::Trace, "log", 10));

    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    // =========================================================================

    Application app;


    bool recording = false;

     base::web_rtc::LiveThread livethread("live", nullptr,nullptr, recording );
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
