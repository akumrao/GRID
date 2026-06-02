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
#include "base/Timer.h"

using namespace base::net;

namespace rtc {

    class Router : public rtc::Transport::Listener, public Timer::Listener {
    public:
        explicit Router(const std::string& id, int agentNo);
        virtual ~Router();


        void HandleRequest(bool server, const Configuration &config, int localPort, int remotePort, std::string, std::string, CertificateFingerprint &dtlsRemoteFingerprint);

        void OnDtlsTransportStatus(DtlsTransport::DtlsState state);
        void OnSctpState(SctpTransport::State state);
        void OnSctpTransportMessageReceived(SctpTransport* sctpAssociation, message_ptr message);

        void OnReceiveData(byte * data, size_t len);
        void OnClose(); 


    private:


        /* Pure virtual methods inherited from rtc::Transport::Listener. */
    public:
        void OnTimer(Timer *timer) override;
        Timer *timer{nullptr};


    public:

        void Close();
        // Passed by argument.
        const std::string id;
        int agentNo{0};
        int count{0};

    private:
        // Allocated by this.
        std::unordered_map<std::string, rtc::WebRtcTransport*> mapTransports;

        std::unordered_map<std::string, rtc::DataProducer*> mapDataProducers;
    };
} // namespace rtc

#endif
