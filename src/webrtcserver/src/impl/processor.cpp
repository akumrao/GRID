
#include "processor.hpp"

namespace rtc::impl {

Processor::Processor(size_t limit) : mTasks(limit) {}

Processor::~Processor() { join(); }

void Processor::join() {
	std::unique_lock lock(mMutex);
	mCondition.wait(lock, [this]() { return !mPending && mTasks.empty(); });
}

void Processor::schedule() {
	std::unique_lock lock(mMutex);
	if (auto next = mTasks.pop()) {
		ThreadPool::Instance().enqueue(std::move(*next));
	} else {
		// No more tasks
		mPending = false;
		mCondition.notify_all();
	}
}

TearDownProcessor &TearDownProcessor::Instance() {
	static TearDownProcessor *instance = new TearDownProcessor;
	return *instance;
}

TearDownProcessor::TearDownProcessor() {}

TearDownProcessor::~TearDownProcessor() {}

} // namespace rtc::impl
