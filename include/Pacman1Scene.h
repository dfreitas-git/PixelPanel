
#pragma once

#include <Arduino.h>
#include "Scene.h"
#include "graphicsAnimations.h"

class Pacman1Scene : public Scene {
public:
    explicit Pacman1Scene(GraphicsAnimations& animations);

    const char* name() const override { return "PAC"; }

    void begin() override;

    void processPose(const WandPacket& packet) override;
    void processGesture(const GestureEvent& event) override;

    void render(PixelFramebuffer& framebuffer) override;

    // Set the value from 0-100 (0 being closest to LEDs, 100 being most diffused - farthest from LEDs)
    int preferredDiffuserPosition() const override { return 0; }  

private:
    GraphicsAnimations& gAnim;
    Pacman1Settings settings{};

};