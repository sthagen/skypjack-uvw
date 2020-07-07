#include <type_traits>
#include <gtest/gtest.h>
#include <uvw/emitter.h>


TEST(ErrorEvent, Functionalities) {
    auto ecode = static_cast<std::underlying_type_t<uv_errno_t>>(UV_EADDRINUSE);

    uvw::ErrorEvent event{ecode};

    ASSERT_EQ(ecode, uvw::ErrorEvent::translate(ecode));
    ASSERT_NE(event.what(), nullptr);
    ASSERT_NE(event.name(), nullptr);
    ASSERT_EQ(event.code(), ecode);

    ASSERT_FALSE(static_cast<bool>(uvw::ErrorEvent{0}));
    ASSERT_TRUE(static_cast<bool>(uvw::ErrorEvent{ecode}));
}


TEST(Emitter, EmptyAndClear) {
    uvw::TestEmitter emitter{};

    ASSERT_TRUE(emitter.empty());

    emitter.on<uvw::ErrorEvent>([](const auto &, auto &){});

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::ErrorEvent>());
    ASSERT_TRUE(emitter.empty<uvw::FakeEvent>());

    emitter.clear<uvw::FakeEvent>();

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::ErrorEvent>());
    ASSERT_TRUE(emitter.empty<uvw::FakeEvent>());

    emitter.clear<uvw::ErrorEvent>();

    ASSERT_TRUE(emitter.empty());
    ASSERT_TRUE(emitter.empty<uvw::ErrorEvent>());
    ASSERT_TRUE(emitter.empty<uvw::FakeEvent>());

    emitter.on<uvw::ErrorEvent>([](const auto &, auto &){});
    emitter.on<uvw::FakeEvent>([](const auto &, auto &){});

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::ErrorEvent>());
    ASSERT_FALSE(emitter.empty<uvw::FakeEvent>());

    emitter.clear();

    ASSERT_TRUE(emitter.empty());
    ASSERT_TRUE(emitter.empty<uvw::ErrorEvent>());
    ASSERT_TRUE(emitter.empty<uvw::FakeEvent>());
}


TEST(Emitter, On) {
    uvw::TestEmitter emitter{};

    emitter.on<uvw::FakeEvent>([](const auto &, auto &){});

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::FakeEvent>());

    emitter.emit();

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::FakeEvent>());
}


TEST(Emitter, Once) {
    uvw::TestEmitter emitter{};

    emitter.once<uvw::FakeEvent>([](const auto &, auto &){});

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::FakeEvent>());

    emitter.emit();

    ASSERT_TRUE(emitter.empty());
    ASSERT_TRUE(emitter.empty<uvw::FakeEvent>());
}


TEST(Emitter, OnceAndErase) {
    uvw::TestEmitter emitter{};

    auto conn = emitter.once<uvw::FakeEvent>([](const auto &, auto &){});

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::FakeEvent>());

    emitter.erase(conn);

    ASSERT_TRUE(emitter.empty());
    ASSERT_TRUE(emitter.empty<uvw::FakeEvent>());
}


TEST(Emitter, OnAndErase) {
    uvw::TestEmitter emitter{};

    auto conn = emitter.on<uvw::FakeEvent>([](const auto &, auto &){});

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::FakeEvent>());

    emitter.erase(conn);

    ASSERT_TRUE(emitter.empty());
    ASSERT_TRUE(emitter.empty<uvw::FakeEvent>());
}


TEST(Emitter, CallbackClear) {
    uvw::TestEmitter emitter{};

    emitter.on<uvw::FakeEvent>([](const auto &, auto &ref) {
        ref.template on<uvw::FakeEvent>([](const auto &, auto &){});
        ref.clear();
    });

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::FakeEvent>());

    emitter.emit();

    ASSERT_TRUE(emitter.empty());
    ASSERT_TRUE(emitter.empty<uvw::FakeEvent>());

    emitter.on<uvw::FakeEvent>([](const auto &, auto &ref) {
        ref.clear();
        ref.template on<uvw::FakeEvent>([](const auto &, auto &){});
    });

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::FakeEvent>());

    emitter.emit();

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::FakeEvent>());
}
