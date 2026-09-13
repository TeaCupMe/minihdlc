#include <gtest/gtest.h>
#include <gmock/gmock.h> // Required for ElementsAreArray
#include "minihdlc.h"
#include <array>
#include <stdint.h>

TEST(MiniHDLC, Initialization_Success) {
    minihdlc::MiniHDLCController controller;
#ifdef MINIHDLC_USE_CALLBACKS
    bool initResult = controller.Init(
        [](const uint8_t* frame_buffer, uint16_t frame_length) {} // Dummy frameHandler function
    );
#else
    bool initResult = controller.Init();
#endif // MINIHDLC_USE_CALLBACKS
    ASSERT_TRUE(initResult);
}

TEST(MiniHDLC, Initialization_Failure_NullPointers) {
    minihdlc::MiniHDLCController controller;
#ifdef MINIHDLC_USE_CALLBACKS
    bool initResult = controller.Init(nullptr);
#else
    bool initResult = controller.Init();
#endif // MINIHDLC_USE_CALLBACKS

#ifndef MINIHDLC_TINY
    // without MINIHDLC_TINY, init should fail with null pointers
    EXPECT_FALSE(initResult);
#else 
    // with MINIHDLC_TINY, init should succeed even with null pointers
    EXPECT_TRUE(initResult);
#endif
}

TEST(MiniHDLC, Encode) {
    minihdlc::MiniHDLCController controller;
    std::array<uint8_t, 20> hdlcFrame;
    std::array<uint8_t, 6> rawFrame = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F}; 
    controller.Init();
    uint8_t length = controller.ConstructFrame(rawFrame.data(), 6, hdlcFrame.data(), 20);

    std::array<uint8_t, 10> expected = {0x7E, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x03, 0xFA, 0x7E};
    EXPECT_EQ(length, 10);
    for (size_t i{0}; i < 10; i++)
    {
        EXPECT_EQ(hdlcFrame[i], expected[i]);
    }
}

TEST(MiniHDLC, Encode_WithEscape) {
#ifndef MINIHDLC_USE_CALLBACKS
    minihdlc::MiniHDLCController controller;
    std::array<uint8_t, 20> hdlcFrame;
    std::array<uint8_t, 6> rawFrame = {0x1A, 0x7D, 0x3C, 0x4D, 0x5E, 0x6F}; 
    controller.Init();
    uint8_t length = controller.ConstructFrame(rawFrame.data(), 6, hdlcFrame.data(), 20);

    std::array<uint8_t, 11> expected = {0x7E, 0x1A, 0x7D, 0x5D, 0x3C, 0x4D, 0x5E, 0x6F, 0xB4, 0x22, 0x7E};
    EXPECT_EQ(length, 11);
    for (size_t i{0}; i < 11; i++)
    {
        EXPECT_EQ(hdlcFrame[i], expected[i]);
    }
#else
    EXPECT_TRUE(FALSE) << "callback testing is not yet implemented!"; //TODO implement
#endif
}

TEST(MiniHDLC, Encode_WithEscapes) {
#ifndef MINIHDLC_USE_CALLBACKS
    minihdlc::MiniHDLCController controller;
    std::array<uint8_t, 20> hdlcFrame;
    std::array<uint8_t, 6> rawFrame = {0x1A, 0x7D, 0x3C, 0x4D, 0x7E, 0x6F}; 
    controller.Init();
    uint8_t length = controller.ConstructFrame(rawFrame.data(), 6, hdlcFrame.data(), 20);

    std::array<uint8_t, 12> expected = {0x7E, 0x1A, 0x7D, 0x5D, 0x3C, 0x4D, 0x7D, 0x5E, 0x6F, 0x52, 0x24, 0x7E};
    EXPECT_EQ(length, 12);
    for (size_t i{0}; i < 12; i++)
    {
        EXPECT_EQ(hdlcFrame[i], expected[i]);
    }
#else
    EXPECT_TRUE(FALSE) << "callback testing is not yet implemented!"; //TODO implement
#endif
}

TEST(MiniHDLC, Decode) {
#ifndef MINIHDLC_USE_CALLBACKS
    minihdlc::MiniHDLCController controller;
    std::array<uint8_t, 10> hdlcFrame = {0x7E, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x03, 0xFA, 0x7E};
    std::array<uint8_t, 20> rawFrame; 
    controller.Init();
    bool result = controller.FeedFromIsr(hdlcFrame.data(), 10);
    EXPECT_TRUE(result);
    EXPECT_EQ(controller.FrameAvailable(), 6);

    std::array<uint8_t, 6> expected = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F};
    for (size_t i{0}; i < 6; i++)
    {
        EXPECT_EQ(controller[i], expected[i]);
    }
#else
    EXPECT_TRUE(FALSE) << "callback testing is not yet implemented!"; //TODO implement
#endif
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}