#pragma once

#include <Arduino.h>
#include "WandProtocol.h"
#include "PixelFramebuffer.h"
#include "graphicsPrimitives.h"

class PanelController; // The menu only queries its public scene interface.

enum class MenuPage : uint8_t {
    Main,
    Scenes,
    Panel,
    Test,
    Count
};

enum class MenuDirection : uint8_t {
    None,
    Left,
    Right,
    Up,
    Down
};

enum class MenuActionType : uint8_t {
    None,
    SelectScene,
    SelectTest,
    SetBrightness, // Reserved for future PANEL functions
    SetDiffuser,
};

struct MenuAction {
    MenuActionType type = MenuActionType::None;
    int value = 0;
};

class PanelMenu {
public:
    explicit PanelMenu(GraphicsPrimitives& graphics) : gPrim(graphics) {}

    // Returns an action only when the menu asks the application to do something.
    MenuAction processPose(const WandPacket& packet, const PanelController& controller);

    void resetNavigation();
    void render(PixelFramebuffer& framebuffer, const PanelController& controller);

    MenuPage getCurrentPage() const { return currentPage; }

private:
    struct PageState {
        uint8_t selectedIndex = 0;
        uint8_t firstVisibleIndex = 0;
    };

    GraphicsPrimitives& gPrim;
    MenuPage currentPage = MenuPage::Main;
    PageState pageState[static_cast<uint8_t>(MenuPage::Count)]{};

    static constexpr uint8_t VISIBLE_ROWS = 4;

    MenuDirection heldDirection = MenuDirection::None;
    uint32_t lastStepMs = 0;
    bool repeatStarted = false;

    static constexpr float ENGAGE_ANGLE_PITCH  = 15.0f * PI / 180.0f;
    static constexpr float RELEASE_ANGLE_PITCH = 10.0f * PI / 180.0f;
    static constexpr float ENGAGE_ANGLE_ROLL   = 28.0f * PI / 180.0f;
    static constexpr float RELEASE_ANGLE_ROLL  = 20.0f * PI / 180.0f;
    static constexpr uint32_t INITIAL_REPEAT_MS = 500;
    static constexpr uint32_t REPEAT_MS = 300;

    PageState& state(MenuPage page) {
        return pageState[static_cast<uint8_t>(page)];
    }
    const PageState& state(MenuPage page) const {
        return pageState[static_cast<uint8_t>(page)];
    }

    MenuDirection processNavigationPose(const WandPacket& packet);
    void changePage(MenuPage page, const PanelController& controller);
    uint8_t itemCount(MenuPage page, const PanelController& controller) const;
    const char* itemName(MenuPage page, uint8_t index, const PanelController& controller) const;
    void updateVisibleWindow(PageState& page, uint8_t count);
    MenuAction selectionAction() const;
};
