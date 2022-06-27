#pragma once
#include <stddef.h>


struct FrameBuffer
{
	void* baseAddress;
	size_t bufferSize;
	unsigned int width;
	unsigned int height;
	unsigned int pixelsPerScanLine;
};

struct PSF1_HEADER
{
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
};

struct PSF1_FONT
{
	PSF1_HEADER* header;
	void* glyphBuffer;
};

class Color
{
    public:
        unsigned int byteRep;
        unsigned int r;
        unsigned int g;
        unsigned int b;
        Color(unsigned int r, unsigned int g, unsigned int b)
        {
            this->r = r;
            this->g = g;
            this->b = b;
            this->byteRep = (r << 4) + (g << 2) + b;
        }
};


class System
{
    public:
        static void Sleep(unsigned int millis);



    private:
        static const unsigned int frequencyMillis = 0x00041893;

};