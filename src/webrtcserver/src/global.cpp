
//
//#include "plog/Appenders/ColorConsoleAppender.h"
//#include "plog/Converters/UTF8Converter.h"
//#include "plog/Formatters/FuncMessageFormatter.h"
//#include "plog/Formatters/TxtFormatter.h"
//#include "plog/Init.h"
//#include "plog/Log.h"
//#include "plog/Logger.h"
////
//#include "global.hpp"
//
#include "impl/init.hpp"
//
//#include <mutex>
//
//namespace {
//
//void plogInit(plog::Severity severity, plog::IAppender *appender) {
//	using Logger = plog::Logger<PLOG_DEFAULT_INSTANCE_ID>;
//	static Logger *logger = nullptr;
//	if (!logger) {
//		PLOG_DEBUG << "Initializing logger";
//		logger = new Logger(severity);
//		if (appender) {
//			logger->addAppender(appender);
//		} else {
//			using ConsoleAppender = plog::ColorConsoleAppender<plog::TxtFormatter>;
//			static ConsoleAppender *consoleAppender = new ConsoleAppender();
//			logger->addAppender(consoleAppender);
//		}
//	} else {
//		logger->setMaxSeverity(severity);
//		if (appender)
//			logger->addAppender(appender);
//	}
//}
//
//} // namespace
//
namespace rtc {
//
//struct LogAppender : public plog::IAppender {
//	synchronized_callback<LogLevel, string> callback;
//
//	void write(const plog::Record &record) override {
//		const auto severity = record.getSeverity();
//		auto formatted = plog::FuncMessageFormatter::format(record);
//		formatted.pop_back(); // remove newline
//
//		const auto &converted =
//		    plog::UTF8Converter::convert(formatted); // does nothing on non-Windows systems
//
//		if (!callback(static_cast<LogLevel>(severity), converted))
//			std::cout << plog::severityToString(severity) << " " << converted << std::endl;
//	}
//};
//
void InitLogger(LogLevel level, LogCallback callback) {
	
}
//
//void InitLogger(plog::Severity severity, plog::IAppender *appender) {
//	plogInit(severity, appender);
//}
//
void Preload() { impl::Init::Instance().preload(); }
std::shared_future<void> Cleanup() { return impl::Init::Instance().cleanup(); }

void SetSctpSettings(SctpSettings s) { impl::Init::Instance().setSctpSettings(std::move(s)); }

RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out, LogLevel level) {
	switch (level) {
	case LogLevel::Fatal:
		out << "fatal";
		break;
	case LogLevel::Error:
		out << "error";
		break;
	case LogLevel::Warning:
		out << "warning";
		break;
	case LogLevel::Info:
		out << "info";
		break;
	case LogLevel::Debug:
		out << "debug";
		break;
	case LogLevel::Verbose:
		out << "verbose";
		break;
	default:
		out << "none";
		break;
	}
	return out;
}

} // namespace rtc
