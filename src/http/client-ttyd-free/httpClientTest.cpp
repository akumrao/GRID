#include <nlohmann/json.hpp>

#include "base/application.h"
#include "base/logger.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "net/netInterface.h"
#include "httpClientTest.h"
#include "http/client.h"

#include "base/logger.h"
#include "base/application.h"
#include "base/platform.h"

#include "http/url.h"
#include "base/filesystem.h"
#include "http/HttpClient.h"
#include "http/HttpsClient.h"
#include "base/platform.h"

#include "http/form.h"
#include "base/tty.h"

using namespace base;
using namespace base::net;
using json = nlohmann::json;

ClientConnecton* m_client = nullptr;

int main(int argc, char** argv) {
    ConsoleChannel* ch = new ConsoleChannel("debug", Level::Trace);
    Logger::instance().add(ch);

    Application app;

    const std::string host = "dabwulkpyquthvearbfw.supabase.co";
    const int port = 443;
    const std::string publishable_key = "sb_publishable_1V4Zaw720lrIPx8IqmuxmA_ri32Bdqu";
    const std::string channel_topic = "realtime:public:room:arvind";
    const std::string broadcast_event = "message_sent";

    const std::string target = "/realtime/v1/websocket?apikey=" + publishable_key + "&vsn=1.0.0";

    const std::string join_ref = "1";
    const std::string join_message_ref = "1";

    LTrace("Initializing HttpsClient...");
    m_client = new HttpsClient("wss", host, port, target);

    // Set Host header explicitly for HTTP request & SSL SNI validation
    m_client->_request.set("Host", host);
    m_client->_request.setKeepAlive(true);

    std::atomic<bool> joined{false};
    std::atomic<bool> running{true};
    std::thread heartbeat_thread;

    m_client->fnComplete = [](const Response & response) {
        std::string reason = response.getReason();
        StatusCode statuscode = response.getStatus();
        STrace << "Handshake complete. Status: " << (int) statuscode
                << " Reason: " << reason;
    };

    m_client->fnConnect = [&](HttpBase * con) {
        LTrace("WebSocket connected!");

        json join_message = {
            {"topic", channel_topic},
            {"event", "phx_join"},
            {
                "payload",
                {
                    {
                        "config",
                        {
                            {
                                "broadcast",
                                {
                                    {"self", true},
                                    {"ack", true}
                                }
                            },
                            {
                                "presence",
                                {
                                    {"enabled", false}
                                }
                            },
                            {"private", false}
                        }
                    }
                }
            },
            {"ref", join_message_ref},
            {"join_ref", join_ref}
        };

        std::string join_str = join_message.dump();
        con->send(join_str.c_str(), join_str.size(), false);
        std::cout << "Join request sent\n";
    };

    m_client->fnPayload = [&](HttpBase* con, const char* data, size_t sz) {
        std::string text(data, sz);
        STrace << "fnPayload received (" << sz << " bytes):\n" << text;

        try {
            json incoming = json::parse(text);
            const std::string event = incoming.value("event", "");

            if (event == "phx_reply") {
                const auto& payload = incoming["payload"];
                if (payload.value("status", "") == "ok") {
                    if (!joined.exchange(true)) {
                        std::cout << "Joined public channel\n";

                        json broadcast_message = {
                            {"topic", channel_topic},
                            {"event", "broadcast"},
                            {
                                "payload",
                                {
                                    {"type", "broadcast"},
                                    {"event", broadcast_event},
                                    {
                                        "payload",
                                        {
                                            {"text", "Hello from C++ Custom WS Client"},
                                            {
                                                "sent_at",
                                                std::chrono::system_clock::now()
                                                .time_since_epoch()
                                                .count()
                                            }
                                        }
                                    }
                                }
                            },
                            {"ref", "2"},
                            {"join_ref", join_ref}
                        };

                        std::string msg = broadcast_message.dump();
                        con->send(msg.c_str(), msg.size(), false);
                        std::cout << "Broadcast sent from C++\n";

                        heartbeat_thread = std::thread([&, con]() {
                            std::uint64_t heartbeat_ref = 100;
                            while (running) {
                                std::this_thread::sleep_for(std::chrono::seconds(20));
                                if (!running) break;

                                json heartbeat = {
                                    {"topic", "phoenix"},
                                    {"event", "heartbeat"},
                                    {"payload", json::object()},
                                    {"ref", std::to_string(heartbeat_ref++)},
                                    {"join_ref", nullptr}
                                };

                                std::string hb_str = heartbeat.dump();
                                con->send(hb_str.c_str(), hb_str.size(), false);
                                std::cout << "Heartbeat sent\n";
                            }
                        });
                    }
                }
            } else if (event == "broadcast") {
                std::cout << "Broadcast received:\n" << incoming.dump(2) << "\n";
            }
        } catch (const json::exception& error) {
            std::cerr << "JSON parse error: " << error.what() << "\n";
        }
    };

    m_client->fnClose = [&](HttpBase* con, std::string str) {
        running = false;
        if (heartbeat_thread.joinable()) {
            heartbeat_thread.join();
        }
    };

    m_client->setReadStream(new std::stringstream);

    // Connect trigger inside a std::thread to let app.run() initiate event loop processing
    std::thread connect_thread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (m_client) {
            m_client->send();
        }
    });
    connect_thread.detach();
    
    
    m_client->send();

    app.run();

    if (m_client) {
        m_client->Close();
        delete m_client;
        m_client = nullptr;
    }

    return 0;
}