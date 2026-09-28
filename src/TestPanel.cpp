
#include "TestPanel.h"
#include "PixelFramebuffer.h"
#include "PixelOutput.h"

void TestPanel::corners(PixelFramebuffer &fb) {

    fb.clear();
    fb.setPixel(0,  0,  20, 0, 0);   // bottom-left red
    fb.setPixel(29, 0,  0, 20, 0);   // bottom-right green
    fb.setPixel(0, 29,  0, 0, 20);   // top-left blue
    fb.setPixel(29,29, 20,20,20);    // top-right white
    
}


void TestPanel::rows(PixelFramebuffer &fb, uint8_t row) {
    fb.clear();
    for(uint8_t x=0; x< PANEL_WIDTH; x++) {
        fb.setPixel(x,row,0,0,32);
    }
}


void TestPanel::columns(PixelFramebuffer &fb, uint8_t column) {
    fb.clear();
    for(uint8_t y=0; y< PANEL_HEIGHT; y++) {
        fb.setPixel(column,y,0,0,32);
    }
}


void TestPanel::rmtBands(PixelFramebuffer &fb) {
    fb.clear();
    uint8_t r[5] = {32,0,0,32,32};
    uint8_t g[5] = {0,32,0,32,0};
    uint8_t b[5] = {0,0,32,32,32};
    for(uint8_t rmt=0; rmt< 5; rmt++) {
        for(uint8_t x=0; x< PANEL_WIDTH; x++) {
            for(uint8_t y=0; y< 6; y++) {
                fb.setPixel(x,rmt*6+y,r[rmt],g[rmt],b[rmt]);
            }
        }
    }
}


void TestPanel::colorTest(PixelFramebuffer &fb, uint8_t r, uint8_t g, uint8_t b) {
    fb.clear();
    for(uint8_t x=0; x< PANEL_WIDTH; x++) {
        for(uint8_t y=0; y< PANEL_HEIGHT; y++) {
            fb.setPixel(x,y,r,g,b);
        }
    }
}


void TestPanel::pixel(PixelFramebuffer &fb, uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    fb.clear();
    fb.setPixel(x,y,r,g,b);
    
}

void TestPanel::diagonals(PixelFramebuffer &fb)
{
    fb.clear();

    for (uint8_t i = 0; i < PANEL_WIDTH; i++) {
        fb.setPixel(i, i, 32, 0, 0);
        fb.setPixel(PANEL_WIDTH - 1 - i, i, 0, 32, 0);
    }
}
void TestPanel::runAllTests(PixelOutput &po, PixelFramebuffer &fb)
{
    fb.clear();

    // Draw one dot in each corner
    corners(fb);
    po.show(fb);
    delay(5000);

    // Draw an X 
    diagonals(fb);
    po.show(fb);
    delay(5000);

    // row test
    for(uint8_t row=0; row< PANEL_HEIGHT; row++) {
        rows(fb,row);
        po.show(fb);
        delay(100);
    }

    // column test
    for(uint8_t column=0; column< PANEL_WIDTH; column++) {
        columns(fb,column);
        po.show(fb);
        delay(100);
    }

    // Solid color for each of the rmt channels
    rmtBands(fb);
    po.show(fb);
    delay(5000);

    // One pixel scanning across all points on the screen
    colorTest(fb, 32,0,0);
    po.show(fb);
    delay(5000);

    // One color over the entire screen
    for(uint8_t y=0; y< PANEL_HEIGHT; y++) {
        for(uint8_t x=0; x< PANEL_WIDTH; x++) {
            pixel(fb,x,y,0,32,0);
            po.show(fb);
            delay(5);
        }
    }
}