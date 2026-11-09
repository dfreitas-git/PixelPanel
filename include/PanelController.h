#pragma once

#include <Arduino.h>
#include "Globals.h"
#include "Scene.h"

class PanelController {
public:
    static constexpr uint8_t MAX_SCENES = 15;
    static constexpr uint8_t MAX_GAMES = 5;

    void addScene(Scene& scene);
    void addGames(Scene& game);

    void selectScene(uint8_t index);
    void selectGames(uint8_t index);

    void processPose(const WandPacket& packet);
    void processGesture(const GestureEvent& event);

    void renderScene(PixelFramebuffer& framebuffer);
    void renderGames(PixelFramebuffer& framebuffer);

    uint8_t getSceneCount() const { return sceneCount; }
    uint8_t getActiveSceneIndex() const { return activeSceneIndex; }
    void setActiveScene(Scene& scene);
    const char* getSceneName(uint8_t index) const;

    uint8_t getGamesCount() const { return gamesCount; }
    uint8_t getActiveGamesIndex() const { return activeGamesIndex; }
    void setActiveGames(Scene& game);
    const char* getGamesName(uint8_t index) const;

    void sampleBrightnessPot();

    void setBrightnessScale(uint8_t value) {
        brightnessScale = constrain(value, 0, 100);
    }

    uint8_t getBrightnessScale() const {
        return min(brightnessScale, potBrightness);
    }

    void setDiffuserOffset(int value) { diffuserOffset = constrain(value, -100, 100); }
    int getDiffuserOffset() const { return diffuserOffset; }

    int getSceneDiffuserPosition() const;

    void onSingleClick();


private:
    static constexpr uint8_t BRIGHTNESS_POT_PIN = 35;
    uint8_t brightnessScale = 100;
    uint8_t potBrightness = 100;
    int diffuserOffset = 0;
    uint8_t sceneCount = 0;
    uint8_t gamesCount = 0;
    uint8_t activeSceneIndex = 0;
    uint8_t activeGamesIndex = 0;
    Scene* scenes[MAX_SCENES]{};
    Scene* games[MAX_GAMES]{};
    Scene* activeScene = nullptr;
};


