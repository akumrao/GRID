#ifndef MS_RTC_AUDIO_LEVEL_OBSERVER_HPP
#define MS_RTC_AUDIO_LEVEL_OBSERVER_HPP

#include "RTC/RtpObserver.h"
#include "base/Timer.h"
#include <json.hpp>
#include <unordered_map>

using namespace base;
using json = nlohmann::json;

namespace rtc
{
	class AudioLevelObserver : public rtc::RtpObserver, public Timer::Listener
	{
	private:
		struct DBovs
		{
			uint16_t totalSum{ 0 }; // Sum of dBvos (positive integer).
			size_t count{ 0 };      // Number of dBvos entries in totalSum.
		};

	public:
		AudioLevelObserver(const std::string& id, json& data);
		~AudioLevelObserver() override;

	public:
		void AddProducer(rtc::Producer* producer) override;
		void RemoveProducer(rtc::Producer* producer) override;
		void ReceiveRtpPacket(rtc::Producer* producer, rtc::RtpPacket* packet) override;
		void ProducerPaused(rtc::Producer* producer) override;
		void ProducerResumed(rtc::Producer* producer) override;

	private:
		void Paused() override;
		void Resumed() override;
		void Update();
		void ResetMapProducerDBovs();

		/* Pure virtual methods inherited from Timer. */
	protected:
		void OnTimer(Timer* timer) override;

	private:
		// Passed by argument.
		uint16_t maxEntries{ 1 };
		int8_t threshold{ -80 };
		uint16_t interval{ 1000 };
		// Allocated by this.
		Timer* periodicTimer{ nullptr };
		// Others.
		std::unordered_map<rtc::Producer*, DBovs> mapProducerDBovs;
		bool silence{ true };
	};
} // namespace rtc

#endif
