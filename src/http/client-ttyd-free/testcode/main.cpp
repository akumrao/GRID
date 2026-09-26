#include <libwebsockets.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

using json = nlohmann::json;

// Global Setup Configurations
const std::string HOST            = "dabwulkpyquthvearbfw.supabase.co";
const std::string PUBLISHABLE_KEY = "sb_publishable_1V4Zaw720lrIPx8IqmuxmA_ri32Bdqu";
const std::string CHANNEL_TOPIC   = "realtime:public:room:arvind";

static std::atomic<bool> force_exit{false};
static std::atomic<bool> join_request_pending{false};
static struct lws *web_socket = nullptr;

// Protocol structural packet generator
std::string get_join_payload() {
    json join_message = {
        {"topic", CHANNEL_TOPIC},
        {"event", "phx_join"},
        {"payload", {
            {"config", {
                {"broadcast", {{"self", true}, {"ack", true}}},
                {"presence", {{"enabled", false}}},
                {"private", false}
            }}
        }},
        {"ref", "1"},
        {"join_ref", "1"}
    };
    return join_message.dump();
}

// Low-level event loop callback handler
static int callback_supabase_realtime(struct lws *wsi, enum lws_callback_reasons reason,
                                      void *user, void *in, size_t len) {
    // Suppress unused parameter warnings safely
    (void)user;

    switch (reason) {
        
        case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: {
            std::cout << "\n========================================\n";
            std::cout << ">>> RAW WEBSOCKET HANDSHAKE REQUEST >>>\n";
            std::cout << "========================================\n";
            std::cout << "GET /realtime/v1/websocket?apikey=" << PUBLISHABLE_KEY << "&vsn=1.0.0 HTTP/1.1\n";
            std::cout << "Host: " << HOST << "\n";
            std::cout << "Upgrade: websocket\n";
            std::cout << "Connection: Upgrade\n";
            std::cout << "Origin: http://localhost\n";
            std::cout << "Sec-WebSocket-Version: 13\n";
            std::cout << "----------------------------------------\n" << std::endl;
            break;
        }

        case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH: {
            std::cout << "========================================\n";
            std::cout << "<<< RAW WEBSOCKET HANDSHAKE RESPONSE <<<\n";
            std::cout << "========================================\n";
            
            // Extract the HTTP status response code directly from the connection object
            int status_code = lws_http_client_http_response(wsi);
            std::cout << "HTTP/1.1 " << status_code << " Switching Protocols\n";

            // Enumerate over the primary raw index spaces safely to poll incoming fragments
            // This reads up to 100 possible token variants supported by libwebsockets internally
            char value_buf[1024];
            for (int i = 0; i < 100; i++) {
                int total_len = lws_hdr_total_length(wsi, (lws_token_indexes)i);
                if (total_len > 0) {
                    const unsigned char* name_ptr = lws_token_to_string((lws_token_indexes)i);
                    if (name_ptr) {
                        std::memset(value_buf, 0, sizeof(value_buf));
                        lws_hdr_copy(wsi, value_buf, sizeof(value_buf) - 1, (lws_token_indexes)i);
                        std::cout << name_ptr << " " << value_buf << "\n";
                    }
                }
            }
            std::cout << "----------------------------------------\n" << std::endl;
            break;
        }

        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            std::cout << "[+] WebSocket Handshake Successful! Upgraded Connection State.\n";
            web_socket = wsi;
            join_request_pending = true;
            lws_callback_on_writable(wsi);
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE: {
            if (join_request_pending) {
                join_request_pending = false;
                std::string msg = get_join_payload();
                
                std::vector<unsigned char> buf(LWS_PRE + msg.length());
                std::copy(msg.begin(), msg.end(), buf.begin() + LWS_PRE);
                
                std::cout << "[->] Dispatched channel authentication payload...\n";
                lws_write(wsi, &buf[LWS_PRE], msg.length(), LWS_WRITE_TEXT);
            }
            break;
        }

        case LWS_CALLBACK_CLIENT_RECEIVE: {
            std::string server_reply((char *)in, len);
            std::cout << "[Server Response] -> " << server_reply << std::endl;
            break;
        }

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            std::cerr << "[-] Connection initialization error: " << (in ? (char *)in : "Unknown") << std::endl;
            force_exit = true;
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            std::cout << "[-] WebSocket Connection Severed.\n";
            force_exit = true;
            break;

        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    { "supabase-realtime", callback_supabase_realtime, 0, 0, 0, nullptr, 0 },
    { nullptr, nullptr, 0, 0, 0, nullptr, 0 }
};

int main() {
    // Activate internal verbose logging layers (DNS, TLS states, headers)
    int log_level = LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_CLIENT | LLL_HEADER;
    lws_set_log_level(log_level, lwsl_emit_stderr);

    struct lws_context_creation_info info;
    std::memset(&info, 0, sizeof(info));

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

    std::cout << "[*] Creating Libwebsockets execution context..." << std::endl;
    struct lws_context *context = lws_create_context(&info);
    if (!context) {
        std::cerr << "[-] Failed to initialize libwebsocket context.\n";
        return 1;
    }

    std::string path = "/realtime/v1/websocket?apikey=" + PUBLISHABLE_KEY + "&vsn=1.0.0";

    struct lws_client_connect_info ccinfo;
    std::memset(&ccinfo, 0, sizeof(ccinfo));
    ccinfo.context = context;
    ccinfo.address = HOST.c_str();
    ccinfo.port = 443;
    ccinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED;
    ccinfo.path = path.c_str();
    ccinfo.host = HOST.c_str();
    ccinfo.origin = "http://localhost";
    ccinfo.protocol = protocols[0].name;

    std::cout << "[*] Launching network execution loop targeting " << HOST << "...\n";
    lws_client_connect_via_info(&ccinfo);

    while (!force_exit) {
        lws_service(context, 100);
    }

    lws_context_destroy(context);
    return 0;
}
