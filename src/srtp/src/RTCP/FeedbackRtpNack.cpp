#define MS_CLASS "rtc::RTCP::FeedbackRtpNack"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/RTCP/FeedbackRtpNack.h"
#include "LoggerTag.h"
#include <bitset> // std::bitset()
#include <cstring>

namespace rtc
{
	namespace RTCP
	{
		/* Instance methods. */
		FeedbackRtpNackItem::FeedbackRtpNackItem(uint16_t packetId, uint16_t lostPacketBitmask)
		{
			this->raw    = new uint8_t[sizeof(Header)];
			this->header = reinterpret_cast<Header*>(this->raw);

			this->header->packetId          = uint16_t{ htons(packetId) };
			this->header->lostPacketBitmask = uint16_t{ htons(lostPacketBitmask) };
		}

		size_t FeedbackRtpNackItem::Serialize(uint8_t* buffer)
		{
			

			// Add minimum header.
			std::memcpy(buffer, this->header, sizeof(Header));

			return sizeof(Header);
		}

		void FeedbackRtpNackItem::Dump() const
		{
			

			std::bitset<16> nackBitset(this->GetLostPacketBitmask());

			MS_DUMP("<FeedbackRtpNackItem>");
			MS_DUMP("  pid : %" PRIu16, this->GetPacketId());
			MS_DUMP("  bpl : %s", nackBitset.to_string().c_str());
			MS_DUMP("</FeedbackRtpNackItem>");
		}
	} // namespace RTCP
} // namespace rtc
