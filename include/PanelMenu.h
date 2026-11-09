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
    Games,
    Count
};

enum class MenuView {
    List,
    Value,
    Scene  // A running animation or test; back returns to its list
};


enum class MenuDirection : uint8_t {
    None,
    Up,
    Down
};

enum class MenuActionType : uint8_t {
    None,
    SelectScene,
    SelectTest,
    SelectGames,
    SetBrightness, 
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
    MenuAction push(const PanelController& controller);  // Single-click
    void pop();                                         // Double-click
    bool isRunningScene() const { return currentView == MenuView::Scene; }

    void resetNavigation();
    void render(PixelFramebuffer& framebuffer, const PanelController& controller);
    void renderValue(PixelFramebuffer& framebuffer);

    MenuPage getCurrentPage() const { return currentPage; }

private:
    struct PageState {
        uint8_t selectedIndex = 0;
        uint8_t firstVisibleIndex = 0;
    };

    GraphicsPrimitives& gPrim;
    MenuPage currentPage = MenuPage::Main;
    MenuView currentView = MenuView::List;
    PageState pageState[static_cast<uint8_t>(MenuPage::Count)]{};

    static constexpr uint8_t VISIBLE_ROWS = 4;

    MenuDirection heldDirection = MenuDirection::None;
    uint32_t lastStepMs = 0;
    bool repeatStarted = false;
    bool navigationArmed = false; // Require neutral pitch after a push/pop

    static constexpr float ENGAGE_ANGLE_PITCH  = 20.0f * PI / 180.0f;
    static constexpr float RELEASE_ANGLE_PITCH = 15.0f * PI / 180.0f;
    static constexpr float FAST_ENGAGE_ANGLE_PITCH  = 45.0f * PI / 180.0f;

    static constexpr uint32_t INITIAL_REPEAT_MS = 1000;
    static constexpr uint32_t REPEAT_MS = 300;
    static constexpr uint32_t FAST_REPEAT_MS = 100;

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
    MenuAction adjustValue(MenuDirection direction);
    uint8_t brightness = 100;
    //uint8_t diffuserOffset = 0;
    int8_t diffuserOffset = 0;
};
