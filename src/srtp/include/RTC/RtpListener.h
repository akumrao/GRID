#ifndef MS_RTC_RTP_LISTENER_HPP
#define MS_RTC_RTP_LISTENER_HPP

#include "common.h"
#include "RTC/Producer.h"
#include "RTC/RtpPacket.h"
#include <json.hpp>
#include <string>
#include <unordered_map>

using json = nlohmann::json;

namespace rtc
{
	class RtpListener
	{
	public:
		void FillJson(json& jsonObject) const;
		void AddProducer(rtc::Producer* producer);
		void RemoveProducer(rtc::Producer* producer);
		rtc::Producer* GetProducer(const rtc::RtpPacket* packet);
		rtc::Producer* GetProducer(uint32_t ssrc) const;

	public:
		// Table of SSRC / Producer pairs.
		std::unordered_map<uint32_t, rtc::Producer*> ssrcTable;
		//  Table of MID / Producer pairs.
		std::unordered_map<std::string, rtc::Producer*> midTable;
		//  Table of RID / Producer pairs.
		std::unordered_map<std::string, rtc::Producer*> ridTable;
	};
} // namespace rtc

#endif
