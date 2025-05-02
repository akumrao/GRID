/**
 * Copyright (c) 2019 Paul-Louis Ageneau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RTC_DATA_CHANNEL_H
#define RTC_DATA_CHANNEL_H

#include "channel.h"
#include "common.h"
#include "reliability.h"
#include "peerconnection.h"
#include <type_traits>

namespace rtc {


struct DataChannel;
struct PeerConnection;


class RTC_CPP_EXPORT DataChannel final : public Channel {
public:
	DataChannel();
	~DataChannel() override;

	uint16_t stream() const;
	uint16_t id() const;
	string label() const;
	string protocol() const;
	Reliability reliability() const;

	bool isOpen(void) const override;
	bool isClosed(void) const override;
	size_t maxMessageSize() const override;

	void close(void) override;
	bool send(std::vector<unsigned char> data) override;
	bool send(const unsigned char *data, size_t size) override;
	template <typename Buffer> bool sendBuffer(const Buffer &buf);
	template <typename Iterator> bool sendBuffer(Iterator first, Iterator last);

private:
	
};

//template <typename Buffer> std::pair<const unsigned char *, size_t> to_unsigned chars(const Buffer &buf) {
//	using T = typename std::remove_pointer<decltype(buf.data())>::type;
//	using E = typename std::conditional<std::is_void<T>::value, unsigned char, T>::type;
//	return std::make_pair(static_cast<const unsigned char *>(static_cast<const void *>(buf.data())),
//	                      buf.size() * sizeof(E));
//}

//template <typename Buffer> bool DataChannel::sendBuffer(const Buffer &buf) {
//	auto [unsigned chars, size] = to_unsigned chars(buf);
//	return send(unsigned chars, size);
//}
//
//template <typename Iterator> bool DataChannel::sendBuffer(Iterator first, Iterator last) {
//	size_t size = 0;
//	for (Iterator it = first; it != last; ++it)
//		size += it->size();
//
//	binary buffer(size);
//	unsigned char *pos = buffer.data();
//	for (Iterator it = first; it != last; ++it) {
//		auto [unsigned chars, len] = to_unsigned chars(*it);
//		pos = std::copy(unsigned chars, unsigned chars + len, pos);
//	}
//	return send(std::move(buffer));
//}


/*
struct OutgoingDataChannel final : public DataChannel {
	OutgoingDataChannel(weak_ptr<PeerConnection> pc, string label, string protocol,
	                    Reliability reliability);
	~OutgoingDataChannel();

	void open(shared_ptr<SctpTransport> transport) override;
	void processOpenMessage(message_ptr message) override;
};

struct IncomingDataChannel final : public DataChannel {
	IncomingDataChannel(weak_ptr<PeerConnection> pc, weak_ptr<SctpTransport> transport);
	~IncomingDataChannel();

	void open(shared_ptr<SctpTransport> transport) override;
	void processOpenMessage(message_ptr message) override;
};

*/

} // namespace rtc

#endif
