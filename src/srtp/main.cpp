/**
 *
  https://webrtc.googlesource.com/src/+/HEAD/modules/pacing/g3doc/index.md

  https://webrtc.googlesource.com/src/+/HEAD/p2p/g3doc/ice.md
 * 
 * https://github.com/creytiv/re
 * https://en.wikipedia.org/wiki/UDP_hole_punching 
  cricket::Candidate represents an address discovered by a cricket::Port. A candidate can be local (i.e discovered by a local port) or remote. Remote candidates are transported using signaling, i.e outside of webrtc. There are 4 types of candidates: local, stun, prflx or relay (standard)

 */


#include "h264fileparser.hpp"
#include "opusfileparser.hpp"
#include "h264rtppacketizer.hpp"
#include "rtppacketizationconfig.hpp"
#include "rtcpnackresponder.hpp"
#include "DepLibSRTP.h"
#include "helpers.hpp"
#include "socketio/socketioClient.h"

//#include "Settings.h"
#include "RestAPI.h"
#include "uv.h"

#include "base/logger.h"
#include "json/configuration.h"
#include "json/confSettings.h"

#include "sctptransport.hpp"
#include "DtlsTransport.h"
#include "stream.hpp"


#define socketio 1
#define localtesting 1
//#define remotetesting 1
#define VIDEOMEDIA 1
#include "peerconnection.h"

#include "http/HttpsClient.h"

#define CERTFROMFILE 1

using namespace rtc;
using namespace std;
using namespace base;
using namespace base::net;


using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) {
    return ptr;
}

/// all connected clients
unordered_map<string, shared_ptr<Client>> clients
{
};

/// Creates peer connection and client representation
/// @param config Configuration
/// @param wws Websocket for signaling
/// @param id Client ID
/// @returns Client
shared_ptr<Client> createPeerConnection(Configuration &config, string id, bool isClient);
void createPeerConnection_lc(Configuration &config);

shared_ptr<Client> createPeerConnection_rm(Configuration &config, string id, Async &async, bool isClient);

/// Creates stream
/// @param h264Samples Directory with H264 samples
/// @param fps Video FPS
/// @param opusSamples Directory with opus samples
/// @returns Stream object
//shared_ptr<Stream> createStream(const string h264Samples, const unsigned fps, const string opusSamples);

/// Add client to stream
/// @param client Client
/// @param adding_video True if adding video
void addToStream(shared_ptr<Client> client, bool isAddingVideo);

/// Start stream
void startStream();

/// Main dispatch queue
//DispatchQueue MainThread("Main");

/// Audio and video stream
optional<shared_ptr<Stream>> avStream = nullopt;

const string defaultRootDirectory = "../examples/streamer/samples/";
const string defaultH264SamplesDirectory = defaultRootDirectory + "h264/";
string h264SamplesDirectory = defaultH264SamplesDirectory;
const string defaultOpusSamplesDirectory = defaultRootDirectory + "opus/";
string opusSamplesDirectory = defaultOpusSamplesDirectory;
const string defaultIPAddress = "127.0.0.1";
const uint16_t defaultPort = 8000;
string ip_address = defaultIPAddress;
uint16_t port = defaultPort;


sockio::Socket *mysocket = nullptr;
std::string from;
std::string room;
Configuration settingconfig;

std::string id;

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
    json desc = {
        {"type", type},
        {"sdp", sdp}
    };

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

//void sendCandidate(const std::string &mid, int mlineindex, const std::string &sdp) {
//    json desc;
//    desc["sdpMid"] = mid;
//    desc["sdpMLineIndex"] = mlineindex;
//    desc["candidate"] = sdp;
//
//    json m;
//    m["type"] = "candidate";
//    m["candidate"] = desc;
//
//    if (!from.empty()) {
//        m["from"] = from;
//        m["to"] = from;
//    }
//
//    m["room"] = room;
//    SInfo << "send:" << sdp << "candidate to: " << from << std::endl;
//
//   
//}

//void sendSdp(const std::string &sdp, const std::string &type) {
//
//    json desc = {
//
//        {"type", type},
//        {"sdp", sdp}
//    };
//
//    json m;
//    m["type"] = type;
//
//    m["desc"] = desc;
//
//
//    if (!from.empty()) {
//        m["from"] = from;
//        m["to"] = from;
//    }
//
//    m["room"] = room;
//
//    // smpl::Message m({ type, {
//
//    SInfo << "send:" << type << " to: " << from << std::endl;
//
//    mysocket->emit("message", m);
//
//}

#endif


#if !defined( localtesting) && !defined(remotetesting)

void wsOnMessage(json const &m) {


    std::string type;

    std::string to;
    std::string user;

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
        if (id.empty())
            id = from;
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
        id = m["cam"].get<std::string>();

    }


    if (m.find("starttime") != m.end()) {
        //  camT.start = m["starttime"].get<std::string>();

    }

    //
    //    if (m.find("camAudio") != m.end()) { camT.camAudio = m["camAudio"].get<bool>(); }
    //
    //    if (m.find("appAudio") != m.end()) { camT.appAudio = m["appAudio"].get<bool>(); }







    if (type == "offer") {


        if (clients.find(id) != clients.end())
            clients.erase(id);

        clients.emplace(id, createPeerConnection(settingconfig, id, false));

        //clients.emplace(id, createPeerConnection(config,  id));
        if (auto jt = clients.find(id); jt != clients.end()) {
            auto pc = jt->second->peerConnection;

            auto sdp = m["desc"]["sdp"].get<string>();
            SInfo << sdp;
            SInfo << "setRemoteDescription " << type;

            auto description = Description(sdp, type);
            pc->setRemoteDescription(description);
            pc->setLocalDescription();
        }


    } else if (type == "answer") {

        //clients.emplace(id, createPeerConnection(config,  id));
        if (auto jt = clients.find(id); jt != clients.end()) {
            auto pc = jt->second->peerConnection;

            auto sdp = m["desc"]["sdp"].get<string>();
            SInfo << sdp;
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
    id = "client"; /// hard coded the id for client(second participant), since it will have only one instance. Second instance should only have one instance, otherwise throw error. TBD 

    if (clients.find(id) != clients.end())
        clients.erase(id);


    clients.emplace(id, createPeerConnection(settingconfig, id, true));
}

#endif

int main(int argc, char **argv) {

    /////////////////////////////////////////////////
    base::cnfg::Configuration cache;

    cache.load("./cache.js");

    ConfSettings::SetConfiguration(cache.root);



    rtc::SctpTransport::Init();
    rtc::SctpSettings mCurrentSctpSettings = {};
    rtc::SctpTransport::SetSettings(mCurrentSctpSettings);

    DepLibSRTP::ClassInit();



    bool printHelp = false;
    //int c = 0;

    Application app;

    Async async;


    if (printHelp) {
        cout << "usage: stream-h264 [-a opus_samples_folder] [-b h264_samples_folder] [-d ip_address] [-p port] [-v] [-h]" << endl
                << "Arguments:" << endl
                << "\t -a " << "Directory with opus samples (default: " << defaultOpusSamplesDirectory << ")." << endl
                << "\t -b " << "Directory with H264 samples (default: " << defaultH264SamplesDirectory << ")." << endl
                << "\t -d " << "Signaling server IP address (default: " << defaultIPAddress << ")." << endl
                << "\t -p " << "Signaling server port (default: " << defaultPort << ")." << endl
                << "\t -v " << "Enable debug logs." << endl
                << "\t -h " << "Print this help and exit." << endl;
        return 0;
    }



    string stunServer = "stun:stun.l.google.com:19302";
    cout << "STUN server is " << stunServer << endl;
    settingconfig.iceServers.emplace_back(stunServer);
    settingconfig.disableAutoNegotiation = true;
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

    string localId = "server";
    cout << "The local ID is: " << localId << endl;

    rtc::DtlsTransport::ClassInit();

#if localtesting 
    settingconfig.console = true;
    std::string id = "server";
    createPeerConnection_lc(settingconfig);
#elif remotetesting




    settingconfig.allocRestApi(async, "test");
    settingconfig.api->List();


    settingconfig.console = true;
    if (cache.loaded()) {

        id = "client"; /// hard coded the id for client(second participant), since it will have only one instance. Second instance should only have one instance, otherwise throw error. TBD 

        clients.emplace(id, createPeerConnection_rm(settingconfig, id, async, true));


    }

#else   
    std::string room = "65f570720af337cec5335a70ee88cbfb7df32b5ee33ed0b4a896a0";
    std::string host = ip_address;
    int port = 8443;

#if socketio
    sockio::SocketioClient *client;


    client = new sockio::SocketioClient(host, port, true);
    client->connect();

    mysocket = client->io();

    mysocket->on(
            "connection",
            sockio::Socket::event_listener_aux(
            [ = ](string const &name, json const &data, bool isAck, json & ack_resp){
        mysocket->on(
        "created",
        sockio::Socket::event_listener_aux(
        [&](string const &name, json const &data, bool isAck, json & ack_resp) {
            SInfo << cnfg::stringify(data);
            SInfo << "ws: Created room " << data[0] << "- my client ID is " << data[1];
            //isInitiator = true;
            // grabWebCamVideo();
        }));


        mysocket->on(
        "join",
        sockio::Socket::event_listener_aux(
        [&](string const &name, json const &data, bool isAck, json & ack_resp) {
            SInfo << "ws join " << cnfg::stringify(data);
            SInfo << "ws: Created room " << data[0] << "- my client ID is " << data[1] << " noClientInRoom: " << data[2];

            std::string room1 = data[0];

            int noClientInRoom = data[2].get<int>();

            if (noClientInRoom > 1)
                    initiate(room1);


                    // LTrace("Another peer made a request to join room " + room)
                    // LTrace("This peer is the initiator of room " + room + "!")
                    //isChannelReady = true;
            }));


        mysocket->on(
        "joined",
        sockio::Socket::event_listener_aux(
        [&](string const &name, json const &m, bool isAck, json & ack_resp) {
            SInfo << "ws joined " << cnfg::stringify(m);
            // LTrace("Another peer made a request to join room " + room)
            // LTrace("This peer is the initiator of room " + room + "!")
            //isChannelReady = true;
            //wsOnMessage(m);
        }));


        /// for webrtc messages
        mysocket->on(
        "message",
        sockio::Socket::event_listener_aux(
        [&](string const &name, json const &m, bool isAck, json & ack_resp) {
            //LTrace(cnfg::stringify(m));
            STrace << "SocketioClient received message:" << cnfg::stringify(m);

            //onPeerMessage((string &) name, m); //arvind
            // signalingMessageCallback(message);

            wsOnMessage(m);
        }));


        // Leaving rooms and disconnecting from peers.
        mysocket->on(
        "disconnectClient",
        sockio::Socket::event_listener_aux(
        [&](string const &name, json const &data, bool isAck, json & ack_resp) {
            std::string from = data.get<std::string>();
            SInfo << "disconnectClient " << from;
            LInfo(cnfg::stringify(data));
            // onPeerDiconnected(from);  //arvind
        }));


        mysocket->on(
        "bye",
        sockio::Socket::event_listener_aux(
        [&](string const &name, json const &data, bool isAck, json & ack_resp) {
            SInfo << cnfg::stringify(data);
            // LTrace("Peer leaving room", room);
        }));

        mysocket->emit("createorjoin", room);
    }));
#else 
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
    m_client->fnComplete = [&](const Response & response) {
        std::string reason = response.getReason();
        StatusCode statuscode = response.getStatus();
        std::string body =
                m_client->readStream() ? m_client->readStream()->str() : "";
        STrace << "SocketIO handshake response:" << "Reason: " << reason
                << " Response: " << body;
    };

    m_client->fnConnect = [&](HttpBase * con) {
        STrace << "client->fnConnect ";
        //  m_con_state = con_opened;
        // m_reconn_timer.Stop();

        SInfo << "Connected securely to native WebSocket server." << std::endl;

        // Map the primary handshake logic registration event sequence
        json joinPayload;
        joinPayload["roomId"] = room;
        joinPayload["client"] =
                false; // Mirrors client state property tracking requirements

        emitWebSocketEvent("createorjoin", joinPayload);


        // char tmp[3] = "{}";

        // con->send(tmp, 2);
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
                SInfo << "ws: Created room " << data[0] << "- my client ID is "
                        << data[1] << std::endl;
            } else if (eventName == "join") {
                SInfo << "ws join " << data.dump() << std::endl;
                SInfo << "ws: Created room " << data[0] << "- my client ID is "
                        << data[1] << " noClientInRoom: " << data[2] << std::endl;

                std::string room1 = data[0].get<std::string>();
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
        STrace << "client->fnClose " << str;
        // close(0,"exit");
        // on_close();
        SInfo << "WebSocket connection closed by endpoint structure.";
        m_client->Close();
        delete m_client;
        //m_client = nullptr;

        //            m_con_state = con_closed;
    };

    //  conn->_request.setKeepAlive(false);
    m_client->setReadStream(new std::stringstream);
    m_client->send();
    LTrace("sendHandshakeRequest over")

#endif

#endif

    //    while (true) {
    //        string id;
    //        cout << "Enter to exit" << endl;
    //        cin >> id;
    //        cin.ignore();
    //        cout << "exiting" << endl;
    //        break;
    //    }

    app.waitForShutdown([&](void*) {

        SInfo << "app.run() is over";
        //    Settings::exit();         
        //    rtc::CleanupSSL();

        DepLibSRTP::ClassDestroy();
        Logger::destroy();

        //    if(ctx->txt)
        //    delete ctx->txt;
        //    ctx->txt = nullptr;

        //    restApi->stop();

        //    restApi->shutdown();

    });



    SInfo << "Cleaning up..." << endl;
    return 0;

}

//catch (const std::exception &e) {
//    SError << "Error: " << e.what() << std::endl;
//    return -1;
//}

shared_ptr<ClientTrackData> addVideo(const shared_ptr<PeerConnection> pc, const uint8_t payloadType, const uint32_t ssrc, const string cname, const string msid, const function<void (void) > onOpen) {
    auto video = Description::Video(cname);
    video.addH264Codec(payloadType);
    video.addSSRC(ssrc, cname, msid, cname);
    auto track = pc->addTrack(video);
    // create RTP configuration
    auto rtpConfig = make_shared<RtpPacketizationConfig>(ssrc, cname, payloadType, H264RtpPacketizer::defaultClockRate);
    //    // create packetizer
    auto packetizer = make_shared<H264RtpPacketizer>(NalUnit::Separator::Length, rtpConfig);
    //    // add RTCP SR handler
    auto srReporter = make_shared<RtcpSrReporter>(rtpConfig);
    packetizer->addToChain(srReporter);
    //    // add RTCP NACK handler
    auto nackResponder = make_shared<RtcpNackResponder>();
    packetizer->addToChain(nackResponder);
    // set handler
    track->setMediaHandler(packetizer);
    track->onOpen(onOpen);
    auto trackData = make_shared<ClientTrackData>(track, srReporter);
    return trackData;


}

shared_ptr<ClientTrackData> addAudio(const shared_ptr<PeerConnection> pc, const uint8_t payloadType, const uint32_t ssrc, const string cname, const string msid, const function<void (void) > onOpen) {
    auto audio = Description::Audio(cname);
    audio.addOpusCodec(payloadType);
    audio.addSSRC(ssrc, cname, msid, cname);
    auto track = pc->addTrack(audio);
    // create RTP configuration
    auto rtpConfig = make_shared<RtpPacketizationConfig>(ssrc, cname, payloadType, OpusRtpPacketizer::DefaultClockRate);
    // create packetizer
    auto packetizer = make_shared<OpusRtpPacketizer>(rtpConfig);
    // add RTCP SR handler
    auto srReporter = make_shared<RtcpSrReporter>(rtpConfig);
    packetizer->addToChain(srReporter);
    // add RTCP NACK handler
    auto nackResponder = make_shared<RtcpNackResponder>();
    packetizer->addToChain(nackResponder);
    // set handler
    track->setMediaHandler(packetizer);
    track->onOpen(onOpen);
    auto trackData = make_shared<ClientTrackData>(track, srReporter);
    return trackData;
}

#if localtesting 
// Create and setup a PeerConnection

void createPeerConnection_lc(Configuration &config) {
    auto pc1 = make_shared<PeerConnection>(config);
    config.portdefault = config.portdefault + 1;
    auto pc2 = make_shared<PeerConnection>(config);
    {
        string id = "server";

        auto client = make_shared<Client>(pc1);

        pc1->onStateChange([id](PeerConnection::State state) {
            SInfo << "pc1 State: " << state << endl;
            if (state == PeerConnection::State::Disconnected ||
                    state == PeerConnection::State::Failed ||
                    state == PeerConnection::State::Closed) {
                // remove disconnected client
                //MainThread.dispatch([id]() 
                {
                    clients.erase(id);


                }
                //);
            }
        });



        pc1->onLocalDescription([ id, pc1, pc2](rtc::Description & description) {
            //		json message = {{"id", id},
            //		                {"type", description.typeString()},
            //		                {"description", std::string(description)}};

            SInfo << "pc1 send sdp:" << description.typeString() << " des " << std::string(description);

            //  pc1->setLocalDescription(Description::Type::Offer);// Description::Type::Answer);          
            //sendSdp( std::string(description), description.typeString());
            // Make the answer
            //		if (auto ws = wws.lock())
            //			ws->send(message.dump());

            // SInfo << "setRemoteDescription " << type ;

            auto description1 = Description(std::string(description), Description::Type::Offer);
            pc2->setRemoteDescription(description1);

        });

        pc1->onLocalCandidate([ id, pc2](rtc::Candidate & candidate) {
            //            json message = {{"id", id},
            //                            {"type", "candidate"},
            //                            {"candidate", std::string(candidate)},
            //                            {"mid", candidate.mid()}};

            //sendCandidate( candidate.mid(), 1,  std::string(candidate)  );
            //            if (auto ws = wws.lock())
            //                    ws->send(message.dump());

            SInfo << "pc1 send candidated:" << candidate.mid() << " des " << std::string(candidate);

            pc2->addRemoteCandidate(candidate);

        });

        pc1->onGatheringStateChange(
                [](PeerConnection::GatheringState state) {

                    if (state == PeerConnection::GatheringState::Complete) {
                        SInfo << "pc1 Gathering State: Complete";
                        //  if(auto pc1 = wpc1.lock())
                        {
                            //                json desc;
                            //                desc["type"] =  description->typeString();
                            //                desc[sdp] = sdp;
                            //    

                        }
                    } else if (state == PeerConnection::GatheringState::InProgress) {
                        SInfo << "pc1 Gathering State: InProgress";
                    }
                });
#if VIDEOMEDIA


        //          shared_ptr<Track> t2;
        //          string newTrackMid;
        //          pc1->onTrack([&t2, &newTrackMid](shared_ptr<Track> t) {
        //            string mid = t->mid();
        //            cout << "Track 2: Received track with mid \"" << mid << "\"" << endl;
        //            if (mid != newTrackMid) {
        //              cerr << "Wrong track mid" << endl;
        //              return;
        //            }
        //
        //            t->onOpen([mid]() {
        //              cout << "Track 2: Track with mid \"" << mid << "\" is open" << endl; });
        //
        //            t->onClosed(
        //              [mid]() {
        //                cout << "Track 2: Track with mid \"" << mid << "\" is closed" << endl; });
        //
        //            std::atomic_store(&t2, t);
        //          });
        //          
        client->video = addVideo(pc1, 102, 1, "video-stream", "stream1", [id, wc = make_weak_ptr(client)](){
            // MainThread.dispatch([wc]() 

            SInfo << "addToStream";

            {
                if (auto c = wc.lock()) {
                    addToStream(c, true);
                }
            }

            //);
            SInfo << "Video from " << id << " opened" << endl;
        });

        //        client->audio = addAudio(pc1, 111, 2, "audio-stream", "stream1", [id, wc = make_weak_ptr(client)](){
        //
        //
        //            //MainThread.dispatch([wc]() 
        //
        //            {
        //                if (auto c = wc.lock()) {
        //                    addToStream(c, false);
        //                }
        //            }
        //            //);
        //            SInfo << "Audio from " << id << " opened" << endl;
        //        });

#endif

        auto dc = pc1->createDataChannel("ping-pong-pc1");
        dc->onOpen([id, wdc = make_weak_ptr(dc)](){
            if (auto dc = wdc.lock()) {
                SInfo << "ping-pong-pc1 onOpen";
                        dc->send("ping-pong-pc1 send on open Ping");
            }
        });

        dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg){
            SInfo << "Pc1 Message from " << id << " received: " << msg << endl;
            if (auto dc = wdc.lock()) {
                dc->send("ping-pong-pc1 send on message Ping");
            }
        });
        client->dataChannel1 = dc;



        pc1->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
            SInfo << "pc1 onDataChannel from " << id << " received with label \"" << dc->label() << "\""
                    << std::endl;

            dc->onOpen([wdc = make_weak_ptr(dc)](){
                if (auto dc = wdc.lock()) {
                    SInfo << "pc1 open ";
                            dc->send("Hello from  pc1");
                }
            });

            dc->onClosed([id]() {
                SInfo << "pc1 DataChannel from " << id << " closed" << std::endl; }

            );

            dc->onMessage([id, dc](auto data) {
                // data holds either std::string or rtc::binary
                if (std::holds_alternative<std::string>(data))
                    SInfo << "pc1 Message from " << id << " received: " << std::get<std::string>(data)
                    << std::endl;
                else
                    SInfo << "pc1  Binary message from " << id
                        << " received, size=" << std::get<rtc::binary>(data).size() << std::endl;


                //  sleep(5);
                //  dc->send("PC1 to PC2");

            });

            client->dataChannel11 = dc;
        });

        pc1->setLocalDescription();
        //   return client;

        clients[id] = client;
    }

    {
        string id = "client";

        auto client = make_shared<Client>(pc2);

        pc2->onStateChange([id](PeerConnection::State state) {
            SInfo << "pc2 State: " << state << endl;
            if (state == PeerConnection::State::Disconnected ||
                    state == PeerConnection::State::Failed ||
                    state == PeerConnection::State::Closed) {
                // remove disconnected client
                //MainThread.dispatch([id]() 
                {
                    clients.erase(id);

                }
                //);
            }
        });



        pc2->onLocalDescription([ id, pc1](rtc::Description description) {
            //		json message = {{"id", id},
            //		                {"type", description.typeString()},
            //		                {"description", std::string(description)}};

            SInfo << "pc2 send sdp:" << description.typeString() << " des " << std::string(description);

            //  pc2->setLocalDescription(Description::Type::Offer);// Description::Type::Answer);          
            //sendSdp( std::string(description), description.typeString());
            // Make the answer
            //		if (auto ws = wws.lock())
            //			ws->send(message.dump());

            pc1->setRemoteDescription(description);
        });

        pc2->onLocalCandidate([ id, pc1](rtc::Candidate candidate) {
            //            json message = {{"id", id},
            //                            {"type", "candidate"},
            //                            {"candidate", std::string(candidate)},
            //                            {"mid", candidate.mid()}};
            SInfo << "pc2 send candidated:" << candidate.mid() << " des " << std::string(candidate);
            //sendCandidate( candidate.mid(), 1,  std::string(candidate)  );
            //            if (auto ws = wws.lock())
            //                    ws->send(message.dump());
            pc1->addRemoteCandidate(candidate);
        });

        pc2->onGatheringStateChange(
                [](PeerConnection::GatheringState state) {

                    if (state == PeerConnection::GatheringState::Complete) {
                        SInfo << "Pc2 Gathering State: Complete";
                        //  if(auto pc2 = wpc2.lock())
                        {
                            //                json desc;
                            //                desc["type"] =  description->typeString();
                            //                desc[sdp] = sdp;
                            //    

                        }
                    }
                });
#if VIDEOMEDIA



        //                
        //#include <fstream>
        //#include <iostream>
        //#include <memory>
        //#include <string>
        //#include <atomic>

        //        // Assume these file streams are initialized safely in your setup code
        //        std::ofstream videoFile("output.h264", std::ios::binary);
        //        std::ofstream audioFile("output.raw_opus", std::ios::binary); // Raw payload packets
        //        shared_ptr<Track> t2;
        //        string newTrackMid;
        //        pc2->onTrack([&t2, newTrackMid](std::shared_ptr<rtc::Track> t) {
        //            std::string mid = t->mid();
        //            std::cout << "Track 2: Received track with mid \"" << mid << "\"\n";
        //
        //            if (mid != newTrackMid) {
        //                return;
        //            }
        //
        //            t->onOpen([mid]() {
        //                std::cout << "Track 2: Track with mid \"" << mid << "\" is open\n";
        //            });
        //
        //
        //
        //            t->onMessage([](rtc::binary data) {
        //                // 1. Enforce a safety check to avoid buffer overreads
        //                if (data.size() < sizeof (rtc::RtpHeader)) {
        //                    std::cerr << "Packet too small to contain an RTP header\n";
        //                    return;
        //                }
        //
        //                // 2. Cast the byte buffer directly to the RtpHeader structure
        //                const auto* rtpHeader = reinterpret_cast<const rtc::RtpHeader*> (data.data());
        //
        //                // 3. Extract the body pointer to calculate exact payload size
        //                const char* body = rtpHeader->getBody();
        //                const uint8_t* payload = reinterpret_cast<const uint8_t*> (body);
        //
        //                // Safety bounds check to make sure the body pointer lies within our binary data size
        //                if (reinterpret_cast<const uint8_t*> (body) < data.data() ||
        //                        reinterpret_cast<const uint8_t*> (body) > data.data() + data.size()) {
        //                    return;
        //                }
        //
        //                size_t payloadSize = data.size() - (body - reinterpret_cast<const char*> (data.data()));
        //
        //                // 4. Access individual field variables directly from the struct
        //                uint8_t payloadType = rtpHeader->payloadType();
        //
        //                // 111 is standard for Opus audio over WebRTC
        //                bool isAudio = (payloadType == 111);
        //
        //                if (isAudio) {
        //                    if (audioFile.is_open() && payloadSize > 0) {
        //                        audioFile.write(reinterpret_cast<const char*> (payload), payloadSize);
        //                    }
        //                } else {
        //                    if (videoFile.is_open() && payloadSize > 0) {
        //                        videoFile.write(reinterpret_cast<const char*> (payload), payloadSize);
        //                    }
        //                }
        //            });
        //
        //            t->onClosed([mid]() {
        //                std::cout << "Track 2: Track with mid \"" << mid << "\" is closed\n";
        //            });
        //
        //            std::atomic_store(&t2, t);
        //        });



        /*

        std::ofstream videoFile("output.h264", std::ios::binary);
        std::ofstream audioFile("output.opus", std::ios::binary);

        pc.onTrack([&t2, newTrackMid](std::shared_ptr<rtc::Track> t) {
            std::string mid = t->mid();
    
            if (mid != newTrackMid) return;

            // 1. Check the track type or payload type to link the correct Depacketizer
            // Let's check the description to see if it's audio or video
            bool isAudio = false;
            if (auto trackDescription = t->description()) {
                isAudio = (trackDescription->type == rtc::Description::Type::Audio);
            }

            // 2. Attach the Media Handler Chain
            // This handles RTCP reports and parses RTP packets into raw codec frames
            if (isAudio) {
                auto audioConfig = std::make_shared<rtc::RtpPacketizationConfig>(111, "opus", 48000);
                auto audioHandler = std::make_shared<rtc::OpusRtpDepacketizer>(audioConfig);
                audioHandler->addToChain(std::make_shared<rtc::RtcpReceivingSession>());
                t->setMediaHandler(audioHandler);
            } else {
                auto videoConfig = std::make_shared<rtc::RtpPacketizationConfig>(96, "H264", 90000);
        
                // Use LongStartSequence so libdatachannel inserts Annex-B codes (\x00\x00\x00\x01) 
                // directly into the video frame payload for you!
                auto videoHandler = std::make_shared<rtc::H264RtpDepacketizer>(
                    videoConfig, rtc::H264RtpDepacketizer::Separator::LongStartSequence
                );
                videoHandler->addToChain(std::make_shared<rtc::RtcpReceivingSession>());
                t->setMediaHandler(videoHandler);
            }

            // 3. Register the onFrame callback
            t->onFrame([isAudio](rtc::binary data, rtc::FrameInfo info) {
                // 'data' contains the perfectly defragmented frame payload!
                // 'info' provides context like info.timestamp and info.payloadType
        
                if (isAudio) {
                    if (audioFile.is_open()) {
                        audioFile.write(reinterpret_cast<const char*>(data.data()), data.size());
                    }
                } else {
                    if (videoFile.is_open()) {
                        // Because we used Separator::LongStartSequence above, 
                        // this data is a standard Annex-B H.264 frame, ready for disk storage or decoding.
                        videoFile.write(reinterpret_cast<const char*>(data.data()), data.size());
                    }
                }
            });

            t->onOpen([mid]() { std::cout << "Track " << mid << " opened.\n"; });
            t->onClosed([mid]() { std::cout << "Track " << mid << " closed.\n"; });
            std::atomic_store(&t2, t);
        });

         */


        shared_ptr<Track> t2;
        string newTrackMid;
        pc2->onTrack([&t2, &newTrackMid](shared_ptr<Track> t) {
            string mid = t->mid();
            cout << "Track 2: Received track with mid \"" << mid << "\"" << endl;
            if (mid != newTrackMid) {
                cerr << "Wrong track mid" << endl;
                return;
            }

            t->onOpen([mid]() {
                cout << "Track 2: Track with mid \"" << mid << "\" is open" << endl; });

            t->onClosed(
                    [mid]() {
                        cout << "Track 2: Track with mid \"" << mid << "\" is closed" << endl; });

            std::atomic_store(&t2, t);
        });

        client->video = addVideo(pc2, 102, 1, "video-stream", "stream1", [id, wc = make_weak_ptr(client)](){
            // MainThread.dispatch([wc]() 

            SInfo << "addToStream";

            {
                if (auto c = wc.lock()) {
                    addToStream(c, true);
                }
            }

            //);
            SInfo << "Video from " << id << " opened" << endl;
        });

        //        client->audio = addAudio(pc2, 111, 2, "audio-stream", "stream1", [id, wc = make_weak_ptr(client)](){
        //
        //
        //            //MainThread.dispatch([wc]() 
        //
        //            {
        //                if (auto c = wc.lock()) {
        //                    addToStream(c, false);
        //                }
        //            }
        //            //);
        //            SInfo << "Audio from " << id << " opened" << endl;
        //        });

#endif

        auto dc = pc2->createDataChannel("ping-pong-pc2");
        dc->onOpen([id, wdc = make_weak_ptr(dc)](){
            if (auto dc = wdc.lock()) {
                SInfo << "pc2 onOpen";
                        dc->send("ping-pong pc2 on open send");
            }
        });

        dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg){
            SInfo << "Message from " << id << "pc2  received: " << msg << endl;
            if (auto dc = wdc.lock()) {
                dc->send("ping-pong pc2 on message send");
            }
        });
        client->dataChannel2 = dc;



        pc2->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
            SInfo << "PC2 onDataChannel from " << id << " received with label \"" << dc->label();


            dc->onOpen([wdc = make_weak_ptr(dc)](){
                if (auto dc = wdc.lock())
                        dc->send("PC2 Hello from  arvind");
                });

            dc->onClosed([id]() {
                SInfo << "DataChannel from " << id << " closed" << std::endl;
            }

            );

            dc->onMessage([id, dc](auto data) {
                // data holds either std::string or rtc::binary
                if (std::holds_alternative<std::string>(data))
                    SInfo << "onDataChannel:onMessage  PC2 Message from " << id << " received: " << std::get<std::string>(data)
                    << std::endl;
                else
                    SInfo << "onDataChannel:onMessage PC2 Binary message from " << id
                        << " received, size=" << std::get<rtc::binary>(data).size() << std::endl;

                //sleep(5);
                //dc->send("PC2 tp PC1");

            });

            client->dataChannel22 = dc;
        });

        pc2->setLocalDescription(); // this will create offfer

        //clients.emplace(id, client);
        clients[id] = client;

    }
};

#elif remotetesting


// Create and setup a PeerConnection

shared_ptr<Client> createPeerConnection_rm(Configuration &config, string id, Async &async, bool isClient) {
    SInfo << "createPeerConnection";


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

                        int x = 1; //arvind
            }
            //);
        }
    });



    pc->onLocalDescription([ id, pc, &async](rtc::Description description) {
        //		json message = {{"id", id},
        //		                {"type", description.typeString()},
        //		                {"description", std::string(description)}};



        SInfo << "pc1 send sdp:" << description.typeString() << " des " << std::string(description);


        //  test(async);

        auto work_fn = [pc, description]() {
            // This runs in a worker thread

            auto descAns = Description(std::string(description), Description::Type::Answer);
            descAns.mRole = Description::Role::Active;
            SInfo << "remote desc Ansp:" << descAns;

            pc->setRemoteDescription(descAns);
        };

        auto after_work_fn = [](int status) {
            // This runs back in the main event loop thread
            std::cout << "Main thread: Work finished with status " << status << std::endl;
        };

        async.queueWork(work_fn, after_work_fn);




    });

    pc->onLocalCandidate([config, id, pc, &async](rtc::Candidate candidate) {


        SInfo << std::string(candidate);


        rtc::Candidate tmp = candidate;
        tmp.mService = std::to_string(config.portdefault);
        tmp.mNode = "192.168.0.20"; //Settings::RemoteIP(); // always change it


        auto work_fn = [pc, tmp]() {

            Application app;

            tmp.resolved = {0};

            SInfo << "addRemoteCandidate: " << std::string(tmp);

            pc->addRemoteCandidate(tmp);

            app.run();

        };

        auto after_work_fn = [](int status) {
            // This runs back in the main event loop thread
            std::cout << "Main thread: Work finished with status " << status << std::endl;
        };

        async.queueWork(work_fn, after_work_fn);


    });

    pc->onGatheringStateChange(
            [](PeerConnection::GatheringState state) {
                SInfo << "Gathering State: " << "state";
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
#if VIDEOMEDIA

    client->video = addVideo(pc, 102, 1, "video-stream", "stream1", [id, wc = make_weak_ptr(client)](){
        // MainThread.dispatch([wc]() 

        SInfo << "addToStream";

        {
            if (auto c = wc.lock()) {
                addToStream(c, true);
            }
        }

        //);
        SInfo << "Video from " << id << " opened" << endl;
    });

    client->audio = addAudio(pc, 111, 2, "audio-stream", "stream1", [id, wc = make_weak_ptr(client)](){


        //MainThread.dispatch([wc]() 

        {
            if (auto c = wc.lock()) {
                addToStream(c, false);
            }
        }
        //);
        SInfo << "Audio from " << id << " opened" << endl;
    });

#endif

    std::string dcchat = "192.168.0.20"; //  Settings::getdatachannel();

    auto dc = pc->createDataChannel(dcchat);
    dc->onOpen([id, wdc = make_weak_ptr(dc)](){
        if (auto dc = wdc.lock()) {
            SInfo << "onOpen: ";
                    dc->send("Ping");
        }
    });

    dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg){
        SInfo << "Message from " << id << " received: " << msg << endl;
        if (auto dc = wdc.lock()) {

            SInfo << "onOpen: " << msg;
                    dc->send("Ping");
        }
    });
    client->dataChannel1 = dc;



    pc->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
        SInfo << "DataChannel from " << id << " received with label \"" << dc->label();


        dc->onOpen([wdc = make_weak_ptr(dc)](){
            if (auto dc = wdc.lock()) {
                SInfo << "DataChannel 2: Open" << endl;
                        dc->send("Hello from 2");
            }
        });


        dc->onClosed([id]() {
            std::cout << "DataChannel from " << id << " closed" << std::endl; });

        dc->onMessage([id, dc](auto data) {
            // data holds either std::string or rtc::binary
            if (std::holds_alternative<std::string>(data))
                SInfo << "Message from " << id << " received: " << std::get<std::string>(data)
                << std::endl;
            else
                SInfo << "Binary message from " << id
                    << " received, size=" << std::get<rtc::binary>(data).size() << std::endl;

            sleep(5);
            dc->send("Send to web");
        });

        client->dataChannel2 = dc;
    });

    if (isClient)
        pc->setLocalDescription();
    return client;
};



#else


// Create and setup a PeerConnection

shared_ptr<Client> createPeerConnection(Configuration &config, string id, bool isClient) {
    SInfo << "createPeerConnection";


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

                        int x = 1; //arvind
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
#if VIDEOMEDIA

    client->video = addVideo(pc, 102, 1, "video-stream", "stream1", [id, wc = make_weak_ptr(client)](){
        // MainThread.dispatch([wc]() 

        SInfo << "addToStream";

        {
            if (auto c = wc.lock()) {
                addToStream(c, true);
            }
        }

        //);
        SInfo << "Video from " << id << " opened" << endl;
    });

    client->audio = addAudio(pc, 111, 2, "audio-stream", "stream1", [id, wc = make_weak_ptr(client)](){


        //MainThread.dispatch([wc]() 

        {
            if (auto c = wc.lock()) {
                addToStream(c, false);
            }
        }
        //);
        SInfo << "Audio from " << id << " opened" << endl;
    });

#endif
    std::string dcchat = "Settings::getdatachannel()";
    auto dc = pc->createDataChannel(dcchat);
    dc->onOpen([id, wdc = make_weak_ptr(dc)](){
        if (auto dc = wdc.lock()) {
            SInfo << "onOpen: ";
                    dc->send("Ping");
        }
    });

    dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg){
        SInfo << "Message from " << id << " received: " << msg << endl;
        if (auto dc = wdc.lock()) {

            SInfo << "onOpen: " << msg;
                    dc->send("Ping");
        }
    });
    client->dataChannel1 = dc;



    pc->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
        SInfo << "DataChannel from " << id << " received with label \"" << dc->label();


        dc->onOpen([wdc = make_weak_ptr(dc)](){
            if (auto dc = wdc.lock()) {
                SInfo << "DataChannel 2: Open" << endl;
                        dc->send("Hello from 2");
            }
        });


        dc->onClosed([id]() {
            std::cout << "DataChannel from " << id << " closed" << std::endl; });

        dc->onMessage([id, dc](auto data) {
            // data holds either std::string or rtc::binary
            if (std::holds_alternative<std::string>(data))
                SInfo << "Message from " << id << " received: " << std::get<std::string>(data)
                << std::endl;
            else
                SInfo << "Binary message from " << id
                    << " received, size=" << std::get<rtc::binary>(data).size() << std::endl;

            sleep(500);
            dc->send("Send to web");
        });

        client->dataChannel2 = dc;
    });

    if (isClient)
        pc->setLocalDescription();
    return client;
};
#endif

/// Create stream

shared_ptr<Stream> createStream(const string h264Samples, const unsigned fps, const string opusSamples) {
    // video source
    auto video = make_shared<H264FileParser>(h264Samples, fps, true);
    // audio source
    auto audio = make_shared<OPUSFileParser>(opusSamples, true);

    auto stream = make_shared<Stream>(video, audio);
    // set callback responsible for sample sending
    stream->onSample([ws = make_weak_ptr(stream)](Stream::StreamSourceType type, uint64_t sampleTime, rtc::binary sample){
        vector<ClientTrack> tracks
        {};
        string streamType = type == Stream::StreamSourceType::Video ? "video" : "audio";
        // get track for given type
        function < optional<shared_ptr < ClientTrackData >> (shared_ptr<Client>) > getTrackData = [type](shared_ptr<Client> client) {
            return type == Stream::StreamSourceType::Video ? client->video : client->audio;
        };
        // get all clients with Ready state
        for (auto id_client : clients) {
            auto id = id_client.first;
                    auto client = id_client.second;
                    auto optTrackData = getTrackData(client);
            if (client->getState() == Client::State::Ready && optTrackData.has_value()) {
                auto trackData = optTrackData.value();
                        tracks.push_back(ClientTrack(id, trackData));
            }
        }
        if (!tracks.empty()) {
            for (auto clientTrack : tracks) {
                auto client = clientTrack.id;
                        auto trackData = clientTrack.trackData;
                        auto rtpConfig = trackData->sender->rtpConfig;

                        // sample time is in us, we need to convert it to seconds
                        auto elapsedSeconds = double(sampleTime) / (1000 * 1000);
                        // get elapsed time in clock rate
                        uint32_t elapsedTimestamp = rtpConfig->secondsToTimestamp(elapsedSeconds);
                        // set new timestamp
                        rtpConfig->timestamp = rtpConfig->startTimestamp + elapsedTimestamp;

                        // get elapsed time in clock rate from last RTCP sender report
                        auto reportElapsedTimestamp = rtpConfig->timestamp - trackData->sender->lastReportedTimestamp();
                        // check if last report was at least 1 second ago
                if (rtpConfig->timestampToSeconds(reportElapsedTimestamp) > 1) {
                    trackData->sender->setNeedsToReport();
                }

                cout << "Sending " << streamType << " sample with size: " << to_string(sample.size()) << " to " << client << endl;
                try {
                    // send sample
                    trackData->track->send(sample);
                } catch (const std::exception &e) {
                    cerr << "Unable to send " << streamType << " packet: " << e.what() << endl;
                }
            }
        }
        //        MainThread.dispatch([ws]() {
        //            if (clients.empty()) {
        //                // we have no clients, stop the stream
        //                if (auto stream = ws.lock()) {
        //                    stream->stop();
        //                }
        //            }
        //        });
    });
    return stream;
}

/// Start stream

void startStream() {
    shared_ptr<Stream> stream;
    if (avStream.has_value()) {
        stream = avStream.value();
        if (stream->isRunning) {
            // stream is already running
            return;
        }
    } else {
        stream = createStream(h264SamplesDirectory, 30, opusSamplesDirectory);
        avStream = stream;
    }
    stream->start();
}

/// Send previous key frame so browser can show something to user
/// @param stream Stream
/// @param video Video track data

void sendInitialNalus(shared_ptr<Stream> stream, shared_ptr<ClientTrackData> video) {
    auto h264 = dynamic_cast<H264FileParser *> (stream->video.get());
    auto initialNalus = h264->initialNALUS();

    // send previous NALU key frame so users don't have to wait to see stream works
    if (!initialNalus.empty()) {
        const double frameDuration_s = double(h264->getSampleDuration_us()) / (1000 * 1000);
        const uint32_t frameTimestampDuration = video->sender->rtpConfig->secondsToTimestamp(frameDuration_s);
        video->sender->rtpConfig->timestamp = video->sender->rtpConfig->startTimestamp - frameTimestampDuration * 2;
        video->track->send(initialNalus);
        video->sender->rtpConfig->timestamp += frameTimestampDuration;
        // Send initial NAL units again to start stream in firefox browser
        video->track->send(initialNalus);
    }
}

/// Add client to stream
/// @param client Client
/// @param adding_video True if adding video

void addToStream(shared_ptr<Client> client, bool isAddingVideo) {
    if (client->getState() == Client::State::Waiting) {
        client->setState(isAddingVideo ? Client::State::WaitingForAudio : Client::State::WaitingForVideo);
    } else if ((client->getState() == Client::State::WaitingForAudio && !isAddingVideo)
            || (client->getState() == Client::State::WaitingForVideo && isAddingVideo)) {

        // Audio and video tracks are collected now
        assert(client->video.has_value());
        auto video = client->video.value();

        if (avStream.has_value()) {
            sendInitialNalus(avStream.value(), video);
        }

        client->setState(Client::State::Ready);
    }
    if (client->getState() == Client::State::Ready) {
        startStream();
    }
}

#if 0
//Defragmentation Logic for dtls: Implement a custom defragmentation layer in your code before passing the packet to MbedTLS. Catch the ClientHello fragments manually, concatenate them to reconstruct the full message in memory, and then feed the complete packet into mbedtls_ssl_read
// ClientHello fragmentation is not supported for connection-oriented TLS. The library requires that the complete ClientHello message fits into a single record
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <uv.h>

// Mbed TLS 3.6+ Core Headers
#include "mbedtls/ssl.h"
#include "mbedtls/error.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/ssl_cookie.h"

#define DTLS_RECORD_HEADER_LEN      13
#define DTLS_HANDSHAKE_HEADER_LEN   12
#define MAX_DTLS_RECV_BUFFER        16384

/* --- Data Structures --- */

typedef struct {
    uint16_t message_seq;
    uint32_t total_length;
    uint32_t assembled_length;
    uint8_t *reassembly_buf;
    uint8_t *bitmap;
} dtls_msg_reassembler_t;

// Custom Asynchronous Timer Context mapping to Libuv

typedef struct {
    uv_timer_t uv_timer;
    uint64_t intermediate_ms;
    uint64_t final_ms;
    uint64_t start_timestamp;
    int is_cancelled;
    void *bio_ctx_ptr; // Void back-pointer to avoid circular compilation paths
} custom_dtls_timer_t;

typedef struct {
    uv_udp_t udp_handle;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config ssl_conf;
    dtls_msg_reassembler_t reassembler;

    // Custom Async Retransmission Timer Context
    custom_dtls_timer_t handshake_timer;

    // Hardened Cryptographic Engine Contexts
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_x509_crt server_cert;
    mbedtls_x509_crt ca_chain;
    mbedtls_pk_context private_key;
    mbedtls_ssl_cookie_ctx cookie_ctx;

    // Active Remote Peer Routing
    struct sockaddr_storage peer_addr;

    // Non-blocking Virtual Pipeline IO 
    uint8_t bio_in_buf[MAX_DTLS_RECV_BUFFER];
    size_t bio_in_len;
    size_t bio_in_offset;

    int handshake_completed;
} custom_uv_context_t;

typedef struct {
    uv_udp_send_t req;
    uint8_t buffer[MAX_DTLS_RECV_BUFFER];
} custom_send_req_t;

/* --- Forward Declarations --- */
static void handle_dtls_handshake_step(custom_uv_context_t *bio_ctx);

/* --- Mbed TLS Asynchronous Timer Callbacks --- */

// Called when the libuv timer reaches the absolute timeout window

static void on_handshake_timer_expiry(uv_timer_t *handle) {
    custom_dtls_timer_t *timer_ctx = (custom_dtls_timer_t *) handle->data;
    custom_uv_context_t *bio_ctx = (custom_uv_context_t *) timer_ctx->bio_ctx_ptr;

    if (timer_ctx->is_cancelled || bio_ctx->handshake_completed) {
        return;
    }

    printf("[Timer] Retransmission timer expired. Stepping engine to resend lost flight...\n");

    // Drive the state machine. Mbed TLS detects the internal timeout and retransmits the last flight.
    handle_dtls_handshake_step(bio_ctx);
}

static void custom_timer_set_delay(void *ctx, uint32_t int_ms, uint32_t fin_ms) {
    custom_dtls_timer_t *timer_ctx = (custom_dtls_timer_t *) ctx;

    // A final delay of 0 cancels the active timer
    if (fin_ms == 0) {
        timer_ctx->is_cancelled = 1;
        uv_timer_stop(&timer_ctx->uv_timer);
        return;
    }

    timer_ctx->intermediate_ms = int_ms;
    timer_ctx->final_ms = fin_ms;
    timer_ctx->start_timestamp = uv_now(timer_ctx->uv_timer.loop);
    timer_ctx->is_cancelled = 0;

    // Fire the libuv timer at the final timeout milestone.
    // If you need dual-interval alerts, change this to fire at int_ms and step iteratively.
    uv_timer_start(&timer_ctx->uv_timer, on_handshake_timer_expiry, fin_ms, 0);
}

static int custom_timer_get_delay(void *ctx) {
    custom_dtls_timer_t *timer_ctx = (custom_dtls_timer_t *) ctx;

    if (timer_ctx->is_cancelled) {
        return -1; // Timer is cancelled or disabled
    }

    uint64_t elapsed = uv_now(timer_ctx->uv_timer.loop) - timer_ctx->start_timestamp;

    if (elapsed >= timer_ctx->final_ms) {
        return 2; // Final timeout expired
    }
    if (elapsed >= timer_ctx->intermediate_ms) {
        return 1; // Intermediate delay passed
    }

    return 0; // No delays have passed
}

/* --- Out-of-Order Bitmap Utilities --- */

static void mark_bitmap(uint8_t *bitmap, uint32_t offset, uint32_t len) {
    for (uint32_t i = offset; i < offset + len; i++) {
        bitmap[i / 8] |= (1 << (i % 8));
    }
}

static int is_bitmap_complete(const uint8_t *bitmap, uint32_t total_len) {
    for (uint32_t i = 0; i < total_len; i++) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) return 0;
    }
    return 1;
}

static int process_incoming_packet_v3(dtls_msg_reassembler_t *reassembler,
        const uint8_t *packet, size_t packet_len,
        uint8_t *out_buf, size_t *out_len) {
    if (packet_len < DTLS_RECORD_HEADER_LEN) return -1;

    uint8_t content_type = packet[0];
    uint16_t epoch = (packet[3] << 8) | packet[4];

    // Intercept Handshake (22) in Epoch 0 (Plaintext Handshake Records)
    if (content_type == 22 && epoch == 0) {
        size_t hs_offset = DTLS_RECORD_HEADER_LEN;
        if (hs_offset + DTLS_HANDSHAKE_HEADER_LEN > packet_len) return -1;

        uint8_t msg_type = packet[hs_offset];
        uint32_t length = (packet[hs_offset + 1] << 16) | (packet[hs_offset + 2] << 8) | packet[hs_offset + 3];
        uint16_t msg_seq = (packet[hs_offset + 4] << 8) | packet[hs_offset + 5];
        uint32_t frag_offset = (packet[hs_offset + 6] << 16) | (packet[hs_offset + 7] << 8) | packet[hs_offset + 8];
        uint32_t frag_length = (packet[hs_offset + 9] << 16) | (packet[hs_offset + 10] << 8) | packet[hs_offset + 11];

        // Isolate Handshake Messages that are actually fragmented (typically ClientHello [1])
        if (length > frag_length) {
            if (length > MAX_DTLS_RECV_BUFFER - (DTLS_RECORD_HEADER_LEN + DTLS_HANDSHAKE_HEADER_LEN)) return -1;

            // Allocate buffering structures on the very first fragment discovery
            if (reassembler->reassembly_buf == NULL) {
                reassembler->total_length = length;
                reassembler->message_seq = msg_seq;
                reassembler->reassembly_buf = (uint8_t *) calloc(1, length);
                reassembler->bitmap = (uint8_t *) calloc(1, (length + 7) / 8);
                if (!reassembler->reassembly_buf || !reassembler->bitmap) return -1;
            }

            // Boundary validation guards against malformed fragments
            if (frag_offset + frag_length > length ||
                    hs_offset + DTLS_HANDSHAKE_HEADER_LEN + frag_length > packet_len) {
                return -1;
            }

            // Safely write incoming payload chunk straight to its relative target block position
            const uint8_t *frag_payload = packet + hs_offset + DTLS_HANDSHAKE_HEADER_LEN;
            memcpy(reassembler->reassembly_buf + frag_offset, frag_payload, frag_length);
            mark_bitmap(reassembler->bitmap, frag_offset, frag_length);

            // Re-evaluate if all holes are filled
            if (is_bitmap_complete(reassembler->bitmap, reassembler->total_length)) {
                size_t total_hs_msg_len = DTLS_HANDSHAKE_HEADER_LEN + reassembler->total_length;
                size_t total_record_len = DTLS_RECORD_HEADER_LEN + total_hs_msg_len;

                if (*out_len < total_record_len) return -1;

                // Synthesize a fresh, completely unfragmented DTLS Record Header Base
                memcpy(out_buf, packet, DTLS_RECORD_HEADER_LEN);
                out_buf[11] = (total_hs_msg_len >> 8) & 0xFF;
                out_buf[12] = total_hs_msg_len & 0xFF;

                // Synthesize Complete Handshake Header Frame
                uint8_t *out_hs = out_buf + DTLS_RECORD_HEADER_LEN;
                out_hs[0] = msg_type;
                out_hs[1] = (length >> 16) & 0xFF;
                out_hs[2] = (length >> 8) & 0xFF;
                out_hs[3] = length & 0xFF;
                out_hs[4] = (msg_seq >> 8) & 0xFF;
                out_hs[5] = msg_seq & 0xFF;
                out_hs[6] = 0;
                out_hs[7] = 0;
                out_hs[8] = 0; // fragment_offset = 0
                out_hs[9] = out_hs[1];
                out_hs[10] = out_hs[2];
                out_hs[11] = out_hs[3]; // fragment_length = total_length

                // Flush collected linear handshake payload sequence right behind the header
                memcpy(out_hs + DTLS_HANDSHAKE_HEADER_LEN, reassembler->reassembly_buf, reassembler->total_length);
                *out_len = total_record_len;

                // Release local allocation memory structures for this message stream
                free(reassembler->reassembly_buf);
                reassembler->reassembly_buf = NULL;
                free(reassembler->bitmap);
                reassembler->bitmap = NULL;
                return 1; // Completed full message reconstruction
            }
            return 0; // Intercepted and cached successfully, awaiting more fragments
        }
    }

    // Pass-through processing lane for unfragmented packets or encrypted epochs
    if (*out_len < packet_len) return -1;
    memcpy(out_buf, packet, packet_len);
    *out_len = packet_len;
    return 2;
}
/* --- Defragmentation Parsing Engine --- */
/* bad
static int process_incoming_packet_v3(dtls_msg_reassembler_t *reassembler,
        const uint8_t *packet, size_t packet_len,
        uint8_t *out_buf, size_t *out_len) {
    if (packet_len < DTLS_RECORD_HEADER_LEN) return -1;

    uint8_t content_type = packet[0];
    uint16_t epoch = (uint16_t)((packet[3] << 8) | packet[4]);

    if (content_type == 22 && epoch == 0) {
        size_t hs_offset = DTLS_RECORD_HEADER_LEN;
        if (hs_offset + DTLS_HANDSHAKE_HEADER_LEN > packet_len) return -1;

        uint8_t msg_type = packet[hs_offset];
        uint32_t length = (packet[hs_offset + 1] << 16) | (packet[hs_offset + 2] << 8) | packet[hs_offset + 3];
        uint16_t msg_seq = (packet[hs_offset + 4] << 8) | packet[hs_offset + 5];
        uint32_t frag_offset = (packet[hs_offset + 6] << 16) | (packet[hs_offset + 7] << 8) | packet[hs_offset + 8];
        uint32_t frag_length = (packet[hs_offset + 9] << 16) | (packet[hs_offset + 10] << 8) | packet[hs_offset + 11];

        if (length > frag_length) {
            if (length > MAX_DTLS_RECV_BUFFER - (DTLS_RECORD_HEADER_LEN + DTLS_HANDSHAKE_HEADER_LEN)) return -1;

            if (reassembler->reassembly_buf == NULL) {
                reassembler->total_length = length;
                reassembler->message_seq = msg_seq;
                reassembler->reassembly_buf = (uint8_t *) calloc(1, length);
                reassembler->bitmap = (uint8_t *) calloc(1, (length + 7) / 8);
                if (!reassembler->reassembly_buf || !reassembler->bitmap) return -1;
            }

            if (frag_offset + frag_length > length ||
                    hs_offset + DTLS_HANDSHAKE_HEADER_LEN + frag_length > packet_len) {
                return -1;
            }

            const uint8_t *frag_payload = packet + hs_offset + DTLS_HANDSHAKE_HEADER_LEN;
            memcpy(reassembler->reassembly_buf + frag_offset, frag_payload, frag_length);
            mark_bitmap(reassembler->bitmap, frag_offset, frag_length);

            if (is_bitmap_complete(reassembler->bitmap, reassembler->total_length)) {
                size_t total_hs_msg_len = DTLS_HANDSHAKE_HEADER_LEN + reassembler->total_length;
                size_t total_record_len = DTLS_RECORD_HEADER_LEN + total_hs_msg_len;

                if (*out_len < total_record_len) return -1;

                memcpy(out_buf, packet, DTLS_RECORD_HEADER_LEN);
                out_buf = (total_hs_msg_len >> 8) & 0xFF;
                out_buf = total_hs_msg_len & 0xFF;

                uint8_t *out_hs = out_buf + DTLS_RECORD_HEADER_LEN;
                out_hs = msg_type;
                out_hs = (length >> 16) & 0xFF;
                out_hs = (length >> 8) & 0xFF;
                out_hs = length & 0xFF;
                out_hs = (msg_seq >> 8) & 0xFF;
                out_hs = msg_seq & 0xFF;
                out_hs = 0;
                out_hs = 0;
                out_hs = 0;
                out_hs = out_hs;
                out_hs = out_hs;
                out_hs = out_hs;

                memcpy(out_hs + DTLS_HANDSHAKE_HEADER_LEN, reassembler->reassembly_buf, reassembler->total_length);
 *out_len = total_record_len;

                free(reassembler->reassembly_buf);
                reassembler->reassembly_buf = NULL;
                free(reassembler->bitmap);
                reassembler->bitmap = NULL;
                return 1;
            }
            return 0;
        }
    }

    if (*out_len < packet_len) return -1;
    memcpy(out_buf, packet, packet_len);
 *out_len = packet_len;
    return 2;
}

 */

/* --- Mbed TLS Virtual BIO Callbacks --- */

static int custom_dtls_bio_recv(void *ctx, unsigned char *buf, size_t len) {
    custom_uv_context_t *bio_ctx = (custom_uv_context_t *) ctx;
    size_t available = bio_ctx->bio_in_len - bio_ctx->bio_in_offset;
    if (available == 0) return MBEDTLS_ERR_SSL_WANT_READ;

    size_t to_copy = (len < available) ? len : available;
    memcpy(buf, bio_ctx->bio_in_buf + bio_ctx->bio_in_offset, to_copy);
    bio_ctx->bio_in_offset += to_copy;

    if (bio_ctx->bio_in_offset >= bio_ctx->bio_in_len) {
        bio_ctx->bio_in_len = 0;
        bio_ctx->bio_in_offset = 0;
    }
    return (int) to_copy;
}

static void on_udp_send_complete(uv_udp_send_t *req, int status) {
    custom_send_req_t *send_req = (custom_send_req_t *) req;
    if (status < 0) fprintf(stderr, "Asynchronous write failed: %s\n", uv_strerror(status));
    free(send_req);
}

static int custom_dtls_bio_send(void *ctx, const unsigned char *buf, size_t len) {
    custom_uv_context_t *bio_ctx = (custom_uv_context_t *) ctx;
    if (len > MAX_DTLS_RECV_BUFFER) return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;

    custom_send_req_t *send_req = (custom_send_req_t *) malloc(sizeof (custom_send_req_t));
    if (!send_req) return MBEDTLS_ERR_SSL_ALLOC_FAILED;

    memcpy(send_req->buffer, buf, len);
    uv_buf_t uv_buf = uv_buf_init((char *) send_req->buffer, len);

    int ret = uv_udp_send(&send_req->req, &bio_ctx->udp_handle, &uv_buf, 1,
            (const struct sockaddr *) &bio_ctx->peer_addr, on_udp_send_complete);
    if (ret < 0) {
        free(send_req);
        return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
    }
    return (int) len;
}

static size_t custom_bio_ctrl_pending(const custom_uv_context_t *bio_ctx) {
    if (bio_ctx == NULL) return 0;
    return bio_ctx->bio_in_len - bio_ctx->bio_in_offset;
}

/* --- Handshake State Machine Step --- */

static void handle_dtls_handshake_step(custom_uv_context_t *bio_ctx) {
    int ret = mbedtls_ssl_handshake(&bio_ctx->ssl);

    if (ret == 0) {
        printf("DTLS Handshake successfully established!\n");


        bio_ctx->handshake_completed = 1; // Stop and cancel the retransmission timer upon handshake completion
        custom_timer_set_delay(&bio_ctx->handshake_timer, 0, 0);
    } else if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
        return;
    } else if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED) {
        printf("HelloVerifyRequest issued. Resetting session context.\n");
        mbedtls_ssl_session_reset(&bio_ctx->ssl);
    } else {
        char err_string[256];
        mbedtls_strerror(ret, err_string, sizeof (err_string));
        fprintf(stderr, "Fatal error executing DTLS Handshake: -0x%04X (%s)\n", -ret, err_string);
        custom_timer_set_delay(&bio_ctx->handshake_timer, 0, 0);
    }
}

/* --- Libuv Runtime Engine Drivers --- */
static void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char *) malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_udp_recv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags) {
    custom_uv_context_t *bio_ctx = (custom_uv_context_t *) handle->data;
    if (nread < 0 || addr == NULL) {
        if (buf->base) free(buf->base);
        return;
    }
    if (addr->sa_family == AF_INET) {
        struct sockaddr_in *client_addr4 = (struct sockaddr_in *) addr;
        memcpy(&bio_ctx->peer_addr, addr, sizeof (struct sockaddr_in));
        mbedtls_ssl_set_client_transport_id(&bio_ctx->ssl, (const unsigned char *) &client_addr4->sin_addr.s_addr, sizeof (client_addr4->sin_addr.s_addr));
    } else if (addr->sa_family == AF_INET6) {
        struct sockaddr_in6 *client_addr6 = (struct sockaddr_in6 *) addr;
        memcpy(&bio_ctx->peer_addr, addr, sizeof (struct sockaddr_in6));
        mbedtls_ssl_set_client_transport_id(&bio_ctx->ssl, (const unsigned char *) &client_addr6->sin6_addr, sizeof (client_addr6->sin6_addr));
    }
    size_t out_len = sizeof (bio_ctx->bio_in_buf);
    int status = process_incoming_packet_v3(&bio_ctx->reassembler, (const uint8_t *) buf->base, nread, bio_ctx->bio_in_buf, &out_len);
    if (status > 0) {
        bio_ctx->bio_in_len = out_len;
        bio_ctx->bio_in_offset = 0;
        if (!bio_ctx->handshake_completed) {
            handle_dtls_handshake_step(bio_ctx);
        } else {
            uint8_t app_buffer[1024];
            int read_ret;
            do {
                read_ret = mbedtls_ssl_read(&bio_ctx->ssl, app_buffer, sizeof (app_buffer));
                if (read_ret > 0) {
                    printf("Decrypted Application Payload processed.\n");
                }
            } while (read_ret == MBEDTLS_ERR_SSL_WANT_READ || custom_bio_ctrl_pending(bio_ctx) > 0 || mbedtls_ssl_check_pending(&bio_ctx->ssl) > 0);
        }
    }
    if (buf->base) free(buf->base);
}

/* --- Context Lifecycle Configurations --- */
custom_uv_context_t *custom_uv_context_init_secure(uv_loop_t *loop, const char *listen_ip, int port, const char *cert_file, const char *key_file, const char *ca_file) {
    int ret;
    custom_uv_context_t *bio_ctx = (custom_uv_context_t *) calloc(1, sizeof (custom_uv_context_t));
    if (!bio_ctx) return NULL;
    bio_ctx->handshake_completed = 0; // Hook the custom asynchronous timer configurations
    uv_timer_init(loop, &bio_ctx->handshake_timer.uv_timer);
    bio_ctx->handshake_timer.uv_timer.data = &bio_ctx->handshake_timer;
    bio_ctx->handshake_timer.bio_ctx_ptr = bio_ctx;
    bio_ctx->handshake_timer.is_cancelled = 1;
    mbedtls_ssl_init(&bio_ctx->ssl);
    mbedtls_ssl_config_init(&bio_ctx->ssl_conf);
    mbedtls_entropy_init(&bio_ctx->entropy);
    mbedtls_ctr_drbg_init(&bio_ctx->ctr_drbg);
    mbedtls_x509_crt_init(&bio_ctx->server_cert);
    mbedtls_x509_crt_init(&bio_ctx->ca_chain);
    mbedtls_pk_init(&bio_ctx->private_key);
    mbedtls_ssl_cookie_init(&bio_ctx->cookie_ctx);
    ret = mbedtls_ctr_drbg_seed(&bio_ctx->ctr_drbg, mbedtls_entropy_func, &bio_ctx->entropy, (const unsigned char *) "dtls_handshake_srv", 18);
    if (ret != 0) goto cleanup_fail;
    ret = mbedtls_x509_crt_parse_file(&bio_ctx->server_cert, cert_file);
    if (ret != 0) goto cleanup_fail;
    ret = mbedtls_pk_parse_keyfile(&bio_ctx->private_key, key_file, NULL, mbedtls_ctr_drbg_random, &bio_ctx->ctr_drbg);
    if (ret != 0) goto cleanup_fail;
    if (ca_file != NULL) {
        //ret = mbedtls_x509_crt_parse_file(&bio_chain, ca_file); // Fix typo mapping reference context target
        ret = mbedtls_x509_crt_parse_file(&bio_ctx->ca_chain, ca_file);
        if (ret != 0) goto cleanup_fail;
    }
    ret = mbedtls_ssl_config_defaults(&bio_ctx->ssl_conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) goto cleanup_fail;
    mbedtls_ssl_conf_rng(&bio_ctx->ssl_conf, mbedtls_ctr_drbg_random, &bio_ctx->ctr_drbg);
    ret = mbedtls_ssl_conf_own_cert(&bio_ctx->ssl_conf, &bio_ctx->server_cert, &bio_ctx->private_key);
    if (ret != 0) goto cleanup_fail;
    if (ca_file != NULL) {
        mbedtls_ssl_conf_ca_chain(&bio_ctx->ssl_conf, &bio_ctx->ca_chain, NULL);
        mbedtls_ssl_conf_authmode(&bio_ctx->ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    }
    ret = mbedtls_ssl_cookie_setup(&bio_ctx->cookie_ctx, mbedtls_ctr_drbg_random, &bio_ctx->ctr_drbg);
    if (ret != 0) goto cleanup_fail;
    mbedtls_ssl_conf_dtls_cookies(&bio_ctx->ssl_conf, mbedtls_ssl_cookie_write, mbedtls_ssl_cookie_check, &bio_ctx->cookie_ctx); // Optional: Tune the minimum and maximum handshake retransmission thresholds (e.g., 1000ms to 60000ms)
    mbedtls_ssl_conf_handshake_timeout(&bio_ctx->ssl_conf, 1000, 60000);
    ret = mbedtls_ssl_setup(&bio_ctx->ssl, &bio_ctx->ssl_conf);
    if (ret != 0) goto cleanup_fail;
    mbedtls_ssl_set_bio(&bio_ctx->ssl, bio_ctx, custom_dtls_bio_send, custom_dtls_bio_recv, NULL); // Bind Asynchronous Timer Strategy to the Mbed TLS Engine
    mbedtls_ssl_set_timer_cb(&bio_ctx->ssl, &bio_ctx->handshake_timer, custom_timer_set_delay, custom_timer_get_delay);
    uv_udp_init(loop, &bio_ctx->udp_handle);
    bio_ctx->udp_handle.data = bio_ctx;
    struct sockaddr_in bind_addr;
    uv_ip4_addr(listen_ip, port, &bind_addr);
    uv_udp_bind(&bio_ctx->udp_handle, (const struct sockaddr *) &bind_addr, UV_UDP_REUSEADDR);
    uv_udp_recv_start(&bio_ctx->udp_handle, on_alloc, on_udp_recv);
    return bio_ctx;
cleanup_fail:
    mbedtls_ssl_free(&bio_ctx->ssl);
    mbedtls_ssl_config_free(&bio_ctx->ssl_conf);
    mbedtls_x509_crt_free(&bio_ctx->server_cert);
    mbedtls_x509_crt_free(&bio_ctx->ca_chain);
    mbedtls_pk_free(&bio_ctx->private_key);
    mbedtls_ssl_cookie_free(&bio_ctx->cookie_ctx);
    mbedtls_ctr_drbg_free(&bio_ctx->ctr_drbg);
    mbedtls_entropy_free(&bio_ctx->entropy);
    free(bio_ctx);
    return NULL;
}

void custom_uv_context_free(custom_uv_context_t *bio_ctx) {
    if (bio_ctx) {
        uv_udp_recv_stop(&bio_ctx->udp_handle);
        uv_close((uv_handle_t *) & bio_ctx->udp_handle, NULL); // Safely halt and close the libuv timer handle
        uv_timer_stop(&bio_ctx->handshake_timer.uv_timer);
        uv_close((uv_handle_t *) & bio_ctx->handshake_timer.uv_timer, NULL);
        if (bio_ctx->reassembler.reassembly_buf) free(bio_ctx->reassembler.reassembly_buf);
        if (bio_ctx->reassembler.bitmap) free(bio_ctx->reassembler.bitmap);
        mbedtls_ssl_free(&bio_ctx->ssl);
        mbedtls_ssl_config_free(&bio_ctx->ssl_conf);
        mbedtls_x509_crt_free(&bio_ctx->server_cert);
        mbedtls_x509_crt_free(&bio_ctx->ca_chain);
        mbedtls_pk_free(&bio_ctx->private_key);
        mbedtls_ssl_cookie_free(&bio_ctx->cookie_ctx);
        mbedtls_ctr_drbg_free(&bio_ctx->ctr_drbg);
        mbedtls_entropy_free(&bio_ctx->entropy);
        free(bio_ctx);
    }
}

int main(int argc, char **argv) {
    uv_loop_t *loop = uv_default_loop();
    printf("Starting Defragmenting, Handshaking DTLS Node with Retransmission Timers...\n");
    custom_uv_context_t *server_context = custom_uv_context_init_secure(loop, "0.0.0.0", 8000, "./certificate.crt", "./private_key.pem", nullptr);
    if (!server_context) {
        fprintf(stderr, "Failed to initialize server loop environments safely.\n");
        return -1;
    }
    int ret = uv_run(loop, UV_RUN_DEFAULT);
    custom_uv_context_free(server_context);
    return ret;
}



#endif
