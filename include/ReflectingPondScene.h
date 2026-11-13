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
    ReflectingPondCore pond;
    ReflectingPondCore::Color pixels[30][30];
    float skipDirection = 0.0f;
    bool skipRunning = false;
    uint8_t skipIndex = 0;
    int32_t nextSkip = 0;
};