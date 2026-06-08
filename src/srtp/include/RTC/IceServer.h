#ifndef MS_RTC_ICE_SERVER_HPP
#define MS_RTC_ICE_SERVER_HPP

#include "common.h"
#include "RTC/StunPacket.h"
#include "RTC/TransportTuple.h"
#include <list>
#include <string>

namespace rtc
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
		class Listener
		{
		public:
			virtual ~Listener() = default;

		public:
			/**
			 * These callbacks are guaranteed to be called before ProcessStunPacket()
			 * returns, so the given pointers are still usable.
			 */
			virtual void OnIceServerSendStunPacket(
			  const rtc::IceServer* iceServer, const rtc::StunPacket* packet, rtc::TransportTuple* tuple) = 0;
			virtual void OnIceServerSelectedTuple(
			  const rtc::IceServer* iceServer, rtc::TransportTuple* tuple)        = 0;
			virtual void OnIceServerConnected(const rtc::IceServer* iceServer)    = 0;
			virtual void OnIceServerCompleted(const rtc::IceServer* iceServer)    = 0;
			virtual void OnIceServerDisconnected(const rtc::IceServer* iceServer) = 0;
		};

	public:
		IceServer(Listener* listener, const std::string& usernameFragment, const std::string& password);

	public:
		void ProcessStunPacket(rtc::StunPacket* packet, rtc::TransportTuple* tuple);
		const std::string& GetUsernameFragment() const;
		const std::string& GetPassword() const;
		IceState GetState() const;
		rtc::TransportTuple* GetSelectedTuple() const;
		void SetUsernameFragment(const std::string& usernameFragment);
		void SetPassword(const std::string& password);
		bool IsValidTuple(const rtc::TransportTuple* tuple) const;
		void RemoveTuple(rtc::TransportTuple* tuple);
		// This should be just called in 'connected' or completed' state
		// and the given tuple must be an already valid tuple.
		void ForceSelectedTuple(const rtc::TransportTuple* tuple);

	private:
		void HandleTuple(rtc::TransportTuple* tuple, bool hasUseCandidate);
		/**
		 * Store the given tuple and return its stored address.
		 */
		rtc::TransportTuple* AddTuple(rtc::TransportTuple* tuple);
		/**
		 * If the given tuple exists return its stored address, nullptr otherwise.
		 */
		rtc::TransportTuple* HasTuple(const rtc::TransportTuple* tuple) const;
		/**
		 * Set the given tuple as the selected tuple.
		 * NOTE: The given tuple MUST be already stored within the list.
		 */
		void SetSelectedTuple(rtc::TransportTuple* storedTuple);

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		// Others.
		std::string usernameFragment;
		std::string password;
		std::string oldUsernameFragment;
		std::string oldPassword;
		IceState state{ IceState::NEW };
		std::list<rtc::TransportTuple> tuples;
		rtc::TransportTuple* selectedTuple{ nullptr };
	};

	/* Inline instance methods. */

	inline const std::string& IceServer::GetUsernameFragment() const
	{
		return this->usernameFragment;
	}

	inline const std::string& IceServer::GetPassword() const
	{
		return this->password;
	}

	inline IceServer::IceState IceServer::GetState() const
	{
		return this->state;
	}

	inline rtc::TransportTuple* IceServer::GetSelectedTuple() const
	{
		return this->selectedTuple;
	}

	inline void IceServer::SetUsernameFragment(const std::string& usernameFragment)
	{
		this->oldUsernameFragment = this->usernameFragment;
		this->usernameFragment    = usernameFragment;
	}

	inline void IceServer::SetPassword(const std::string& password)
	{
		this->oldPassword = this->password;
		this->password    = password;
	}
} // namespace rtc

#endif
