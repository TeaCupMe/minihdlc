#include <gtest/gtest.h>
#include "minihdlc.h"


/* Comparing with https://www.sunshine2k.de/coding/javascript/crc/crc_js.html 
    Parametrization: Custom
    - Non-reflected input, Non-reflected result
    - Polynomial: 0x1021
    - Initial:    0xFFFF
    - Final Xor:  0x0
*/ 
TEST(MiniHDLC, CrcCcitt_SingleDigit)
{
    uint16_t crc = 0xFFFF;
    uint8_t data = 1;

    crc = minihdlc::CrcUpdate(crc, data);

    ASSERT_EQ(crc, 0xF1D1);
}

TEST(MiniHDLC, CrcCcitt_Incremental_NumbersString)
{
    uint16_t crc = 0xFFFF;
    uint8_t* data = (uint8_t*) "123456789";
    
    for (size_t i{0}; i < 9; i++)
    {
        crc = minihdlc::CrcUpdate(crc, data[i]);
    }

    ASSERT_EQ(crc, 0x29B1) << crc;
}

TEST(MiniHDLC, CrcCcitt_Incremental_Numbers)
{
    uint16_t crc = 0xFFFF;
    uint8_t data[6] = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F};
    
    for (size_t i{0}; i < 6; i++)
    {
        crc = minihdlc::CrcUpdate(crc, data[i]);
    }

    ASSERT_EQ(crc, 0xFA03) << crc;
}
TEST(MiniHDLC, CrcCcitt_Block_NumbersString)
{
    // uint16_t crc = 0xFFFF;
    uint8_t* data = (uint8_t*) "123456789";
    uint16_t crc = minihdlc::CrcBlock(data, strlen((char*)data));
    printf("%x\n", crc);
    ASSERT_EQ(crc, 0x29B1);
}

TEST(MiniHDLC, CrcCcitt_Block_StringOnes)
{
    // uint16_t crc = 0xFFFF;
    uint8_t* data = (uint8_t*) "1111111111";
    uint16_t crc = minihdlc::CrcBlock(data, strlen((char*)data));
    printf("%x\n", crc);
    ASSERT_EQ(crc, 0xA8A8);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}