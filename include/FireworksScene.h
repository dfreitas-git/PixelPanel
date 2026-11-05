
#pragma once

#include "Scene.h"
#include "graphicsAnimations.h"

class FireworksScene : public Scene {

public:
    explicit FireworksScene(GraphicsAnimations& animations);

    const char* name() const {
        return "FIREW";
    }

    void begin() override;

    void processPose(const WandPacket& packet) override;
    void processGesture(const GestureEvent& event) override;

    void render(PixelFramebuffer& framebuffer) override;

    // Set the value from 0-100 (0 being closest to LEDs, 100 being most diffused - farthest from LEDs)
    int preferredDiffuserPosition() const override { return 10; }  

private:
    GraphicsAnimations& gAnim;
};