#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


struct SignalEvent: Event<SignalEvent> {
    explicit SignalEvent(int sig) noexcept: signum(sig) { }

    int signal() const noexcept { return signum; }

private:
    const int signum;
};


class Signal final: public Handle<Signal, uv_signal_t> {
    static void startCallback(uv_signal_t *handle, int signum) {
        Signal &signal = *(static_cast<Signal*>(handle->data));
        signal.publish(SignalEvent{signum});
    }

    using Handle::Handle;

public:
    template<typename... Args>
    static std::shared_ptr<Signal> create(Args&&... args) {
        return std::shared_ptr<Signal>{new Signal{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_signal_t>(&uv_signal_init); }

    void start(int signum) { invoke(&uv_signal_start, get<uv_signal_t>(), &startCallback, signum); }
    void stop() { invoke(&uv_signal_stop, get<uv_signal_t>()); }

    int signal() const noexcept { return get<uv_signal_t>()->signum; }
};


}
