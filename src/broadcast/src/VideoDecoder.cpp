// Copyright Epic Games, Inc. All Rights Reserved.


#include "webrtc/VideoDecoder.h"
#include "base/logger.h"
#include <list>

#include "third_party/openh264/src/codec/api/svc/codec_app_def.h"
#include "common_video/libyuv/include/webrtc_libyuv.h"
#include "modules/video_coding/utility/simulcast_rate_allocator.h"

#include "common_video/h264/sps_parser.h"
#include "common_video/h264/h264_common.h"


#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "modules/video_coding/codecs/h264/include/h264.h"

#include <api/video_codecs/sdp_video_format.h>
#include <media/base/media_constants.h>
#include "absl/strings/match.h"

const size_t kYPlaneIndex = 0;
const size_t kUPlaneIndex = 1;
const size_t kVPlaneIndex = 2;


const uint8_t start_code[4] = {0, 0, 0, 1};

//#include "rtc_base/ref_counted_object.h"
//#include "rtc_base/atomic_ops.h"


inline webrtc::SdpVideoFormat CreateH264Format(webrtc::H264::Profile profile, webrtc::H264::Level level, const std::string &packetization_mode) {
    const absl::optional<std::string> profile_string =
            webrtc::H264::ProfileLevelIdToString(webrtc::H264::ProfileLevelId(profile, level));
    //	check(profile_string);
    return webrtc::SdpVideoFormat
            (
            cricket::kH264CodecName,{
        {cricket::kH264FmtpProfileLevelId, *profile_string},
        {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
        {cricket::kH264FmtpPacketizationMode, packetization_mode}
    }
    );
}

//////////////////////////////////////////////////////////////////////////

namespace base {
    namespace web_rtc {
        
        
        
        bool IsFormatSupported(
                const std::vector<webrtc::SdpVideoFormat>& supported_formats,
                const webrtc::SdpVideoFormat& format) {
            for (const webrtc::SdpVideoFormat& supported_format : supported_formats) {
                if (cricket::IsSameCodec(format.name, format.parameters,
                        supported_format.name,
                        supported_format.parameters)) {
                    return true;
                }
            }
            return false;
        }
        
        ////////////////////////////////////////////////////////////////////////////////////////////// 

        FVideoDecoderFactory::FVideoDecoderFactory() : supported_codecs(webrtc::SupportedH264Codecs()) {
        }

        //void FVideoDecoderFactory::AddSession(FPlayerSession& PlayerSession)
        //{
        //	PendingPlayerSessions.Enqueue(&PlayerSession);
        //}

      

        std::vector<webrtc::SdpVideoFormat> FVideoDecoderFactory::GetSupportedFormats() const {
            std::vector<webrtc::SdpVideoFormat> supported_codecs;

   // if (NULLEncoder::nativeInstance < Settings::encSetting.native)
    {
//        SInfo << NULLEncoder::nativeInstance;

        const absl::optional<std::string> profile_string = webrtc::H264::ProfileLevelIdToString(
            webrtc::H264::ProfileLevelId(webrtc::H264::kProfileMain, webrtc::H264::kLevel3_1));

        supported_codecs.push_back(webrtc::SdpVideoFormat(

            cricket::kH264CodecName,
            {{cricket::kH264FmtpProfileLevelId, *profile_string},
             {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
             {cricket::kH264FmtpPacketizationMode, "1"}
            }
            ));
    }
       
    {
       

        const absl::optional<std::string> profile_string = webrtc::H264::ProfileLevelIdToString(
            webrtc::H264::ProfileLevelId(webrtc::H264::kProfileBaseline, webrtc::H264::kLevel3_1));

            supported_codecs.push_back(webrtc::SdpVideoFormat(
            cricket::kH264CodecName,
            {{cricket::kH264FmtpProfileLevelId, *profile_string},
             {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
             {cricket::kH264FmtpPacketizationMode, "1"}
            }
            ));
    }
    
    
    {
        const absl::optional<std::string> profile_string = webrtc::H264::ProfileLevelIdToString(
            webrtc::H264::ProfileLevelId(webrtc::H264::kProfileBaseline, webrtc::H264::kLevel3_1));
        supported_codecs.push_back(webrtc::SdpVideoFormat(

            cricket::kH264CodecName,
            {{cricket::kH264FmtpProfileLevelId, *profile_string},
             {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
             {cricket::kH264FmtpPacketizationMode, "0"}
            }
            ));
    }

    
    
    
    
    {
       

        const absl::optional<std::string> profile_string = webrtc::H264::ProfileLevelIdToString(
            webrtc::H264::ProfileLevelId(webrtc::H264::kProfileConstrainedBaseline, webrtc::H264::kLevel3_1));

        supported_codecs.push_back(webrtc::SdpVideoFormat(

            cricket::kH264CodecName,
            {{cricket::kH264FmtpProfileLevelId, *profile_string},
             {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
             {cricket::kH264FmtpPacketizationMode, "1"}
            }
            ));
    }
    
    
    {
       

        const absl::optional<std::string> profile_string = webrtc::H264::ProfileLevelIdToString(
            webrtc::H264::ProfileLevelId(webrtc::H264::kProfileConstrainedBaseline, webrtc::H264::kLevel3_1));

        supported_codecs.push_back(webrtc::SdpVideoFormat(

            cricket::kH264CodecName,
            {{cricket::kH264FmtpProfileLevelId, *profile_string},
             {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
             {cricket::kH264FmtpPacketizationMode, "0"}
            }
            ));
    }
       
  
    

    return supported_codecs;
            
            
}

  template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

 std::unique_ptr<webrtc::VideoDecoder> FVideoDecoderFactory::CreateVideoDecoder(const webrtc::SdpVideoFormat& format) {
            //FPlayerSession* Session;
            //bool res = PendingPlayerSessions.Dequeue(Session);
            //checkf(res, TEXT("no player session associated with encoder instance"));

            
     
           auto VideoEncoder = make_unique<FVideoDecoder>();  // for cam encoders
            return VideoEncoder;
//            
//            
//           //  std::unique_ptr<webrtc::VideoDecoder> = std::make_unique<FVideoDecoder>();   // for cam encoders
//           // return VideoEncoder;
//
//
//            if (!IsFormatSupported(GetSupportedFormats(), format)) {
//               SInfo << "Trying to create decoder for unsupported format";
//                return nullptr;
//            }
//
////            if (absl::EqualsIgnoreCase(format.name, cricket::kVp8CodecName))
////                return VP8Decoder::Create();
////            if (absl::EqualsIgnoreCase(format.name, cricket::kVp9CodecName))
////                return VP9Decoder::Create();
//            if (absl::EqualsIgnoreCase(format.name, cricket::kH264CodecName))
//                return webrtc::H264Decoder::Create();
//
//            RTC_NOTREACHED();
            return nullptr;
            
            
}
    }// ns webrtc
}//ns base
