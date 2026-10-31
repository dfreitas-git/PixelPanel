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

enum class MenuActionType {
    None,
    SelectScene,
    SetBrightness,
    SetDiffuser,
    RunTest
};

struct MenuAction {
    MenuActionType type = MenuActionType::None;
    int value = 0;
};

class PanelMenu {

public:
    explicit PanelMenu(GraphicsPrimitives& graphics) : gPrim(graphics) {}
    void resetNavigation();
    MenuDirection processNavigationPose(const WandPacket& packet);
    void render(PixelFramebuffer& framebuffer, const PanelController& controller);


private:
    GraphicsPrimitives& gPrim;
    uint8_t firstVisibleIndex = 0;
    static constexpr uint8_t VISIBLE_ROWS = 4;
    void updateVisibleWindow(uint8_t selectedIndex);

    MenuDirection heldDirection = MenuDirection::None;
    MenuActionType menuActionType;

    uint32_t lastStepMs = 0;
    bool repeatStarted = false;

    static constexpr float ENGAGE_ANGLE_PITCH = 15.0f * PI / 180.0f;
    static constexpr float RELEASE_ANGLE_PITCH = 10.0f * PI / 180.0f;
    static constexpr float ENGAGE_ANGLE_ROLL = 28.0f * PI / 180.0f;
    static constexpr float RELEASE_ANGLE_ROLL = 20.0f * PI / 180.0f;
    static constexpr uint32_t INITIAL_REPEAT_MS = 500;
    static constexpr uint32_t REPEAT_MS = 300;

};