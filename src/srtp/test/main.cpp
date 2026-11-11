/**
 *
  https://googlesource.com

  https://googlesource.com
 *
 * https://github.com
 * https://wikipedia.org
  cricket::Candidate represents an address discovered by a cricket::Port. A
 candidate can be local (i.e discovered by a local port) or remote. Remote
 candidates are transported using signaling, i.e outside of webrtc. There are 4
 types of candidates: local, stun, prflx or relay (standard)

 */
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
#include "rembhandler.hpp"          
#include "peerconnection.h"
#define CERTFROMFILE 1
#define VIDEOMEDIA 1
using namespace rtc;
using namespace std;
using namespace base;
using namespace base::net;
using json = nlohmann::json;


class TestResult {
public:
	bool success;
	string err_reason;

	TestResult(bool success, string err_reason = "") : success(success), err_reason(err_reason) {}
};

class Test {
public:
	string name;
	function<TestResult(void)> f;

	Test(string name, std::function<TestResult(void)> testFunc) : name(name), f(testFunc) {}

	TestResult run() {
		SInfo <<   "*** Running " << name << " test" ;
		TestResult res = this->f();
		if (res.success) {
			SInfo << "*** Finished " << name << " test" << endl;
		} else {
			SError << name << " test failed. Reason: " << res.err_reason << endl;
		}

		return res;
	}
};


TestResult test_track(   Application & app) {
	

 	base::cnfg::Configuration cache;
    cache.load("./cache.js");
    ConfSettings::SetConfiguration(cache.root);


	Configuration config1;
	// STUN server example
	 config1.iceServers.emplace_back("stun:stun.l.google.com:19302");

	PeerConnection pc1(config1);

	Configuration config2;
	// STUN server example
	config2.iceServers.emplace_back("stun:stun.l.google.com:19302");
	// Port range example
	//config2.portRangeBegin = 5000;
	//config2.portRangeEnd = 6000;

	PeerConnection pc2(config2);

	pc1.onLocalDescription([&pc2](Description sdp) {
		SInfo << "Description 1: " << sdp << endl;
		pc2.setRemoteDescription(sdp);
	});

	pc1.onLocalCandidate([&pc2](Candidate candidate) {
		SInfo << "Candidate 1: " << candidate << endl;
		pc2.addRemoteCandidate(candidate);
	});

	std::function<void(PeerConnection::State)> pc1StateCallback;
	pc1.onStateChange([&](PeerConnection::State state) {
		SInfo << "State 1: " << state << endl;
		if (pc1StateCallback) pc1StateCallback(state);
	});

	pc1.onGatheringStateChange([](PeerConnection::GatheringState state) {
		SInfo << "Gathering state 1: " << PeerConnection::printState(state) << endl;
	});

	pc2.onLocalDescription([&pc1](Description sdp) {
		SInfo << "Description 2: " << sdp << endl;
		pc1.setRemoteDescription(sdp);
	});

	pc2.onLocalCandidate([&pc1](Candidate candidate) {
		SInfo << "Candidate 2: " << candidate << endl;
		pc1.addRemoteCandidate(candidate);
	});

	std::function<void(PeerConnection::State)> pc2StateCallback;
	pc2.onStateChange([&](PeerConnection::State state) {
		SInfo << "State 2: " << state << endl;
		if (pc2StateCallback) pc2StateCallback(state);
	});

	pc2.onGatheringStateChange([](PeerConnection::GatheringState state) {
		SInfo << "Gathering state 2: " << PeerConnection::printState(state) << endl;
	});

	std::mutex mtx;
	std::condition_variable cv;
	bool testFinished = false;
	TestResult testResult(true);

	shared_ptr<Track> t1;
	shared_ptr<Track> t2;
	string newTrackMid;
	int phase = 1; // 1: initial track, 2: renegotiation, 3: media, 4: closing

	std::function<void(rtc::binary)> rtpCallback;
	std::function<void()> startRenegotiation;
	std::function<void()> startMediaTest;
	std::function<void()> startClosePeers;

	auto failTest = [&](const string &reason) {
		std::lock_guard<std::mutex> lock(mtx);
		if (!testFinished) {
			testResult = TestResult(false, reason);
			testFinished = true;
			cv.notify_one();
		}
	};

	// Checks if both tracks are open and dispatches the next phase
	auto checkTracksReady = [&]() {
		int nextAction = 0;
		{
			std::lock_guard<std::mutex> lock(mtx);
			if (phase == 1 && t1 && t1->isOpen() && t2 && t2->isOpen()) {
				phase = 2;
				nextAction = 1;
			} else if (phase == 2 && t1 && t1->isOpen() && t2 && t2->isOpen()) {
				phase = 3;
				nextAction = 2;
			}
		}

		if (nextAction == 1) {
			startRenegotiation();
		} else if (nextAction == 2) {
			startMediaTest();
		}
	};

	pc2.onTrack([&](shared_ptr<Track> t) {
		string mid = t->mid();
		SInfo << "Track 2: Received track with mid \"" << mid << "\"" << endl;
		if (mid != newTrackMid) {
			SError << "Wrong track mid" << endl;
			failTest("Wrong track mid");
			return;
		}

		t2 = t;

		t2->onOpen([&, mid]() {
			SInfo << "Track 2: Track with mid \"" << mid << "\" is open" << endl;
			checkTracksReady();
		});

		t2->onClosed([mid]() {
			SInfo << "Track 2: Track with mid \"" << mid << "\" is closed" << endl;
		});

		t2->onMessage([&](rtc::binary message) {
			if (rtpCallback) {
				rtpCallback(std::move(message));
			}
		}, nullptr);
	});

	// Test renegotiation setup
	startRenegotiation = [&]() {
		newTrackMid = "added";

		Description::Video media2(newTrackMid, Description::Direction::SendOnly);
		media2.addH264Codec(96);
		media2.setBitrate(3000);
		media2.addSSRC(2468, "video-send");

		t2.reset();
		t1 = pc1.addTrack(media2);

		t1->onOpen([&]() {
			SInfo << "Track 1: Renegotiated track is open" << endl;
			checkTracksReady();
		});

		pc1.setLocalDescription();
	};

#if RTC_ENABLE_MEDIA
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
				failTest("Received RTP packet is different than the packet that was sent");
				return;
			}

			// Chained RTCP REMB callback on t1
			t1->setMediaHandler(make_shared<RembHandler>([&](unsigned int bitrate) {
				if (bitrate != 1000000) {
					failTest("Incorrect bitrate in REMB message");
					return;
				}
				startClosePeers();
			}));

			std::vector<std::byte> rtcpRembRaw(RtcpRemb::SizeWithSSRCs(2));
			auto *rtcpRemb = reinterpret_cast<RtcpRemb *>(rtcpRembRaw.data());
			rtcpRemb->preparePacket(6666, 2, 1000000);
			rtcpRemb->setSsrc(0, 2468);
			rtcpRemb->setSsrc(1, 2469);

			if (!t2->send(rtcpRembRaw.data(), rtcpRembRaw.size())) {
				failTest("Couldn't send RTCP REMB message");
			}
		};

		if (!t1->send(sentRtpRaw.data(), sentRtpRaw.size())) {
			failTest("Couldn't send RTP packet");
		}
	};
#else
	startMediaTest = [&]() {
		startClosePeers();
	};
#endif

	startClosePeers = [&]() {
		phase = 4;
		pc1StateCallback = [&](PeerConnection::State state) {
			if (state == PeerConnection::State::Closed) {
				pc2.close();
			}
		};

		pc2StateCallback = [&](PeerConnection::State state) {
			if (state == PeerConnection::State::Closed) {
				std::lock_guard<std::mutex> lock(mtx);
				if (!t1->isClosed() || !t2->isClosed()) {
					testResult = TestResult(false, "Track is not closed");
				} else {
					testResult = TestResult(true);
				}
				testFinished = true;
				cv.notify_one();
			}
		};

		pc1.close();
		if (pc1.state() == PeerConnection::State::Closed && pc2.state() != PeerConnection::State::Closed) {
			pc2.close();
		}
	};

	// Test opening initial track
	newTrackMid = "test";

	Description::Video media(newTrackMid, Description::Direction::SendOnly);
	media.addH264Codec(96);
	media.setBitrate(3000);
	media.addSSRC(1234, "video-send");

	const auto mediaSdp1 = string(media);
	const auto mediaSdp2 = string(Description::Media(mediaSdp1));
	if (mediaSdp2 != mediaSdp1) {
		SInfo << mediaSdp2 << endl;
		return TestResult(false, "Media description parsing test failed");
	}

	t1 = pc1.addTrack(media);
	t1->onOpen([&]() {
		SInfo << "Track 1: Track with mid \"" << t1->mid() << "\" is open" << endl;
		checkTracksReady();
	});

	pc1.setLocalDescription();

	// Wait only for the overall callback pipeline to finish
	//std::unique_lock<std::mutex> lock(mtx);
	//cv.wait(lock, [&]() { return testFinished; });
        
            app.waitForShutdown([&](void *) {

//        {
//
//            for (auto& pair : clients) {
//                SInfo << "Closing and cleaning up active clients and peer connections " << pair.first;
//                if (pair.second && pair.second->peerConnection) {
//                    pair.second->peerConnection->close();
//                }
//            }
//
//        }
//
//        {
//
//            for (auto &pair : clients) {
//                SInfo << "Closing and cleaning up active clients and peer "
//                        "connections "
//                        << pair.first;
//                if (pair.second && pair.second->peerConnection) {
//                    uv_sleep(100);
//
//                }
//            }
//
//        }

//        {
//            std::lock_guard<std::mutex> lock(clients_mutex);
//            clients.clear();
//        }

        SInfo << "app.run() is over";

       
    });

	return testResult;
}














//template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) {
//    return ptr;
//}
//unordered_map<string, shared_ptr<Client>> clients
//{
//};
//std::mutex clients_mutex;
//
//shared_ptr<Client> createPeerConnection_lc1(Configuration &config, string id,
//        shared_ptr<PeerConnection> pc1,
//        shared_ptr<PeerConnection> pc2);
//shared_ptr<Client> createPeerConnection_lc2(Configuration &config, string id,
//        shared_ptr<PeerConnection> pc2,
//        shared_ptr<PeerConnection> pc1);
//void addToStream(shared_ptr<Client> client, bool isAddingVideo);
//Configuration settingconfig;
//std::string id;

int main(int argc, char **argv) {
    base::cnfg::Configuration cache;
    cache.load("./cache.js");
    ConfSettings::SetConfiguration(cache.root);
    Application app;
    Async async;
    string stunServer = "stun:stun.l.google.com:19302";
    SInfo << "STUN server is " << stunServer << endl;
    
    
//    settingconfig.iceServers.emplace_back(stunServer);
//
//   // settingconfig.iceServers.emplace_back("turn:testuser:testpassword@192.168.0.20:3478?transport=udp");
//    
//    if(!ConfSettings::configuration.turn.empty())
//    settingconfig.iceServers.emplace_back(ConfSettings::configuration.turn);
//    
//
//    settingconfig.disableAutoNegotiation = true;
//    settingconfig.staticPort = ConfSettings::configuration.staticPort;
//    settingconfig.gconfig->serverdtsRole =
//            ConfSettings::configuration.serverdtsRole;
//    settingconfig.gconfig->serverdtsRole =
//            ConfSettings::configuration.serverdtsRole;
//    settingconfig.enableTcp =  ConfSettings::configuration.enableTcp;;
//    settingconfig.enableUdp =  ConfSettings::configuration.enableUdp;;
//    settingconfig.publicIP = ConfSettings::configuration.publicIP;
//    settingconfig.noPivateIP = ConfSettings::configuration.noPivateIP;
//    bool relayOnly = ConfSettings::configuration.relayOnly;
//    if (relayOnly)
//        settingconfig.iceTransportPolicy = TransportPolicy::Relay;


//#if CERTFROMFILE == 1
//    settingconfig.gconfig->keyPemFile = ConfSettings::configuration.keyFile;
//    settingconfig.gconfig->certificatePemFile =
//            ConfSettings::configuration.certFile;
//    settingconfig.gconfig->keyPemPass = "12345678";
//#elif CERTFROMFILE == 2
//    settingconfig.keyPemFile = "";
//    settingconfig.certificatePemFile = "";
//#else
//#endif
    string localId = "server";
    SInfo << "The local ID is: " << localId << endl;
    rtc::DtlsTransport::ClassInit();
    DepUsrSCTP::ClassInit();
    
    GetNetInterface::ClassInit();
        
    DepLibSRTP::ClassInit();

//    std::string id1 = "server1";
//    std::string id2 = "server2";
//    auto pc1 = make_shared<PeerConnection>(settingconfig);
//    settingconfig.staticPort = settingconfig.staticPort + 1;
//    auto pc2 = make_shared<PeerConnection>(settingconfig);
//
//    auto client1 = createPeerConnection_lc1(settingconfig, id1, pc1, pc2);
//    {
//        std::lock_guard<std::mutex> lg(clients_mutex);
//        clients.emplace(id1, client1);
//
//    }
//    auto client2 = createPeerConnection_lc2(settingconfig, id2, pc2, pc1);
//    {
//        std::lock_guard<std::mutex> lg(clients_mutex);
//        clients.emplace(id2, client2);
//    }
//
//    
//    

    test_track(app);
        


    
    
    DepUsrSCTP::ClassDestroy();
        Logger::destroy();
        
        
    SInfo << "Exiting main..." ;
    return 0;
}

