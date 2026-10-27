#pragma once

#include <Arduino.h>
#include "Globals.h"
#include "Scene.h"

class PanelController {
public:
    //explicit PanelController(Scene& initialScene);

    void setActiveScene(Scene& scene);

    void processPose(const WandPacket& packet);
    void processGesture(const GestureEvent& event);

    void renderScene(PixelFramebuffer& framebuffer);

    WandMode getMode() const { return currentMode; }

private:
    Scene* activeScene = nullptr;
    WandMode currentMode = WandMode::Scene;

    void processPanelPose(const WandPacket& packet);
    void processPanelGesture(const GestureEvent& event);
};
