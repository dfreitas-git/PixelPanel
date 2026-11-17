
#pragma once
#include <Arduino.h>
#include "Scene.h"
#include "graphicsPrimitives.h"

/* Class that implements a simple Oscilloscope screen.  The screen will
   Have two sine waves.  One with a fixed frequency.  The other where we can
   change the amplitude, frequency, or phase (depending on the mode) with 
   roll/pitch of the wand.  A thrid wave will show the sum of the first two waves.
   
   The four modes are: frequency/amplitude (sum or difference)  and
   phase (sum or difference) indicated by a tag in the lower left of the screen
   displaying:  F+, F-, P+, P-  for frequency or phase, sum/difference
   Sum means the composite wave will be the reference waveA Plus variable waveB.  
   Difference means the composite wave will be the reference waveA minus variable waveB.
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

    enum class Mode {FreqSum, FreqDiff, PhaseSum, PhaseDiff};

    struct Wave {
        float frequency;    
        float phase;
        float amplitude;
        float zeroPosition;
        RGB color;
    };

    palette c_;

    GraphicsPrimitives& graphics_;

    Mode mode_ = Mode::PhaseSum;

    Wave waveA_ {33.0, 0.0, 8.0, 14.5, c_.red32};
    Wave waveB_ {33.0, PI, 8.0, 14.5, c_.green32};

};
