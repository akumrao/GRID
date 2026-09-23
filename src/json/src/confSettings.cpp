/*  For testing reset api use Postman
 * https://web.postman.co/workspace/My-Workspace~292b44c7-cae4-44d6-8253-174622f0233e/request/create?requestId=e6995876-3b8c-4b7e-b170-83a733a631db
 */


#include "json/confSettings.h"
#include "base/error.h"
#include "base/logger.h"


#include <cctype>  // isprint()
#include <cerrno>
#include <iterator>  // std::ostream_iterator
#include <sstream>  // std::ostringstream

//#define LOGGING_LOG_TO_FILE 1
/* Class variables. */

struct ConfSettings::Configuration ConfSettings::configuration;


void ConfSettings::SetConfiguration(json &cnfg)
{


 
    if (cnfg.find("log") != cnfg.end())
    {
        ConfSettings::configuration.log = cnfg["log"];
    }
        
    
    if (cnfg.find("logLevel") != cnfg.end())
    {  // trace, debug, info, warn
        // TBD // Move logger setting from main to here
        //  Initialize the Logger.

        std::string loglevel = cnfg["logLevel"].get<std::string>();

        base::Level ld = base::getLevelFromString(loglevel.c_str());

#if LOGGING_LOG_TO_FILE
        base::Logger::instance().add(
            new base::RotatingFileChannel("webrtcserver",ConfSettings::configuration.log, ld));
        base::Logger::instance().setWriter(new base::AsyncLogWriter);
#else
        base::Logger::instance().add(new base::ConsoleChannel("webrtcserver", ld));
#endif
    }
    


    if (cnfg.find("certFile") != cnfg.end())
    {
        ConfSettings::configuration.certFile = cnfg["certFile"].get<std::string>();
    }


    if (cnfg.find("keyFile") != cnfg.end())
    {
        ConfSettings::configuration.keyFile = cnfg["keyFile"].get<std::string>();
    }
    
    

   // if (cnfg.find("listenIps") != cnfg.end()) { ConfSettings::configuration.listenIps = cnfg["listenIps"]; }
    
    // Parsing logic for the new configuration parameters
    if (cnfg.find("serverdtsRole") != cnfg.end()) { ConfSettings::configuration.serverdtsRole = cnfg["serverdtsRole"].get<bool>(); }
    if (cnfg.find("enableTcp") != cnfg.end()) { ConfSettings::configuration.enableTcp = cnfg["enableTcp"].get<bool>(); }
    if (cnfg.find("enableUdp") != cnfg.end()) { ConfSettings::configuration.enableUdp = cnfg["enableUdp"].get<bool>(); }
    if (cnfg.find("publicIP") != cnfg.end()) { ConfSettings::configuration.publicIP = cnfg["publicIP"].get<bool>(); }
    if (cnfg.find("noPivateIP") != cnfg.end()) { ConfSettings::configuration.noPivateIP = cnfg["noPivateIP"].get<bool>(); }
    
    if (cnfg.find("websoc_host") != cnfg.end()) { ConfSettings::configuration.websoc_host = cnfg["websoc_host"].get<std::string>(); }
    if (cnfg.find("websoc_port") != cnfg.end()) { ConfSettings::configuration.websoc_port = cnfg["websoc_port"].get<int>(); }
}

#undef LOGGING_LOG_TO_FILE
