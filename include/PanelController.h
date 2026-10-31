#pragma once

#include <Arduino.h>
#include "Globals.h"
#include "Scene.h"

enum class MenuPage {
    Main,
    Scenes,
    Panel,
    Test
};


class PanelController {
public:
    static constexpr uint8_t MAX_MAIN = 3;
    static constexpr uint8_t MAX_SCENES = 20;

    void addScene(Scene& scene);
    void selectScene(uint8_t index);
    void selectMain(uint8_t index);

    void processPose(const WandPacket& packet);
    void processGesture(const GestureEvent& event);
    void renderScene(PixelFramebuffer& framebuffer);

    WandMode getMode() const { return currentMode; }

    MenuPage getCurrentPage() const { return currentPage; }
    void setCurrentPage(MenuPage page) { currentPage = page; }

    uint8_t getMainCount() const { return mainCount; }
    uint8_t getSceneCount() const { return sceneCount; }

    uint8_t getActiveMainIndex() const { return activeMainIndex; }
    uint8_t getActiveSceneIndex() const { return activeSceneIndex; }

    const char* getMainName(uint8_t index) const {return main[index];}
    const char* getSceneName(uint8_t index) const;

private:
    uint8_t mainCount = MAX_MAIN;
    uint8_t sceneCount = 0;
    uint8_t activeMainIndex = 0;
    uint8_t activeSceneIndex = 0;

    const char* main[MAX_MAIN] =  { "SCENE", "PANEL", "TEST" };
    Scene* scenes[MAX_SCENES]{};

    Scene* activeScene = nullptr;
    WandMode currentMode = WandMode::Scene;
    MenuPage currentPage = MenuPage::Main;

    void processPanelPose(const WandPacket& packet);
    void processPanelGesture(const GestureEvent& event);
};