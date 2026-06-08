#ifndef MS_RTC_DATA_PRODUCER_HPP
#define MS_RTC_DATA_PRODUCER_HPP

#include "common.h"
#include "Channel/Request.h"
#include "RTC/SctpDictionaries.h"
#include <json.hpp>
#include <string>

namespace rtc
{
	class DataProducer
	{
	public:
		class Listener
		{
		public:
			virtual void OnDataProducerSctpMessageReceived(
			  rtc::DataProducer* dataProducer, uint32_t ppid, const uint8_t* msg, size_t len) = 0;
		};

	public:
		DataProducer(const std::string& id, rtc::DataProducer::Listener* listener, json& data);
		virtual ~DataProducer();

	public:
		void FillJson(json& jsonObject) const;
		void FillJsonStats(json& jsonArray) const;
		void HandleRequest(Channel::Request* request);
		 rtc::SctpStreamParameters& GetSctpStreamParameters() ;
		void ReceiveSctpMessage(uint32_t ppid, const uint8_t* msg, size_t len);

	public:
		// Passed by argument.
		const std::string id;
                std::string label;
		std::string protocol;

	private:
		// Passed by argument.
		rtc::DataProducer::Listener* listener{ nullptr };
		// Others.
		rtc::SctpStreamParameters sctpStreamParameters;
		size_t messagesReceived{ 0 };
		size_t bytesReceived{ 0 };
	};

	/* Inline methods. */

	inline  rtc::SctpStreamParameters& DataProducer::GetSctpStreamParameters() 
	{
		return this->sctpStreamParameters;
	}
} // namespace rtc

#endif
