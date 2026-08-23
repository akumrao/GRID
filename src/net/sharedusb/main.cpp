/**
 *
  https://webrtc.googlesource.com/src/+/HEAD/modules/pacing/g3doc/index.md

  https://webrtc.googlesource.com/src/+/HEAD/p2p/g3doc/ice.md
 * 
 * https://github.com/creytiv/re
 * https://en.wikipedia.org/wiki/UDP_hole_punching 
  cricket::Candidate represents an address discovered by a cricket::Port. A candidate can be local (i.e discovered by a local port) or remote. Remote candidates are transported using signaling, i.e outside of webrtc. There are 4 types of candidates: local, stun, prflx or relay (standard)

 */


#include "helpers.hpp"

//#include "socketio/socketioClient.h"

#include "http/HttpClient.h"
#include "http/HttpsClient.h"


//#include "Settings.h"
#include "RestAPI.h"
#include "uv.h"

#include "base/logger.h"
#include "json/configuration.h"
#include "json/confSettings.h"

#include "sctptransport.hpp"
#include "DtlsTransport.h"

#include "peerconnection.h"

#include "http/HttpsClient.h"

#include "server.h"
#include "DepUsrSCTP.h"


//volatile bool force_exit = false;

//struct server *server{nullptr};

#define CERTFROMFILE 1

using namespace rtc;
using namespace std;
using namespace base;
using namespace base::net;


using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) {
    return ptr;
}


std::mutex clients_mutex;
/// all connected clients
unordered_map<string, shared_ptr<Client>> clients;
;

shared_ptr<Client> createPeerConnection(Configuration &config, string id, bool isClient);

void addToStream(shared_ptr<Client> client, bool isAddingVideo);
void startStream();

const string defaultIPAddress = "127.0.0.1";
const uint16_t defaultPort = 8000;
string ip_address = defaultIPAddress;
//uint16_t port = defaultPort;


//sockio::Socket *mysocket = nullptr;
std::string from;
std::string room;
Configuration settingconfig;

bool isClient = false;// only one id possible. Multiple connect of client will connect one server 

// Explicit function to completely destroy an active peer session
void destroyClient(const string& clientId) {
    
    SInfo << "destroyClient clientId: " << clientId;
    
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = clients.find(clientId);
    if (it != clients.end()) {
        SInfo << "Cleaning up and destroying peer connection object: " << clientId << std::endl;
        if (it->second && it->second->peerConnection) {
            try {
                it->second->peerConnection->close(); // Cease background async processing thread loops
            } catch (const std::exception& e) {
                SError << "Exception thrown during WebRTC object close: " << e.what() << std::endl;
            }
        }
        clients.erase(it); // Erase from mapping container to drop shared_ptr allocation
    }
}

#if 1

 ClientConnecton *m_client = nullptr;

  // Helper function to emit data over native websocket matching the server
// protocol wrapper
void emitWebSocketEvent(const std::string &eventName, const json &payload) {
  json outerPacket;
  outerPacket["event"] = eventName;
  outerPacket["payload"] = payload;

  // Convert to string and send as text frame
  m_client->send(outerPacket.dump());
}


void sendCandidate(const std::string &mid, int mlineindex,
                   const std::string &sdp) {
  json desc;
  desc["sdpMid"] = mid;
  desc["sdpMLineIndex"] = mlineindex;
  desc["candidate"] = sdp;

  json m;
  m["type"] = "candidate";
  m["candidate"] = desc;

  if (!from.empty()) {
    m["from"] = from;
    m["to"] = from;
  }

  m["room"] = room;
  SInfo << "send:" << sdp << "candidate to: " << from << std::endl;

  // Converted to native protocol wrapper routing
#if socketio
  mysocket->emit("message", m);
#else
  emitWebSocketEvent("message", m);
#endif
}

void sendSdp(const std::string &sdp, const std::string &type) {
  json desc = {{"type", type}, {"sdp", sdp}};

  json m;
  m["type"] = type;
  m["desc"] = desc;

  if (!from.empty()) {
    m["from"] = from;
    m["to"] = from;
  }

  m["room"] = room;

  SInfo << "send:" << type << " to: " << from << std::endl;

  // Converted to native protocol wrapper routing
#if socketio
  mysocket->emit("message", m);
#else
  emitWebSocketEvent("message", m);
#endif
}




#endif


#if 1

void wsOnMessage(json const &m) {


    std::string type;

    std::string to;
    std::string user;
    
    std::string id;

    if (m.find("room") != m.end()) {
        room = m["room"].get<std::string>();
    } else {
        SError << " On Peer message is missing room id ";
        return;
    }



    if (m.find("type") != m.end()) {
        type = m["type"].get<string>();

    }



    if (m.find("to") != m.end()) {
        to = m["to"].get<std::string>();
    }

    if (m.find("from") != m.end()) {
        from = m["from"].get<std::string>();
        if (!isClient)
            id = from;
        else
            id = "client";// only one id possible. Multiple connect of client will connect one server 
    } else {
        SError << " On Peer message is missing participant id ";
        return;
    }

    if (m.find("type") != m.end()) {
        type = m["type"].get<std::string>();
    } else {
        SError << " On Peer message is missing SDP type";
    }




    if (m.find("cam") != m.end()) {
        std::string id = m["cam"].get<std::string>();

    }


    if (m.find("starttime") != m.end()) {
        //  camT.start = m["starttime"].get<std::string>();

    }


    if (type == "offer") {

       // if (clients.find(id) != clients.end())
       //     clients.erase(id);
        
        SInfo << " offer from clinet id " <<   id;
        
        destroyClient(id); // Safe erasure structure handles cleanup safely instead of raw `.erase()`

        {
          std::lock_guard<std::mutex> lock(clients_mutex);
          clients.emplace(id, createPeerConnection(settingconfig, id, false));
        }

        //clients.emplace(id, createPeerConnection(config,  id));
        if (auto jt = clients.find(id); jt != clients.end()) {
            auto pc = jt->second->peerConnection;

            auto sdp = m["desc"]["sdp"].get<string>();

            SInfo << "setRemoteDescription " << type;

            auto description = Description(sdp, type);
            pc->setRemoteDescription(description);
            pc->setLocalDescription();
        }


    } else if (type == "answer") {

        
        SInfo << "Answer to id " << id;
        //clients.emplace(id, createPeerConnection(config,  id));
        if (auto jt = clients.find(id); jt != clients.end()) {
            auto pc = jt->second->peerConnection;

            auto sdp = m["desc"]["sdp"].get<string>();

            SInfo << "setRemoteDescription " << type;

            auto description = Description(sdp, type);
            pc->setRemoteDescription(description);
            // pc->setLocalDescription( Description::Type::Answer);
        }
    } else if (type == "candidate") {

        json cand = m["candidate"];

        auto sdp = cand["candidate"].get<std::string>();
        auto mid = cand["sdpMid"].get<std::string>();

        if (auto jt = clients.find(id); jt != clients.end()) {
            auto pc = jt->second->peerConnection;
            //auto sdp = m["desc"].get<string>();
            //auto description = Description(sdp, type);
            // pc->setRemoteDescription(description);

            SInfo << sdp;


            Candidate tmp = rtc::Candidate(sdp, mid);
            pc->addRemoteCandidate(tmp);
        }



    }

}

void initiate(std::string rm) {

    
    room = rm;
    std::string id = "client";//   only one id possible. Multiple connect of client will connect one server 
    isClient = true;
    destroyClient(id); // Safe structural cleanup

    std::lock_guard<std::mutex> lock(clients_mutex);
    clients.emplace(id, createPeerConnection(settingconfig, id, true));
}

#endif

int main(int argc, char **argv) {

  {
    /////////////////////////////////////////////////
    base::cnfg::Configuration cache;

    cache.load("./cache.js");

    ConfSettings::SetConfiguration(cache.root);



   //rtc::SctpTransport::Init();
    

        
    //rtc::SctpSettings mCurrentSctpSettings = {};
   // rtc::SctpTransport::SetSettings(mCurrentSctpSettings);


    bool printHelp = false;
    //int c = 0;

    Application app;

    //Async async;


    if (printHelp) {
        cout << "usage: stream-h264 [-a opus_samples_folder] [-b h264_samples_folder] [-d ip_address] [-p port] [-v] [-h]" << endl
                << "Arguments:" << endl
                << "\t -p " << "Signaling server port (default: " << defaultPort << ")." << endl
                << "\t -v " << "Enable debug logs." << endl
                << "\t -h " << "Print this help and exit." << endl;
        return 0;
    }



    string stunServer = "stun:stun.l.google.com:19302";
    cout << "STUN server is " << stunServer << endl;
    settingconfig.iceServers.emplace_back(stunServer);
    settingconfig.disableAutoNegotiation = true;
    
    settingconfig.portdefault =0;
    
      
    // read cert from file
#if CERTFROMFILE == 1
    settingconfig.gconfig->keyPemFile = ConfSettings::configuration.keyFile;
    settingconfig.gconfig->certificatePemFile = ConfSettings::configuration.certFile;
    settingconfig.gconfig->keyPemPass = "12345678";

#elif CERTFROMFILE == 2

    /* convert pem to single line
     * # awk 'NF {sub(/\r/, ""); printf "%s\\n",$0;}' certificate.crt  
     */

    settingconfig.keyPemFile = "";
    settingconfig.certificatePemFile = "";
    // settingconfig.keyPemPass = "12345678";

#else

#endif

  //  string localId = "server";
  //  cout << "The local ID is: " << localId << endl;

    rtc::DtlsTransport::ClassInit();
       DepUsrSCTP::ClassInit();


#if 1   
    std::string room = "65f570720af337cec5335a70ee88cbfb7df32b5ee33ed0b4a896a0";
    std::string host = ip_address;
    int port = 8443;

    #if 1
    std::ostringstream url;
    bool ssl = true;
    //std::string host = SERVER_HOST;
    //int port = SERVER_PORT;

    url << "/";

   

    if (!ssl) {
      m_client = new HttpClient("ws", host, port, url.str());
    } else {
      m_client = new HttpsClient("wss", host, port, url.str());
    }

    // conn->Complete += sdelegate(&context,
    // &CallbackContext::onClientConnectionComplete);
    m_client->fnComplete = [&](const Response &response) {
      std::string reason = response.getReason();
      StatusCode statuscode = response.getStatus();
      std::string body =
          m_client->readStream() ? m_client->readStream()->str() : "";
      STrace << "SocketIO handshake response:" << "Reason: " << reason
             << " Response: " << body;
    };

    m_client->fnConnect = [&](HttpBase *con) {
      STrace << "client->fnConnect ";
      //  m_con_state = con_opened;
      // m_reconn_timer.Stop();

      SInfo << "Connected securely to native WebSocket server." << std::endl;

      // Map the primary handshake logic registration event sequence
      json joinPayload;
      joinPayload["roomId"] = room;
      joinPayload["client"] =    false; // Mirrors client state property tracking requirements

      emitWebSocketEvent("createorjoin", joinPayload);

    };

    m_client->fnPayload = [&](HttpBase *con, const char *data, size_t sz) {
      STrace << "client->fnPayload " << std::string(data, sz);
      try {
        // Parse the outer payload protocol layer out of the text string frame
        // execution
        json packet = json::parse(std::string(data, sz));
        std::string eventName = packet["event"].get<std::string>();
        json data = packet["payload"];

        if (eventName == "created") {
          SInfo << data.dump() << std::endl;
          SInfo << "ws: Created room " << data[0] << "- my  ID is "
                << data[1] << std::endl;
        } else if (eventName == "join") {
          SInfo << "ws join " << data.dump() << std::endl;
          SInfo << "ws: Created room " << data[0] << "- my ID is "
                << data[1] << " noClientInRoom: " << data[2] << std::endl;

          std::string room1 = data[0].get<std::string>();
          std::string id = data[1].get<std::string>();
          int noClientInRoom = data[2].get<int>();

          if (noClientInRoom > 1) {
            initiate(room1);
          }
        } else if (eventName == "joined") {
          SInfo << "ws joined " << data.dump() << std::endl;
        } else if (eventName == "message") {
          STrace << "SocketioClient received message: " << data.dump()
                 << std::endl;
          wsOnMessage(data);
        } else if (eventName == "disconnectClient") {
          std::string clientFrom = data.get<std::string>();
          SInfo << "disconnectClient " << clientFrom << std::endl;
          LInfo(data.dump());
        } else if (eventName == "bye") {
          SInfo << data.dump() << std::endl;
        }
      } catch (const std::exception &e) {
        std::cerr << "JSON Parsing runtime error handling text frames: "
                  << e.what() << std::endl;
      }
   
    };

    m_client->fnClose = [&](HttpBase *con, std::string str) {
      SInfo << "client->fnClose " << str;
      // close(0,"exit");
      // on_close();
      //emitWebSocketEvent("bye", "");
      
      
    
      
      SInfo << "WebSocket connection closed by endpoint structure.";
      m_client->Close();
      delete m_client;
      m_client = nullptr;

      //            m_con_state = con_closed;
    };

    //  conn->_request.setKeepAlive(false);
    m_client->setReadStream(new std::stringstream);
    m_client->send();
    LTrace("sendHandshakeRequest over")

    #endif

#endif

    app.waitForShutdown([&](void*) {

      
      
        json m;
      m["type"] = "bye";
      emitWebSocketEvent("message", m);
      
      
           json joinPayload;
      joinPayload["roomId"] = "oom";
      joinPayload["client"] =   false; // Mirrors client state property tracking requirements

      emitWebSocketEvent("createorjoin", joinPayload);
      
      
      
     
      
      {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (auto& pair : clients) {
            if (pair.second && pair.second->peerConnection) {
                pair.second->peerConnection->close();
            }
        }
        clients.clear();
      }

    
      
      
        m_client->Close();
        //delete m_client;
        
     
       //  rtc::SctpTransport::Cleanup();
      
       // ClassDestroy();
        
        SInfo << "app.run() is over";
         
//        restApi->shutdown();
//        Settings::exit();         
//        rtc::CleanupSSL();
        
        
        DepUsrSCTP::ClassDestroy();
        Logger::destroy();
        

        //    if(ctx->txt)
        //    delete ctx->txt;
        //    ctx->txt = nullptr;

        //    restApi->stop();

        //    restApi->shutdown();

    });

    
  }


    SInfo << "Cleaning up..." << endl;
    return 0;

}



#if 1


shared_ptr<Client> createPeerConnection(Configuration &config, string id, bool isClient) {
    SInfo << "createPeerConnection id " << id  << " is client " << isClient ;


    auto pc = make_shared<PeerConnection>(config);
    auto client = make_shared<Client>(pc);

    pc->onStateChange([id](PeerConnection::State state) {
        SInfo << "State: " << state << endl;
        if (state == PeerConnection::State::Disconnected ||
                state == PeerConnection::State::Failed ||
                state == PeerConnection::State::Closed) {
            // remove disconnected client
            //MainThread.dispatch([id]() 
            {
                clients.erase(id);

                       // int x = 1; //arvind
            }
            //);
        }
    });



    pc->onLocalDescription([ id, pc](rtc::Description description) {
        //		json message = {{"id", id},
        //		                {"type", description.typeString()},
        //		                {"description", std::string(description)}};

        SInfo << "send:" << description.typeString() << " des " << std::string(description);

        //  pc->setLocalDescription(Description::Type::Offer);// Description::Type::Answer);          
        sendSdp(std::string(description), description.typeString());
        // Make the answer
        //		if (auto ws = wws.lock())
        //			ws->send(message.dump());
    });

    pc->onLocalCandidate([ id](rtc::Candidate candidate) {
        //            json message = {{"id", id},
        //                            {"type", "candidate"},
        //                            {"candidate", std::string(candidate)},
        //                            {"mid", candidate.mid()}};

        SInfo << std::string(candidate);
        sendCandidate(candidate.mid(), 1, std::string(candidate));
        //            if (auto ws = wws.lock())
        //                    ws->send(message.dump());
    });

    pc->onGatheringStateChange(
            [](PeerConnection::GatheringState state) {
                SInfo << "Gathering State";
                if (state == PeerConnection::GatheringState::Complete) {
                    //  if(auto pc = wpc.lock())
                    {
                        //                json desc;
                        //                desc["type"] =  description->typeString();
                        //                desc[sdp] = sdp;
                        //    

                    }
                }
            });

    {
          
    std::string dcchat = "Settings::getdatachannel()";
    auto dc = pc->createDataChannel(dcchat);
    dc->onOpen([id, wdc = make_weak_ptr(dc)](){
        if (auto dc = wdc.lock()) {
            SInfo << "onOpen: ";
                    dc->send("Ping2");
        }
    });

//    dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg){
//        SInfo << "Message from " << id << " received: " << msg << endl;
//        if (auto dc = wdc.lock()) {
//
//            SInfo << "onOpen: " << msg;
//            sleep(1);
//            dc->send(" onMessage Ping");
//        }
//    });
//    
    
    
        dc->onClosed([id]() 
        {
             SInfo << "DataChannel from " << id << " closed" ;
        }
        );
    
          dc->onMessage([id, dc](auto data) {
            // data holds either std::string or rtc::binary
            if (std::holds_alternative<std::string>(data))
                SInfo << "Message from " << id << " received: " << std::get<std::string>(data)
                << std::endl;
            else
                SInfo << "Binary message from " << id
                    << " received, size=" << std::get<rtc::binary>(data).size() << std::endl;

          //  sleep(1);
            
             //dc->close();
            
      //  rtc::binary buffer = { std::byte(0x01), std::byte(0x02), std::byte(0x03) };
    //    dc->send(buffer);

        // Approach 2: Sending from a raw data chunk (e.g., loaded file or hardware frame)
       // uint8_t raw_bytes[] = { 0x04, 0x05, 0x06, 0x07 };
       // dc->send(reinterpret_cast<const std::byte*>(raw_bytes), sizeof(raw_bytes));

    
    
    
            
         //  dc->send("Send to web2");
        });
    
    
    client->dataChannel1 = dc;
    }



    pc->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
        SInfo << "DataChannel from " << id << " received with label \"" << dc->label();


        dc->onOpen([wdc = make_weak_ptr(dc)](){
            if (auto dc = wdc.lock()) {
                SInfo << "DataChannel 2: Open" << endl;
                dc->send("Ping1");
            }
        });


        dc->onClosed([id]() 
        {
             SInfo << "DataChannel from " << id << " closed" ;
        }
        );

        dc->onMessage([id, dc](auto data) {
            // data holds either std::string or rtc::binary
            if (std::holds_alternative<std::string>(data))
                SInfo << "Message from " << id << " received: " << std::get<std::string>(data)
                << std::endl;
            else
                SInfo << "Binary message from " << id
                    << " received, size=" << std::get<rtc::binary>(data).size() << std::endl;

            sleep(1);
            dc->send("Send to web1");
           // dc->close();
        });

        client->dataChannel2 = dc;
    });

    if (isClient)
        pc->setLocalDescription();
    return client;
};
#endif
