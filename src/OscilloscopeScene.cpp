
#include "OscilloscopeScene.h"


void OscilloscopeScene::onSingleClick() { 
    switch (mode_) {
        case Mode::FreqSum :
            mode_ = Mode::FreqDiff;
            break;

        case Mode::FreqDiff :
            mode_ = Mode::PhaseSum;
            break;

        case Mode::PhaseSum :
            mode_ = Mode::PhaseDiff;
            break;

        case Mode::PhaseDiff :
            mode_ = Mode::FreqSum;
            break;
    }
}

void OscilloscopeScene::processPose(const WandPacket& packet) {

    if(mode_ == Mode::FreqSum || mode_ == Mode::FreqDiff) {
        // Roll controls frequency
        const float roll = constrain(packet.roll, -PI / 4, PI / 4);
        waveB_.frequency = 33.0f - roll * (33.0f / (PI / 4));

        // Pitch controls amplitude
        const float pitch = constrain(packet.pitch, -PI / 4, PI / 4);
        waveB_.amplitude = 8.0f + pitch * (8.0f / (PI / 2));

    } else if(mode_== Mode::PhaseSum || mode_== Mode::PhaseDiff ) {
        // Roll controls phase.  Amplitude fixed. Frequency fixed
        waveB_.amplitude = 8.0f;
        waveB_.frequency = 33.0f;
        const float roll = constrain(packet.roll, -PI / 4, PI / 4);
        waveB_.phase =  -roll * 4;  
    }
}

void OscilloscopeScene::render(PixelFramebuffer& fb) {

    // Clear the framebuffer
    fb.clear();

    // Draw the grid lines
    graphics_.drawLine(fb.data(),0,15,29,15,1,c_.darkblue);

    // Add a plus/minus sign depending on if we are summing or subtracting
    switch (mode_) {
        case Mode::FreqSum :
            graphics_.drawString(fb.data(),"F+",0,0,c_.mediumyellow,c_.black);
            break;
        case Mode::FreqDiff :
            graphics_.drawString(fb.data(),"F-",0,0,c_.mediumyellow,c_.black);
            break;
        case Mode::PhaseSum :
            graphics_.drawString(fb.data(),"P+",0,0,c_.mediumyellow,c_.black);
            break;
        case Mode::PhaseDiff :
            graphics_.drawString(fb.data(),"P-",0,0,c_.mediumyellow,c_.black);
            break;
    }
    graphics_.drawLine(fb.data(),6,2,10,2,1,c_.mediumyellow);
    if(mode_ == Mode::FreqSum || mode_ == Mode::PhaseSum) {
        graphics_.drawLine(fb.data(),8,0,8,4,1,c_.mediumyellow);
    }

    // Pixels for the sine waves
    const float phaseStepA = TWO_PI * waveA_.frequency / 1000.0f;
    const float phaseStepB = TWO_PI * waveB_.frequency / 1000.0f;

    int prevYA = 0;
    int prevYB = 0;
    int prevYR = 0;

    for (int x = 0; x < COLUMNS; x++) {
        const float a = sinf(x * phaseStepA) * waveA_.amplitude;
        float b;
        if(mode_ == Mode::FreqSum || mode_ == Mode::FreqDiff) {
            waveB_.phase = 0.0f;
        }
        b = sinf(x * phaseStepB + waveB_.phase) * waveB_.amplitude;
    
        const int yA = lroundf(waveA_.zeroPosition + a);
        const int yB = lroundf(waveB_.zeroPosition + b);
    
        const float res = (mode_ == Mode::FreqSum || mode_ == Mode::PhaseSum) ? a + b : a - b;
        const int yR = lroundf(waveA_.zeroPosition + res);
    
        if (x == 0) {
            graphics_.writePixel(fb.data(), x, yA, waveA_.color);
            graphics_.writePixel(fb.data(), x, yB, waveB_.color);
            graphics_.writePixel(fb.data(), x, yR, c_.mediumyellow);
        } else {
            if (abs(yA - prevYA) > 2) {
                graphics_.drawLine(fb.data(), x-1, prevYA, x, yA, 1, waveA_.color);
            } else {
                graphics_.writePixel(fb.data(), x, yA, waveA_.color);
            }
            if (abs(yB - prevYB) > 2) {
                graphics_.drawLine(fb.data(), x-1, prevYB, x, yB, 1, waveB_.color);
            } else {
                graphics_.writePixel(fb.data(), x, yB, waveB_.color);
            }
            if (abs(yR - prevYR) > 2) {
                graphics_.drawLine(fb.data(), x-1, prevYR, x, yR, 1, c_.mediumyellow);
            } else {
                graphics_.writePixel(fb.data(), x, yR, c_.mediumyellow);
            }
        }
    
        prevYA = yA;
        prevYB = yB;
        prevYR = yR;
    }
}

