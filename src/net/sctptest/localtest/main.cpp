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
#define localtesting 1
#include "peerconnection.h"
#define CERTFROMFILE 1
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
    settingconfig.disableAutoNegotiation = true;
    settingconfig.staticPort = ConfSettings::configuration.staticPort;
    settingconfig.gconfig->serverdtsRole =
            ConfSettings::configuration.serverdtsRole;
    settingconfig.gconfig->serverdtsRole =
            ConfSettings::configuration.serverdtsRole;
    settingconfig.enableTcp = false;
    settingconfig.enableUdp = true;
    settingconfig.publicIP = ConfSettings::configuration.publicIP;
    settingconfig.noPivateIP = ConfSettings::configuration.noPivateIP;
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
#if localtesting
    std::string id1 = "server1";
    std::string id2 = "server2";
    auto pc1 = make_shared<PeerConnection>(settingconfig);
    settingconfig.staticPort = settingconfig.staticPort + 1;
    auto pc2 = make_shared<PeerConnection>(settingconfig);
    clients.emplace(id1, createPeerConnection_lc1(settingconfig, id1, pc1, pc2));
    clients.emplace(id2, createPeerConnection_lc2(settingconfig, id2, pc2, pc1));
#else
#endif
    app.waitForShutdown([&](void *) {
        SInfo << "app.run() is over";
        {
            //std::lock_guard<std::mutex> lock(clients_mutex);
            for (auto& pair : clients) {
                 SInfo << "Closing and cleaning up active clients and peer connections " << pair.first;
                if (pair.second && pair.second->peerConnection) {
                    pair.second->peerConnection->close();
                }
            }
            clients.clear();
        }

        DepUsrSCTP::ClassDestroy();
        Logger::destroy();
    });
    SInfo << "Cleaning up..." << endl;
    return 0;
}


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
                {
                    SInfo << "createPeerConnection_lc1 close " << id;
                      
                    clients.erase(id);
                }
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
        pc1->onGatheringStateChange([](PeerConnection::GatheringState state) {
            SInfo << "Gathering State" << PeerConnection::printState(state);
            if (state == PeerConnection::GatheringState::Complete) {
                SInfo << "pc1 Gathering State: Complete";
                {
                }
            } else if (state == PeerConnection::GatheringState::InProgress) {
                SInfo << "pc1 Gathering State: InProgress";
            }
        });
#if VIDEOMEDIA
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
            }
        });
        client->dataChannel1 = dc;
        pc1->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
            SInfo << "pc1 onDataChannel from " << id << " received with label \""
                    << dc->label() << "\"" << std::endl;
            dc->onOpen([wdc = make_weak_ptr(dc)](){
                if (auto dc = wdc.lock()) {
                    SInfo << "pc1 open ";
                            dc->send("Hello from pc1");
                }
            });
            dc->onClosed([id]() {
                SInfo << "pc1 DataChannel from " << id << " closed" << std::endl;
            });
            dc->onMessage([id, dc](auto data) {
                if (std::holds_alternative<std::string>(data))
                    SInfo << "pc1 Message from " << id
                        << " received: " << std::get<std::string>(data) << std::endl;
                else
                    SInfo << "pc1 Binary message from " << id
                        << " received, size=" << std::get<rtc::binary>(data).size()
                    << std::endl;
            });
            client->dataChannel11 = dc;
        });
        pc1->setLocalDescription();
        return client;
    }
}
;

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
                {
                     SInfo << "createPeerConnection_lc2 close " << id;
                    clients.erase(id);
                }
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
        pc2->onGatheringStateChange([](PeerConnection::GatheringState state) {
            if (state == PeerConnection::GatheringState::Complete) {
                SInfo << "Pc2 Gathering State: Complete";
                {
                }
            }
        });
#if VIDEOMEDIA
#endif
        auto dc = pc2->createDataChannel("ping-pong-pc2");
        dc->onOpen([id, wdc = make_weak_ptr(dc)](){
            if (auto dc = wdc.lock()) {
                SInfo << "pc2 onOpen";
                        dc->send("ping-pong pc2 on open send");
            }
        });
        dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg){
            if (auto dc = wdc.lock()) {
                dc->send("ping-pong pc2 on message send");
            }
        });
        client->dataChannel2 = dc;
        pc2->onDataChannel([id, client](shared_ptr<rtc::DataChannel> dc) {
            SInfo << "PC2 onDataChannel from " << id << " received with label \""
                    << dc->label();
            dc->onOpen([wdc = make_weak_ptr(dc)](){
                if (auto dc = wdc.lock())
                        dc->send("PC2 Hello from arvind");
                });
            dc->onClosed(
                    [id]() {
                        SInfo << "DataChannel from " << id << " closed" << std::endl; });
            dc->onMessage([id, dc](auto data) {
                if (std::holds_alternative<std::string>(data))
                    SInfo << "onDataChannel:onMessage PC2 Message from " << id
                        << " received: " << std::get<std::string>(data) << std::endl;
                else
                    SInfo << "onDataChannel:onMessage PC2 Binary message from " << id
                        << " received, size=" << std::get<rtc::binary>(data).size()
                    << std::endl;
                // sleep(5);
                dc->send("PC2 tp PC1");
            });
            client->dataChannel22 = dc;
        });
        pc2->setLocalDescription();

        return client;


    }
}

