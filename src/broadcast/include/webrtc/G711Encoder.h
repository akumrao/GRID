/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef CODECS_G711_AUDIO_ENCODER_PCM_H_
#define CODECS_G711_AUDIO_ENCODER_PCM_H_

#include <vector>
#if  defined(__x86_64__) || defined(__arm64__) 
#include <fstream>
#endif

#include "modules/audio_coding/codecs/g711/audio_encoder_pcm.h"

#include "modules/audio_coding/codecs/g711/audio_decoder_pcm.h"
namespace base
{
namespace web_rtc
{


class AudioEncoderPcmACAM : public webrtc::AudioEncoderPcm
{
public:
    struct Config : public AudioEncoderPcm::Config
    {
        Config() : AudioEncoderPcm::Config(0) {}
    };

    explicit AudioEncoderPcmACAM(const Config &config);


    webrtc::AudioEncoder::EncodedInfo
        EncodeImpl(uint32_t rtp_timestamp, rtc::ArrayView<const int16_t> audio, rtc::Buffer *encoded);


    // webrtc::AudioEncoder::EncodedInfo Encode(  uint32_t rtp_timestamp,    rtc::ArrayView<const int16_t>
    // audio,    rtc::Buffer* encoded);


protected:
    size_t EncodeCall(const int16_t *audio, size_t input_len, uint8_t *encoded) override;

    size_t BytesPerSample() const override;

    AudioEncoder::CodecType GetCodecType() const override;

private:
    static const int kSampleRateHz = 8000;
    RTC_DISALLOW_COPY_AND_ASSIGN(AudioEncoderPcmACAM);

    // FILE* in_file;

    uint8_t frame_buf[160];

    int ncount{0};

    int64_t _nextFrameTime{0};

    int64_t diff{0};


    int payload_type_;

    int full_frame_samples_{160};


    // bool first_frame_ {true};
    // uint32_t last_timestamp_ ;
    uint32_t last_rtp_timestamp_;
};


class AudioDecoderPcmACAM  : public webrtc::AudioDecoder {
 public:
  explicit AudioDecoderPcmACAM(size_t num_channels); 
  
  ~AudioDecoderPcmACAM();
  
  void Reset() override;
  std::vector<webrtc::AudioDecoder::ParseResult> ParsePayload(rtc::Buffer&& payload,
                                        uint32_t timestamp) override;
  int PacketDuration(const uint8_t* encoded, size_t encoded_len) const override;
  int SampleRateHz() const override;
  size_t Channels() const override;

 protected:
  int DecodeInternal(const uint8_t* encoded,
                     size_t encoded_len,
                     int sample_rate_hz,
                     int16_t* decoded,
                     SpeechType* speech_type) override;

 private:
  const size_t num_channels_;
  
  #if  defined(__x86_64__) || defined(__arm64__) 
  std::ofstream stream;
  #endif  

   
  //RTC_DISALLOW_COPY_AND_ASSIGN(AudioDecoderPcmACAM);
};

}  // namespace web_rtc
}  // namespace base

#endif
