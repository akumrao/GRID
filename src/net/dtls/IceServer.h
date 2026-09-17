#ifndef RTC_ICE_SERVER_HPP
#define RTC_ICE_SERVER_HPP

#include "TransportTuple.h"
#include <list>
#include <string>
#include <map>            
#include <mutex>          
#include <cstring>        
#include <sys/socket.h>   


#include "net/TcpConnection.h"
//#include "net/UdpSocket.h"


using namespace base::net;

namespace rtc {

    struct SockAddrCompare {

        bool operator()(const base::net::addr_record_t& lhs, const base::net::addr_record_t& rhs) const {
            int lhs_family = 0, rhs_family = 0;
            std::string lhs_ip, rhs_ip;
            uint16_t lhs_port = 0, rhs_port = 0;

            base::net::IP::GetAddressInfo(reinterpret_cast<const struct sockaddr*> (&lhs.addr), lhs_family, lhs_ip, lhs_port);
            base::net::IP::GetAddressInfo(reinterpret_cast<const struct sockaddr*> (&rhs.addr), rhs_family, rhs_ip, rhs_port);

            if (lhs_family != rhs_family) {
                return lhs_family < rhs_family;
            }
            if (lhs_ip != rhs_ip) {
                return lhs_ip < rhs_ip;
            }
            return lhs_port < rhs_port;
        }
    };

    class IceServer {
    public:

        enum class IceState {
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
        void ForceSelectedTuple(const TransportTuple* tuple);
        void SetSelectedTuple(TransportTuple* storedTuple);

    public:
        void HandleTuple(TransportTuple* tuple, bool hasUseCandidate);
        TransportTuple* AddTuple(TransportTuple* tuple);
        TransportTuple* HasTuple(const TransportTuple* tuple) const;


    public:
        TcpConnectionBase* find_tcp_connection(const addr_record_t& record);
        void register_tcp_connection(const addr_record_t& record, TcpConnectionBase* conn);
        void unregister_tcp_connection(const addr_record_t& record);
        void clear_connections();

    public:
        //        UdpConnectionBase* find_udp_connection(const addr_record_t& record);
        //        void register_udp_connection(const addr_record_t& record, UdpConnectionBase* conn);
        //        void unregister_udp_connection(const addr_record_t& record);

    private:
        IceState state{ IceState::NEW};
        std::list<TransportTuple> tuples;
        TransportTuple* selectedTuple{ nullptr};


    private:
        mutable std::mutex m_connectionMutex; // Renamed from m_tcpMutex to represent both protocols
        // std::map<sockaddr_storage, TcpConnectionBase*, SockAddrCompare> m_tcpConnections;
        std::map<base::net::addr_record_t, TcpConnectionBase*, SockAddrCompare> m_tcpConnections;
        //  std::map<sockaddr_storage, UdpConnectionBase*, SockAddrCompare> m_udpConnections; // Added for UDP
    };

    inline IceServer::IceState IceServer::GetState() const {
        return this->state;
    }

    inline TransportTuple* IceServer::GetSelectedTuple() const {
        return this->selectedTuple;
    }
} // namespace rtc

#endif
