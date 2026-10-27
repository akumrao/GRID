

#include "RTC/RtpObserver.h"
#include "LoggerTag.h"

namespace rtc
{
	/* Instance methods. */

	RtpObserver::RtpObserver(const std::string& id) : id(id)
	{
		
	}

	RtpObserver::~RtpObserver()
	{
		
	}

	void RtpObserver::Pause()
	{
		

		if (this->paused)
			return;

		this->paused = true;

		Paused();
	}

	void RtpObserver::Resume()
	{
		

		if (!this->paused)
			return;

		this->paused = false;

		Resumed();
	}
} // namespace rtc
