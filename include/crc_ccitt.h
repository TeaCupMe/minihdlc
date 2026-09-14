#ifndef MINIHDLC_CRC_CCITT_H
#define MINIHDLC_CRC_CCITT_H

#include <stdint.h>

namespace minihdlc
{
    uint16_t CrcUpdate(uint16_t currentCrc, uint8_t data);
    uint16_t CrcBlock(uint8_t* block, uint16_t len);
} // namespace minihdlc

#endif // MINIHDLC_CRC_CCITT_H