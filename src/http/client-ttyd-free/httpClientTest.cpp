#include <nlohmann/json.hpp>  
#include "base/application.h" 
#include "base/logger.h"  

#include <atomic> 
#include <chrono> 
#include <iostream> 
#include <memory> 
#include <string> 
#include <thread> 
#include <iomanip> 
#include <sstream>  
#include <fstream> // Fixed: Added missing header for std::ofstream

#include "net/netInterface.h" 
#include "httpClientTest.h" 
#include "http/client.h"  

#include "base/platform.h"  
#include "http/url.h" 
#include "base/filesystem.h" 
#include "http/HttpClient.h" 
#include "http/HttpsClient.h" 

#include "http/form.h" 
#include "base/tty.h" 
#include "base/uuid.h"  

using namespace base; 
using namespace base::net; 
using json = nlohmann::json;  

ClientConnecton* m_client = nullptr;  

// Track total unique client endpoints present in the server sync tracker state container
std::atomic<int> m_current_client_count{0};  

// Helper function to generate a random unique UUIDv4 string 
std::string generateUniqueUUID() {     
    std::string ret = "arvind";// uuid4::uuid();     
    return ret;  
}  

// Helper function to generate ISO 8601 Timestamp matching JS new Date().toISOString() 
std::string getISOTimestamp() {     
    auto now = std::chrono::system_clock::now();     
    auto itt = std::chrono::system_clock::to_time_t(now);     
    
    // Fixed: Added explicit <std::chrono::milliseconds> template argument
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(         
        now.time_since_epoch()     
    ).count() % 1000;                
    
    std::stringstream ss;     
    ss << std::put_time(std::gmtime(&itt), "%Y-%m-%dT%H:%M:%S")        
       << '.' << std::setfill('0') << std::setw(3) << ms << 'Z';     
    return ss.str(); 
}  

// Helper function to append incoming payloads to a local file stream secure log 
void appendMessageToFile(const std::string& raw_json) {     
    std::ofstream log_file("realtime_messages.log", std::ios::app);     
    if (log_file.is_open()) {         
        log_file << "[" << getISOTimestamp() << "] " << raw_json << "";        
        log_file.close();     
    } else {         
        std::cerr << "Failed to open realtime_messages.log file for writing.";     
    } 
}  

/**  
 * Creates the Phoenix join protocol payload for a specific room and sends it over the connection.  
 */ 
void addNewChannelAndJoin(HttpBase* con, const std::string& channel_topic, const std::string& join_message_ref, const std::string& join_ref) {     
    std::cout << "Creating and joining new channel: " << channel_topic << std::endl;      
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
                                {"enabled", true}                             
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
}  

/**  
 * Separate function to send back a message received acknowledgment.  
 */ 
void sendAcknowledgmentMessage(HttpBase* con, const std::string& channel_topic, const std::string& join_ref) {     
    json ack_message = {         
        {"topic", channel_topic},         
        {"event", "broadcast"},         
        {             
            "payload",             
            {                 
                {"type", "broadcast"},                 
                {"event", "message_received"},                 
                {                     
                    "payload",                     
                    {                         
                        {"text", "message received"},                         
                        {"received_at", getISOTimestamp()}                     
                    }                 
                }             
            }         
        },         
        {"ref", "4"},         
        {"join_ref", join_ref}     
    };      
    std::string ack_str = ack_message.dump();     
    con->send(ack_str.c_str(), ack_str.size(), false);     
    std::cout << "Sent message received acknowledgment on channel"; 
}  

int main(int argc, char** argv) {     
    ConsoleChannel* ch = new ConsoleChannel("debug", Level::Trace);     
    Logger::instance().add(ch);      
    Application app;      
    const std::string host = "dabwulkpyquthvearbfw.supabase.co";     
    const int port = 443;     
    const std::string publishable_key = "sb_publishable_1V4Zaw720lrIPx8IqmuxmA_ri32Bdqu";     
    const std::string broadcast_event = "message_sent";      
    
    // Dynamic unique channel room generation     
    const std::string unique_id = generateUniqueUUID();     
    const std::string channel_topic = "realtime:public:room:" + unique_id;           
    std::cout << "Target Unique Room generated: " << channel_topic << std::endl;      
    const std::string target = "/realtime/v1/websocket?apikey=" + publishable_key + "&vsn=1.0.0";      
    const std::string join_ref = "1";     
    const std::string join_message_ref = "1";      
    LTrace("Initializing HttpsClient...");     
    m_client = new HttpsClient("wss", host, port, target);          
    m_client->setHostName(host);      
    
    // Set Host header explicitly for HTTP request & SSL SNI validation     
    m_client->_request.set("Host", host);     
    m_client->_request.set("Origin", "http://localhost");      
    m_client->_request.setKeepAlive(true);      
    
    // Fixed: Added explicit <bool> template arguments to atomic instantiations
    std::atomic<bool> joined(false);     
    std::atomic<bool> running(true);     
    std::thread heartbeat_thread;      
    
    m_client->fnComplete = [](const Response & response) {         
        std::string reason = response.getReason();         
        StatusCode statuscode = response.getStatus();         
        STrace << "Handshake complete. Status: " << (int) statuscode                 
                << " Reason: " << reason;     
    };      
    m_client->fnConnect = [&](HttpBase * con) {         
        LTrace("WebSocket connected!");                  
        addNewChannelAndJoin(con, channel_topic, join_message_ref, join_ref);     
    };      
    m_client->fnPayload = [&](HttpBase* con, const char* data, size_t sz) {         
        if (!data || sz == 0) {             
            std::cerr << "Warning: Received empty payload slice. Skipping.";             
            return;         
        }          
        std::string text(data, sz);         
        STrace << "fnPayload received (" << sz << " bytes):" << text;          
        if (!json::accept(text)) {             
            std::cerr << "Validation Error: Received invalid non-JSON format string raw payload packet text.";             
            return;         
        }          
        try {             
            json incoming = json::parse(text);             
            const std::string event = incoming.value("event", "");             
            const std::string topic = incoming.value("topic", "");              
            if (event == "phx_reply" && topic == channel_topic) {                 
                const auto& payload = incoming["payload"];                 
                if (payload.value("status", "") == "ok") {                     
                    if (!joined.exchange(true)) {                         
                        std::cout << "Joined dynamic unique channel successfully!";                          
                        json track_message = {                             
                            {"topic", channel_topic},                             
                            {"event", "presence"},                             
                            {                                 
                                "payload",                                 
                                {                                     
                                    {"type", "track"},                                     
                                    {                                         
                                        "payload",                                         
                                        {                                             
                                            {"online_at", getISOTimestamp()}                                         
                                        }                                     
                                    }                                 
                                }                             
                            },                             
                            {"ref", "2"},                             
                            {"join_ref", join_ref}                         
                        };                          
                        std::string track_str = track_message.dump();                         
                        con->send(track_str.c_str(), track_str.size(), false);                          
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
                                            {"text", "Hello from C++ Dynamic Unique Client Room"},                                             
                                            {"sent_at", getISOTimestamp()}                                         
                                        }                                     
                                    }                                 
                                }                             
                            },                             
                            {"ref", "3"},                             
                            {"join_ref", join_ref}                         
                        };                          
                        std::string msg = broadcast_message.dump();                         
                        con->send(msg.c_str(), msg.size(), false);                         
                        std::cout << "Broadcast sent on dynamic topic";                          
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
                                std::cout << "Heartbeat sent";                             
                            }                         
                        });                     }                 }             
            } else if (event == "presence_diff" && topic == channel_topic) {                 
                if (incoming.contains("payload")) {                     
                    const auto& diff = incoming["payload"];                                                               
                    if (diff.contains("joins")) {                         
                        m_current_client_count += diff["joins"].size();                     
                    }                     
                    if (diff.contains("leaves")) {                         
                        m_current_client_count -= diff["leaves"].size();                         
                        if (m_current_client_count < 0) m_current_client_count = 0;                     
                    }                                                               
                    std::cout << "[tracer] Total connected clients online: " << m_current_client_count.load() << "";                 
                }             
            } else if (event == "broadcast" && topic == channel_topic) {                 
                appendMessageToFile(text);                  
                if (incoming.contains("payload")) {                     
                    const auto& payload_node = incoming["payload"];                     
                    std::string message_text = "";                                                               
                    if (payload_node.contains("payload") && payload_node["payload"].contains("text")) {                         
                        if (payload_node["payload"]["text"].is_string()) {                             
                            // Fixed: Added explicit <std::string> template argument to get()
                            message_text = payload_node["payload"]["text"].get<std::string>();                         
                        }                     
                    } else if (payload_node.contains("text")) {                         
                        if (payload_node["text"].is_string()) {                             
                            // Fixed: Added explicit <std::string> template argument to get()
                            message_text = payload_node["text"].get<std::string>();                         
                        }                     
                    }                      
                    if (!message_text.empty()) {                         
                        std::cout << "========================================";                         
                        std::cout << "Incoming Text Msg: " << message_text << "";                         
                        std::cout << "========================================";                     
                    }                 
                }                 
                std::cout << "Broadcast received on target channel:" << incoming.dump(2) << "";                  
                // Call separate function to send back message received acknowledgment
                //sendAcknowledgmentMessage(con, channel_topic, join_ref);             
            }         
        } catch (const json::exception& error) {             
            std::cerr << "JSON parse error: " << error.what() << "";         
        }     
    };      
    m_client->fnClose = [&](HttpBase* con, std::string str) {         
        running = false;         
    };      
    m_client->setReadStream(new std::stringstream);               
    m_client->send();      
    app.run();      
    
    running = false;     
    if (heartbeat_thread.joinable()) {         
        heartbeat_thread.join();     
    }      
    if (m_client) {         
        m_client->Close();         
        delete m_client;         
        m_client = nullptr;     
    }      
    return 0; 
}
