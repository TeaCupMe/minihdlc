#ifndef minihdlc_h
#define minihdlc_h

/*  
Tiny HDLC-codec library for use in various application
including, but not limited to, embedded.

Heavily based on https://github.com/mengguang/minihdlc

Ported to C++/CMake/GTest by Aleksey <TeaCupMe> Gilenko, aleksey@gilenko.net, 2026
*/

#include <stdint.h>
#include <stddef.h>

namespace minihdlc
{
#ifdef MINIHDLC_USE_CALLBACK
using frameHandlerFunction_t = void (*)(const uint8_t *frame_buffer, uint16_t frame_length);
#endif

#ifndef MINIHDLC_MAX_FRAME_LENGTH
#define MINIHDLC_MAX_FRAME_LENGTH (64)
#endif

#ifdef MINIHDLC_TINY
#pragma message("MINIHDLC_TINY is defined, so pointer/length checks are disabled. Use with caution and only if you know what you are doing.")
#endif



class MiniHDLCController {
private:
	uint8_t receiveBuffer[MINIHDLC_MAX_FRAME_LENGTH + 1];
#ifdef MINIHDLC_USE_CALLBACK
	frameHandlerFunction_t frameHandler;
#else
	bool frameReceived;
	uint16_t frameLength;
#endif
	bool escapeCharacter;
	uint16_t framePosition;
	uint16_t frameChecksum;

public:
	MiniHDLCController();
#ifdef MINIHDLC_USE_CALLBACK
	bool Init(frameHandlerFunction_t frameHandler);
#else
	bool Init();
	bool AcceptNext();
	uint8_t FrameAvailable() {return this->frameLength ? this->frameLength : 0;}
	uint8_t operator[](size_t index);
#endif

	bool FeedFromIsr(uint8_t data);
	bool FeedFromIsr(const uint8_t *data, uint16_t length);

	uint8_t ConstructFrame(const uint8_t *frameBuffer, uint8_t frameLength, uint8_t* output, uint8_t outputBufferLength);

	void Reset();
};

}
#endif
