#ifndef SFU_SETTINGS_HPP
#define SFU_SETTINGS_HPP

#include <json/json.hpp>
#include <map>
#include <string>
#include <vector>
//#include <mutex>          // std::mutex

#include <uv.h>

using json = nlohmann::json;

class ConfSettings {
public:
    static void init();
    static void exit();

    struct LogTags {
        bool info{ false };
    };

public:
    // Struct holding the configuration.
    struct Configuration {
        uint16_t recordsize{250};
        // uint16_t SegSize_key{5};

        std::string certFile;
        std::string keyFile;

        std::string log{"/tmp/log/"};
        
        std::string server{"localhost"};
        int port{443};

    };


public:
    static void SetConfiguration(json& config);
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

};

#endif
