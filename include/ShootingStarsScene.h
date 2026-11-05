
#pragma once

#include <Arduino.h>
#include "Scene.h"
#include "graphicsAnimations.h"

class ShootingStarsScene : public Scene {
public:
    explicit ShootingStarsScene(GraphicsAnimations& animations);

    const char* name() const override { return "STAR"; }

    void begin() override;

    void processPose(const WandPacket& packet) override;
    void processGesture(const GestureEvent& event) override;

    void render(PixelFramebuffer& framebuffer) override;

    // Set the value from 0-100 (0 being closest to LEDs, 100 being most diffused - farthest from LEDs)
    int preferredDiffuserPosition() const override { return 60; }  

private:
    GraphicsAnimations& gAnim;

    ShootingStarSettings settings{};

    bool superStarLaunchPending = false;
    float superStarDirection = 0.0f;
};