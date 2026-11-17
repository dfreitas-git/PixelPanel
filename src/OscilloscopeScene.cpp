
#include "OscilloscopeScene.h"


void OscilloscopeScene::onSingleClick() { 
    if(resultant_ == Resultant::Sum) {
        resultant_ = Resultant::Difference;
    } else if(resultant_ == Resultant::Difference) {
        resultant_ = Resultant::Sum;
    }
}

void OscilloscopeScene::processPose(const WandPacket& packet) {

    // Roll controls frequency
    const float roll = constrain(packet.roll, -PI / 4, PI / 4);
    waveB_.frequency = 33.0f - roll * (33.0f / (PI / 4));

    // Pitch controls amplitude
    const float pitch = constrain(packet.pitch, -PI / 4, PI / 4);
    waveB_.amplitude = 8.0f + pitch * (8.0f / (PI / 2));
}

void OscilloscopeScene::render(PixelFramebuffer& fb) {

    // Clear the framebuffer
    fb.clear();

    // Draw the grid lines
    graphics_.drawLine(fb.data(),0,15,29,15,1,{0,0,16});

    // Add a plus/minus sign depending on if we are summing or subtracting
    graphics_.drawLine(fb.data(),0,2,4,2,1,{128,75,0});
    if(resultant_ == Resultant::Sum) {
        graphics_.drawLine(fb.data(),2,0,2,4,1,{128,75,0});
    }

    // Pixels for the sine waves
    int yA,yB,yR;
    const float phaseStepA = TWO_PI * waveA_.frequency / 1000.0f;
    const float phaseStepB = TWO_PI * waveB_.frequency / 1000.0f;

    int prevYA = 0;
    int prevYB = 0;
    int prevYR = 0;

    for (int x = 0; x < COLUMNS; x++) {
        const float a = sinf(x * phaseStepA) * waveA_.amplitude;
        const float b = sinf(x * phaseStepB) * waveB_.amplitude;
    
        const int yA = lroundf(waveA_.zeroPosition + a);
        const int yB = lroundf(waveB_.zeroPosition + b);
    
        const float resultant = (resultant_ == Resultant::Sum) ? a + b : a - b;
        const int yR = lroundf(waveA_.zeroPosition + resultant);
    
        if (x == 0) {
            graphics_.writePixel(fb.data(), x, yA, waveA_.color);
            graphics_.writePixel(fb.data(), x, yB, waveB_.color);
            graphics_.writePixel(fb.data(), x, yR, {128,75,0});
        } else {
            if (x > 0 && abs(yA - prevYA) > 2) {
                graphics_.drawLine(fb.data(), x-1, prevYA, x, yA, 1, waveA_.color);
            } else {
                graphics_.writePixel(fb.data(), x, yA, waveA_.color);
            }
           // graphics_.drawLine(fb.data(), x-1, prevYA, x, yA, 1, waveA_.color);
            if (x > 0 && abs(yB - prevYB) > 2) {
                graphics_.drawLine(fb.data(), x-1, prevYB, x, yB, 1, waveB_.color);
            } else {
                graphics_.writePixel(fb.data(), x, yB, waveB_.color);
            }
            //graphics_.drawLine(fb.data(), x-1, prevYB, x, yB, 1, waveB_.color);
            if (x > 0 && abs(yR - prevYR) > 2) {
                graphics_.drawLine(fb.data(), x-1, prevYR, x, yR, 1, {128,75,0});
            } else {
                graphics_.writePixel(fb.data(), x, yR, {128,75,0});
            }
            //graphics_.drawLine(fb.data(), x-1, prevYR, x, yR, 1, {128,75,0});
        }
    
        prevYA = yA;
        prevYB = yB;
        prevYR = yR;
    }
}

