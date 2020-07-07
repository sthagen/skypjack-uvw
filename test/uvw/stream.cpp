#include <gtest/gtest.h>
#include <uvw.hpp>


struct fake_stream_t { void *data; };


struct FakeStreamHandle: uvw::StreamHandle<FakeStreamHandle, fake_stream_t, uvw::CloseEvent, uvw::ErrorEvent> {
    using StreamHandle::StreamHandle;

    template<typename... Args>
    bool init(Args&&...) { return true; }
};
