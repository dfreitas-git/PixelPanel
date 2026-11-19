

#include "ArchimedeanSpiralScene.h"


void ArchimedeanSpiralScene::onSingleClick() { 
    reverseMode_ = !reverseMode_;
}

void ArchimedeanSpiralScene::processPose(const WandPacket& packet) {

    // Roll controls drawing speed
    const float roll = constrain(packet.roll, -PI / 4, PI / 4);
    speed_ = -roll * 50 ;  

    // Pitch controls shape of the spiral.  Allowing larger travel in the + 
    // direction (PI/2) so the user has equal control range up or down (since the
    // amplitude_ scale factor is .5)
    const float pitch = constrain(packet.pitch, -PI / 4, PI / 2);
    shape_ = -pitch ;  
}

void ArchimedeanSpiralScene::updateAnimation() {
    uint8_t R, G, B;

    // Guard against divide by zero later in the tick calculation
    uint16_t animatePeriod = constrain(25 + speed_, 1, 50);

    uint32_t now = millis();

    if (now - lastUpdateMs_ >= animatePeriod) {
        lastUpdateMs_ = now;
        ccpIndex_ += animateIncrement_;

        // Move the color-change-index when we see a tick transition
        ccpIndex_ += animateIncrement_;

        if(ccpIndex_ >= COLUMNS * numberOfRotations_) {
            R = random(0,128);
            G = random(0,128);
            B = random(0,128);


           // Preserve current direction when changing modes.
           // In continuous mode, the user can select the direction
           // by timing the button press during reverse mode.
            if(reverseMode_) {
                animateIncrement_ = -1;
                rightColor_ = {R,G,B};
            } else {
               // If we are in continuous direction mode, then we need to wrap the color-change-point
               // and swap what used to be the left color to now be the right color since we are 
               // injecting the new color from the left
                ccpIndex_ = 0;
                rightColor_ = leftColor_;
                leftColor_ = {R,G,B};
            }
        } else if(ccpIndex_ <= 0) {
            R = random(0,128);
            G = random(0,128);
            B = random(0,128);
            if(reverseMode_) {
                animateIncrement_ = 1;
                leftColor_ = {R,G,B};
            } else {
                ccpIndex_ = COLUMNS * numberOfRotations_;
                leftColor_ = rightColor_;
                rightColor_ = {R,G,B};
            }
        } 
    }
}

void ArchimedeanSpiralScene::drawSpiral(PixelFramebuffer& fb) {
    // ArchimedeanSpiral using (atCos(t), atSin(t))
    for (int i = 0; i <= COLUMNS * numberOfRotations_; i++) {
        float t = (i  * TWO_PI) / period_;
        const int x = lroundf(14.5 + amplitude_  * t * cosf(t)); 
        const int y = lroundf(14.5 + (amplitude_ + shape_) * t * sinf(t)); 
        if(ccpIndex_ <= i) {
            graphics_.writePixel(fb.data(),x,y,rightColor_);
        } else {
            graphics_.writePixel(fb.data(),x,y,leftColor_);
        }
    }
}
    
void ArchimedeanSpiralScene::render(PixelFramebuffer& fb) {

    updateAnimation();
    fb.clear();
    drawSpiral(fb);

}