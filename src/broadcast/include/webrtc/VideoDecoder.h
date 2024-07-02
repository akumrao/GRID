// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

//#include "media/engine/internal_encoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "api/video_codecs/video_encoder.h"


#include "rtc_base/critical_section.h"
#include "modules/video_coding/utility/quality_scaler.h"
#include "common_video/h264/h264_bitstream_parser.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "media/base/h264_profile_level_id.h"
//#include "framefilter.h"

#include "api/video_codecs/sdp_video_format.h"
#include "api/video_codecs/video_decoder.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "rtc_base/system/rtc_export.h"


namespace base {
    namespace web_rtc {




        
class FVideoDecoder : public webrtc::VideoDecoder {
 public:
  FVideoDecoder(){}
  virtual ~FVideoDecoder(){};
  int32_t InitDecode(const webrtc::VideoCodec* codec_settings,
                             int32_t number_of_cores){return 0;}

   int32_t Decode(const webrtc::EncodedImage& input_image,
                         bool missing_frames,
                         int64_t render_time_ms) {return 0;}

   int32_t RegisterDecodeCompleteCallback(
      webrtc::DecodedImageCallback* callback) {return 0;}

   int32_t Release(){return 0;};

  // Returns true if the decoder prefer to decode frames late.
  // That is, it can not decode infinite number of frames before the decoded
  // frame is consumed.
   bool PrefersLateDecoding() const{return 0;};

    //char* ImplementationName() const{ return nullptr;}
};
        


class FVideoDecoderFactory : public webrtc::VideoDecoderFactory
{
public:
	//explicit FVideoDecoderFactory(FHWEncoderDetails& HWEncoderDetails);
        
       FVideoDecoderFactory();
	/**
	* This is used from the FPlayerSession::OnSucess to let the factory know
	* what session the next created encoder should belong to.
	* It allows us to get the right FPlayerSession <-> FVideoDecoder relationship
	*/
	//void AddSession(FPlayerSession& PlayerSession);

	//
	// webrtc::VideoEncoderFactory implementation
	//
	//std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
	//CodecInfo QueryVideoEncoder(const webrtc::SdpVideoFormat& Format) const override;
	//std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(const webrtc::SdpVideoFormat& Format) override;
        
        std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
        std::unique_ptr<webrtc::VideoDecoder> CreateVideoDecoder(const webrtc::SdpVideoFormat& format) override;
  

private:
	//FHWEncoderDetails& HWEncoderDetails;
	//TQueue<FPlayerSession*> PendingPlayerSessions;
    
    
     std::vector<webrtc::SdpVideoFormat> supported_codecs;
};


}//ns webrtc
}//base
