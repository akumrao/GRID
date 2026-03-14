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
    
    
    if (cnfg.find("server") != cnfg.end())
    {
        ConfSettings::configuration.server = cnfg["server"].get<std::string>();
    }


    if (cnfg.find("port") != cnfg.end()) { ConfSettings::configuration.port = cnfg["port"].get<int>(); }

   // if (cnfg.find("listenIps") != cnfg.end()) { ConfSettings::configuration.listenIps = cnfg["listenIps"]; }
    
    

}



#undef LOGGING_LOG_TO_FILE
