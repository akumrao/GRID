#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <random>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>

// Dynamic JSON parser 
#include <nlohmann/json.hpp>

// Custom HTTP/HTTPS Client Networking Layer
#include "http/HttpClient.h" 
#include "http/HttpsClient.h" 

// Core libwebrtc headers
#include "api/peer_connection_interface.h"
#include "api/create_peerconnection_factory.h"
#include "rtc_base/thread.h"
#include "rtc_base/ssl_adapter.h"
#include "system_wrappers/include/field_trial.h"

// Audio codec factory headers
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"

using json = nlohmann::json;

class CustomPeerConnectionObserver;
class CustomDataChannelObserver;
void DestroyHostPeerConnection(const std::string& clientId);
void OnViewerJoinedRoom(const std::string& clientId);

struct ViewerSession {
    std::string clientId;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection;
    rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel;
    std::unique_ptr<CustomPeerConnectionObserver> pcObserver;
    std::unique_ptr<CustomDataChannelObserver> dcObserver;
};

// Global states & networking context
std::unordered_map<std::string, std::unique_ptr<ViewerSession>> activeViewers;
rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> g_pcfactory;
std::string g_currentRoomTopic = ""; 
rtc::Thread* g_signalingThreadPtr = nullptr;
std::atomic<bool> g_keepRunning{true};

// Global Client Connection Instance
ClientConnecton* m_client = nullptr;

// Configuration definitions
const std::string SUPABASE_URL = "dabwulkpyquthvearbfw.supabase.co";
const std::string SUPABASE_ANON_KEY = "sb_publishable_1V4Zaw720lrIPx8IqmuxmA_ri32Bdqu";

// Standard fallback inline implementation for modern SetLocal/Remote description requirements
class ModernSetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver {
public:
    explicit ModernSetSessionDescriptionObserver(std::string context) : context_(std::move(context)) {}
    static rtc::scoped_refptr<ModernSetSessionDescriptionObserver> Create(const std::string& context = "SDP Observer") {
        return rtc::make_ref_counted<ModernSetSessionDescriptionObserver>(context);
    }
    void OnSuccess() override { 
        std::cout << "[SDP Observer Success] [" << context_ << "] Session description set successfully." << std::endl; 
    }
    void OnFailure(webrtc::RTCError error) override { 
        std::cerr << "[SDP Observer Failure] [" << context_ << "] Error setting description: " << error.message() << std::endl; 
    }
private:
    std::string context_;
};

// Helper room generator
std::string GenerateUniqueRoomCode() {
    const std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, characters.size() - 1);
    std::stringstream ss;
    for (int i = 0; i < 7; ++i) ss << characters[distribution(generator)];
    return ss.str();
}

// Enqueue arbitrary strings safely for outbound transmission via m_client
void EnqueueOutboundMessage(const std::string& msg) {
    if (m_client) {
        m_client->send(msg.c_str(), msg.size(), false);
    } else {
        std::cerr << "[WS Send Failure] Cannot send packet: Client connection instance unavailable!" << std::endl;
    }
}

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
                    OnViewerJoinedRoom(clientId);
                }
            }
            else if (subEvent == "signal-to-host") {
                std::string clientId = innerPayload.value("clientId", "");
                json signal = innerPayload["signal"];
                std::string type = signal.value("type", "");

                std::cout << "[Signaling] Inbound signal targeting client: " << clientId << " | Signal Type: " 
                          << (type.empty() ? "ICE Candidate" : type) << std::endl;

                auto it = activeViewers.find(clientId);
                if (it != activeViewers.end()) {
                    if (type == "answer") {
                        std::string sdpData = signal.value("sdp", signal.value("data", ""));
                        std::cout << "[WebRTC SDP Answer Received] Processing remote description for " << clientId 
                                  << ":\n--- SDP Answer Payload ---\n" << sdpData << "\n--------------------------" << std::endl;

                        webrtc::SdpParseError error;
                        std::unique_ptr<webrtc::SessionDescriptionInterface> sessionDesc = 
                            webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, sdpData, &error);

                        if (sessionDesc) {
                            it->second->peerConnection->SetRemoteDescription(
                                ModernSetSessionDescriptionObserver::Create("SetRemoteDescription Answer").get(), sessionDesc.release());
                        } else {
                            std::cerr << "[WebRTC SDP Parse Failure] Answer error line " << error.line 
                                      << ": " << error.description << std::endl;
                        }
                    } 
                    else if (signal.contains("candidate")) {
                        json cand = signal["candidate"];
                        std::string candidateStr = "";
                        std::string sdpMid = "0";
                        int sdpMLineIndex = 0;

                        if (cand.is_string()) {
                            candidateStr = cand.get<std::string>();
                        } else if (cand.is_object()) {
                            candidateStr = cand.value("candidate", "");
                            sdpMid = cand.value("sdpMid", "0");
                            sdpMLineIndex = cand.value("sdpMLineIndex", 0);
                        }

                        if (!candidateStr.empty()) {
                            std::cout << "[WebRTC Candidate Received] Adding Remote ICE Candidate for " << clientId 
                                      << " [sdpMid: " << sdpMid << ", mLineIndex: " << sdpMLineIndex 
                                      << "]: " << candidateStr << std::endl;

                            webrtc::SdpParseError error;
                            std::unique_ptr<webrtc::IceCandidateInterface> rtcCandidate(
                                webrtc::CreateIceCandidate(sdpMid, sdpMLineIndex, candidateStr, &error));
                            if (rtcCandidate) {
                                if (!it->second->peerConnection->AddIceCandidate(rtcCandidate.get())) {
                                    std::cerr << "[WebRTC ICE Failed] Failed to apply Remote ICE Candidate for " << clientId << std::endl;
                                } else {
                                    std::cout << "[WebRTC ICE Success] Remote Candidate added successfully for " << clientId << std::endl;
                                }
                            } else {
                                std::cerr << "[WebRTC ICE Error] Error parsing Candidate: " << error.description << std::endl;
                            }
                        }
                    }
                } else {
                    std::cerr << "[Signaling Warn] Received signal for unknown/untracked peer: " << clientId << std::endl;
                }
            }
        }
        else if (event == "presence") {
            if (topPayload.contains("leaves")) {
                for (auto& leaf : topPayload["leaves"]) {
                    if (leaf.contains("clientId")) {
                        std::string abandonedClientId = leaf.value("clientId", "");
                        std::cout << "[Presence Leave] Client departed room: " << abandonedClientId << std::endl;
                        DestroyHostPeerConnection(abandonedClientId);
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[JSON Parse Failure] Error parsing packet: " << e.what() << std::endl;
    }
}

// Outbound signaling encoder targeting Supabase Realtime mesh
void SendSignalToSupabase(const std::string& targetId, const std::string& eventType, const std::string& sdpOrCandidate) {
    json root;
    root["topic"] = g_currentRoomTopic;
    root["event"] = "broadcast";
    root["ref"] = "2";

    json payload;
    payload["type"] = "broadcast";
    payload["event"] = "signal-to-viewer";
    
    json innerPayload;
    innerPayload["targetId"] = targetId;

    if (eventType == "offer") {
        innerPayload["signal"]["type"] = "offer";
        innerPayload["signal"]["sdp"] = sdpOrCandidate;
        std::cout << "[WS Signal Outbound] Enqueueing SDP Offer broadcast targeting: " << targetId << std::endl;
    } else if (eventType == "candidate") {
        innerPayload["signal"]["candidate"]["candidate"] = sdpOrCandidate;
        innerPayload["signal"]["candidate"]["sdpMid"] = "0";
        innerPayload["signal"]["candidate"]["sdpMLineIndex"] = 0;
        std::cout << "[WS Signal Outbound] Enqueueing ICE Candidate targeting: " << targetId << std::endl;
    }

    payload["payload"] = innerPayload;
    root["payload"] = payload;

    std::string dumpedMsg = root.dump();
    std::cout << "[WS Queue Add] Output payload: " << dumpedMsg << std::endl;
    EnqueueOutboundMessage(dumpedMsg);
}

// Initialize HttpsClient configuration and connection handlers
void InitializeSupabaseRealtime(const std::string& roomCode) {
    g_currentRoomTopic = "realtime:room-" + roomCode;
    std::cout << "[Init Realtime] Binding topic channel: " << g_currentRoomTopic << std::endl;

    const std::string host = SUPABASE_URL;
    const int port = 443;
    const std::string target = "/realtime/v1/websocket?apikey=" + SUPABASE_ANON_KEY + "&vsn=1.0.0";

    m_client = new HttpsClient("wss", host, port, target);
    m_client->setHostName(host);
    m_client->_request.set("Host", host);
    m_client->_request.set("Origin", "http://localhost");
    m_client->_request.setKeepAlive(true);

    m_client->fnConnect = [](HttpBase* con) {
        std::cout << "[WebSocket State] Connected to Supabase Gateway! Requesting Phoenix subscription..." << std::endl;

        json joinMsg;
        joinMsg["topic"] = g_currentRoomTopic;
        joinMsg["event"] = "phx_join";
        joinMsg["payload"] = json::object();
        joinMsg["ref"] = "1";

        std::string joinPayload = joinMsg.dump();
        std::cout << "[WebSocket Tx] Subscription Handshake: " << joinPayload << std::endl;
        con->send(joinPayload.c_str(), joinPayload.size(), false);
    };

    m_client->fnPayload = [](HttpBase* con, const char* data, size_t sz) {
        if (!data || sz == 0) return;
        std::string incomingData(data, sz);
        if (g_signalingThreadPtr) {
            g_signalingThreadPtr->PostTask([incomingData]() {
                OnInboundSupabaseMessage(incomingData);
            });
        }
    };

    m_client->fnClose = [](HttpBase* con, std::string str) {
        std::cout << "[WebSocket State] Connection channel closed cleanly: " << str << std::endl;
    };

    m_client->setReadStream(new std::stringstream);
    m_client->send();
}

// ==========================================
// DATA CHANNEL LIFECYCLE OBSERVER
// ==========================================
class CustomDataChannelObserver : public webrtc::DataChannelObserver {
public:
    explicit CustomDataChannelObserver(std::string clientId) : clientId_(std::move(clientId)) {}

    void OnStateChange() override {
        auto it = activeViewers.find(clientId_);
        if (it != activeViewers.end()) {
            auto state = it->second->dataChannel->state();
            std::cout << "[DataChannel State] Peer: " << clientId_ << " | State: " << static_cast<int>(state) << std::endl;

            if (state == webrtc::DataChannelInterface::kOpen) {
                std::cout << "[DataChannel Opened] SCTP Pipeline open with viewer: " << clientId_ << std::endl;
                json welcome;
                welcome["text"] = "Connected to C++ Backend Host inside room code!";
                std::string welcomeStr = welcome.dump();
                
                std::cout << "[DataChannel Send] Sending welcome payload: " << welcomeStr << std::endl;
                webrtc::DataBuffer buffer(welcomeStr);
                it->second->dataChannel->Send(buffer);
            }
        }
    }

    void OnMessage(const webrtc::DataBuffer& buffer) override {
        std::string message(reinterpret_cast<const char*>(buffer.data.data()), buffer.data.size());
        
        try {
            json data = json::parse(message);
            // Ignore custom keep-alive responses silently to limit log overflow
            if (data.value("type", "") == "pong") {
                return; 
            }
            std::cout << "[DataChannel Message] From Viewer [" << clientId_ << "]: " << data.value("text", "") << std::endl;
        } catch(...) {
            std::cout << "[DataChannel Unparsed Payload] From Viewer [" << clientId_ << "]: " << message << std::endl;
        }
    }

    void OnBufferedAmountChange(uint64_t previous_amount) override {
        std::cout << "[DataChannel Buffer Change] Client: " << clientId_ << " Previous Buffer: " << previous_amount << " bytes." << std::endl;
    }
private:
    std::string clientId_;
};

// ==========================================
// PEER CONNECTION LIFECYCLE OBSERVER
// ==========================================
class CustomPeerConnectionObserver : public webrtc::PeerConnectionObserver {
public:
    explicit CustomPeerConnectionObserver(std::string clientId) : clientId_(std::move(clientId)) {}

    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {
        std::string candidateSdp;
        if (candidate->ToString(&candidateSdp)) {
            std::cout << "[ICE Candidate Local] Discovered Local Candidate for " << clientId_ 
                      << " [sdpMid: " << candidate->sdp_mid() 
                      << ", mLineIndex: " << candidate->sdp_mline_index() 
                      << "]: " << candidateSdp << std::endl;

            SendSignalToSupabase(clientId_, "candidate", candidateSdp);
        } else {
            std::cerr << "[ICE Candidate Error] Failed to serialize local ICE candidate string for " << clientId_ << std::endl;
        }
    }

    void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {
        std::string stateName = "Unknown";
        switch (new_state) {
            case webrtc::PeerConnectionInterface::kIceGatheringNew: stateName = "New"; break;
            case webrtc::PeerConnectionInterface::kIceGatheringGathering: stateName = "Gathering"; break;
            case webrtc::PeerConnectionInterface::kIceGatheringComplete: stateName = "Complete"; break;
        }
        std::cout << "[ICE Gathering State] Client " << clientId_ << " shifted to: " << stateName << std::endl;
    }

    void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override {
        std::cout << "[PeerConnection State] Viewer " << clientId_ << " state changed to enum index: " 
                  << static_cast<int>(new_state) << std::endl;

        if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kConnected) {
            std::cout << "[PeerConnection Success] WebRTC P2P direct link established with viewer: " << clientId_ << std::endl;
        } else if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kFailed) {
            std::cerr << "[PeerConnection Failed] Handshake failed for viewer: " << clientId_ << ". Cleaning up resources..." << std::endl;
            DestroyHostPeerConnection(clientId_);
        }
    }

    void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {
        std::cout << "[PeerConnection Signaling] Client " << clientId_ << " state: " << static_cast<int>(new_state) << std::endl;
    }

    void OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) override {
        std::cout << "[ICE Candidate Event] Candidates removed for client: " << clientId_ << std::endl;
    }

    void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override {
        std::cout << "[DataChannel Remote Discovery] Inbound data channel opened by remote peer: " << data_channel->label() << std::endl;
    }

    void OnRenegotiationNeeded() override {
        std::cout << "[PeerConnection Event] Renegotiation requested for client: " << clientId_ << std::endl;
    }
private:
    std::string clientId_;
};

// ==========================================
// SDP OFFER CREATION CALLBACK
// ==========================================
class DummyCreateSessionDescriptionObserver : public webrtc::CreateSessionDescriptionObserver {
public:
    DummyCreateSessionDescriptionObserver(std::string clientId, webrtc::PeerConnectionInterface* pc)
        : clientId_(std::move(clientId)), pc_(pc) {}

    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {
        std::cout << "[SDP Offer Success] Local offer created for viewer: " << clientId_ << std::endl;
        
        std::string sdp;
        if (desc->ToString(&sdp)) {
            std::cout << "[WebRTC SDP Offer Built] Local Description Details for " << clientId_ 
                      << ":\n--- Local SDP Offer Payload ---\n" << sdp << "\n-------------------------------" << std::endl;

            pc_->SetLocalDescription(ModernSetSessionDescriptionObserver::Create("SetLocalDescription Offer").get(), desc);
            SendSignalToSupabase(clientId_, "offer", sdp);
        } else {
            std::cerr << "[SDP Offer Error] Failed to serialize SessionDescription object into string!" << std::endl;
        }
    }

    void OnFailure(webrtc::RTCError error) override { 
        std::cerr << "[SDP Offer Failure] Peer: " << clientId_ << " Error: " << error.message() << std::endl; 
    }
private:
    std::string clientId_;
    webrtc::PeerConnectionInterface* pc_;
};

void OnViewerJoinedRoom(const std::string& clientId) {
    if (activeViewers.find(clientId) != activeViewers.end()) {
        std::cout << "[Peer Engine] Viewer " << clientId << " already exists. Skipping duplicate creation." << std::endl;
        return;
    }

    std::cout << "[Peer Engine] Constructing new RTCPeerConnection object for viewer: " << clientId << std::endl;

    webrtc::PeerConnectionInterface::RTCConfiguration config;
    webrtc::PeerConnectionInterface::IceServer stun_server;
    stun_server.urls.push_back("stun:stun.l.google.com:19302");
    
    config.tcp_candidate_policy = webrtc::PeerConnectionInterface::kTcpCandidatePolicyEnabled;
    config.servers.push_back(stun_server);

    std::cout << "[Peer Engine] Configured STUN Endpoint: stun:stun.l.google.com:19302" << std::endl;

    auto session = std::make_unique<ViewerSession>();
    session->clientId = clientId;
    session->pcObserver = std::make_unique<CustomPeerConnectionObserver>(clientId);

    auto result = g_pcfactory->CreatePeerConnectionOrError(config, webrtc::PeerConnectionDependencies(session->pcObserver.get()));
    if (!result.ok()) {
        std::cerr << "[Peer Engine Error] Failed to create PeerConnection: " << result.error().message() << std::endl;
        return;
    }
    session->peerConnection = result.MoveValue();
    std::cout << "[Peer Engine Success] RTCPeerConnection instantiated for: " << clientId << std::endl;

    webrtc::DataChannelInit dcConfig;
    dcConfig.ordered = true;
    
    std::cout << "[DataChannel Init] Creating channel 'bidirectional-chat' for client: " << clientId << std::endl;
    auto dcResult = session->peerConnection->CreateDataChannelOrError("bidirectional-chat", &dcConfig);
    if (!dcResult.ok()) {
        std::cerr << "[DataChannel Error] Failed to initialize RTCDataChannel: " << dcResult.error().message() << std::endl;
        return;
    }
    
    session->dataChannel = dcResult.MoveValue();
    session->dcObserver = std::make_unique<CustomDataChannelObserver>(clientId);
    session->dataChannel->RegisterObserver(session->dcObserver.get());

    auto* pcPtr = session->peerConnection.get();
    activeViewers[clientId] = std::move(session);

    std::cout << "[SDP Offer Engine] Dispatching CreateOffer for client: " << clientId << std::endl;
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
    auto observer = rtc::make_ref_counted<DummyCreateSessionDescriptionObserver>(clientId, pcPtr);
    pcPtr->CreateOffer(observer.get(), options);
}

void DestroyHostPeerConnection(const std::string& clientId) {
    auto it = activeViewers.find(clientId);
    if (it != activeViewers.end()) {
        std::cout << "[Cleanup Engine] Destroying WebRTC resources for viewer: " << clientId << std::endl;
        it->second->dataChannel->UnregisterObserver();
        it->second->peerConnection->Close();
        activeViewers.erase(it);
        std::cout << "[Cleanup Engine] Complete. Active Sessions Remaining: " << activeViewers.size() << std::endl;
    }
}

int main() {
    std::cout << "[Host Process] Initializing WebRTC subsystem and SSL adapters..." << std::endl;
    rtc::InitializeSSL();
    webrtc::field_trial::InitFieldTrialsFromString("");

    rtc::ThreadManager::Instance()->WrapCurrentThread();
    rtc::Thread* main_thread = rtc::Thread::Current();

    std::unique_ptr<rtc::Thread> network_thread = rtc::Thread::CreateWithSocketServer();
    std::unique_ptr<rtc::Thread> worker_thread = rtc::Thread::Create();
    std::unique_ptr<rtc::Thread> signaling_thread = rtc::Thread::Create();

    network_thread->Start();
    worker_thread->Start();
    signaling_thread->Start();

    std::cout << "[Host Threads] Network, Worker, and Signaling threads running." << std::endl;

    g_signalingThreadPtr = signaling_thread.get();

    webrtc::PeerConnectionFactoryDependencies dependencies;
    dependencies.network_thread = network_thread.get();
    dependencies.worker_thread = worker_thread.get();
    dependencies.signaling_thread = signaling_thread.get();
    dependencies.audio_encoder_factory = webrtc::CreateBuiltinAudioEncoderFactory();
    dependencies.audio_decoder_factory = webrtc::CreateBuiltinAudioDecoderFactory();
    dependencies.video_encoder_factory = nullptr;
    dependencies.video_decoder_factory = nullptr;

    g_pcfactory = webrtc::CreateModularPeerConnectionFactory(std::move(dependencies));
    if (!g_pcfactory) {
        std::cerr << "[Fatal Error] Failed to initialize PeerConnectionFactory!" << std::endl;
        return -1;
    }
    std::cout << "[Host Engine] PeerConnectionFactory created successfully." << std::endl;

    std::string roomCode = GenerateUniqueRoomCode();
    InitializeSupabaseRealtime(roomCode);
    std::cout << "[Room Initialized] Unique channel token -> " << g_currentRoomTopic << std::endl;

    // Background Keep-Alive / Heartbeat loop thread
    std::thread heartbeatThread([]() {
        int hbSeq = 0;
        while (g_keepRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(25));
            
            // Maintain Phoenix Signaling Link directly with the Server
            json hb;
            hb["topic"] = "phoenix";
            hb["event"] = "phx_heartbeat";
            hb["payload"] = json::object();
            hb["ref"] = "hb_" + std::to_string(hbSeq++);

            std::string hbStr = hb.dump();
            
            if (g_signalingThreadPtr) {
                g_signalingThreadPtr->PostTask([hbStr]() {
                    EnqueueOutboundMessage(hbStr);
                });
            }

            // Print the current number of active viewer connections safely
            if (g_signalingThreadPtr) {
                g_signalingThreadPtr->PostTask([]() {
                    std::cout << "[Host Status] Currently connected viewers: " << activeViewers.size() << std::endl;
                });
            }
        }
    });

    std::cout << "[Host Loop] Entering main message processing loop..." << std::endl;
    while (g_keepRunning) {
        main_thread->ProcessMessages(5);
    }

    g_keepRunning = false;
    if (heartbeatThread.joinable()) {
        heartbeatThread.join();
    }

    if (m_client) {
        m_client->Close();
        delete m_client;
        m_client = nullptr;
    }
    
    rtc::ThreadManager::Instance()->UnwrapCurrentThread();
    rtc::CleanupSSL();
    return 0;
}