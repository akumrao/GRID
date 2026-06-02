#ifndef MS_RTC_ROUTER_HPP
#define MS_RTC_ROUTER_HPP

#include "common.h"
#include "DataConsumer.h"
#include "DataProducer.h"
#include "WebRtcTransport.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "net/IP.h"
#include "net/UdpSocket.h"


using namespace base::net;

namespace rtc {

    class Router : public rtc::Transport::Listener {
    public:
        explicit Router();
        virtual ~Router();


        void HandleRequest(std::string id, int agentNo, bool server, const Configuration &config, int localPort, int remotePort, std::string, std::string, CertificateFingerprint &dtlsRemoteFingerprint);

        void OnDtlsTransportStatus(std::string id, DtlsTransport::DtlsState state);
        void OnSctpState(std::string id, SctpTransport::State state);
        void OnSctpTransportMessageReceived(std::string id, SctpTransport* sctpAssociation, message_ptr message);

        void OnReceiveData(std::string id, byte * data, size_t len);
        void OnClose(std::string id); 


    private:


        /* Pure virtual methods inherited from rtc::Transport::Listener. */
    public:
 
        void Close();
        // Passed by argument.
        int count{0};

    private:
        // Allocated by this.
        std::unordered_map<std::string, rtc::WebRtcTransport*> mapTransports;

        std::unordered_map<std::string, rtc::DataProducer*> mapDataProducers;
    };
} // namespace rtc

#endif
