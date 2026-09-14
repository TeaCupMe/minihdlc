#include "minihdlc.h"
#include "crc_ccitt.h"

/* HDLC Asynchronous framing */
/* The frame boundary octet is 01111110, (7E in hexadecimal notation) */
#define FRAME_BOUNDARY_OCTET 0x7E

/* A "control escape octet", has the bit sequence '01111101', (7D hexadecimal) */
#define CONTROL_ESCAPE_OCTET 0x7D

/* If either of these two octets appears in the transmitted data, an escape octet is sent, */
/* followed by the original data octet with bit 5 inverted */
#define INVERT_OCTET 0x20

/* The frame check sequence (FCS) is a 16-bit CRC-CCITT */
/* AVR Libc CRC function is _crc_ccitt_update() */
/* Corresponding CRC function in Qt (www.qt.io) is qChecksum() */
#define CRC16_CCITT_INIT_VAL 0xFFFF

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif // UNUSED
/* 16bit low and high bytes copier */
static constexpr uint8_t lowByte(uint16_t x) { return ((x)    & 0xFF); }
static constexpr uint8_t highByte(uint16_t x){ return ((x>>8) & 0xFF); }

static void putByte(uint8_t* buf, uint8_t &position, uint8_t data, uint8_t &bufSize) 
{
	#ifndef MINIHDLC_TINY
	if (position == bufSize)
	{
		bufSize = 0;
		position = 0;
		return;
	}
	#else
	UNUSED(bufSize);
	#endif
	buf[position++] = data;
}

namespace minihdlc
{

MiniHDLCController::MiniHDLCController()
{
	this->framePosition = 0;
	this->frameChecksum = CRC16_CCITT_INIT_VAL;
	this->escapeCharacter = false;
}

#ifdef MINIHDLC_USE_CALLBACK
[[nodiscard]] bool MiniHDLCController::Init(frameHandlerFunction_t frameHandler)
{
#ifndef MINIHDLC_TINY
	if (frameHandler == nullptr) 
	{
		return false;
	}
#endif
	this->frameHandler = frameHandler;
	return true;
}
#else
[[nodiscard]] bool MiniHDLCController::Init()
{
	this->frameReceived = false;
	this->frameLength = 0;
	return true;
}
#endif // MINIHDLC_USE_CALLBACK

#ifndef MINIHDLC_USE_CALLBACK
uint8_t MiniHDLCController::operator[](size_t index)
{
#ifndef MINIHDLC_TINY
    if (index > MINIHDLC_MAX_FRAME_LENGTH + 1) return 0;
#endif
	return this->receiveBuffer[index];
}
#endif // MINIHDLC_USE_CALLBACK

/* Function to find valid HDLC frame from incoming data */
[[nodiscard]] bool MiniHDLCController::FeedFromIsr(uint8_t data) {
#ifndef MINIHDLC_USE_CALLBACK
	if (this->frameReceived) return false;
#endif
	/* FRAME FLAG */
	if (data == FRAME_BOUNDARY_OCTET) {
		if (this->escapeCharacter == true) {
			this->escapeCharacter = false;
		}
		else if ((this->framePosition >= 2) && /* If a valid frame is detected */ 
				 (this->frameChecksum == 
					((this->receiveBuffer[this->framePosition - 1] << 8) | 
						(this->receiveBuffer[this->framePosition - 2]	   )))) 	// (msb << 8 ) | (lsb)
		{
			/* Call the user defined function and pass frame to it */
		#ifdef MINIHDLC_USE_CALLBACK
			(*this->frameHandler)(this->receiveBuffer,
					this->framePosition - 2);
		#else
			this->frameLength = this->framePosition - 2;
			this->frameReceived = true;
		#endif
		}

		this->framePosition = 0;
		this->frameChecksum = CRC16_CCITT_INIT_VAL;
		return true;
	}

	if (this->escapeCharacter) {
		this->escapeCharacter = false;
		data ^= INVERT_OCTET;
	} else if (data == CONTROL_ESCAPE_OCTET) {
		this->escapeCharacter = true;
		return true;
	}

	this->receiveBuffer[this->framePosition] = data;

	if (this->framePosition >= 2) {
		this->frameChecksum = CrcUpdate(this->frameChecksum,
				this->receiveBuffer[this->framePosition - 2]);
	}

	this->framePosition++;
	#ifndef MINIHDLC_TINY
	if (this->framePosition == MINIHDLC_MAX_FRAME_LENGTH) {
		this->framePosition = 0;
		this->frameChecksum = CRC16_CCITT_INIT_VAL;
		return false;
	}
	#endif

	return true;
}

[[nodiscard]] bool MiniHDLCController::FeedFromIsr(const uint8_t *data, uint16_t length)
{
	for (uint16_t i = 0; i < length; i++) {
		if (!FeedFromIsr(data[i])) {
			return false;
		}
	}
	return true;
}

uint8_t MiniHDLCController::ConstructFrame(const uint8_t *frameBuffer, uint8_t frameLength, uint8_t* outputBuffer, uint8_t outputLength)
{
    uint8_t data;
	uint16_t fcs = CRC16_CCITT_INIT_VAL;
	uint8_t outputCounter = 0;
	putByte(outputBuffer, outputCounter, (uint8_t) FRAME_BOUNDARY_OCTET, outputLength);

	while (frameLength) {
		data = *frameBuffer++;
		fcs = CrcUpdate(fcs, data);
		if ((data == CONTROL_ESCAPE_OCTET) || (data == FRAME_BOUNDARY_OCTET)) {
			putByte(outputBuffer, outputCounter, (uint8_t) CONTROL_ESCAPE_OCTET, outputLength);
			data ^= INVERT_OCTET;
		}
		putByte(outputBuffer, outputCounter, (uint8_t) data, outputLength);
		frameLength--;
	}

	data = lowByte(fcs);
	
	if ((data == CONTROL_ESCAPE_OCTET) || (data == FRAME_BOUNDARY_OCTET)) {
		putByte(outputBuffer, outputCounter, (uint8_t) CONTROL_ESCAPE_OCTET, outputLength);
		data ^= (uint8_t) INVERT_OCTET;
	}
	
	putByte(outputBuffer, outputCounter, (uint8_t) data, outputLength);
	data = highByte(fcs);
	if ((data == CONTROL_ESCAPE_OCTET) || (data == FRAME_BOUNDARY_OCTET)) {
		putByte(outputBuffer, outputCounter, CONTROL_ESCAPE_OCTET, outputLength);
		data ^= INVERT_OCTET;
	}
	putByte(outputBuffer, outputCounter, data, outputLength);
	putByte(outputBuffer, outputCounter, FRAME_BOUNDARY_OCTET, outputLength);
	return outputCounter;
}

void MiniHDLCController::Reset()
{
	this->framePosition = 0;
	this->frameChecksum = CRC16_CCITT_INIT_VAL;
	this->escapeCharacter = false;
#ifndef MINIHDLC_USE_CALLBACK
	this->frameReceived = false;
	this->frameLength = 0;
#endif
}

} // namespace minihdlc