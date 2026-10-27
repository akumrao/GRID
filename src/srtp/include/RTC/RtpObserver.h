#ifndef MS_RTC_RTP_PACKET_OBSERVER_HPP
#define MS_RTC_RTP_PACKET_OBSERVER_HPP

#include "common.h"
#include "RTC/Producer.h"
#include "RTC/RtpPacket.h"
#include <string>

namespace rtc
{
	class RtpObserver
	{
	public:
		RtpObserver(const std::string& id);
		virtual ~RtpObserver();

	public:
		void Pause();
		void Resume();
		bool IsPaused() const;
		virtual void AddProducer(rtc::Producer* producer)                              = 0;
		virtual void RemoveProducer(rtc::Producer* producer)                           = 0;
		virtual void ReceiveRtpPacket(rtc::Producer* producer, rtc::RtpPacket* packet) = 0;
		virtual void ProducerPaused(rtc::Producer* producer)                           = 0;
		virtual void ProducerResumed(rtc::Producer* producer)                          = 0;

	protected:
		virtual void Paused()  = 0;
		virtual void Resumed() = 0;

	public:
		// Passed by argument.
		const std::string id;

	private:
		// Others.
		bool paused{ false };
	};

	inline bool RtpObserver::IsPaused() const
	{
		return this->paused;
	}
} // namespace rtc

#endif
