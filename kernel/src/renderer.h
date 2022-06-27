#pragma once
#include "BBPstd.h"


class renderer
{
    public:
        static FrameBuffer* buffer;
        static PSF1_FONT* activeFont;

        static unsigned int strokeColor;
        static unsigned int rectColor;

        static bool strokeQ;
        static bool fillQ;

        static unsigned int strokeW;

        static void init(FrameBuffer* buff, PSF1_FONT* font);
        static void setPixel(unsigned int x, unsigned int y, unsigned int col);

        static void background(unsigned int color);
        static void background(Color color);

        static void Fill(unsigned int color);
        static void Stroke(unsigned int color);
        static void Fill(Color color);
        static void Stroke(Color color);

        static void strokeWeight(unsigned int weight);

        static void noStroke();
        static void noFill();

        static void Rect(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

        static void Char(char text, unsigned int x, unsigned int y);
        static void Text(const char* str, unsigned int x, unsigned int y);
        


};