#ifndef MS_RTC_ROUTER_HPP
#define MS_RTC_ROUTER_HPP

#include "common.h"
#include "DataConsumer.h"
#include "DataProducer.h"
#include "Transport.h"
#include <string>
#include <unordered_map>
#include <unordered_set>



namespace RTC
{
	class Router : public RTC::Transport::Listener
	{
	public:
		explicit Router(const std::string& id);
		virtual ~Router();


		void HandleRequest( bool server , const Configuration &config,  int localPort, int remotePort );

	private:


		/* Pure virtual methods inherited from RTC::Transport::Listener. */
	public:


	public:
		// Passed by argument.
		const std::string id;

	private:
		// Allocated by this.
		std::unordered_map<std::string, RTC::Transport*> mapTransports;

		std::unordered_map<std::string, RTC::DataProducer*> mapDataProducers;
	};
} // namespace RTC

#endif
