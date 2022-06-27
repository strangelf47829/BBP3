#include "renderer.h"

FrameBuffer* renderer::buffer;
PSF1_FONT* renderer::activeFont;

unsigned int renderer::strokeColor;
unsigned int renderer::rectColor;
bool renderer::strokeQ;
bool renderer::fillQ;
unsigned int renderer::strokeW;

void renderer::init(FrameBuffer* buff, PSF1_FONT* font)
{
    renderer::buffer = buff;
    renderer::activeFont = font;
}

void renderer::setPixel(unsigned int x, unsigned int y, unsigned int col)
{
    unsigned int BBP = 1;
    *(unsigned int*)(x*BBP + (y * (buffer)->pixelsPerScanLine) + (unsigned int*)(buffer)->baseAddress) = col;
}

void renderer::background(Color color){ background(color.byteRep); }
void renderer::background(unsigned int color)
{
    for(unsigned int x = 0; x <= renderer::buffer->pixelsPerScanLine/2; x++)
    {
        for(unsigned int y = 0; y <= renderer::buffer->height/2; y++)
        {
            setPixel(x,y,color);
            setPixel(x,renderer::buffer->height-y,color);
            setPixel(renderer::buffer->width-x,y,color);
            setPixel(renderer::buffer->width-x,renderer::buffer->height-y,color);
        }
    }
}




void renderer::Fill(Color color) { Fill(color.byteRep); }
void renderer::Stroke(Color color) { Stroke(color.byteRep); }
void renderer::Fill(unsigned int color) { renderer::fillQ = true; renderer::rectColor = color; }
void renderer::Stroke(unsigned int color){ renderer::strokeQ = true; renderer::strokeColor = color; }
void renderer::strokeWeight(unsigned int weight){ renderer::strokeW = weight; }

void renderer::noStroke(){ renderer::strokeQ = false; }
void renderer::noFill(){ renderer::fillQ = false; }

void renderer::Rect(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2){}


void renderer::Char(char text, unsigned int x, unsigned int y)
{
    unsigned int* pixPtr = (unsigned int*)renderer::buffer->baseAddress;
    char* fontPtr = (char*)renderer::activeFont->glyphBuffer + (text * renderer::activeFont->header->charsize);
    for(unsigned long py = y; py < y + 16; py++)
    {
        for(unsigned long px = x; px < x + 8; px++)
        {
            if((*fontPtr & (0b10000000 >> (px - x))) > 0)
            {
                *(unsigned int*)(pixPtr + px + (py * renderer::buffer->pixelsPerScanLine)) = renderer::strokeColor;
                
            }
        }
        fontPtr++;
    }
}
void renderer::Text(const char* str, unsigned int x, unsigned int y)
{
    char* chr = (char*)str;
    while(*chr != 0)
    {
        Char(*chr, x, y);
        x += 8;
        if(x + 8 > renderer::buffer->width)
        {
            x = 0;
            y += 16;
        }
        chr++;
    }
}

