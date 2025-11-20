/*  For testing reset api use Postman
 * https://web.postman.co/workspace/My-Workspace~292b44c7-cae4-44d6-8253-174622f0233e/request/create?requestId=e6995876-3b8c-4b7e-b170-83a733a631db
 */


#include "Settings.h"
#include "base/error.h"
#include "base/logger.h"

#include "base/filesystem.h"

#include <cctype>  // isprint()
#include <cerrno>
#include <iterator>  // std::ostream_iterator
#include <sstream>  // std::ostringstream
#include <Utils.h>


//#define LOGGING_LOG_TO_FILE 1
/* Class variables. */

struct Settings::Configuration Settings::configuration;
//struct Settings::EncSetting Settings::encSetting;

//uv_rwlock_t Settings::rwlock_t;
/* Class methods. */

//void Settings::init()
//{
//    uv_rwlock_init(&rwlock_t);
//}
//
//void Settings::exit()
//{
//    uv_rwlock_destroy(&rwlock_t);
//}

//void Settings::SetEncoderConf(json &cnfg)
//{
//    if( cnfg.is_null() )
//    {
//         Settings::encSetting.root["rtsp"] = json::object();
//        
//    }
//    else
//     Settings::encSetting.root = cnfg; 
//   
//   
//   
//}

void Settings::User(char *buf, size_t size) {

    if(!configuration.user.empty())
    { 
        memcpy( buf, configuration.user.data() , size   ) ;
    }
    else
    {
        stun::random_str64(buf, size);
    }    
}

void Settings::Passwd(char *buf, size_t size) {
    if(!configuration.passwd.empty())
    {
        memcpy( buf, configuration.passwd.data() , size   ) ;
    }
    else
    {
        stun::random_str64(buf, size);
    }
}


uint16_t Settings::RemotePort() {
    return configuration.remoteport;
}


uint16_t Settings::LocalPort() {
    return configuration.localport++;
}

std::string Settings::getdatachannel() {
    return configuration.datachannel;
}


std::string Settings::RemoteIP() {
    return configuration.remoteip;
}


    


void Settings::SetConfiguration(  base::cnfg::Configuration &cnfg , int argc)
{
#if 1
    //if(!cnfg.loaded())     
    if(argc == 1)
    {
        cnfg.root = {
          {"logLevel", "info"},
          {"datachannel", "dc1"},
          {"localport", 7001},
          {"remoteport", 8001},
          {"user", "4Pfs"},
          {"passwd", "BsX4g0brln0+kXB/SxXSfI"},
          {"is_ipv4", false},
          {"remoteip", {"10.168.243.73" }}
          };

        std::string dir = base::fs::dirname(cnfg.path());
        if (dir != "." && dir != ".." && !base::fs::exists(dir))
        {
            base::fs::mkdir(dir);
        }
        
        cnfg.save();
    }
    else
    {
        cnfg.root = {
          {"logLevel", "info"},
          {"datachannel", "dc2"},
          {"localport", 8001},
          {"remoteport", 7001},
          {"user", "4Pfs"},
          {"passwd", "BsX4g0brln0+kXB/SxXSfI"},
          {"is_ipv4", true},
          {"remoteip", {"2401:4900:61c5:5479:3115:7ea0:3167:c048" }}
          };

        std::string dir = base::fs::dirname(cnfg.path());
        if (dir != "." && dir != ".." && !base::fs::exists(dir))
        {
            base::fs::mkdir(dir);
        }
        
        cnfg.save();


    }

    

#else
    
      //if(!cnfg.loaded())     
    if(argc == 1)
    {
        cnfg.root = {
          {"logLevel", "info"},
          {"datachannel", "dc1"},
          {"localport", 7001},
          {"remoteport", 8001},
          {"user", "4Pfs"},
          {"passwd", "BsX4g0brln0+kXB/SxXSfI"},
          {"is_ipv4", true},
          {"remoteip", {"10.168.243.73" }}
          };

        std::string dir = base::fs::dirname(cnfg.path());
        if (dir != "." && dir != ".." && !base::fs::exists(dir))
        {
            base::fs::mkdir(dir);
        }
        
        cnfg.save();
    }
    else
    {
        cnfg.root = {
          {"logLevel", "info"},
          {"datachannel", "dc2"},
          {"localport", 8001},
          {"remoteport", 7001},
          {"user", "4Pfs"},
          {"passwd", "BsX4g0brln0+kXB/SxXSfI"},
          {"is_ipv4", true},
          {"remoteip", {"10.168.243.73" }}
          };

        std::string dir = base::fs::dirname(cnfg.path());
        if (dir != "." && dir != ".." && !base::fs::exists(dir))
        {
            base::fs::mkdir(dir);
        }
        
        cnfg.save();


    }
#endif

    json &node = cnfg.root;
    
    if (node.find("localport") != node.end())
    {
        Settings::configuration.localport = node["localport"].get<uint16_t>();
    }
        
    if (node.find("remoteport") != node.end())
    {
        Settings::configuration.remoteport = node["remoteport"].get<uint16_t>();
    }

    if (node.find("datachannel") != node.end())
    {
        Settings::configuration.datachannel = node["datachannel"].get<std::string>();
    }
    
    if (node.find("remoteip") != node.end())
    {
        Settings::configuration.remoteip = node["remoteip"][0].get<std::string>();
    }
    

    if (node.find("is_ipv4") != node.end()) { Settings::configuration.is_ipv4 = node["is_ipv4"].get<bool>(); }
    
    if (node.find("logLevel") != node.end())
    {  // trace, debug, info, warn
        // TBD // Move logger setting from main to here
        //  Initialize the Logger.

        std::string loglevel = node["logLevel"].get<std::string>();

        base::Level ld = base::getLevelFromString(loglevel.c_str());

#if LOGGING_LOG_TO_FILE
        base::Logger::instance().add(
            new base::RotatingFileChannel("webrtcserver",Settings::configuration.log, ld));
        base::Logger::instance().setWriter(new base::AsyncLogWriter);
#else
        base::Logger::instance().add(new base::ConsoleChannel("webrtcserver",  base::Level::Info));
#endif
    }
    


//    if (cnfg.find("dtlsCertificateFile") != cnfg.end())
//    {
//        Settings::configuration.dtlsCertificateFile = cnfg["dtlsCertificateFile"].get<std::string>();
//    }
//
//
//    if (cnfg.find("dtlsPrivateKeyFile") != cnfg.end())
//    {
//        Settings::configuration.dtlsPrivateKeyFile = cnfg["dtlsPrivateKeyFile"].get<std::string>();
//    }
    
    if (node.find("user") != node.end())
    {
        Settings::configuration.user = node["user"].get<std::string>();
    }
    
    if (node.find("passwd") != node.end())
    {
        Settings::configuration.passwd = node["passwd"].get<std::string>();
    }

   // if (cnfg.find("listenIps") != cnfg.end()) { Settings::configuration.listenIps = cnfg["listenIps"]; }
    

}




#undef LOGGING_LOG_TO_FILE
