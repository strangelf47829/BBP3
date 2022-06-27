#include <stddef.h>
#include "BBPstd.h"
#include "renderer.h"


void blueScreen()
{
    renderer::background(0x000000FF);
    renderer::Stroke(0xFFFFFFFF);
    renderer::Text("Uh Oh!",20,20);
    renderer::Text("BBP3 Utils has run into an unexpected issue... ",20,40);
    renderer::Text("Please reboot the PC",20,60);
}
extern "C" void _start(FrameBuffer* buffer, PSF1_FONT* font, int it){

    if(it) { blueScreen(); while(true) {} }
    renderer::init(buffer, font);
    renderer::background(0x00000000);
    renderer::Stroke(0x00FFFF00);
    renderer::Text("Hello, Console!",20,20);
    System::Sleep(0x0FFFFFFF);
}


