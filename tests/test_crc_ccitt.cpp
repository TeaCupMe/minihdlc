#include <gtest/gtest.h>
#include "minihdlc.h"


/* Comparing with https://www.sunshine2k.de/coding/javascript/crc/crc_js.html 
    Parametrization: Custom
    - Non-reflected input, Non-reflected result
    - Polynomial: 0x1021
    - Initial:    0xFFFF
    - Final Xor:  0x0
*/ 
TEST(CrcCcitt, SingleDigit)
{
    uint16_t crc = 0xFFFF;
    uint8_t data = 1;

    crc = minihdlc::CrcUpdate(crc, data);

    ASSERT_EQ(crc, 0xF1D1);
}

TEST(CrcCcitt, Incremental_NumbersString)
{
    uint16_t crc = 0xFFFF;
    uint8_t* data = (uint8_t*) "123456789";
    
    for (size_t i{0}; i < 9; i++)
    {
        crc = minihdlc::CrcUpdate(crc, data[i]);
    }

    ASSERT_EQ(crc, 0x29B1) << crc;
}

TEST(CrcCcitt, Incremental_Numbers)
{
    uint16_t crc = 0xFFFF;
    uint8_t data[6] = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F};
    
    for (size_t i{0}; i < 6; i++)
    {
        crc = minihdlc::CrcUpdate(crc, data[i]);
    }

    ASSERT_EQ(crc, 0xFA03) << crc;
}
TEST(CrcCcitt, Block_NumbersString)
{
    // uint16_t crc = 0xFFFF;
    uint8_t* data = (uint8_t*) "123456789";
    uint16_t crc = minihdlc::CrcBlock(data, strlen((char*)data));
    printf("%x\n", crc);
    ASSERT_EQ(crc, 0x29B1);
}

TEST(CrcCcitt, Block_StringOnes)
{
    // uint16_t crc = 0xFFFF;
    uint8_t* data = (uint8_t*) "1111111111";
    uint16_t crc = minihdlc::CrcBlock(data, strlen((char*)data));
    printf("%x\n", crc);
    ASSERT_EQ(crc, 0xA8A8);
}
