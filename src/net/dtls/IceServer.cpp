

#include <utility>
#include "base/logger.h"
#include "IceServer.h"

namespace RTC
{

	


	bool IceServer::IsValidTuple(const TransportTuple* tuple) const
	{

		return HasTuple(tuple) != nullptr;
	}

	void IceServer::RemoveTuple(TransportTuple* tuple)
	{
		

		TransportTuple* removedTuple{ nullptr };

		// Find the removed tuple.
		auto it = this->tuples.begin();

		for (; it != this->tuples.end(); ++it)
		{
			TransportTuple* storedTuple = std::addressof(*it);

			if (storedTuple->Compare(tuple))
			{
				removedTuple = storedTuple;

				break;
			}
		}

		// If not found, ignore.
		if (!removedTuple)
			return;

		// Remove from the list of tuples.
		this->tuples.erase(it);

		// If this is not the selected tuple, stop here.
		if (removedTuple != this->selectedTuple)
			return;

		// Otherwise this was the selected tuple.
		this->selectedTuple = nullptr;

		// Mark the first tuple as selected tuple (if any).
		if (this->tuples.begin() != this->tuples.end())
		{
			SetSelectedTuple(std::addressof(*this->tuples.begin()));
		}
		// Or just emit 'disconnected'.
		else
		{
			// Update state.
			this->state = IceState::DISCONNECTED;
			// Notify the listener.
			//this->listener->OnIceServerDisconnected(this);
		}
	}

	void IceServer::ForceSelectedTuple(const TransportTuple* tuple)
	{
		

		auto* storedTuple = HasTuple(tuple);

		// Mark it as selected tuple.
		SetSelectedTuple(storedTuple);
	}

	void IceServer::HandleTuple(TransportTuple* tuple, bool hasUseCandidate)
	{
		


	}

	inline TransportTuple* IceServer::AddTuple(TransportTuple* tuple)
	{
		

		// Add the new tuple at the beginning of the list.
		this->tuples.push_front(*tuple);

		auto* storedTuple = std::addressof(*this->tuples.begin());

		// If it is UDP then we must store the remote address (until now it is
		// just a pointer that will be freed soon).
		if (storedTuple->GetProtocol() == TransportTuple::Protocol::UDP)
			storedTuple->StoreUdpRemoteAddress();

		// Return the address of the inserted tuple.
		return storedTuple;
	}

	inline TransportTuple* IceServer::HasTuple(const TransportTuple* tuple) const
	{
		

		// If there is no selected tuple yet then we know that the tuples list
		// is empty.
		if (!this->selectedTuple)
			return nullptr;

		// Check the current selected tuple.
		if (this->selectedTuple->Compare(tuple))
			return this->selectedTuple;

		// Otherwise check other stored tuples.
		for (const auto& it : this->tuples)
		{
			auto* storedTuple = const_cast<TransportTuple*>(std::addressof(it));

			if (storedTuple->Compare(tuple))
				return storedTuple;
		}

		return nullptr;
	}

	inline void IceServer::SetSelectedTuple(TransportTuple* storedTuple)
	{
		

		// If already the selected tuple do nothing.
		if (storedTuple == this->selectedTuple)
			return;

		this->selectedTuple = storedTuple;

		// Notify the listener.
		//this->listener->OnIceServerSelectedTuple(this, this->selectedTuple);
	}
} // namespace RTC
