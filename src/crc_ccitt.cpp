#include <stdint.h>

namespace minihdlc
{
    /*
    Polynomial: x^16 + x^12 + x^5 + 1 (0x1201) Initial value: 0xffff
    This is the CRC used by PPP and IrDA.
    See RFC1171 (PPP protocol) and IrDA IrLAP 1.1
    */
    uint16_t CrcUpdate(uint16_t currentCrc, uint8_t data)
    {        
        uint8_t x = (currentCrc >> 8) ^ data;
        x ^= (x >> 4);
        return (currentCrc << 8) ^ 
              (static_cast<uint16_t>(x) << 12) ^ 
              (static_cast<uint16_t>(x) << 5) ^ 
              static_cast<uint16_t>(x);
    }

    uint16_t CrcBlock(uint8_t* block, uint16_t len)
    {
        uint16_t crc = 0xFFFF;
        uint8_t i;

        while (len--)
        {
            crc ^= *block++ << 8;

            for (i = 0; i < 8; i++)
                crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
        }
        return crc;
    }
} // namespace minihdlc
