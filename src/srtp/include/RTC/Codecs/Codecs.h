#ifndef MS_RTC_CODECS_HPP
#define MS_RTC_CODECS_HPP

#include "common.h"
#include "RTC/Codecs/H264.h"
#include "RTC/Codecs/PayloadDescriptorHandler.h"
#include "RTC/Codecs/VP8.h"
#include "RTC/Codecs/VP9.h"
#include "RTC/RtpDictionaries.h"
#include "RTC/RtpPacket.h"

namespace rtc
{
	namespace Codecs
	{
		bool CanBeKeyFrame(const rtc::RtpCodecMimeType& mimeType);
		void ProcessRtpPacket(rtc::RtpPacket* packet, const rtc::RtpCodecMimeType& mimeType);
		bool IsValidTypeForCodec(rtc::RtpParameters::Type type, const rtc::RtpCodecMimeType& mimeType);
		EncodingContext* GetEncodingContext(
		  const rtc::RtpCodecMimeType& mimeType, uint8_t spatialLayers, uint8_t temporalLayers);

		/* Inline namespace methods. */

		inline bool CanBeKeyFrame(const rtc::RtpCodecMimeType& mimeType)
		{
			switch (mimeType.type)
			{
				case rtc::RtpCodecMimeType::Type::VIDEO:
				{
					switch (mimeType.subtype)
					{
						case rtc::RtpCodecMimeType::Subtype::VP8:
						case rtc::RtpCodecMimeType::Subtype::VP9:
						case rtc::RtpCodecMimeType::Subtype::H264:
							return true;
						default:
							return false;
					}
				}

				default:
				{
					return false;
				}
			}
		}

		inline bool IsValidTypeForCodec(rtc::RtpParameters::Type type, const rtc::RtpCodecMimeType& mimeType)
		{
			switch (type)
			{
				case rtc::RtpParameters::Type::NONE:
				{
					return false;
				}

				case rtc::RtpParameters::Type::SIMPLE:
				{
					return true;
				}

				case rtc::RtpParameters::Type::SIMULCAST:
				{
					switch (mimeType.type)
					{
						case rtc::RtpCodecMimeType::Type::VIDEO:
						{
							switch (mimeType.subtype)
							{
								case rtc::RtpCodecMimeType::Subtype::VP8:
								case rtc::RtpCodecMimeType::Subtype::H264:
									return true;
								default:
									return false;
							}
						}

						default:
						{
							return false;
						}
					}
				}

				case rtc::RtpParameters::Type::SVC:
				{
					switch (mimeType.type)
					{
						case rtc::RtpCodecMimeType::Type::VIDEO:
						{
							switch (mimeType.subtype)
							{
								case rtc::RtpCodecMimeType::Subtype::VP9:
									return true;
								default:
									return false;
							}
						}

						default:
						{
							return false;
						}
					}
				}

				case rtc::RtpParameters::Type::PIPE:
				{
					return true;
				}

				default:
				{
					return false;
				}
			}
		}

		inline EncodingContext* GetEncodingContext(
		  const rtc::RtpCodecMimeType& mimeType, rtc::Codecs::EncodingContext::Params& params)
		{
			switch (mimeType.type)
			{
				case rtc::RtpCodecMimeType::Type::VIDEO:
				{
					switch (mimeType.subtype)
					{
						case rtc::RtpCodecMimeType::Subtype::VP8:
							return new rtc::Codecs::VP8::EncodingContext(params);
						case rtc::RtpCodecMimeType::Subtype::VP9:
							return new rtc::Codecs::VP9::EncodingContext(params);
						case rtc::RtpCodecMimeType::Subtype::H264:
							return new rtc::Codecs::H264::EncodingContext(params);
						default:
							return nullptr;
					}
				}

				default:
				{
					return nullptr;
				}
			}
		}
	} // namespace Codecs
} // namespace rtc

#endif
