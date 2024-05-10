 
#include "live.h"
#include "constant.h"
//#include "tools.h"
#include "base/logger.h"
#include "livethread.h"
#include "Settings.h"

#define SEND_PARAMETER_SETS // keep this always defined
 static const unsigned LIVE_GET_PARAMETER_PING = 50;
 
// enum PayloadSizes {
//  DEFAULT_PAYLOAD_SIZE            = 1024,    ///< Default buffer size in Live555 for h264 // debug // not used anymore
//  
//  // DEFAULT_PAYLOAD_SIZE_H264       = 1024*100, ///< Default buffer size in Live555 for h264
//  DEFAULT_PAYLOAD_SIZE_H264       = 1024*500, ///< Default buffer size in Live555 for h264 
//  // DEFAULT_PAYLOAD_SIZE_H264       = 1024*500, ///< Default buffer size in Live555 for h264
//  // DEFAULT_PAYLOAD_SIZE_H264       = 1024*10,  ///< Default buffer size in Live555 for h264 // debug
//  // DEFAULT_PAYLOAD_SIZE_H264       = 1024, // use this small value for debugging (( debug
//  
//  DEFAULT_PAYLOAD_SIZE_PCMU       = 1024,    ///< Default buffer size in Live555 for pcmu
//};

 
namespace base {
    namespace web_rtc {






// Implementation of "FrameSink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 600000






}
}
