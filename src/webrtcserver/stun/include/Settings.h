#ifndef SFU_SETTINGS_HPP
#define SFU_SETTINGS_HPP

#include "json/configuration.h"
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
        
        //uint16_t remoteport{0};
        uint16_t localport{0};
        std::string user;
        std::string passwd;
        //bool is_ipv4;
        std::string remoteip;
        std::string datachannel;

    };



public:
    static void SetConfiguration(  base::cnfg::Configuration& config, int argc);
    
    static void User(char *buf, size_t size) ;
     
    static void Passwd(char *buf, size_t size) ;
    
    static uint16_t LocalPort();
    static uint16_t RemotePort();
    static std::string RemoteIP();
    static std::string getdatachannel();

private:


public:
    static struct Configuration configuration;


private:
 

public:

};

#endif
