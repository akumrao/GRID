

#ifndef RTC_IMPL_INIT_H
#define RTC_IMPL_INIT_H

#include "common.hpp"
#include "global.hpp" // for SctpSettings

#include <chrono>
#include <future>
#include <mutex>

namespace rtc::impl {

using init_token = shared_ptr<void>;

class Init {
public:
	static Init &Instance();

	Init(const Init &) = delete;
	Init &operator=(const Init &) = delete;
	Init(Init &&) = delete;
	Init &operator=(Init &&) = delete;

	init_token token();
	void preload();
	std::shared_future<void> cleanup();
	void setSctpSettings(SctpSettings s);

private:
	Init();
	~Init();

	void doInit();
	void doCleanup();

	std::optional<shared_ptr<void>> mGlobal;
	weak_ptr<void> mWeak;
	bool mInitialized = false;
	SctpSettings mCurrentSctpSettings = {};
	std::mutex mMutex;
	std::shared_future<void> mCleanupFuture;

	struct TokenPayload;
};

} // namespace rtc::impl

#endif
