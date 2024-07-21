#ifndef SFU_SETTINGS_HPP
#define SFU_SETTINGS_HPP

#include <json/json.hpp>
#include <map>
#include <string>
#include <vector>
//#include <mutex>          // std::mutex

#include <uv.h>

using json = nlohmann::json;

class Settings {
public:
    static void init();
    static void exit();

    struct LogTags {
        bool info{ false };
    };

public:
    // Struct holding the configuration.
    struct Configuration {
        //LogLevel logLevel{ LogLevel::LOG_ERROR };
        // struct LogTags logTags;
        // uint16_t rtcMinPort{ 10000 };
        // uint16_t rtcMaxPort{ 59999 };

        // bool tcpRtsp{true};
        
        uint16_t recordsize{250};
        // uint16_t SegSize_key{5};

        //std::string dtlsCertificateFile;
       // std::string dtlsPrivateKeyFile;
        std::string storage;
        std::string OTA{"/mnt/OTA"};
        std::string log{"/mnt/log"};
        
        std::string cam;
        std::string server;
        int port{443};
        bool recording{false};
        bool cloud{false};
        bool ai{true};
        bool facedetect;
        bool motionevent;
        //json rtsp;

        //json root;
       // json listenIps;
    };

    // struct EncSetting {
    //     uint16_t vp9Enc{ 10 };
    //     uint16_t nvidiaEnc{ 10 };
    //     uint16_t quicksyncEnc{ 16 };
    //     uint16_t native{ 16 };
    //     uint16_t x264Enc{ 10 };
    //     uint16_t VAAPIEnc{ 16 };
    //     json root;
    // };

public:
    static void SetConfiguration(json& config);
 //   static void SetEncoderConf(json& cnfg);
    //static void PrintConfiguration();

private:
    //static void SetLogLevel(std::string& level);
    //static void SetLogTags(const std::vector<std::string>& tags);
    //static void SetDtlsCertificateAndPrivateKeyFiles();

public:
    static struct Configuration configuration;
//    static struct EncSetting encSetting;

private:
    //static void saveFile(const std::string& path, const std::string& dump);

public:
    //static void postNode(json& node);

   // static bool deleteNode(json& node, std::vector<std::string>& vec);

    //static uv_rwlock_t rwlock_t;

    //static std::string getNode();

   // static json getJsonNode();

   // static bool putNode(json& node, std::vector<std::string>& vec);

   // static bool setNodeState(std::string& id, std::string status);

   // static bool getNodeState(std::string id, std::string key, std::string& value);
    
    //static bool getJsonNodeState(std::string id, json& value);
};

#endif
