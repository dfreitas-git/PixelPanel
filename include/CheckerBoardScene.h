
#pragma once

#include <Arduino.h>
#include "Scene.h"
#include "graphicsAnimations.h"

class CheckerBoardScene : public Scene {
public:
    explicit CheckerBoardScene(GraphicsAnimations& animations);

    const char* name() const override { return "CHKRB"; }

    void begin() override;

    void processPose(const WandPacket& packet) override;
    void processGesture(const GestureEvent& event) override;

    void render(PixelFramebuffer& framebuffer) override;

private:
    GraphicsAnimations& gAnim;
    CheckerBoardSettings settings{};

};