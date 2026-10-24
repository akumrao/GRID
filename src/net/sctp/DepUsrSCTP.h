#ifndef DEP_USRSCTP_HPP
#define DEP_USRSCTP_HPP

#include "common.h"
#include "base/Timer.h"

using namespace base;

class DepUsrSCTP {
private:

    class Checker : public Timer::Listener {
    public:
        Checker();
        ~Checker() override;

    public:
        void Start();
        void Stop();

        /* Pure virtual methods inherited from Timer::Listener. */
    public:
        void OnTimer(Timer* timer) override;

    private:
        Timer* timer{ nullptr};
        uint64_t lastCalledAtMs{ 0u};
    };

public:
    static void ClassInit();
    static void ClassDestroy();
    static void IncreaseSctpTransports();
    static void DecreaseSctpTransports();

private:
    static Checker* checker;
    static uint64_t numSctpTransports;
};

#endif
