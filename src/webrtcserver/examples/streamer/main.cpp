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

using namespace rtc;
using namespace std;
using namespace std::chrono_literals;

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
shared_ptr<Client> createPeerConnection(const Configuration &config,
                                         
                                        string id);

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

Configuration config;

void sendCandidate( const std::string &mid, int mlineindex, const std::string &sdp )
{
    json desc;
    desc["sdpMid"] = mid;
    desc["sdpMLineIndex"] = mlineindex;
    desc["candidate"] = sdp;

    json m;
    m["type"] = "candidate";
    m["candidate"] = desc;
    m["from"] = from;
    m["to"] = from;

    SInfo << "send:"  <<  sdp << "candidate to: "<< from<< std::endl;
    
    //mysocket->emit("message", m);
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
    m["from"] =from;
    m["to"] = from;
    
    // smpl::Message m({ type, {

    SInfo << "send:"  << type << " to: "<< from<< std::endl;
    
  //  mysocket->emit("message", m);
                
}

/// Incomming message handler for websocket
/// @param message Incommint message
/// @param config Configuration
/// @param ws Websocket


void wsOnMessage(json const &m ) {
    
    
    std::string type;
    std::string room;
    std::string to;
    std::string user;
    std::string id ="server";

    if (m.find("to") != m.end()) { to = m["to"].get<std::string>(); }

    if (m.find("from") != m.end())
    {
        from = m["from"].get<std::string>();
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

    if (m.find("room") != m.end())
    {
        room = m["room"].get<std::string>();

    }
    else
    {
        SError << " On Peer message is missing room id ";
        return;
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
    
    
     
    if (m.find("type") != m.end())
    {
      type = m["type"].get<string>();

    }
     
    if (type == "request") {
         clients.emplace(id, createPeerConnection(config,  id));
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

int main(int argc, char **argv) try {
    bool enableDebugLogs = false;
    bool printHelp = false;
    //int c = 0;
    
    Logger::instance().add(new ConsoleChannel("info", Level::Trace));
    
     Application app;
    
    //auto parser = ArgParser({{"a", "audio"}, {"b", "video"}, {"d", "ip"}, {"p","port"}}, {{"h", "help"}, {"v", "verbose"}});
//    auto parsingResult = parser.parse(argc, argv, [](string key, string value) {
//        if (key == "audio") {
//            opusSamplesDirectory = value + "/";
//        } else if (key == "video") {
//            h264SamplesDirectory = value + "/";
//        } else if (key == "ip") {
//            ip_address = value;
//        } else if (key == "port") {
//            port = atoi(value.data());
//        } else {
//            cerr << "Invalid option --" << key << " with value " << value << endl;
//            return false;
//        }
//        return true;
//    }, [&enableDebugLogs, &printHelp](string flag){
//        if (flag == "verbose") {
//            enableDebugLogs = true;
//        } else if (flag == "help") {
//            printHelp = true;
//        } else {
//            cerr << "Invalid flag --" << flag << endl;
//            return false;
//        }
//        return true;
//    });
//    if (!parsingResult) {
//        return 1;
//    }

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
    if (enableDebugLogs) {
      //  InitLogger(LogLevel::Debug);
    }

   
    string stunServer = "stun:stun.l.google.com:19302";
    cout << "STUN server is " << stunServer << endl;
    config.iceServers.emplace_back(stunServer);
    config.disableAutoNegotiation = true;

    string localId = "server";
    cout << "The local ID is: " << localId << endl;
    
    
    std::string id ="server";

   clients.emplace(id, createPeerConnection(config,  id));
    

//    auto ws = make_shared<WebSocket>();
//
//    ws->onOpen([]() { cout << "WebSocket connected, signaling ready" << endl; });
//
//    ws->onClosed([]() { cout << "WebSocket closed" << endl; });
//
//    ws->onError([](const string &error) { cout << "WebSocket failed: " << error << endl; });
//
//    ws->onMessage([&](variant<binary, string> data) {
//        if (!holds_alternative<string>(data))
//            return;
//
//        json message = json::parse(get<string>(data));
//        MainThread.dispatch([message, config, ws]() {
//            wsOnMessage(message, config, ws);
//        });
//    });
//
//    const string url = "ws://" + ip_address + ":" + to_string(port) + "/" + localId;
//    cout << "URL is " << url << endl;
//    ws->open(url);
//
//    cout << "Waiting for signaling to be connected..." << endl;
//    while (!ws->isOpen()) {
//        if (ws->isClosed())
//            return 1;
//        this_thread::sleep_for(100ms);
//    }
    
    
    std::string room = "65f570720af337cec5335a70ee88cbfb7df32b5ee33ed0b4a896a0";
    std::string host = ip_address;
    int port = 443;
    
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
                            STrace << cnfg::stringify(data);
                            STrace << "Created room " << data[0] << "- my client ID is " << data[1];
                            //isInitiator = true;
                            // grabWebCamVideo();
                        }));

                mysocket->on(
                    "full",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &data, bool isAck, json &ack_resp)
                        {
                            STrace << cnfg::stringify(data);
                            // LTrace("Room " + room + " is full.")
                        }));


                mysocket->on(
                    "join",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &data, bool isAck, json &ack_resp)
                        {
                            STrace << cnfg::stringify(data);
                            // LTrace("Another peer made a request to join room " + room)
                            // LTrace("This peer is the initiator of room " + room + "!")
                            //isChannelReady = true;
                        }));

                /// for webrtc messages
                mysocket->on(
                    "message",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &m, bool isAck, json &ack_resp)
                        {
                            //LTrace(cnfg::stringify(m));
                            // LTrace('SocketioClient received message:', cnfg::stringify(m));

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

} catch (const std::exception &e) {
    SError << "Error: " << e.what() << std::endl;
    return -1;
}

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

// Create and setup a PeerConnection
shared_ptr<Client> createPeerConnection(const Configuration &config,  string id)
{
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
              //  clients.erase(id);
                
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

        sendCandidate( candidate.mid(), 1,  std::string(candidate)  );
//            if (auto ws = wws.lock())
//                    ws->send(message.dump());
    });

    pc->onGatheringStateChange(
        [](PeerConnection::GatheringState state) {
        SInfo << "Gathering State: " << state << endl;
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

    auto dc = pc->createDataChannel("ping-pong");
    dc->onOpen([id, wdc = make_weak_ptr(dc)]() {
        if (auto dc = wdc.lock()) {
            dc->send("Ping");
        }
    });

    dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg) {
        cout << "Message from " << id << " received: " << msg << endl;
        if (auto dc = wdc.lock()) {
            dc->send("Ping");
        }
    });
    client->dataChannel = dc;
    
    
    
//    pc->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
//		std::cout << "DataChannel from " << id << " received with label \"" << dc->label() << "\""
//		          << std::endl;
//
//		dc->onOpen([wdc = make_weak_ptr(dc)]() {
//			if (auto dc = wdc.lock())
//				dc->send("Hello from  arvind");
//		});
//
//		dc->onClosed([id]() { std::cout << "DataChannel from " << id << " closed" << std::endl; });
//
//		dc->onMessage([id](auto data) {
//			// data holds either std::string or rtc::binary
//			if (std::holds_alternative<std::string>(data))
//				std::cout << "Message from " << id << " received: " << std::get<std::string>(data)
//				          << std::endl;
//			else
//				std::cout << "Binary message from " << id
//				          << " received, size=" << std::get<rtc::binary>(data).size() << std::endl;
//		});
//
//		 client->dataChannel = dc;
//	});

    pc->setLocalDescription();
    return client;
};

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
