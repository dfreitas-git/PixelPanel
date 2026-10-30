#pragma once

#include <Arduino.h>
#include "Globals.h"
#include "WandProtocol.h"
#include "PixelFramebuffer.h"
#include "PanelController.h"
#include "graphicsPrimitives.h"

enum class MenuDirection {
    None,
    Left,
    Right,
    Up,
    Down
};

class PanelMenu {

public:
    explicit PanelMenu(GraphicsPrimitives& graphics) : gPrim(graphics) {}

    MenuDirection processNavigationPose(const WandPacket& packet);
    void resetNavigation();

    void render(PixelFramebuffer& framebuffer, const PanelController& controller);

private:
    GraphicsPrimitives& gPrim;
    uint8_t firstVisibleIndex = 0;
    static constexpr uint8_t VISIBLE_ROWS = 4;
    void updateVisibleWindow(uint8_t selectedIndex);

    MenuDirection heldDirection = MenuDirection::None;

    uint32_t lastStepMs = 0;
    bool repeatStarted = false;

    static constexpr float ENGAGE_ANGLE = 28.0f * PI / 180.0f;
    static constexpr float RELEASE_ANGLE = 20.0f * PI / 180.0f;
    static constexpr uint32_t INITIAL_REPEAT_MS = 500;
    static constexpr uint32_t REPEAT_MS = 300;

};