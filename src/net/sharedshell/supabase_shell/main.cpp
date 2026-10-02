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
#include "sdpcommon.h"

/*************************************************************************/
#include "ttydutils.h"

volatile bool force_exit = false;


TTYServer ttyServer;

static void signal_cb(uv_signal_t *handle, int signum) {
    char sig_name[20];

    switch (handle->signum) {
        case SIGINT:
        case SIGTERM:
            get_sig_name(handle->signum, sig_name, sizeof (sig_name));
            printf("received signal: %s (%d), exiting...\n", sig_name, handle->signum);
            break;
        default:
            signal(SIGABRT, SIG_DFL);
            abort();
    }

    if (force_exit) exit(EXIT_FAILURE);
    force_exit = true;

    //lws_cancel_service(context); arvind
    uv_signal_stop(handle);
    uv_stop(handle->loop);

    printf("send ^C to force exit.\n");
}



/*************************************************************************/





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



 std::thread heartbeat_thread;

std::string g_currentRoomTopic = "";
std::atomic running(true);
std::atomic phx_replied(false);




std::mutex clients_mutex;
/// all connected clients
unordered_map<string, shared_ptr<Client>> clients;
;

shared_ptr<Client> createPeerConnection(Configuration &config, string id, bool isClient);

void addToStream(shared_ptr<Client> client, bool isAddingVideo);
void startStream();




//sockio::Socket *mysocket = nullptr;
std::string from;
std::string room;
Configuration settingconfig;

bool isClient = false; // only one id possible. Multiple connect of client will connect one server 

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

// Enqueue arbitrary strings safely for outbound transmission via m_client
void EnqueueOutboundMessage(const std::string& msg) {
    if (m_client) {
        m_client->send(msg.c_str(), msg.size(), false);
    } else {
        std::cerr << "[WS Send Failure] Cannot send packet: Client connection instance unavailable!" << std::endl;
    }
}


// =========================================================================
// CORRECTED SIGNALING INTERFACE (CLEAN JSON PAYLOADS - NO DOUBLE SERIALIZATION)
// =========================================================================

void sendCandidate(const std::string &clientId_, const std::string &mid, int mlineindex, const std::string &sdp) {
    // 1. Build the clean, internal candidate payload structure
    json desc;
    desc["sdpMid"] = mid;
    desc["sdpMLineIndex"] = mlineindex;
    desc["candidate"] = sdp;

    // 2. Wrap it natively inside the Supabase Broadcast envelope structure
    json root;
    root["topic"] = g_currentRoomTopic;
    root["event"] = "broadcast";
    root["ref"] = "2";

    json payload;
    payload["type"] = "broadcast";
    payload["event"] = "signal-to-viewer";
    
    json innerPayload;
    innerPayload["targetId"] = clientId_;
    innerPayload["signal"]["type"] = "candidate";
    
    // Embed the structured json block directly to eliminate string escaping
    innerPayload["signal"]["candidate"] = desc;

    payload["payload"] = innerPayload;
    root["payload"] = payload;

    std::string dumpedMsg = root.dump();
    std::cout << "[WS Outbound Candidate] Output payload: " << dumpedMsg << std::endl;
    
    // 3. Dispatch straight onto the low-level transmission interface
    EnqueueOutboundMessage(dumpedMsg);
}

void sendSdp(const std::string &clientId_, const std::string &sdp, const std::string &type) {
    // 1. Setup the structural frame envelope targeted towards the viewer connection mesh
    json root;
    root["topic"] = g_currentRoomTopic;
    root["event"] = "broadcast";
    root["ref"] = "2";

    json payload;
    payload["type"] = "broadcast";
    payload["event"] = "signal-to-viewer";
    
    json innerPayload;
    innerPayload["targetId"] = clientId_;
    innerPayload["signal"]["type"] = type; // "answer" or "offer"
    innerPayload["signal"]["sdp"] = sdp;

    payload["payload"] = innerPayload;
    root["payload"] = payload;

    std::string dumpedMsg = root.dump();
    std::cout << "[WS Outbound SDP] Output payload: " << dumpedMsg << std::endl;

    // 2. Dispatch straight onto the low-level transmission interface
    EnqueueOutboundMessage(dumpedMsg);
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
            id = "client"; // only one id possible. Multiple connect of client will connect one server 
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

        SInfo << " offer from clinet id " << id;

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
    std::string id = "client"; //   only one id possible. Multiple connect of client will connect one server 
    isClient = true;
    destroyClient(id); // Safe structural cleanup

    std::lock_guard<std::mutex> lock(clients_mutex);
    clients.emplace(id, createPeerConnection(settingconfig, id, true));
}

#endif



/////////////////////////////////////////////////////////////////////////////////////////////////
// Helper function to generate a random unique UUIDv4 string 
std::string generateUniqueUUID() {     
    std::string ret = "ARVIND"; // uuid4::uuid();     
    return ret;  
}  

// Helper function to generate ISO 8601 Timestamp matching JS new Date().toISOString() 
//std::string getISOTimestamp() {     
//    auto now = std::chrono::system_clock::now();     
//    auto itt = std::chrono::system_clock::to_time_t(now);     
//    
//    // Fixed: Added explicit <std::chrono::milliseconds> template argument
//    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(         
//        now.time_since_epoch()     
//    ).count() % 1000;              
//    
//    std::stringstream ss;     
//    ss << std::put_time(std::gmtime(&itt), "%Y-%m-%dT%H:%M:%S")        
//       << '.' << std::setfill('0') << std::setw(3) << ms << 'Z';     
//    return ss.str(); 
//}
////////////////////////////////////////////////////////////////////////////////////////////////




// Inbound packet parser with detailed logging
void OnInboundSupabaseMessage(const std::string& rawJsonString) {
    std::cout << "[WS Inbound Frame] Raw byte size: " << rawJsonString.size() << " bytes." << std::endl;

    // GUARD: Filter out control frames or binary non-JSON payloads
    if (rawJsonString.empty() || (rawJsonString[0] != '{' && rawJsonString[0] != '[')) {
        std::cout << "[WS Control Frame Ignored] Non-JSON payload dropped (likely ping/pong/binary keepalive)." << std::endl;
        return;
    }

    try {
        json msg = json::parse(rawJsonString);
        std::cout << "[WS Inbound Parsed JSON] Event: '" << msg.value("event", "") 
                  << "' | Topic: '" << msg.value("topic", "") << "'" << std::endl;

        // Handle Phoenix heartbeat responses or system messages
        std::string event = msg.value("event", "");
        if (event == "phx_reply") {
            std::cout << "[Phoenix System] Received phx_reply acknowledgement." << std::endl;
            phx_replied = true;
            return;
        }

        if (msg.value("topic", "") != g_currentRoomTopic) {
            std::cout << "[WS Notice] Topic mismatch ignored (Expected: " << g_currentRoomTopic 
                      << ", Received: " << msg.value("topic", "") << ")" << std::endl;
            return;
        }
        
        json topPayload = msg["payload"];

        // Handle Phoenix/Supabase Broadcast Event Wrapper
        if (event == "broadcast") {
            std::string subEvent = topPayload.value("event", "");
            json innerPayload = topPayload["payload"];

            std::cout << "[Supabase Broadcast Detected] Event Subtype: '" << subEvent << "'" << std::endl;

            if (subEvent == "viewer-joined") {
                std::string clientId = innerPayload.value("clientId", "");
                std::cout << "[Signaling] Viewer requested peer initialization: " << clientId << std::endl;
                if (!clientId.empty()) {
                    //  OnViewerJoinedRoom(clientId);  //arvind TBD
                }
            } else if (subEvent == "signal-to-host") {
                std::string id = innerPayload.value("clientId", "");
                json signal = innerPayload["signal"];
                std::string type = signal.value("type", "");

                std::cout << "[Signaling] Inbound signal targeting client: " << id << " | Signal Type: "
                        << (type.empty() ? "ICE Candidate" : type) << std::endl;


                if (type == "offer") {
                    std::string sdpData = signal.value("sdp", signal.value("data", ""));

                    auto sdp = signal["sdp"].get<string>();

                    SInfo << "Answer to id " << id;

                    SInfo << " offer from clinet id " << id;

                    destroyClient(id); // Safe erasure structure handles cleanup safely instead of raw `.erase()`

                    {
                        std::lock_guard<std::mutex> lock(clients_mutex);
                        clients.emplace(id, createPeerConnection(settingconfig, id, false));
                    }

                    //clients.emplace(id, createPeerConnection(config,  id));
                    if (auto jt = clients.find(id); jt != clients.end()) {
                        auto pc = jt->second->peerConnection;

                        auto sdp = signal["sdp"].get<string>();

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

                        auto sdp = signal["sdp"].get<string>();

                        SInfo << "setRemoteDescription " << type;

                        auto description = Description(sdp, type);
                        pc->setRemoteDescription(description);
                        // pc->setLocalDescription( Description::Type::Answer);
                    }
                } else if (type == "candidate") {

                    json cand = signal["candidate"];

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
                

//                auto it = activeViewers.find(clientId);
//                if (it != activeViewers.end()) {
//                    if (type == "answer") {
//                        std::string sdpData = signal.value("sdp", signal.value("data", ""));
//                        std::cout << "[WebRTC SDP Answer Received] Processing remote description for " << clientId 
//                                  << ":\n--- SDP Answer Payload ---\n" << sdpData << "\n--------------------------" << std::endl;
//
//                        webrtc::SdpParseError error;
//                        std::unique_ptr<webrtc::SessionDescriptionInterface> sessionDesc = 
//                            webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, sdpData, &error);
//
//                        if (sessionDesc) {
//                            it->second->peerConnection->SetRemoteDescription(
//                                ModernSetSessionDescriptionObserver::Create("SetRemoteDescription Answer").get(), sessionDesc.release());
//                        } else {
//                            std::cerr << "[WebRTC SDP Parse Failure] Answer error line " << error.line 
//                                      << ": " << error.description << std::endl;
//                        }
//                    } 
//                    else if (signal.contains("candidate")) {
//                        json cand = signal["candidate"];
//                        std::string candidateStr = "";
//                        std::string sdpMid = "0";
//                        int sdpMLineIndex = 0;
//
//                        if (cand.is_string()) {
//                            candidateStr = cand.get<std::string>();
//                        } else if (cand.is_object()) {
//                            candidateStr = cand.value("candidate", "");
//                            sdpMid = cand.value("sdpMid", "0");
//                            sdpMLineIndex = cand.value("sdpMLineIndex", 0);
//                        }
//
//                        if (!candidateStr.empty()) {
//                            std::cout << "[WebRTC Candidate Received] Adding Remote ICE Candidate for " << clientId 
//                                      << " [sdpMid: " << sdpMid << ", mLineIndex: " << sdpMLineIndex 
//                                      << "]: " << candidateStr << std::endl;
//
//                            webrtc::SdpParseError error;
//                            std::unique_ptr<webrtc::IceCandidateInterface> rtcCandidate(
//                                webrtc::CreateIceCandidate(sdpMid, sdpMLineIndex, candidateStr, &error));
//                            if (rtcCandidate) {
//                                if (!it->second->peerConnection->AddIceCandidate(rtcCandidate.get())) {
//                                    std::cerr << "[WebRTC ICE Failed] Failed to apply Remote ICE Candidate for " << clientId << std::endl;
//                                } else {
//                                    std::cout << "[WebRTC ICE Success] Remote Candidate added successfully for " << clientId << std::endl;
//                                }
//                            } else {
//                                std::cerr << "[WebRTC ICE Error] Error parsing Candidate: " << error.description << std::endl;
//                            }
//                        }
//                    }
//                } else {
//                    std::cerr << "[Signaling Warn] Received signal for unknown/untracked peer: " << clientId << std::endl;
//                }
            }
        }
        else if (event == "presence") {
            if (topPayload.contains("leaves")) {
                for (auto& leaf : topPayload["leaves"]) {
                    if (leaf.contains("clientId")) {
                        std::string abandonedClientId = leaf.value("clientId", "");
                        std::cout << "[Presence Leave] Client departed room: " << abandonedClientId << std::endl;
                       // DestroyHostPeerConnection(abandonedClientId);  arvind TBD
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[JSON Parse Failure] Error parsing packet: " << e.what() << std::endl;
    }
}







int main(int argc, char **argv) {

    {
        /////////////////////////////////////////////////
        base::cnfg::Configuration cache;

        std::string room = "65f570720af337cec5335a70ee88cbfb7df32b5ee33ed0b4a896a0";
        std::string websoc_host = "dabwulkpyquthvearbfw.supabase.co";
        int websoc_port = 443;


#ifdef _WIN32
        if (!conpty_init()) {
            fprintf(stderr, "ERROR: ConPTY init failed! Make sure you are on Windows "
                    "10 1809 or later.");
            return 1;
        }
#endif

        cache.load("./cache.js");

        ConfSettings::SetConfiguration(cache.root);



        //rtc::SctpTransport::Init();



        //rtc::SctpSettings mCurrentSctpSettings = {};
        // rtc::SctpTransport::SetSettings(mCurrentSctpSettings);


        bool printHelp = false;
        //int c = 0;

        Application app;

        ///////////////////////////////////////////////////////////////////////////////////////////




        if (ttyServer.server_init(app.uvGetLoop(), argc, argv)) {
            return -1;
        }





#define sig_count 2
        int sig_nums[] = {SIGINT, SIGTERM};
        uv_signal_t signals[sig_count];
        for (int i = 0; i < sig_count; i++) {
            uv_signal_init(app.uvGetLoop(), &signals[i]);
            uv_signal_start(&signals[i], signal_cb, sig_nums[i]);
        }





        string stunServer = "stun:stun.l.google.com:19302";
        cout << "STUN server is " << stunServer << endl;
        settingconfig.iceServers.emplace_back(stunServer);
        settingconfig.disableAutoNegotiation = true;

       settingconfig.staticPort = ConfSettings::configuration.staticPort;

        settingconfig.gconfig->serverdtsRole = ConfSettings::configuration.serverdtsRole;
        settingconfig.enableTcp = ConfSettings::configuration.enableTcp;
        settingconfig.enableUdp = ConfSettings::configuration.enableUdp;
        settingconfig.publicIP = ConfSettings::configuration.publicIP;
        settingconfig.noPivateIP = ConfSettings::configuration.noPivateIP;
       // websoc_host = ConfSettings::configuration.websoc_host;
        websoc_port = ConfSettings::configuration.websoc_port;


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
        GetNetInterface::ClassInit();



        
        
        
        
        
        
        
        
        
        
        
        
        
        

        const std::string publishable_key = "sb_publishable_1V4Zaw720lrIPx8IqmuxmA_ri32Bdqu";     
        const std::string broadcast_event = "message_sent";      

        // Dynamic unique channel room generation     
  
                
//        std::cout << "Target Unique Room generated: " << channel_topic << std::endl;      
        const std::string target = "/realtime/v1/websocket?apikey=" + publishable_key + "&vsn=1.0.0";      
       // const std::string join_ref = "1";     
      //  const std::string join_message_ref = "1";      
        
        
        
         

        std::cout << "[Init Realtime] Gateway Path: " << target << std::endl;
        
        
        
        m_client = new HttpsClient("wss",  websoc_host, websoc_port, target);          
        m_client->setHostName(websoc_host);      

        // Set Host header explicitly for HTTP request & SSL SNI validation     
        m_client->_request.set("Host", websoc_host);     
        m_client->_request.set("Origin", "http://localhost");      
        m_client->_request.setKeepAlive(true);

        
         //const std::string channel_topic = "realtime:public:room:" + unique_id;   
         
           g_currentRoomTopic = "realtime:" + generateUniqueUUID();
         
           std::cout << "[Init Realtime] Binding topic channel: " << g_currentRoomTopic << std::endl;
        
        // Start heartbeat thread safely outside the message handler
        heartbeat_thread = std::thread([]() {
            std::uint64_t hbSeq = 0;
            while (running) {
                std::this_thread::sleep_for(std::chrono::seconds(20));
                if (!running) break;

                // Send heartbeats only after receiving phx_reply
                if (phx_replied) {
                    json hb;
                    hb["topic"] = "phoenix";
                    hb["event"] = "phx_heartbeat";
                    hb["payload"] = json::object();
                    hb["ref"] = "hb_" + std::to_string(hbSeq++);

                    std::string hbStr = hb.dump();
                    if (m_client) {
                        m_client->send(hbStr.c_str(), hbStr.size(), false);
                        std::cout << "Heartbeat sent" << std::endl;
                    }
                }
            }
        });

        // conn->Complete += sdelegate(&context,
        // &CallbackContext::onClientConnectionComplete);
        m_client->fnComplete = [&](const Response & response) {
            std::string reason = response.getReason();
            StatusCode statuscode = response.getStatus();
            STrace << "Handshake complete. Status: " << (int) statuscode
                    << " Reason: " << reason;     

    
        };

       m_client->fnConnect = [&](HttpBase * con) {
            SInfo << "Connected securely to native WebSocket server." << std::endl;

            json joinMsg;
            joinMsg["topic"] = g_currentRoomTopic;
            joinMsg["event"] = "phx_join";
            joinMsg["payload"] = json::object();
            joinMsg["ref"] = "1";

            std::string joinPayload = joinMsg.dump();
            std::cout << "[WebSocket Tx] Subscription Handshake: " << joinPayload << std::endl;
            m_client->send(joinPayload.c_str(), joinPayload.size(), false);

            // 2. CRITICAL FIX: Track host presence so the client gets an alert if we go offline
            json trackMsg;
            trackMsg["topic"] = g_currentRoomTopic;
            trackMsg["event"] = "presence_track";
            trackMsg["ref"] = "2";
            // Set a static tracking key so viewers can recognize the host
            trackMsg["payload"] = {
                {"body", {{"clientId", "host_server"}}}
            };

            std::string trackPayload = trackMsg.dump();
            std::cout << "[WebSocket Tx] Presence Registration: " << trackPayload << std::endl;
            m_client->send(trackPayload.c_str(), trackPayload.size(), false);
        };


        m_client->fnPayload = [&](HttpBase *con, const char *data, size_t sz) {
            if (!data || sz == 0) return;
            std::string incomingData(data, sz);

            OnInboundSupabaseMessage(incomingData);


        };

        m_client->fnClose = [&](HttpBase *con, std::string str) {
            SInfo << "client->fnClose " << str;
            // close(0,"exit");
            // on_close();
            //emitWebSocketEvent("bye", "");

            running = false;
            phx_replied = true; // Unblock thread if waiting


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




        app.waitForShutdown([&](void*) {


            running = false;
            phx_replied = true; // Unblock thread if waiting
            if (heartbeat_thread.joinable()) {
                heartbeat_thread.join();
            }
              
        //    json m;
       //     m["type"] = "bye";
//            emitWebSocketEvent("message", m);


        //    json joinPayload;
        //    joinPayload["roomId"] = "oom";
        //    joinPayload["client"] = false; // Mirrors client state property tracking requirements

          //  emitWebSocketEvent("createorjoin", joinPayload);





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
            GetNetInterface::ClassDestroy();

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
    SInfo << "createPeerConnection id " << id << " is client " << isClient;


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
        //sendSdp(std::string(description), description.typeString());
        
        
         sendSdp(id, std::string(description), description.typeString());

        // Make the answer
        //		if (auto ws = wws.lock())
        //			ws->send(message.dump());
    });

    pc->onLocalCandidate([ id](rtc::Candidate candidate) {
        //            json message = {{"id", id},
        //                            {"type", "candidate"},
        //                            {"candidate", std::string(candidate)},
        //                            {"mid", candidate.mid()}};

        //SInfo << std::string(candidate);
        sendCandidate(id, candidate.mid(), 1, std::string(candidate));
        //            if (auto ws = wws.lock())
        //                    ws->send(message.dump());
    });

    pc->onGatheringStateChange(
            [](PeerConnection::GatheringState state) {
                SInfo << "Gathering State" << PeerConnection::printState(state);
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
                        // dc->send("Ping2");

                        struct pss_tty *pss = ttyServer.server_wsconnect(&dc->user);
                if (pss) {
                    pss->con = dc;
                }
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


        dc->onClosed([id, dc]() {
            SInfo << "DataChannel from " << id << " closed";
            ttyServer.server_wsclose(&dc->user);
        }
        );

        dc->onMessage([id, dc](auto data) {
            // data holds either std::string or rtc::binary

            const char* msg = nullptr;
            size_t len = 0;

            // 1. Check if the incoming variant is a String
            if (std::holds_alternative<rtc::string>(data)) {
                const std::string& strData = std::get<rtc::string>(data);
                        msg = strData.data();
                        len = strData.size();
            }// 2. Check if the incoming variant is Binary (std::vector<std::byte>)
            else if (std::holds_alternative<rtc::binary>(data)) {
                const rtc::binary& binData = std::get<rtc::binary>(data);
                        // reinterpret_cast is required to change std::byte* or uint8_t* to const char*
                        msg = reinterpret_cast<const char*> (binData.data());
                        len = binData.size();
            }


            // SInfo << "Message from " << id << " len " << len << " received: " << msg;

            ttyServer.server_wsread(dc->user, msg, len);


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
                        // dc->send("Ping1");


            }


        });


        dc->onClosed([id, dc]() {

            SInfo << "DataChannel from " << id << " closed";
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





            // dc->close();
        });

        client->dataChannel2 = dc;
    });

    if (isClient)
        pc->setLocalDescription();
    return client;
};
#endif
