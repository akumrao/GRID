#include <webrtc/G711Encoder.h>
//#include "base/logger.h"
/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <cstdint>

#include "modules/audio_coding/codecs/g711/g711_interface.h"
#include "rtc_base/checks.h"
#include "rtc_base/time_utils.h"
#include <iostream>

namespace base
{
namespace web_rtc
{


AudioEncoderPcmACAM::AudioEncoderPcmACAM(const Config &config)
    : AudioEncoderPcm(config, kSampleRateHz),
      payload_type_(config.payload_type)
{
    // in_file = fopen("/var/tmp/audio/out.ul", "rb");

    //  frame_buf = new uint8_t[3000];
}

//
// webrtc::AudioEncoder::EncodedInfo  AudioEncoderPcmACAM::Encode(
//    uint32_t rtp_timestamp,
//    rtc::ArrayView<const int16_t> audio,
//    rtc::Buffer* encoded) {
////  TRACE_EVENT0("webrtc", "AudioEncoder::Encode");
////  RTC_CHECK_EQ(audio.size(),
//  //             static_cast<size_t>(NumChannels() * SampleRateHz() / 100));
//
//  const size_t old_size = encoded->size();
//
//  webrtc::AudioEncoder::EncodedInfo info = EncodeImpl(rtp_timestamp, audio, encoded);
// // RTC_CHECK_EQ(encoded->size() - old_size, info.encoded_bytes);
//  return info;
//}


webrtc::AudioEncoder::EncodedInfo AudioEncoderPcmACAM::EncodeImpl(
    uint32_t rtp_timestamp, rtc::ArrayView<const int16_t> audio, rtc::Buffer *encoded)
{
    //    std::cout << " rtp_timestamp "  << rtp_timestamp << std::endl << std::flush;


    uint32_t rtp_timestamp1 = rtp_timestamp * 2;
    //
    //      first_frame_
    //          ? rtp_timestamp
    //          : last_rtp_timestamp_ +   160;
    // // last_timestamp_ = input_data.input_timestamp;
    //  last_rtp_timestamp_ = rtp_timestamp1;
    //  first_frame_ = false;
    //


    //  first_timestamp_in_buffer_ = rtp_timestamp;
    //}
    //  speech_buffer_.insert(speech_buffer_.end(), audio.begin(), audio.end());
    //  if (speech_buffer_.size() < full_frame_samples_) {
    //    return EncodedInfo();
    //  }
    //  RTC_CHECK_EQ(speech_buffer_.size(), full_frame_samples_);
    webrtc::AudioEncoder::EncodedInfo info;
    info.encoded_timestamp = rtp_timestamp1;
    info.payload_type = payload_type_;
    info.encoded_bytes = encoded->AppendData(
        full_frame_samples_ * BytesPerSample(),
        [&](rtc::ArrayView<uint8_t> encoded)
        {
            return EncodeCall(&audio[0], full_frame_samples_, encoded.data());
        });

    info.encoder_type = GetCodecType();
    return info;
}


size_t AudioEncoderPcmACAM::EncodeCall(const int16_t *audio, size_t input_len, uint8_t *encoded)
{
    // int sz =  WebRtcG711_EncodeU(audio, input_len, encoded);

    //    memcpy(frame_buf, audio, 160  );
    //
    //    char *p = (char*)  frame_buf;
    //
    //    char p1 =   frame_buf[0];
    //    char p22 =   frame_buf[81];
    //    char p12 =   frame_buf[82];
    //    char p224 =   frame_buf[83];
    //    char p62 =   frame_buf[159];


    int sz;
    memcpy(encoded, audio, input_len);
    ////////////////////////////////////////////////////
    //    encoded = frame_buf;
    return input_len;
}

size_t AudioEncoderPcmACAM::BytesPerSample() const
{
    return 1;
}

webrtc::AudioEncoder::CodecType AudioEncoderPcmACAM::GetCodecType() const
{
    return AudioEncoder::CodecType::kPcmA;
}








//
//std::vector<AudioDecoder::ParseResult> AudioDecoder::ParsePayload(
//    rtc::Buffer&& payload,
//    uint32_t timestamp) {
//  std::vector<ParseResult> results;
//  std::unique_ptr<EncodedAudioFrame> frame(
//      new OldStyleEncodedFrame(this, std::move(payload)));
//  results.emplace_back(timestamp, 0, std::move(frame));
//  return results;
//}


int audioPlayerWriteFrame( void* pData, const size_t size);
int audioPlayerSetFormat( );
void audioPlayerDestroy();

	
AudioDecoderPcmACAM::AudioDecoderPcmACAM(size_t num_channels) : num_channels_(num_channels) {
    
      
     
      
    #if  defined(__x86_64__) || defined(__arm64__) 

    stream.close();
    stream.open("/tmp/test2.alaw", std::ofstream::out | std::ofstream::binary);

  
    #else 
    audioPlayerDestroy();  
    audioPlayerSetFormat( );
    
    #endif
      
}
   
   



AudioDecoderPcmACAM::~AudioDecoderPcmACAM()
{
    
    #if defined(__x86_64__) || defined(__arm64__) 

     stream.close();
     
    #else 
    
    audioPlayerDestroy();
    
    #endif
	
      
}
  

std::vector<webrtc::AudioDecoder::ParseResult> AudioDecoderPcmACAM::ParsePayload(
    rtc::Buffer&& payload,
    uint32_t timestamp) {
  
    
    
    #if defined(__x86_64__) || defined(__arm64__) 

     stream.write((const char*) payload.data(),  payload.size());// for stereo 
		//stream.write(reinterpret_cast<const char*>(rec_buffer_), number_of_bytes); // for mono audio
     stream.flush();
     
     //printf("audio size  %d", payload.size());

    #else 
     
     audioPlayerWriteFrame( payload.data(),  payload.size());
    
    #endif
	

    std::vector<webrtc::AudioDecoder::ParseResult>  nullResult;
    return nullResult;
    
   
    
    
}

int AudioDecoderPcmACAM::SampleRateHz() const {
    

  return 8000;
}

size_t AudioDecoderPcmACAM::Channels() const {

  return 1;
}


int AudioDecoderPcmACAM::DecodeInternal(const uint8_t* encoded,
                     size_t encoded_len,
                     int sample_rate_hz,
                     int16_t* decoded,
                     SpeechType* speech_type) 
 {
 


    return 0;
    
}





void AudioDecoderPcmACAM::Reset() {}

int AudioDecoderPcmACAM::PacketDuration(const uint8_t* encoded,
                                     size_t encoded_len) const {
    
  
  // One encoded byte per sample per channel.
  return static_cast<int>(encoded_len / Channels());
}





}  // namespace web_rtc
}  // namespace base
