/**
 * libdatachannel client example
  https://github.com/Focusrite-Novation/libdatachannel
  Pacer
  https://webrtc.googlesource.com/src/+/HEAD/modules/pacing/g3doc/index.md

  https://webrtc.googlesource.com/src/+/HEAD/p2p/g3doc/ice.md
 * 
 * https://github.com/creytiv/re
 * https://en.wikipedia.org/wiki/UDP_hole_punching 
  cricket::Candidate represents an address discovered by a cricket::Port. A candidate can be local (i.e discovered by a local port) or remote. Remote candidates are transported using signaling, i.e outside of webrtc. There are 4 types of candidates: local, stun, prflx or relay (standard)

 */

#include "nlohmann/json.hpp"

#include "h264fileparser.hpp"
#include "opusfileparser.hpp"
#include "helpers.hpp"
//#include "ArgParser.hpp"
#include "socketio/socketioClient.h"
#include "base/async.h"
#include "Settings.h"

//#define localtesting 1
//#define VIDEOMEDIA 1

#define remotetesting 1


#define CERTFROMFILE 2

using namespace rtc;
using namespace std;
//using namespace std::chrono_literals;

using namespace base;
using namespace base::net;


using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }

/// all connected clients
unordered_map<string, shared_ptr<Client>> clients{};

/// Creates peer connection and client representation
/// @param config Configuration
/// @param wws Websocket for signaling
/// @param id Client ID
/// @returns Client
shared_ptr<Client> createPeerConnection(const Configuration &config, string id, bool isClient);
shared_ptr<Client> createPeerConnection_lc(const Configuration &config, string id);

shared_ptr<Client> createPeerConnection_rm(const Configuration &config, string id, Async &async, bool isClient);

/// Creates stream
/// @param h264Samples Directory with H264 samples
/// @param fps Video FPS
/// @param opusSamples Directory with opus samples
/// @returns Stream object
shared_ptr<Stream> createStream(const string h264Samples, const unsigned fps, const string opusSamples);

/// Add client to stream
/// @param client Client
/// @param adding_video True if adding video
void addToStream(shared_ptr<Client> client, bool isAddingVideo);

/// Start stream
void startStream();

/// Main dispatch queue
DispatchQueue MainThread("Main");

/// Audio and video stream
optional<shared_ptr<Stream>> avStream = nullopt;

const string defaultRootDirectory = "../../../examples/streamer/samples/";
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
Configuration config;

std::string id;
  

void sendCandidate( const std::string &mid, int mlineindex, const std::string &sdp )
{
    json desc;
    desc["sdpMid"] = mid;
    desc["sdpMLineIndex"] = mlineindex;
    desc["candidate"] = sdp;

    json m;
    m["type"] = "candidate";
    m["candidate"] = desc;
    
    if(!from.empty())
    {
        m["from"] = from;
        m["to"] = from;
    }
    
    m["room"] = room;
    SInfo << "send:"  <<  sdp << "candidate to: "<< from<< std::endl;
    
    mysocket->emit("message", m);
}

void sendSdp( const std::string &sdp, const std::string &type   )
{

    json desc = {

        {"type", type},
        {"sdp", sdp}
    };

    json m;
    m["type"] = type;
    
    m["desc"] = desc;
    
    
    if(!from.empty())
    {
        m["from"] = from;
        m["to"] = from;
    }
     
    m["room"] = room;
    
    // smpl::Message m({ type, {

    SInfo << "send:"  << type << " to: "<< from<< std::endl;
    
    mysocket->emit("message", m);
                
}

/// Incomming message handler for websocket
/// @param message Incommint message
/// @param config Configuration
/// @param ws Websocket


#if !defined( localtesting) && !defined(remotetesting)

void wsOnMessage(json const &m ) {
    
    
    std::string type;
   
    std::string to;
    std::string user;
  
     if (m.find("room") != m.end())
    {
        room = m["room"].get<std::string>();
    }
    else
    {
        SError << " On Peer message is missing room id ";
        return;
    }


    
    if (m.find("type") != m.end())
    {
      type = m["type"].get<string>();

    }
     
 
    
    if (m.find("to") != m.end()) { to = m["to"].get<std::string>(); }

    if (m.find("from") != m.end())
    {
        from = m["from"].get<std::string>();
        if(id.empty())
        id =from;
    }
    else
    {
        SError << " On Peer message is missing participant id ";
        return;
    }

    if (m.find("type") != m.end()) { type = m["type"].get<std::string>(); }
    else
    {
        SError << " On Peer message is missing SDP type";
    }

   

    
    if (m.find("cam") != m.end())
    {
        id = m["cam"].get<std::string>();

    }


    if (m.find("starttime") != m.end())
    {
      //  camT.start = m["starttime"].get<std::string>();

    }

//
//    if (m.find("camAudio") != m.end()) { camT.camAudio = m["camAudio"].get<bool>(); }
//
//    if (m.find("appAudio") != m.end()) { camT.appAudio = m["appAudio"].get<bool>(); }
    
    
     

     
    

    if (type == "offer") {
         clients.emplace(id, createPeerConnection(config,  id, false));
         
         //clients.emplace(id, createPeerConnection(config,  id));
        if (auto jt = clients.find(id); jt != clients.end()) {
            auto pc = jt->second->peerConnection;
           
            auto sdp = m["desc"]["sdp"].get<string>();
            
            SInfo << "setRemoteDescription " << type ;
            
            auto description = Description(sdp, type);
            pc->setRemoteDescription(description);
            pc->setLocalDescription( );
        }
         
         
    } else if (type == "answer") {
        
       //clients.emplace(id, createPeerConnection(config,  id));
        if (auto jt = clients.find(id); jt != clients.end()) {
            auto pc = jt->second->peerConnection;
           
            auto sdp = m["desc"]["sdp"].get<string>();
            
            SInfo << "setRemoteDescription " << type ;
            
            auto description = Description(sdp, type);
            pc->setRemoteDescription(description);
           // pc->setLocalDescription( Description::Type::Answer);
        }
    }
    else if (type == "candidate")
    {
        
         json cand =  m["candidate"];
         
         auto sdp = cand["candidate"].get<std::string>();
         auto mid = cand["sdpMid"].get<std::string>();
            
         if (auto jt = clients.find(id); jt != clients.end()) {
            auto pc = jt->second->peerConnection;
            //auto sdp = m["desc"].get<string>();
            //auto description = Description(sdp, type);
           // pc->setRemoteDescription(description);
            
            SInfo << sdp;
            
            pc->addRemoteCandidate(rtc::Candidate(sdp, mid));
        }
        
           
           
    }
    
}


void initiate(std::string rm)
{

    room = rm;
    id = "client"; /// hard coded the id for client(second participant), since it will have only one instance. Second instance should only have one instance, otherwise throw error. TBD 

    clients.emplace(id, createPeerConnection(config,  id, true));
}

#endif


int main(int argc, char **argv) 
{

/////////////////////////////////////////////////
    base::cnfg::Configuration cache;

    cache.load("./cache.js");

    Settings::SetConfiguration(cache, argc);




////////////////////////////////////////////////////    


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
    config.iceServers.emplace_back(stunServer);
    config.disableAutoNegotiation = true;
    // read cert from file
#if CERTFROMFILE == 1
    config.keyPemFile = "/var/tmp/key/private_key.pem";
    config.certificatePemFile = "/var/tmp/key/certificate.crt";  
    config.keyPemPass = "12345678";
    
#elif CERTFROMFILE == 2

    /* convert pem to single line
     * # awk 'NF {sub(/\r/, ""); printf "%s\\n",$0;}' certificate.crt  
    */

    config.keyPemFile = "-----BEGIN PRIVATE KEY-----\nMIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDAPJVsM+7tQxKy\n2IBp+8i2aCuv3xl1wftDxXqG7GYuatDid8rwHBH68JcnTU09T8RHi+Ezj+0YPYV4\nIDGTUDufxK1snv5V6wdKESZM2ZYvzDIDuHCiXrtl5Tee+tnh1XYk4CXk9h+SsB/X\n70FXIW98XqR+2iVl1ezwjEeu7X1ET9wh1UHOiLB0do5+dSDo/nNIP+K+QnG/YC9v\nYUViozWO1JvZ0KgEybOrTbRKWsHKRyRNOyZtXNUMcLt2vJLCm5dmDfPCeqbEagyN\nZLPpucd+HRoZ1U1aXvZ36l30sJlvIjnkeLkccd3Bv85fhAvzK5WoAqSsB0nFOAYm\nDIVcyDcfAgMBAAECggEBALOfqGtLd4SBONaeUBc36kruuWuDRnHvCM5BlwS9nZjf\nvEDweFK1l+NnrYVOyM5yW1ATFyGr6XnN+onNYyVoQd4+02F8iuBTVSNTNPt4EMqm\nvVEWpUBCzk4eyUMm2DIZ2GQKgb4YcFYLdiW57M7ycg6/DGtvgKRQKS53lX+Rb4xE\nhYrhhfDuKJf1qYy4h/IKYBEruRjrinLjYACIdQ5Wi1A/Bjci5+rN9tsjVBmlXrND\nwCu2cyitsrvKDm3/jZFScP4Ydv+78qlHC6EqL3XZacJ+BqCaRmv4UC4x+VhSt85L\nE0V5RlgJzwmbr0f26bJ/scq0qzJ+mdUhXkhvmsfVBhkCgYEA+HNOO42Jj1yoXRqH\nkEDAGCM5c/uUTDZT6FrMT254okjPMkDj6SRFwCjc7v7a3yKBI4DzLfoySvlLXyBa\nhQe9BJo6MNPElAmeJKj1UmzQz8uBGhWuAoFfkgySe7WtThRCxEAtZ3wF06/6Y3Q+\nMoDUFsIYq68UMdbXDwGTZfnVdpsCgYEAxhP9176Bt4XZ7wtm3gO7ZdxwuoQ0Wu03\nl5FKwTnrVZ7ljzq3mVVV0i++5Py7sayZmsehPbHdl3vVmbTmSLRaHiiZNZKqaHDr\nmTK3H6MKa+C2bKOgat+O0ulTWH+EdDK9pCsP7XPkE8mL0tsG1ZLkvpgp2cnaOPIO\nh1++k4BuB80CgYAUkt/QmKjigUbD5vWA4YvGs+wHCbc/FGSgYhx3G2vL7IGT5MG6\nxbEs93VMKTiQr7fH6963WPefM8OlDfXQ/FIPtoHJF1A4/g7ldERUXgRwoKaBNXhi\nZro2Suo6alH+nDjnLXVVE3UcEX+HitG3tulZNRt75BSlB+hpKrU9BZJCrwKBgQCQ\ndsgub50/8nmOJKyzw9kLY4k8H2vn3RcsjiUNZGbFHYyjt9lsFZbwIy6A5+sknJOz\nFWH+ExlggEq7PfqukAsh784+CmgKoEDUjO6OPmU9ZLjn5zb6e245WT8WTnqWHOO/\nNkD5mAqCe/5knKYRYn8+ms/7LYLhAXmjNitSfNrDCQKBgQDdhcjA4aGenydX49pU\n7TnizzXPaU449hQvXL5DLHOXMifnxntQdXv87yyZy5WmX3GSkobV7gu9y7EGOryZ\n7h4irr7ecphOI2/7Qgfpd0b+MaI8TcHK8BkpMCC5iKfpNNfzXQrXYfXHKvdhlbi2\n5yXcrRZxBSvWu+IDiGqFBdL+ig==\n-----END PRIVATE KEY-----\n";
    config.certificatePemFile = "-----BEGIN CERTIFICATE-----\nMIICwjCCAaqgAwIBAgIJAOKZBPq4tcY7MA0GCSqGSIb3DQEBCwUAMBkxFzAVBgNV\nBAMTDkFydmluZFNjb3BlQXBwMB4XDTIwMDYxMzA0MjE0N1oXDTMwMDYxMTA0MjE0\nN1owGTEXMBUGA1UEAxMOQXJ2aW5kU2NvcGVBcHAwggEiMA0GCSqGSIb3DQEBAQUA\nA4IBDwAwggEKAoIBAQDAPJVsM+7tQxKy2IBp+8i2aCuv3xl1wftDxXqG7GYuatDi\nd8rwHBH68JcnTU09T8RHi+Ezj+0YPYV4IDGTUDufxK1snv5V6wdKESZM2ZYvzDID\nuHCiXrtl5Tee+tnh1XYk4CXk9h+SsB/X70FXIW98XqR+2iVl1ezwjEeu7X1ET9wh\n1UHOiLB0do5+dSDo/nNIP+K+QnG/YC9vYUViozWO1JvZ0KgEybOrTbRKWsHKRyRN\nOyZtXNUMcLt2vJLCm5dmDfPCeqbEagyNZLPpucd+HRoZ1U1aXvZ36l30sJlvIjnk\neLkccd3Bv85fhAvzK5WoAqSsB0nFOAYmDIVcyDcfAgMBAAGjDTALMAkGA1UdEwQC\nMAAwDQYJKoZIhvcNAQELBQADggEBAC5KyEK9/Z4VM2CSNbFm6IzND0AACqYT2e8d\nHsT5/cLo+Zc7NWvMagq+myAAYEptarbvHNVWS/gsYWSg5+pHhrs1VPCXZTLjelGG\nnSqEZSXl4ANV9yNP/KdG8z8zruHKsqwJ0LDLem2KOnA0WzcEO1IRH59EnVsV4CkT\nCs1DH2i20NCZklwFREd3AOgkPR7pruxITN6hQ6MH/MHC6FyQbbvJEl7ceV1adON/\nXJNYomKwCVkxLss8PV/TcyPA9CWJA/c9blh/GPRAerqbBF7OwPVKmt3RxBr02tGT\nTFTokCgkm2d9DYtf0rtQOOL82zZB/YmgQytMYxaiUCf31xJTR/I=\n-----END CERTIFICATE-----\n";  
   // config.keyPemPass = "12345678";
    
#else
    
#endif

    string localId = "server";
    cout << "The local ID is: " << localId << endl;
    
   
#if localtesting 
    std::string id ="server";

   clients.emplace(id, createPeerConnection_lc(config,  id));
#elif remotetesting

    if(cache.loaded())
    {
       
        id = "client"; /// hard coded the id for client(second participant), since it will have only one instance. Second instance should only have one instance, otherwise throw error. TBD 

        clients.emplace(id, createPeerConnection_rm(config,  id, async, true));
    
       
    }
  
#else   
    std::string room = "65f570720af337cec5335a70ee88cbfb7df32b5ee33ed0b4a896a0";
    std::string host = ip_address;
    int port = 8443;
    
    sockio::SocketioClient *client;

    
    client  = new sockio::SocketioClient(host, port, true);
    client->connect();

    mysocket = client->io();

    mysocket->on(
        "connection",
        sockio::Socket::event_listener_aux(
            [=](string const &name, json const &data, bool isAck, json &ack_resp)
            {
                mysocket->on(
                    "created",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &data, bool isAck, json &ack_resp)
                        {
                            SInfo << cnfg::stringify(data);
                            SInfo << "ws: Created room " << data[0] << "- my client ID is " << data[1];
                            //isInitiator = true;
                            // grabWebCamVideo();
                        }));


                mysocket->on(
                    "join",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &data, bool isAck, json &ack_resp)
                        {
                            SInfo << "ws join " << cnfg::stringify(data);
                             SInfo << "ws: Created room " << data[0] << "- my client ID is " << data[1] << " noClientInRoom: " << data[2];
                           
                            std::string room1 = data[0];
                            
                            int noClientInRoom = data[2].get<int>();
                            
                            if(noClientInRoom > 1)
                             initiate(room1 );

                             
                            // LTrace("Another peer made a request to join room " + room)
                            // LTrace("This peer is the initiator of room " + room + "!")
                            //isChannelReady = true;
                        }));

                
                mysocket->on(
                    "joined",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &m, bool isAck, json &ack_resp)
                        {
                            SInfo << "ws joined "  <<  cnfg::stringify(m);
                            // LTrace("Another peer made a request to join room " + room)
                            // LTrace("This peer is the initiator of room " + room + "!")
                            //isChannelReady = true;
                              //wsOnMessage(m);
                        }));
                        
                        
                /// for webrtc messages
                mysocket->on(
                    "message",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &m, bool isAck, json &ack_resp)
                        {
                            //LTrace(cnfg::stringify(m));
                             STrace << "SocketioClient received message:" <<  cnfg::stringify(m);

                            //onPeerMessage((string &) name, m); //arvind
                            // signalingMessageCallback(message);
                            
                             wsOnMessage(m);
                        }));


                // Leaving rooms and disconnecting from peers.
                mysocket->on(
                    "disconnectClient",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &data, bool isAck, json &ack_resp)
                        {
                            std::string from = data.get<std::string>();
                             SInfo << "disconnectClient " <<  from;
                             LInfo(cnfg::stringify(data));
                           // onPeerDiconnected(from);  //arvind
                        }));


                mysocket->on(
                    "bye",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &data, bool isAck, json &ack_resp)
                        {
                            SInfo << cnfg::stringify(data);
                            // LTrace("Peer leaving room", room);
                        }));
 
                mysocket->emit("createorjoin" , room);
            }));

#endif

//    while (true) {
//        string id;
//        cout << "Enter to exit" << endl;
//        cin >> id;
//        cin.ignore();
//        cout << "exiting" << endl;
//        break;
//    }

   app.waitForShutdown([&](void*)
   {

    SInfo << "app.run() is over";
//    Settings::exit();         
//    rtc::CleanupSSL();
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

shared_ptr<ClientTrackData> addVideo(const shared_ptr<PeerConnection> pc, const uint8_t payloadType, const uint32_t ssrc, const string cname, const string msid, const function<void (void)> onOpen) {
    auto video = Description::Video(cname);
    video.addH264Codec(payloadType);
    video.addSSRC(ssrc, cname, msid, cname);
    auto track = pc->addTrack(video);
    // create RTP configuration
    auto rtpConfig = make_shared<RtpPacketizationConfig>(ssrc, cname, payloadType, H264RtpPacketizer::defaultClockRate);
    // create packetizer
    auto packetizer = make_shared<H264RtpPacketizer>(NalUnit::Separator::Length, rtpConfig);
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

shared_ptr<ClientTrackData> addAudio(const shared_ptr<PeerConnection> pc, const uint8_t payloadType, const uint32_t ssrc, const string cname, const string msid, const function<void (void)> onOpen) {
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
shared_ptr<Client> createPeerConnection_lc(const Configuration &config,  string id)
{
    auto pc1 = make_shared<PeerConnection>(config);
    auto pc2 = make_shared<PeerConnection>(config); 
    {
       
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

                    int x = 1; //arvind
                }
                //);
            }
        });



        pc1->onLocalDescription([ id, pc1, pc2](rtc::Description description) {
    //		json message = {{"id", id},
    //		                {"type", description.typeString()},
    //		                {"description", std::string(description)}};

            SInfo << "pc1 send sdp:"  << description.typeString() <<  " des "<<  std::string(description);

         //  pc1->setLocalDescription(Description::Type::Offer);// Description::Type::Answer);          
            //sendSdp( std::string(description), description.typeString());
            // Make the answer
    //		if (auto ws = wws.lock())
    //			ws->send(message.dump());
            
           // SInfo << "setRemoteDescription " << type ;
            
           // auto description = Description(sdp, type);
            pc2->setRemoteDescription(description);
            
        });

        pc1->onLocalCandidate([ id, pc2](rtc::Candidate candidate) {
    //            json message = {{"id", id},
    //                            {"type", "candidate"},
    //                            {"candidate", std::string(candidate)},
    //                            {"mid", candidate.mid()}};

            //sendCandidate( candidate.mid(), 1,  std::string(candidate)  );
    //            if (auto ws = wws.lock())
    //                    ws->send(message.dump());
            
            SInfo << "pc1 send candidated:"  << candidate.mid() <<  " des "<<  std::string(candidate);
            
            pc2->addRemoteCandidate(candidate);
            
        });

        pc1->onGatheringStateChange(
            [](PeerConnection::GatheringState state) {
           
            if (state == PeerConnection::GatheringState::Complete)
            {
                 SInfo << "pc1 Gathering State: Complete"  ;
              //  if(auto pc1 = wpc1.lock())
                {
    //                json desc;
    //                desc["type"] =  description->typeString();
    //                desc[sdp] = sdp;
    //    

                }
            }
        });
    #if VIDEOMEDIA

        client->video = addVideo(pc1, 102, 1, "video-stream", "stream1", [id, wc = make_weak_ptr(client)]() {
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

        client->audio = addAudio(pc1, 111, 2, "audio-stream", "stream1", [id, wc = make_weak_ptr(client)]() {


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

        auto dc = pc1->createDataChannel("ping-pong-pc1");
        dc->onOpen([id, wdc = make_weak_ptr(dc)]() {
            if (auto dc = wdc.lock()) {
                 SInfo << "ping-pong-pc1 onOpen";
                dc->send("ping-pong-pc1 send on open Ping");
            }
        });

        dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg) {
            SInfo << "Pc1 Message from " << id << " received: " << msg << endl;
            if (auto dc = wdc.lock()) {
                dc->send("ping-pong-pc1 send on message Ping");
            }
        });
        client->dataChannel1 = dc;



        pc1->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
    		SInfo << "pc1 onDataChannel from " << id << " received with label \"" << dc->label() << "\""
    		          << std::endl;
    
    		dc->onOpen([wdc = make_weak_ptr(dc)]() {
    			if (auto dc = wdc.lock())
                        {       SInfo << "pc1 open ";
    				dc->send("Hello from  pc1");
                        }
    		});
    
    		dc->onClosed([id]() {
                    SInfo << "pc1 DataChannel from " << id << " closed" << std::endl; }
                
                );
    
    		dc->onMessage([id,dc](auto data) {
    			// data holds either std::string or rtc::binary
    			if (std::holds_alternative<std::string>(data))
    				SInfo << "pc1 Message from " << id << " received: " << std::get<std::string>(data)
    				          << std::endl;
    			else
    				SInfo << "pc1  Binary message from " << id
    				          << " received, size=" << std::get<rtc::binary>(data).size() << std::endl;
                        
                        
                        sleep(500);
                        dc->send("PC1 to PC2");
                        
    		});
    
    		 client->dataChannel11 = dc;
    	});

        pc1->setLocalDescription();
     //   return client;
    }
    
    {
       
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

                    int x = 1; //arvind
                }
                //);
            }
        });



        pc2->onLocalDescription([ id, pc1](rtc::Description description) {
    //		json message = {{"id", id},
    //		                {"type", description.typeString()},
    //		                {"description", std::string(description)}};

            SInfo << "pc2 send sdp:"  << description.typeString() <<  " des "<<  std::string(description);

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
          SInfo << "pc2 send candidated:"  << candidate.mid() <<  " des "<<  std::string(candidate);
            //sendCandidate( candidate.mid(), 1,  std::string(candidate)  );
    //            if (auto ws = wws.lock())
    //                    ws->send(message.dump());
            pc1->addRemoteCandidate(candidate);
        });

        pc2->onGatheringStateChange(
            [](PeerConnection::GatheringState state) {
            
            if (state == PeerConnection::GatheringState::Complete)
            {
                SInfo << "Pc2 Gathering State: Complete" ;
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

        client->video = addVideo(pc2, 102, 1, "video-stream", "stream1", [id, wc = make_weak_ptr(client)]() {
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

        client->audio = addAudio(pc2, 111, 2, "audio-stream", "stream1", [id, wc = make_weak_ptr(client)]() {


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

        auto dc = pc2->createDataChannel("ping-pong-pc2");
        dc->onOpen([id, wdc = make_weak_ptr(dc)]() {
            if (auto dc = wdc.lock()) {
                SInfo << "pc2 onOpen";
                dc->send("ping-pong pc2 on open send");
            }
        });

        dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg) {
            SInfo << "Message from " << id << "pc2  received: " << msg << endl;
            if (auto dc = wdc.lock()) {
                dc->send("ping-pong pc2 on message send");
            }
        });
        client->dataChannel2 = dc;



        pc2->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
    		SInfo << "PC2 onDataChannel from " << id << " received with label \"" << dc->label() ;
    		     
    
    		dc->onOpen([wdc = make_weak_ptr(dc)]() {
    			if (auto dc = wdc.lock())
    				dc->send("PC2 Hello from  arvind");
    		});
    
    		dc->onClosed([id]() {
                    SInfo << "DataChannel from " << id << " closed" << std::endl;
                }
                
                );
    
    		dc->onMessage([id,dc](auto data) {
    			// data holds either std::string or rtc::binary
    			if (std::holds_alternative<std::string>(data))
    				SInfo << "onDataChannel:onMessage  PC2 Message from " << id << " received: " << std::get<std::string>(data)
    				          << std::endl;
    			else
    				SInfo << "onDataChannel:onMessage PC2 Binary message from " << id
    				          << " received, size=" << std::get<rtc::binary>(data).size() << std::endl;
                        
                        sleep(500);
                        dc->send("PC2 tp PC1");
                        
    		});
    
    		 client->dataChannel22 = dc;
    	});

        pc2->setLocalDescription();  // this will create offfer
        return client;
    }
};

#elif remotetesting


// Create and setup a PeerConnection
shared_ptr<Client> createPeerConnection_rm(const Configuration &config,  string id, Async &async, bool isClient)
{
    SInfo << "createPeerConnection" ;
    
    
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
        

       
          SInfo << "pc1 send sdp:"  << description.typeString() <<  " des "<<  std::string(description);

          
        //  test(async);
        
        auto work_fn = [pc, description]() {
            // This runs in a worker thread
                
            auto descAns = Description(std::string(description), Description::Type::Answer);
            descAns.mRole  = Description::Role::Active;
            SInfo << "remote desc Ansp:"  << descAns;
   
            pc->setRemoteDescription(descAns);
        };

        auto after_work_fn = [](int status) {
            // This runs back in the main event loop thread
            std::cout << "Main thread: Work finished with status " << status << std::endl;
        };

        async.queueWork(work_fn, after_work_fn);
        
    


    });

    pc->onLocalCandidate([ id, pc, &async](rtc::Candidate candidate) {


        SInfo << std::string(candidate);


        rtc::Candidate tmp = candidate;
        tmp.mService = std::to_string( Settings::RemotePort());
        tmp.mNode = Settings::RemoteIP();

        
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
        SInfo << "Gathering State: " << state ;
        if (state == PeerConnection::GatheringState::Complete)
        {
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

    client->video = addVideo(pc, 102, 1, "video-stream", "stream1", [id, wc = make_weak_ptr(client)]() {
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

    client->audio = addAudio(pc, 111, 2, "audio-stream", "stream1", [id, wc = make_weak_ptr(client)]() {
        
        
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

 std::string dcchat =   Settings::getdatachannel();
            
auto dc = pc->createDataChannel(dcchat);
    dc->onOpen([id, wdc = make_weak_ptr(dc)]() {
        if (auto dc = wdc.lock()) {
            SInfo << "onOpen: "  ;
            dc->send("Ping");
        }
    });

    dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg) {
        SInfo << "Message from " << id << " received: " << msg << endl;
        if (auto dc = wdc.lock()) {
            
            SInfo << "onOpen: " << msg  ;
            dc->send("Ping");
        }
    });
    client->dataChannel1 = dc;
    
    
    
pc->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
		SInfo << "DataChannel from " << id << " received with label \"" << dc->label() ;

                
                dc->onOpen([wdc = make_weak_ptr(dc)]() {
			if (auto dc = wdc.lock()) {
				SInfo << "DataChannel 2: Open" << endl;
				dc->send("Hello from 2");
			}
		});
                

		dc->onClosed([id]() { std::cout << "DataChannel from " << id << " closed" << std::endl; });

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
        
    if(isClient)    
    pc->setLocalDescription();
    return client;
};



#else


// Create and setup a PeerConnection
shared_ptr<Client> createPeerConnection(const Configuration &config,  string id, bool isClient)
{
    SInfo << "createPeerConnection" ;
    
    
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
        
        SInfo << "send:"  << description.typeString() <<  " des "<<  std::string(description);
          
     //  pc->setLocalDescription(Description::Type::Offer);// Description::Type::Answer);          
        sendSdp( std::string(description), description.typeString());
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
        sendCandidate( candidate.mid(), 1,  std::string(candidate)  );
//            if (auto ws = wws.lock())
//                    ws->send(message.dump());
    });

    pc->onGatheringStateChange(
        [](PeerConnection::GatheringState state) {
        SInfo << "Gathering State: " << state ;
        if (state == PeerConnection::GatheringState::Complete)
        {
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

    client->video = addVideo(pc, 102, 1, "video-stream", "stream1", [id, wc = make_weak_ptr(client)]() {
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

    client->audio = addAudio(pc, 111, 2, "audio-stream", "stream1", [id, wc = make_weak_ptr(client)]() {
        
        
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
     std::string dcchat =   Settings::getdatachannel();
    auto dc = pc->createDataChannel(dcchat);
    dc->onOpen([id, wdc = make_weak_ptr(dc)]() {
        if (auto dc = wdc.lock()) {
            SInfo << "onOpen: "  ;
            dc->send("Ping");
        }
    });

    dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg) {
        SInfo << "Message from " << id << " received: " << msg << endl;
        if (auto dc = wdc.lock()) {
            
            SInfo << "onOpen: " << msg  ;
            dc->send("Ping");
        }
    });
    client->dataChannel1 = dc;
    
    
    
    pc->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
		SInfo << "DataChannel from " << id << " received with label \"" << dc->label() ;

                
                dc->onOpen([wdc = make_weak_ptr(dc)]() {
			if (auto dc = wdc.lock()) {
				SInfo << "DataChannel 2: Open" << endl;
				dc->send("Hello from 2");
			}
		});
                

		dc->onClosed([id]() { std::cout << "DataChannel from " << id << " closed" << std::endl; });

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
        
    if(isClient)    
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
    stream->onSample([ws = make_weak_ptr(stream)](Stream::StreamSourceType type, uint64_t sampleTime, rtc::binary sample) {
        vector<ClientTrack> tracks{};
        string streamType = type == Stream::StreamSourceType::Video ? "video" : "audio";
        // get track for given type
        function<optional<shared_ptr<ClientTrackData>> (shared_ptr<Client>)> getTrackData = [type](shared_ptr<Client> client) {
            return type == Stream::StreamSourceType::Video ? client->video : client->audio;
        };
        // get all clients with Ready state
        for(auto id_client: clients) {
            auto id = id_client.first;
            auto client = id_client.second;
            auto optTrackData = getTrackData(client);
            if (client->getState() == Client::State::Ready && optTrackData.has_value()) {
                auto trackData = optTrackData.value();
                tracks.push_back(ClientTrack(id, trackData));
            }
        }
        if (!tracks.empty()) {
            for (auto clientTrack: tracks) {
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
                    cerr << "Unable to send "<< streamType << " packet: " << e.what() << endl;
                }
            }
        }
        MainThread.dispatch([ws]() {
            if (clients.empty()) {
                // we have no clients, stop the stream
                if (auto stream = ws.lock()) {
                    stream->stop();
                }
            }
        });
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
    auto h264 = dynamic_cast<H264FileParser *>(stream->video.get());
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
        assert(client->video.has_value() && client->audio.has_value());
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
