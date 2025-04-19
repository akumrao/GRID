

#ifndef RTC_IMPL_SHA_H
#define RTC_IMPL_SHA_H

#if RTC_ENABLE_WEBSOCKET

#include "common.hpp"

namespace rtc::impl {

binary Sha1(const binary &input);
binary Sha1(const string &input);

} // namespace rtc::impl

#endif

#endif
