
#include "logcounter.hpp"

namespace rtc::impl {

LogCounter::LogCounter(plog::Severity severity, const std::string &text,
                       std::chrono::seconds duration) {
	mData = std::make_shared<LogData>();
	mData->mDuration = duration;
	mData->mSeverity = severity;
	mData->mText = text;
}

LogCounter &LogCounter::operator++(int) {
	if (mData->mCount++ == 0) {
		ThreadPool::Instance().schedule(
		    mData->mDuration,
		    [](weak_ptr<LogData> data) {
			    if (auto ptr = data.lock()) {
				    int countCopy;
				    countCopy = ptr->mCount.exchange(0);
				    PLOG(ptr->mSeverity)
				        << ptr->mText << ": " << countCopy << " (over "
				        << std::chrono::duration_cast<std::chrono::seconds>(ptr->mDuration).count()
				        << " seconds)";
			    }
		    },
		    mData);
	}
	return *this;
}

} // namespace rtc::impl
