#include <gtest/gtest.h>
#include "minihdlc.h"

#include <array>
#include <algorithm>
#include <stdint.h>

namespace {

#ifdef MINIHDLC_USE_CALLBACK
struct CallbackCapture {
    static inline std::array<uint8_t, MINIHDLC_MAX_FRAME_LENGTH + 1> buffer{};
    static inline uint16_t length = 0;
    static inline int callCount = 0;

    static void Handler(const uint8_t* frame_buffer, uint16_t frame_length)
    {
        length = frame_length;
        const uint16_t copyLen = (frame_length < buffer.size())
            ? frame_length
            : static_cast<uint16_t>(buffer.size());
        std::copy_n(frame_buffer, copyLen, buffer.begin());
        ++callCount;
    }

    static void Reset()
    {
        length = 0;
        callCount = 0;
        buffer.fill(0);
    }
};
#endif

bool InitController(minihdlc::MiniHDLCController& controller)
{
#ifdef MINIHDLC_USE_CALLBACK
    CallbackCapture::Reset();
    return controller.Init(&CallbackCapture::Handler);
#else
    return controller.Init();
#endif
}

} // namespace

TEST(MiniHDLC, Initialization_Success)
{
    minihdlc::MiniHDLCController controller;
    ASSERT_TRUE(InitController(controller));
}

TEST(MiniHDLC, Initialization_Failure_NullPointers)
{
#ifdef MINIHDLC_USE_CALLBACK
    minihdlc::MiniHDLCController controller;
    bool initResult = controller.Init(nullptr);
#ifndef MINIHDLC_TINY
    EXPECT_FALSE(initResult);
#else
    EXPECT_TRUE(initResult);
#endif
#else
    GTEST_SKIP() << "Null pointer init check only applies in callback mode";
#endif
}

TEST(MiniHDLC, Encode)
{
    minihdlc::MiniHDLCController controller;
    ASSERT_TRUE(InitController(controller));

    std::array<uint8_t, 20> hdlcFrame{};
    std::array<uint8_t, 6> rawFrame = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F};
    uint8_t length = controller.ConstructFrame(rawFrame.data(), 6, hdlcFrame.data(), 20);

    std::array<uint8_t, 10> expected = {0x7E, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x03, 0xFA, 0x7E};
    EXPECT_EQ(length, 10);
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_EQ(hdlcFrame[i], expected[i]);
    }
}

TEST(MiniHDLC, Encode_WithEscape)
{
    minihdlc::MiniHDLCController controller;
    ASSERT_TRUE(InitController(controller));

    std::array<uint8_t, 20> hdlcFrame{};
    std::array<uint8_t, 6> rawFrame = {0x1A, 0x7D, 0x3C, 0x4D, 0x5E, 0x6F};
    uint8_t length = controller.ConstructFrame(rawFrame.data(), 6, hdlcFrame.data(), 20);

    std::array<uint8_t, 11> expected = {0x7E, 0x1A, 0x7D, 0x5D, 0x3C, 0x4D, 0x5E, 0x6F, 0xB4, 0x22, 0x7E};
    EXPECT_EQ(length, 11);
    for (size_t i = 0; i < 11; ++i) {
        EXPECT_EQ(hdlcFrame[i], expected[i]);
    }
}

TEST(MiniHDLC, Encode_WithEscapes)
{
    minihdlc::MiniHDLCController controller;
    ASSERT_TRUE(InitController(controller));

    std::array<uint8_t, 20> hdlcFrame{};
    std::array<uint8_t, 6> rawFrame = {0x1A, 0x7D, 0x3C, 0x4D, 0x7E, 0x6F};
    uint8_t length = controller.ConstructFrame(rawFrame.data(), 6, hdlcFrame.data(), 20);

    std::array<uint8_t, 12> expected = {0x7E, 0x1A, 0x7D, 0x5D, 0x3C, 0x4D, 0x7D, 0x5E, 0x6F, 0x52, 0x24, 0x7E};
    EXPECT_EQ(length, 12);
    for (size_t i = 0; i < 12; ++i) {
        EXPECT_EQ(hdlcFrame[i], expected[i]);
    }
}

TEST(MiniHDLC, Decode)
{
    minihdlc::MiniHDLCController controller;
    ASSERT_TRUE(InitController(controller));

    std::array<uint8_t, 10> hdlcFrame = {0x7E, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x03, 0xFA, 0x7E};
    std::array<uint8_t, 6> expected = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F};

    bool result = controller.FeedFromIsr(hdlcFrame.data(), static_cast<uint16_t>(hdlcFrame.size()));
    EXPECT_TRUE(result);

#ifdef MINIHDLC_USE_CALLBACK
    EXPECT_EQ(CallbackCapture::callCount, 1);
    EXPECT_EQ(CallbackCapture::length, 6);
    for (size_t i = 0; i < 6; ++i) {
        EXPECT_EQ(CallbackCapture::buffer[i], expected[i]);
    }
#else
    EXPECT_EQ(controller.FrameAvailable(), 6);
    for (size_t i = 0; i < 6; ++i) {
        EXPECT_EQ(controller[i], expected[i]);
    }
#endif
}

TEST(MiniHDLC, Decode_WithEscape)
{
    minihdlc::MiniHDLCController controller;
    ASSERT_TRUE(InitController(controller));

    // Encoded form of {0x1A, 0x7D, 0x3C, 0x4D, 0x5E, 0x6F}
    std::array<uint8_t, 11> hdlcFrame = {
        0x7E, 0x1A, 0x7D, 0x5D, 0x3C, 0x4D, 0x5E, 0x6F, 0xB4, 0x22, 0x7E};
    std::array<uint8_t, 6> expected = {0x1A, 0x7D, 0x3C, 0x4D, 0x5E, 0x6F};

    bool result = controller.FeedFromIsr(hdlcFrame.data(), static_cast<uint16_t>(hdlcFrame.size()));
    EXPECT_TRUE(result);

#ifdef MINIHDLC_USE_CALLBACK
    EXPECT_EQ(CallbackCapture::callCount, 1);
    EXPECT_EQ(CallbackCapture::length, 6);
    for (size_t i = 0; i < 6; ++i) {
        EXPECT_EQ(CallbackCapture::buffer[i], expected[i]);
    }
#else
    EXPECT_EQ(controller.FrameAvailable(), 6);
    for (size_t i = 0; i < 6; ++i) {
        EXPECT_EQ(controller[i], expected[i]);
    }
#endif
}

TEST(MiniHDLC, Decode_RoundTrip)
{
    minihdlc::MiniHDLCController controller;
    ASSERT_TRUE(InitController(controller));

    std::array<uint8_t, 6> rawFrame = {0x10, 0x20, 0x7E, 0x30, 0x7D, 0x40};
    std::array<uint8_t, 32> hdlcFrame{};
    uint8_t encodedLen = controller.ConstructFrame(
        rawFrame.data(),
        static_cast<uint8_t>(rawFrame.size()),
        hdlcFrame.data(),
        static_cast<uint8_t>(hdlcFrame.size()));
    ASSERT_GT(encodedLen, 0);

    bool result = controller.FeedFromIsr(hdlcFrame.data(), encodedLen);
    EXPECT_TRUE(result);

#ifdef MINIHDLC_USE_CALLBACK
    EXPECT_EQ(CallbackCapture::callCount, 1);
    EXPECT_EQ(CallbackCapture::length, rawFrame.size());
    for (size_t i = 0; i < rawFrame.size(); ++i) {
        EXPECT_EQ(CallbackCapture::buffer[i], rawFrame[i]);
    }
#else
    EXPECT_EQ(controller.FrameAvailable(), rawFrame.size());
    for (size_t i = 0; i < rawFrame.size(); ++i) {
        EXPECT_EQ(controller[i], rawFrame[i]);
    }
#endif
}

#ifdef MINIHDLC_USE_CALLBACK
TEST(MiniHDLC, Decode_MultipleFrames_ViaCallback)
{
    minihdlc::MiniHDLCController controller;
    ASSERT_TRUE(InitController(controller));

    std::array<uint8_t, 10> frame1 = {0x7E, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x03, 0xFA, 0x7E};
    std::array<uint8_t, 10> frame2 = {0x7E, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x03, 0xFA, 0x7E};

    EXPECT_TRUE(controller.FeedFromIsr(frame1.data(), static_cast<uint16_t>(frame1.size())));
    EXPECT_EQ(CallbackCapture::callCount, 1);

    EXPECT_TRUE(controller.FeedFromIsr(frame2.data(), static_cast<uint16_t>(frame2.size())));
    EXPECT_EQ(CallbackCapture::callCount, 2);
    EXPECT_EQ(CallbackCapture::length, 6);
}
#endif
