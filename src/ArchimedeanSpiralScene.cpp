

#include "ArchimedeanSpiralScene.h"


void ArchimedeanSpiralScene::onSingleClick() { 
    reverseMode_ = !reverseMode_;
}

void ArchimedeanSpiralScene::processPose(const WandPacket& packet) {

    // Roll controls drawing speed
    const float roll = constrain(packet.roll, -PI / 4, PI / 4);
    speed_ = -roll * 50 ;  

    // Pitch controls shape of the spiral
    const float pitch = constrain(packet.pitch, -PI / 4, PI / 2);
    shape_ = -pitch ;  
}

void ArchimedeanSpiralScene::render(PixelFramebuffer& fb) {

    // Clear the framebuffer
    fb.clear();

    uint8_t R, G, B;
    static uint16_t ccpIndex = 0;  // the color-change-point
    static RGB rightColor = {0,64,0};
    static RGB leftColor = {64,0,0};

    // Guard against divide by zero later in the tick calculation
    uint16_t animatePeriod = constrain(25 + speed_, 1,50);  
    uint32_t now = millis();
  
    // What animate tick are we in?
    uint32_t tick = now / animatePeriod; 
    static uint32_t lastTick = 0;

    static int animateIncrement = 1;

    // Move the color-change-index when we see a tick transition
    if( tick != lastTick){
        lastTick = tick;
        ccpIndex += animateIncrement;

        if(ccpIndex >= COLUMNS * numberOfRotations_) {
            R = random(0,128);
            G = random(0,128);
            B = random(0,128);

           // if we are not reversing direction, then we need to wrap the color-change-point
           // and swap what used to be the left color to now be the right color since we are 
           // injecting the new color from the left
            if(reverseMode_) {
                animateIncrement = -1;
                rightColor = {R,G,B};
            } else {
                ccpIndex = 0;
                rightColor = leftColor;
                leftColor = {R,G,B};
            }
        } else if(ccpIndex <= 0) {
            R = random(0,128);
            G = random(0,128);
            B = random(0,128);
            if(reverseMode_) {
                animateIncrement = 1;
                leftColor = {R,G,B};
            } else {
                ccpIndex = COLUMNS * numberOfRotations_;
                leftColor = rightColor;
                rightColor = {R,G,B};
            }
        } 
    }

    // ArchimedeanSpiral using (atCos(t), atSin(t))
    for (int i = 0; i <= COLUMNS * numberOfRotations_; i++) {
        float t = (i  * TWO_PI) / period_;
        const int x = 14.5 + amplitude_  * t * cosf(t); 
        const int y = 14.5 + (amplitude_ + shape_) * t * sinf(t); 
        if(ccpIndex <= i) {
            graphics_.writePixel(fb.data(),x,y,rightColor);
        } else {
            graphics_.writePixel(fb.data(),x,y,leftColor);
        }
    }
}