#ifndef MS_RTC_CODECS_VP8_HPP
#define MS_RTC_CODECS_VP8_HPP

#include "common.h"
#include "RTC/Codecs/PayloadDescriptorHandler.h"
#include "RTC/RtpPacket.h"
#include "RTC/SeqManager.h"

/* RFC 7741
 * VP8 Payload Descriptor

  Single octet PictureID (M = 0)        Dual octet PictureID (M = 1)
  ==============================        ============================

      0 1 2 3 4 5 6 7                       0 1 2 3 4 5 6 7
     +-+-+-+-+-+-+-+-+                     +-+-+-+-+-+-+-+-+
     |X|R|N|S|R| PID | (REQUIRED)          |X|R|N|S|R| PID | (REQUIRED)
     +-+-+-+-+-+-+-+-+                     +-+-+-+-+-+-+-+-+
X:   |I|L|T|K| RSV   | (OPTIONAL)       X: |I|L|T|K| RSV   | (OPTIONAL)
     +-+-+-+-+-+-+-+-+                     +-+-+-+-+-+-+-+-+
I:   |M| PictureID   | (OPTIONAL)       I: |M| PictureID   | (OPTIONAL)
     +-+-+-+-+-+-+-+-+                     +-+-+-+-+-+-+-+-+
L:   |   TL0PICIDX   | (OPTIONAL)          |   PictureID   |
     +-+-+-+-+-+-+-+-+                     +-+-+-+-+-+-+-+-+
T/K: |TID|Y| KEYIDX  | (OPTIONAL)       L: |   TL0PICIDX   | (OPTIONAL)
     +-+-+-+-+-+-+-+-+                     +-+-+-+-+-+-+-+-+
                                      T/K: |TID|Y| KEYIDX  | (OPTIONAL)
                                           +-+-+-+-+-+-+-+-+
 */

namespace rtc
{
	namespace Codecs
	{
		class VP8
		{
		public:
			struct PayloadDescriptor : public rtc::Codecs::PayloadDescriptor
			{
				/* Pure virtual methods inherited from rtc::Codecs::PayloadDescriptor. */
				~PayloadDescriptor() = default;

				void Dump() const override;
				// Rewrite the buffer with the given pictureId and tl0PictureIndex values.
				void Encode(uint8_t* data, uint16_t pictureId, uint8_t tl0PictureIndex) const;
				void Restore(uint8_t* data) const;

				// Mandatory fields.
				uint8_t extended : 1;
				uint8_t nonReference : 1;
				uint8_t start : 1;
				uint8_t partitionIndex : 4;
				// Optional field flags.
				uint8_t i : 1; // PictureID present.
				uint8_t l : 1; // TL0PICIDX present.
				uint8_t t : 1; // TID present.
				uint8_t k : 1; // KEYIDX present.
				// Optional fields.
				uint16_t pictureId;
				uint8_t tl0PictureIndex;
				uint8_t tlIndex : 2;
				uint8_t y : 1;
				uint8_t keyIndex : 5;
				// Parsed values.
				bool isKeyFrame{ false };
				bool hasPictureId{ false };
				bool hasOneBytePictureId{ false };
				bool hasTwoBytesPictureId{ false };
				bool hasTl0PictureIndex{ false };
				bool hasTlIndex{ false };
			};

		public:
			static VP8::PayloadDescriptor* Parse(
			  const uint8_t* data,
			  size_t len,
			  rtc::RtpPacket::FrameMarking* frameMarking = nullptr,
			  uint8_t frameMarkingLen                    = 0);
			static void ProcessRtpPacket(rtc::RtpPacket* packet);

		public:
			class EncodingContext : public rtc::Codecs::EncodingContext
			{
			public:
				explicit EncodingContext(rtc::Codecs::EncodingContext::Params& params);
				~EncodingContext() = default;

				/* Pure virtual methods inherited from rtc::Codecs::EncodingContext. */
			public:
				void SyncRequired() override;

			public:
				rtc::SeqManager<uint16_t> pictureIdManager;
				rtc::SeqManager<uint8_t> tl0PictureIndexManager;
				bool syncRequired{ false };
			};

		public:
			class PayloadDescriptorHandler : public rtc::Codecs::PayloadDescriptorHandler
			{
			public:
				explicit PayloadDescriptorHandler(PayloadDescriptor* payloadDescriptor);
				~PayloadDescriptorHandler() = default;

			public:
				void Dump() const override;
				bool Process(rtc::Codecs::EncodingContext* encodingContext, uint8_t* data, bool& marker) override;
				void Restore(uint8_t* data) override;
				uint8_t GetSpatialLayer() const override;
				uint8_t GetTemporalLayer() const override;
				bool IsKeyFrame() const override;

			private:
				std::unique_ptr<PayloadDescriptor> payloadDescriptor;
			};
		};

		/* Inline EncondingContext methods. */

		inline VP8::EncodingContext::EncodingContext(rtc::Codecs::EncodingContext::Params& params)
		  : rtc::Codecs::EncodingContext(params)
		{
		}

		inline void VP8::EncodingContext::SyncRequired()
		{
			this->syncRequired = true;
		}

		/* Inline PayloadDescriptorHandler methods. */

		inline void VP8::PayloadDescriptorHandler::Dump() const
		{
			this->payloadDescriptor->Dump();
		}

		inline uint8_t VP8::PayloadDescriptorHandler::GetSpatialLayer() const
		{
			return 0u;
		}

		inline uint8_t VP8::PayloadDescriptorHandler::GetTemporalLayer() const
		{
			return this->payloadDescriptor->hasTlIndex ? this->payloadDescriptor->tlIndex : 0u;
		}

		inline bool VP8::PayloadDescriptorHandler::IsKeyFrame() const
		{
			return this->payloadDescriptor->isKeyFrame;
		}
	} // namespace Codecs
} // namespace rtc

#endif
