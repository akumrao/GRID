/**
 * Copyright (c) 2019-2021 Paul-Louis Ageneau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "datachannel.h"
#include "common.h"
//#include "peerconnection.hpp"

//#include "impl/datachannel.hpp"
//#include "impl/internals.hpp"
//#include "impl/peerconnection.hpp"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace rtc {

DataChannel::DataChannel()
   {}

DataChannel::~DataChannel() {}

void DataChannel::close() { return close(); }

uint16_t DataChannel::stream() const { return stream(); }

uint16_t DataChannel::id() const { return stream(); }

string DataChannel::label() const { return label(); }

string DataChannel::protocol() const { return protocol(); }

Reliability DataChannel::reliability() const { return reliability(); }

bool DataChannel::isOpen(void) const { return isOpen(); }

bool DataChannel::isClosed(void) const { return isClosed(); }

size_t DataChannel::maxMessageSize() const { return maxMessageSize(); }

bool DataChannel::send(std::vector<unsigned char> data) {
	//return outgoing(make_message(std::move(data)));
}

bool DataChannel::send(const unsigned char *data, size_t size) {
	//return outgoing(std::make_shared<Message>(data, data + size, Message::Binary));
}

} // namespace rtc
