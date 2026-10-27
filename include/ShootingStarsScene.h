
#pragma once

#include <Arduino.h>
#include "Scene.h"
#include "graphicsAnimations.h"

class ShootingStarsScene : public Scene {
public:
    explicit ShootingStarsScene(GraphicsAnimations& animations);

    void begin() override;

    void processPose(const WandPacket& packet) override;
    void processGesture(const GestureEvent& event) override;

    void render(PixelFramebuffer& framebuffer) override;

private:
    GraphicsAnimations& gAnim;

    ShootingStarSettings settings{};

    bool superStarLaunchPending = false;
    float superStarDirection = 0.0f;
};