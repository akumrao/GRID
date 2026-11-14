
#include "DepUsrSCTP.h"
#include "DtlsTransport.h"
#include "RestAPI.h"
#include "base/logger.h"
#include "helpers.hpp"
#include "sctptransport.hpp"
#include "uv.h"
#include "json/confSettings.h"
#include "json/configuration.h"

#include "h264fileparser.hpp"
#include "opusfileparser.hpp"
#include "h264rtppacketizer.hpp"
#include "rtppacketizationconfig.hpp"
#include "rtcpnackresponder.hpp"
#include "DepLibSRTP.h"
#include "sdpcommon.h"

#include "mediahandler.hpp"

#include "peerconnection.h"

#include "rembhandler.hpp"
#include "rtp.hpp"










#define CERTFROMFILE 1
#define VIDEOMEDIA 1
using namespace rtc;
using namespace std;
using namespace base;
using namespace base::net;
using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) {
    return ptr;
}
unordered_map<string, shared_ptr<Client>> clients
{
};
std::mutex clients_mutex;

shared_ptr<Client> createPeerConnection_lc1(Configuration &config, string id,
        shared_ptr<PeerConnection> pc1,
        shared_ptr<PeerConnection> pc2);
shared_ptr<Client> createPeerConnection_lc2(Configuration &config, string id,
        shared_ptr<PeerConnection> pc2,
        shared_ptr<PeerConnection> pc1);
void addToStream(shared_ptr<Client> client, bool isAddingVideo);
Configuration settingconfig;
//std::string id;

int main(int argc, char **argv) {
    base::cnfg::Configuration cache;
    cache.load("./cache.js");
    ConfSettings::SetConfiguration(cache.root);
    Application app;
    Async async;
    string stunServer = "stun:stun.l.google.com:19302";
    cout << "STUN server is " << stunServer << endl;
    
    
    settingconfig.iceServers.emplace_back(stunServer);

   // settingconfig.iceServers.emplace_back("turn:testuser:testpassword@192.168.0.20:3478?transport=udp");
    
    if(!ConfSettings::configuration.turn.empty())
    settingconfig.iceServers.emplace_back(ConfSettings::configuration.turn);
    

    settingconfig.disableAutoNegotiation = true;
    settingconfig.staticPort = ConfSettings::configuration.staticPort;
    settingconfig.gconfig->serverdtsRole =
            ConfSettings::configuration.serverdtsRole;
    settingconfig.gconfig->serverdtsRole =
            ConfSettings::configuration.serverdtsRole;
    settingconfig.enableTcp =  ConfSettings::configuration.enableTcp;;
    settingconfig.enableUdp =  ConfSettings::configuration.enableUdp;;
    settingconfig.publicIP = ConfSettings::configuration.publicIP;
    settingconfig.noPivateIP = ConfSettings::configuration.noPivateIP;
    bool relayOnly = ConfSettings::configuration.relayOnly;
    if (relayOnly)
        settingconfig.iceTransportPolicy = TransportPolicy::Relay;


#if CERTFROMFILE == 1
    settingconfig.gconfig->keyPemFile = ConfSettings::configuration.keyFile;
    settingconfig.gconfig->certificatePemFile =
            ConfSettings::configuration.certFile;
    settingconfig.gconfig->keyPemPass = "12345678";
#elif CERTFROMFILE == 2
    settingconfig.keyPemFile = "";
    settingconfig.certificatePemFile = "";
#else
#endif
    string localId = "server";
    cout << "The local ID is: " << localId << endl;
    rtc::DtlsTransport::ClassInit();
    DepUsrSCTP::ClassInit();
    
    GetNetInterface::ClassInit();
        
    DepLibSRTP::ClassInit();

    std::string id1 = "server1";
    std::string id2 = "server2";
    auto pc1 = make_shared<PeerConnection>(settingconfig);
    settingconfig.staticPort = settingconfig.staticPort + 1;
    auto pc2 = make_shared<PeerConnection>(settingconfig);

    auto client1 = createPeerConnection_lc1(settingconfig, id1, pc1, pc2);
    {
        std::lock_guard<std::mutex> lg(clients_mutex);
        clients.emplace(id1, client1);

    }
    auto client2 = createPeerConnection_lc2(settingconfig, id2, pc2, pc1);
    {
        std::lock_guard<std::mutex> lg(clients_mutex);
        clients.emplace(id2, client2);
    }

    
    

        

    app.waitForShutdown([&](void *) {

        {

            for (auto& pair : clients) {
                SInfo << "Closing and cleaning up active clients and peer connections " << pair.first;
                if (pair.second && pair.second->peerConnection) {
                    pair.second->peerConnection->close();
                }
            }

        }

        {

            for (auto &pair : clients) {
                SInfo << "Closing and cleaning up active clients and peer "
                        "connections "
                        << pair.first;
                if (pair.second && pair.second->peerConnection) {
                    uv_sleep(100);

                }
            }

        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.clear();
        }

        SInfo << "app.run() is over";

        DepUsrSCTP::ClassDestroy();
        Logger::destroy();
    });
    std::cout << "Exiting main..." << endl;
    return 0;
}

std::shared_ptr<rtc::Track> rtrack;


std::function<void(rtc::binary)> rtpCallback;
std::function<void()> startRenegotiation;
std::function<void()> startMediaTest;
std::function<void()> startClosePeers;

	


shared_ptr<Client> createPeerConnection_lc1(Configuration &config, string id,
        shared_ptr<PeerConnection> pc1,
        shared_ptr<PeerConnection> pc2) {
    {
        auto client = make_shared<Client>(pc1);
        pc1->onStateChange([id](PeerConnection::State state) {
            SInfo << "pc1 State: " << state << endl;
            if (state == PeerConnection::State::Disconnected ||
                    state == PeerConnection::State::Failed ||
                    state == PeerConnection::State::Closed) {

                SInfo << "createPeerConnection_lc1 close " << id;

            }
        });
        pc1->onLocalDescription([id, pc1, pc2](rtc::Description & description) {
            SInfo << "pc1 send sdp:" << description.typeString() << " des "
                    << std::string(description);
            auto description1 =
                    Description(std::string(description), Description::Type::Offer);
            pc2->setRemoteDescription(description1);
        });
        pc1->onLocalCandidate([id, pc2](rtc::Candidate & candidate) {
            SInfo << "pc1 send candidated:" << candidate.mid() << " des "
                    << std::string(candidate);
            pc2->addRemoteCandidate(candidate);
        });


        pc1->onIceStateChange([pc1](PeerConnection::IceState state) {
            SInfo << "ICE state 1: " << state << endl;

            if (state == PeerConnection::IceState::Completed) {

//                Candidate local; Candidate remote;
//
//                pc1->getSelectedCandidatePair(&local, &remote);
//
//                SInfo << "getSelectedCandidatePair1 local " << local << " remote " << remote;
//                std::string msg = "PC1 Hello world";
//                pc1->send((unsigned char *) msg.data(), msg.size() + 1);


            }
        }

        );


        pc1->onGatheringStateChange([pc1](PeerConnection::GatheringState state) {
            SInfo << "Gathering State" << PeerConnection::printState(state);
            if (state == PeerConnection::GatheringState::Complete) {
                SInfo << "pc1 Gathering State: Complete";

            } else if (state == PeerConnection::GatheringState::InProgress) {
                SInfo << "pc1 Gathering State: InProgress";
            }
        });
#if VIDEOMEDIA
        
        std::string newTrackMid = "test";
        Description::Video media(newTrackMid, Description::Direction::SendRecv);
	media.addH264Codec(96);
	media.setBitrate(3000);
	media.addSSRC(1234, "video-send");

	const auto mediaSdp1 = string(media);
	const auto mediaSdp2 = string(Description::Media(mediaSdp1));
	if (mediaSdp2 != mediaSdp1) {
		SInfo << mediaSdp2 << endl;
		//return TestResult(false, "Media description parsing test failed");
	}

	rtrack = pc1->addTrack(media);
        

        rtrack->onOpen([]() {
           //>track->requestKeyframe(); // So the receiver can start playing immediately
            
             SInfo << "onOpen";
             
             // RTP test
	std::vector<std::byte> payload = {std::byte{0}, std::byte{1}, std::byte{2}, std::byte{3}};
	std::vector<std::byte> rtpRaw(sizeof(RtpHeader) + payload.size());
	auto *rtp = reinterpret_cast<RtpHeader *>(rtpRaw.data());
	rtp->preparePacket();
	rtp->setPayloadType(96);
	rtp->setSeqNumber(1);
	rtp->setTimestamp(3000);
	rtp->setSsrc(2468);
	std::memcpy(rtpRaw.data() + sizeof(RtpHeader), payload.data(), payload.size());

	if (!rtrack->send(rtpRaw.data(), rtpRaw.size())) {
		throw runtime_error("Couldn't send RTP packet");
	}


        if (!rtrack->send(rtpRaw.data(), rtpRaw.size())) {
            throw runtime_error("Couldn't send RTP packet");
        }

	// wait for an RTP packet to be received
//	auto future = recvRtpPromise.get_future();
//	if (future.wait_for(2s) == std::future_status::timeout) {
//		throw runtime_error("Didn't receive RTP packet on pc2");
//	}
//
//	auto receivedRtpRaw = future.get();
//	if (receivedRtpRaw.empty()) {
//		throw runtime_error("Didn't receive RTP packet on pc2");
//	}
//
//	if (receivedRtpRaw.size() != rtpRaw.size() ||
//	    memcmp(receivedRtpRaw.data(), rtpRaw.data(), rtpRaw.size()) != 0) {
//		throw runtime_error("Received RTP packet is different than the packet that was sent");
//	}

	// RTCP REMB test
//	std::promise<unsigned int> rembPromise;
//	rtrack->setMediaHandler(make_shared<RembHandler>([&rembPromise](unsigned int bitrate) {
//		rembPromise.set_value(bitrate);
//	}));
//
//	std::vector<std::byte> rtcpRembRaw(RtcpRemb::SizeWithSSRCs(2));
//	auto *rtcpRemb = reinterpret_cast<RtcpRemb*>(rtcpRembRaw.data());
//	rtcpRemb->preparePacket(6666, 2, 1000000);
//	rtcpRemb->setSSRC(0, 2468);
//	rtcpRemb->setSSRC(1, 2469);
//	if (!rtrack->send(rtcpRembRaw.data(), rtcpRembRaw.size())) {
//		throw runtime_error("Couldn't send RTCP REMB message");
//	}

//	auto rembFuture = rembPromise.get_future();
//	if (rembFuture.wait_for(2s) == std::future_status::timeout) {
//		throw runtime_error("Didn't receive REMB message");
//	}
//
//	unsigned int bitrate = rembFuture.get();
//	if (bitrate != 1000000) {
//		throw runtime_error("Incorrect bitrate in REMB message");
//	}
             
             
             
             
             
             
             
             
             
             
             
             
             
             
             
             
             
        });
        
        
       rtrack->onMessage([](rtc::binary var) {
           
            SInfo << "onMessage";
        },  nullptr
                
                );

        
        //

//	pc1.setLocalDescription();


#endif
//        auto dc = pc1->createDataChannel("ping-pong-pc1");
//        dc->onOpen([id, wdc = make_weak_ptr(dc)](){
//            if (auto dc = wdc.lock()) {
//                SInfo << "ping-pong-pc1 onOpen";
//                        dc->send("ping-pong-pc1 send on open Ping");
//            }
//        });
//        dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg){
//            SInfo << "Pc1 Message from " << id << " received: " << msg << endl;
//            if (auto dc = wdc.lock()) {
//            }
//        });
//
//        client->dataChannel1 = dc;
//        pc1->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
//            SInfo << "pc1 onDataChannel from " << id << " received with label \""
//                    << dc->label() << "\"" << std::endl;
//            dc->onOpen([wdc = make_weak_ptr(dc)](){
//                if (auto dc = wdc.lock()) {
//                    SInfo << "pc1 open ";
//                            dc->send("Hello from pc1");
//                }
//            });
//            dc->onClosed([id]() {
//                SInfo << "pc1 DataChannel from " << id << " closed" << std::endl;
//            });
//            dc->onMessage([id, dc](auto data) {
//                if (std::holds_alternative<std::string>(data))
//                    SInfo << "pc1 Message from " << id
//                        << " received: " << std::get<std::string>(data) << std::endl;
//                else
//                    SInfo << "pc1 Binary message from " << id
//                        << " received, size=" << std::get<rtc::binary>(data).size()
//                    << std::endl;
//            });
//            client->dataChannel11 = dc;
//        });
        pc1->setLocalDescription();
        return client;
    }
}





shared_ptr<Client> createPeerConnection_lc2(Configuration &config, string id,
        shared_ptr<PeerConnection> pc2,
        shared_ptr<PeerConnection> pc1) {
    {
        auto client = make_shared<Client>(pc2);
        pc2->onStateChange([id](PeerConnection::State state) {
            SInfo << "pc2 State: " << state << endl;
            if (state == PeerConnection::State::Disconnected ||
                    state == PeerConnection::State::Failed ||
                    state == PeerConnection::State::Closed) {

                SInfo << "createPeerConnection_lc2 close " << id;


            }
        });
        pc2->onLocalDescription([id, pc1](rtc::Description description) {
            SInfo << "pc2 send sdp:" << description.typeString() << " des "
                    << std::string(description);
            pc1->setRemoteDescription(description);
        });
        pc2->onLocalCandidate([id, pc1](rtc::Candidate candidate) {
            SInfo << "pc2 send candidated:" << candidate.mid() << " des "
                    << std::string(candidate);
            pc1->addRemoteCandidate(candidate);
        });



        pc2->onIceStateChange([pc2](PeerConnection::IceState state) {
            SInfo << "ICE state 2: " << state << endl;

            if (state == PeerConnection::IceState::Completed) {

//                Candidate local; Candidate remote;
//
//                pc2->getSelectedCandidatePair(&local, &remote);
//
//                SInfo << "getSelectedCandidatePair1 local " << local << " remote " << remote;
//                std::string msg = "PC2 Hello world";
//                pc2->send((unsigned char *) msg.data(), msg.size() + 1);

            }
        }

        );



        pc2->onGatheringStateChange([pc2](PeerConnection::GatheringState state) {
            if (state == PeerConnection::GatheringState::Complete) {
                SInfo << "Pc2 Gathering State: Complete";
                {


                }
            }
        });
#if VIDEOMEDIA
        


        
        
        auto checkTracksReady = [&]() {
		int nextAction = 2;
//		{
//			std::lock_guard<std::mutex> lock(mtx);
//			if (phase == 1 && t1 && t1->isOpen() && t2 && t2->isOpen()) {
//				phase = 2;
//				nextAction = 1;
//			} else if (phase == 2 && t1 && t1->isOpen() && t2 && t2->isOpen()) {
//				phase = 3;
//				nextAction = 2;
//			}
//		}

		if (nextAction == 1) {
			startRenegotiation();
		} else if (nextAction == 2) {
			startMediaTest();
		}
	};

#if 0
	std::vector<std::byte> sentRtpRaw;

	startMediaTest = [&]() {
		std::vector<std::byte> payload = {std::byte{0}, std::byte{1}, std::byte{2}, std::byte{3}};
		sentRtpRaw.resize(sizeof(RtpHeader) + payload.size());
		auto *rtp = reinterpret_cast<RtpHeader *>(sentRtpRaw.data());
		rtp->preparePacket();
		rtp->setPayloadType(96);
		rtp->setSeqNumber(1);
		rtp->setTimestamp(3000);
		rtp->setSsrc(2468);
		std::memcpy(sentRtpRaw.data() + sizeof(RtpHeader), payload.data(), payload.size());

		// Callback for RTP packet arrival on pc2
		rtpCallback = [&](rtc::binary message) {
			if (message.empty() || message.size() != sentRtpRaw.size() ||
			    memcmp(message.data(), sentRtpRaw.data(), sentRtpRaw.size()) != 0) {
				SError << "Received RTP packet is different than the packet that was sent";
				return;
			}

			// Chained RTCP REMB callback on t1
			t1->setMediaHandler(make_shared<RembHandler>([&](unsigned int bitrate) {
				if (bitrate != 1000000) {
					SError << "Incorrect bitrate in REMB message";
					return;
				}
				startClosePeers();
			}));

			std::vector<std::byte> rtcpRembRaw(RtcpRemb::SizeWithSSRCs(2));
			auto *rtcpRemb = reinterpret_cast<RtcpRemb *>(rtcpRembRaw.data());
			rtcpRemb->preparePacket(6666, 2, 1000000);
			rtcpRemb->setSSRC(0, 2468);
			rtcpRemb->setSSRC(1, 2469);

			if (!t2->send(rtcpRembRaw.data(), rtcpRembRaw.size())) {
				failTest("Couldn't send RTCP REMB message");
			}
		};

		if (!t1->send(sentRtpRaw.data(), sentRtpRaw.size())) {
			failTest("Couldn't send RTP packet");
		}
	};
#endif

        pc2->onTrack([&](shared_ptr<Track> t) {
            string mid = t->mid();
            SInfo << "Track 2: Received track with mid \"" << mid << "\"" ;


            t->onOpen([&, mid, t]() {
                SInfo << "Track 2: Track with mid \"" << mid << "\" is open" ;
               // checkTracksReady();
                
                         // RTP test
                std::vector<std::byte> payload = {std::byte
                    {0}, std::byte
                    {1}, std::byte
                    {2}, std::byte
                    {3}};
                std::vector<std::byte> rtpRaw(sizeof (RtpHeader) + payload.size());
                auto *rtp = reinterpret_cast<RtpHeader *> (rtpRaw.data());
                rtp->preparePacket();
                rtp->setPayloadType(96);
                rtp->setSeqNumber(1);
                rtp->setTimestamp(3000);
                rtp->setSsrc(2468);
                std::memcpy(rtpRaw.data() + sizeof (RtpHeader), payload.data(), payload.size());

                if (!t->send(rtpRaw.data(), rtpRaw.size())) {
                    throw runtime_error("Couldn't send RTP packet");
                }


                if (!t->send(rtpRaw.data(), rtpRaw.size())) {
                    throw runtime_error("Couldn't send RTP packet");
                }

                
                
                
                
                
                
                
                
                
            });

            t->onClosed([mid]() {
                SInfo << "Track 2: Track with mid \"" << mid << "\" is closed" ;
            });

            t->onMessage([&, t](rtc::binary message) {
               // if (rtpCallback) {
                //    rtpCallback(std::move(message));
                    
                std::vector<std::byte> sentRtpRaw;
                std::vector<std::byte> payload = {std::byte{0}, std::byte{1}, std::byte{2}, std::byte{3}};
		sentRtpRaw.resize(sizeof(RtpHeader) + payload.size());
		auto *rtp = reinterpret_cast<RtpHeader *>(sentRtpRaw.data());
		rtp->preparePacket();
		rtp->setPayloadType(96);
		rtp->setSeqNumber(1);
		rtp->setTimestamp(3000);
		rtp->setSsrc(2468);
		std::memcpy(sentRtpRaw.data() + sizeof(RtpHeader), payload.data(), payload.size());

		// Callback for RTP packet arrival on pc2
		//rtpCallback = [&](rtc::binary message)
                //{
			if (message.empty() || message.size() != sentRtpRaw.size() ||
			    memcmp(message.data(), sentRtpRaw.data(), sentRtpRaw.size()) != 0) {
				SError << "Received RTP packet is different than the packet that was sent";
				return;
			}

			// Chained RTCP REMB callback on t1
			t->setMediaHandler(make_shared<rtc::RembHandler>([&](unsigned int bitrate) {
				if (bitrate != 1000000) {
					SError << "Incorrect bitrate in REMB message";
					return;
				}
				startClosePeers();
			}));

			std::vector<std::byte> rtcpRembRaw(RtcpRemb::SizeWithSSRCs(2));
			auto *rtcpRemb = reinterpret_cast<RtcpRemb *>(rtcpRembRaw.data());
			rtcpRemb->preparePacket(6666, 2, 1000000);
			rtcpRemb->setSsrc(0, 2468);
			rtcpRemb->setSsrc(1, 2469);

			if (!t->send(rtcpRembRaw.data(), rtcpRembRaw.size())) {
				SError << "Couldn't send RTCP REMB message";
			}
		//};

		if (!t->send(sentRtpRaw.data(), sentRtpRaw.size())) {
			SError << "Couldn't send RTP packet";
		}
                
                
                    
            
                
                SInfo << "On RTP Message";
            }, nullptr);
        });
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        
#endif
//        auto dc = pc2->createDataChannel("ping-pong-pc2");
//        dc->onOpen([id, wdc = make_weak_ptr(dc)](){
//            if (auto dc = wdc.lock()) {
//                SInfo << "pc2 onOpen";
//                        dc->send("ping-pong pc2 on open send1");
//            }
//        });
//        dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg){
//            if (auto dc = wdc.lock()) {
//                dc->send("ping-pong pc2 on message send2");
//            }
//        });
//        client->dataChannel2 = dc;
//        pc2->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
//            SInfo << "PC2 onDataChannel from " << id << " received with label \""
//                    << dc->label();
//            dc->onOpen([wdc = make_weak_ptr(dc)](){
//                if (auto dc = wdc.lock())
//                dc->send("PC2 Hello from arvind");
//            });
//            dc->onClosed(
//                    [id]() {
//                        SInfo << "DataChannel from " << id << " closed" << std::endl; });
//            dc->onMessage([id, dc](auto data) {
//                if (std::holds_alternative<std::string>(data))
//                    SInfo << "onDataChannel:onMessage PC2 Message from " << id
//                        << " received: " << std::get<std::string>(data) << std::endl;
//                else
//                    SInfo << "onDataChannel:onMessage PC2 Binary message from " << id
//                        << " received, size=" << std::get<rtc::binary>(data).size()
//                    << std::endl;
//                // sleep(5);
//                dc->send("PC2 tp PC1");
//            });
//            client->dataChannel22 = dc;
//        });
        pc2->setLocalDescription();

        return client;


    }
}

