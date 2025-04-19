

#ifndef RTC_PLI_RESPONDER_H
#define RTC_PLI_RESPONDER_H

#if RTC_ENABLE_MEDIA

#include "mediahandler.hpp"
#include "utils.hpp"

namespace rtc {

/// Responds to PLI and FIR messages sent by the receiver. The sender should respond to these
/// messages by sending an intra.
class RTC_CPP_EXPORT PliHandler final : public MediaHandler {
    rtc::synchronized_callback<> mOnPli;

public:
	/// Constructs the PLIResponder object to notify whenever a new intra frame is requested
	/// @param onPli The callback that gets called whenever an intra frame is requested by the receiver
    PliHandler(std::function<void(void)> onPli);

	void incoming(message_vector &messages, const message_callback &send) override;
};

}

#endif // RTC_ENABLE_MEDIA

#endif // RTC_PLI_RESPONDER_H
