
#pragma once
#include <Arduino.h>
#include "Scene.h"
#include "graphicsPrimitives.h"

/* class that implements a simple Oscilloscope screen.  The screen will
   Have two sine waves.  One with a fixed frequency.  The other where we can
   change the amplitude and frequency with roll/pitch of the wand.  A thrid wave
   will show the sum of the first two waves.
 */

class OscilloscopeScene : public Scene {
public:
    explicit OscilloscopeScene(GraphicsPrimitives& graphics) : graphics_(graphics) {}
    const char* name() const override { return "SCOPE"; }
    void onSingleClick() override;
    void processPose(const WandPacket& packet) override;
    void render(PixelFramebuffer& framebuffer) override;
    int preferredDiffuserPosition() const override { return 0; }

private:

    enum class Resultant { Sum, Difference };

    struct Wave {
        float frequency;    
        float amplitude;
        float zeroPosition;
        RGB color;
    };

    GraphicsPrimitives& graphics_;

    Resultant resultant_ = Resultant::Sum;

    Wave waveA_ {33.0, 8.0, 14.5, {32,0,0}};
    Wave waveB_ {66.0, 8.0, 14.5, {0,32,0}};

};
