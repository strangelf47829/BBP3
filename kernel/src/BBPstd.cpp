#include "BBPstd.h"
#include "renderer.h"

const unsigned int frequencyMillis = 0x00041893;

void System::Sleep(unsigned int millis)
{
    unsigned int whi = 0;
    lop: whi++;
    if(whi != millis) goto lop;
    //renderer::background(whi);
    renderer::Text("Done waiting",20,40);
    //while(true){}
}