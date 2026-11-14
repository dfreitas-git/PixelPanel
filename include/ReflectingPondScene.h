#pragma once
#include "Scene.h"
#include "ReflectingPondCore.h"


class ReflectingPondScene : public Scene {
public:
    const char* name() const override {
        return "POND";
    }

    void begin() override;
    void render(PixelFramebuffer& framebuffer) override;
    void processGesture(const GestureEvent& event) override;
    void onSingleClick() override;

    int preferredDiffuserPosition() const override {
        return 73;
    }

private:
    enum SwipeEffect { SKIP, BUBBLES };
    SwipeEffect swipeEffect = SKIP;

    static const int MAX_BUBBLES = 160;

    struct Bubble {
        float x, y;
        uint32_t birthMs;
        uint16_t lifetimeMs;
        uint8_t brightness;
        bool active;
    };

    ReflectingPondCore pond;
    ReflectingPondCore::Color pixels[30][30];

    Bubble bubbles[MAX_BUBBLES] = {};
    int nextBubble = 0;

    bool bubbleRunning = false;
    uint32_t bubbleStartMs = 0;
    float bubbleDirection = 0.0f;
    int nextBubbleStep = 0;

    void addBubbleCluster(float x, float y, uint32_t now);
    void renderBubbles(uint32_t now);

    float skipDirection = 0.0f;
    bool skipRunning = false;
    uint8_t skipIndex = 0;
    uint32_t nextSkip = 0;
};