#pragma once

#include <Arduino.h>
#include "Globals.h"
#include "Scene.h"

class PanelController {
public:
    static constexpr uint8_t MAX_SCENES = 20;

    void addScene(Scene& scene);
    void selectScene(uint8_t index);

    void processPose(const WandPacket& packet);
    void processGesture(const GestureEvent& event);
    void renderScene(PixelFramebuffer& framebuffer);

    WandMode getMode() const { return currentMode; }

    uint8_t getSceneCount() const { return sceneCount; }

    uint8_t getActiveSceneIndex() const { return activeSceneIndex; }

    const char* getSceneName(uint8_t index) const;

private:
    Scene* scenes[MAX_SCENES]{};

    uint8_t sceneCount = 0;
    uint8_t activeSceneIndex = 0;

    Scene* activeScene = nullptr;
    WandMode currentMode = WandMode::Scene;

    void processPanelPose(const WandPacket& packet);
    void processPanelGesture(const GestureEvent& event);
};