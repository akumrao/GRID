#ifndef RTC_ICE_SERVER_HPP
#define RTC_ICE_SERVER_HPP

#include "TransportTuple.h"
#include <list>
#include <string>

using namespace base::net;

namespace RTC
{
	class IceServer
	{
	public:
		enum class IceState
		{
			NEW = 1,
			CONNECTED,
			COMPLETED,
			DISCONNECTED
		};

	
	public:
		
		IceState GetState() const;
		TransportTuple* GetSelectedTuple() const;
		
		bool IsValidTuple(const TransportTuple* tuple) const;
		void RemoveTuple(TransportTuple* tuple);
		// This should be just called in 'connected' or completed' state
		// and the given tuple must be an already valid tuple.
		void ForceSelectedTuple(const TransportTuple* tuple);

	private:
		void HandleTuple(TransportTuple* tuple, bool hasUseCandidate);
		/**
		 * Store the given tuple and return its stored address.
		 */
		TransportTuple* AddTuple(TransportTuple* tuple);
		/**
		 * If the given tuple exists return its stored address, nullptr otherwise.
		 */
		TransportTuple* HasTuple(const TransportTuple* tuple) const;
		/**
		 * Set the given tuple as the selected tuple.
		 * NOTE: The given tuple MUST be already stored within the list.
		 */
		void SetSelectedTuple(TransportTuple* storedTuple);

	private:
		// Passed by argument.
		// Others.
                IceState state{ IceState::NEW };
		std::list<TransportTuple> tuples;
		TransportTuple* selectedTuple{ nullptr };
	};

	/* Inline instance methods. */

	inline IceServer::IceState IceServer::GetState() const
	{
		return this->state;
	}

	inline TransportTuple* IceServer::GetSelectedTuple() const
	{
		return this->selectedTuple;
	}

	
} // namespace RTC

#endif
