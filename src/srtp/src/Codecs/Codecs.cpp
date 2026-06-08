#define MS_CLASS "rtc::Codecs"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/Codecs/Codecs.h"
#include "LoggerTag.h"
#include "RTC/Codecs/VP8.h"

namespace rtc
{
	namespace Codecs
	{
		void ProcessRtpPacket(rtc::RtpPacket* packet, const rtc::RtpCodecMimeType& mimeType)
		{
			

			switch (mimeType.type)
			{
				case rtc::RtpCodecMimeType::Type::VIDEO:
				{
					switch (mimeType.subtype)
					{
						case rtc::RtpCodecMimeType::Subtype::VP8:
						{
							rtc::Codecs::VP8::ProcessRtpPacket(packet);

							break;
						}

						case rtc::RtpCodecMimeType::Subtype::VP9:
						{
							rtc::Codecs::VP9::ProcessRtpPacket(packet);

							break;
						}

						case rtc::RtpCodecMimeType::Subtype::H264:
						{
							rtc::Codecs::H264::ProcessRtpPacket(packet);

							break;
						}

						default:;
					}
				}

				default:;
			}
		}
	} // namespace Codecs
} // namespace rtc
