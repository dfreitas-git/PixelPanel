
#pragma once
#include <Arduino.h>
#include "Scene.h"
#include "graphicsPrimitives.h"

/* Class that implements a simple Archimedean Sprial drawing.
   The speed of drawing will be controlled with pitch, the shape will be 
   controlled with roll.
 */

class ArchimedeanSpiralScene : public Scene {
public:
    explicit ArchimedeanSpiralScene(GraphicsPrimitives& graphics) : graphics_(graphics) {}
    const char* name() const override { return "ARCHI"; }
    void onSingleClick() override;
    void processPose(const WandPacket& packet) override;
    void render(PixelFramebuffer& framebuffer) override;
    int preferredDiffuserPosition() const override { return 0; }

private:
    GraphicsPrimitives& graphics_;
    uint8_t numberOfRotations_ = 6;
    float period_ = 30.0;
    float amplitude_ = 0.5;
    int speed_;
    float shape_;
    bool reverseMode_ = true;
};