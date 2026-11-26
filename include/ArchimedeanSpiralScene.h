
#pragma once
#include <Arduino.h>
#include "Scene.h"
#include "graphicsPrimitives.h"

/* Class that implements a simple Archimedean Sprial drawing.
   The speed of drawing will be controlled with roll, the shape will be 
   controlled with pitch.  There is a "onSingleClick()" mode switch that 
   will draw the advancing color always in one contunious direction or 
   have it bounce back and forth (reverse mode).
 */

class ArchimedeanSpiralScene : public Scene {
public:
    explicit ArchimedeanSpiralScene(GraphicsPrimitives& graphics) : graphics_(graphics) {}
    const char* name() const override { return "SPIRL"; }
    void onSingleClick() override;
    void processPose(const WandPacket& packet) override;
    void render(PixelFramebuffer& framebuffer) override;
    int preferredDiffuserPosition() const override { return 0; }
    void updateAnimation();
    void drawSpiral(PixelFramebuffer& framebuffer);

private:
    GraphicsPrimitives& graphics_;
    uint8_t numberOfRotations_ = 6;
    float period_ = 30.0;
    float amplitude_ = 0.5;

    int speed_ = 0;
    float shape_ = 0.0f;

    bool reverseMode_ = true;

    int ccpIndex_ = 0;  // The color-change-point along the moving color boundary
    int animateIncrement_ = 1;
    uint32_t lastUpdateMs_ = 0;

    RGB rightColor_ = {0,64,0};
    RGB leftColor_ = {64,0,0};

};