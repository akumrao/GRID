

#ifndef RTC_IMPL_HTTP_H
#define RTC_IMPL_HTTP_H

#include "common.hpp"

#include <list>
#include <map>

namespace rtc::impl {

// Check the buffer contains the beginning of an HTTP request
bool isHttpRequest(const byte *buffer, size_t size);

// Parse an http message into lines
size_t parseHttpLines(const byte *buffer, size_t size, std::list<string> &lines);

// Parse headers of a http message
std::multimap<string, string> parseHttpHeaders(const std::list<string> &lines);

} // namespace rtc::impl

#endif
