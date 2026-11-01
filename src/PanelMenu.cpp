#include "PanelMenu.h"
#include "PanelController.h"
#include <math.h>

namespace {
    // Five characters maximum per item for the 30x30 display.
    const char* const MAIN_ITEMS[]  = { "SCENE", "PANEL", "TEST" };
    // These two lists are placeholders for future action handling.
    const char* const PANEL_ITEMS[] = { "BRITE", "DIFFD" };
    const char* const TEST_ITEMS[]  = {
        "CORNR", "ROWS", "COLS", "BAND", "COLOR", "PIXEL", "DIAG", "ALL"
    };

    constexpr uint8_t MAIN_COUNT  = sizeof(MAIN_ITEMS)  / sizeof(MAIN_ITEMS[0]);
    constexpr uint8_t PANEL_COUNT = sizeof(PANEL_ITEMS) / sizeof(PANEL_ITEMS[0]);
    constexpr uint8_t TEST_COUNT  = sizeof(TEST_ITEMS)  / sizeof(TEST_ITEMS[0]);
}

uint8_t PanelMenu::itemCount(MenuPage page, const PanelController& controller) const {
    switch (page) {
        case MenuPage::Main:   return MAIN_COUNT;
        case MenuPage::Scenes: return controller.getSceneCount();
        case MenuPage::Panel:  return PANEL_COUNT;
        case MenuPage::Test:   return TEST_COUNT;
        default:               return 0;
    }
}

const char* PanelMenu::itemName(MenuPage page, uint8_t index, const PanelController& controller) const {
    if (index >= itemCount(page, controller)) return "";
    switch (page) {
        case MenuPage::Main:   return MAIN_ITEMS[index];
        case MenuPage::Scenes: return controller.getSceneName(index);
        case MenuPage::Panel:  return PANEL_ITEMS[index];
        case MenuPage::Test:   return TEST_ITEMS[index];
        default:               return "";
    }
}

void PanelMenu::resetNavigation() {
    heldDirection = MenuDirection::None;
    repeatStarted = false;
    lastStepMs = 0;
}

MenuDirection PanelMenu::processNavigationPose(const WandPacket& packet) {
    const float roll = packet.roll;
    const float pitch = packet.pitch;
    const uint32_t now = millis();

    if (heldDirection != MenuDirection::None) {
        const bool horizontal = (heldDirection == MenuDirection::Left ||
                                 heldDirection == MenuDirection::Right);
        const bool released = horizontal
            ? fabsf(roll) < RELEASE_ANGLE_ROLL
            : fabsf(pitch) < RELEASE_ANGLE_PITCH;

        if (released) {
            resetNavigation();
            return MenuDirection::None;
        }

        // Left/right changes menu LEVEL, so it is deliberately one-shot.
        if (horizontal) return MenuDirection::None;

        // Up/down scrolls items: immediate first step, then key-repeat.
        const uint32_t interval = repeatStarted ? REPEAT_MS : INITIAL_REPEAT_MS;
        if (now - lastStepMs >= interval) {
            lastStepMs = now;
            repeatStarted = true;
            return heldDirection;
        }
        return MenuDirection::None;
    }

    if (fabsf(roll) > ENGAGE_ANGLE_ROLL && fabsf(roll) >= fabsf(pitch)) {
        heldDirection = roll > 0 ? MenuDirection::Right : MenuDirection::Left;
    } else if (fabsf(pitch) > ENGAGE_ANGLE_PITCH) {
        heldDirection = pitch > 0 ? MenuDirection::Up : MenuDirection::Down;
    }

    if (heldDirection != MenuDirection::None) {
        lastStepMs = now;
        repeatStarted = false;
        return heldDirection;
    }
    return MenuDirection::None;
}

void PanelMenu::changePage(MenuPage page, const PanelController& controller) {
    currentPage = page;
    if (page == MenuPage::Scenes) {
        // Synchronize with the scene that is actually selected.
        state(page).selectedIndex = controller.getActiveSceneIndex();
    }
    // Do not reset the navigator here: the user must release the roll
    // before another left/right page transition is accepted.
}

MenuAction PanelMenu::processPose(const WandPacket& packet, const PanelController& controller) {
    MenuAction action{};
    const MenuDirection direction = processNavigationPose(packet);
    PageState& page = state(currentPage);
    const uint8_t count = itemCount(currentPage, controller);

    if (direction == MenuDirection::Up || direction == MenuDirection::Down) {
        if (count == 0) return action;

        if (direction == MenuDirection::Down)
            page.selectedIndex = (page.selectedIndex + 1) % count;
        else
            page.selectedIndex = (page.selectedIndex + count - 1) % count;

        if (currentPage == MenuPage::Scenes) {
            action.type = MenuActionType::SelectScene;
            action.value = page.selectedIndex;
        }
    } else if (direction == MenuDirection::Right && currentPage == MenuPage::Main) {
        switch (page.selectedIndex) {
            case 0: changePage(MenuPage::Scenes, controller); break;
            case 1: changePage(MenuPage::Panel, controller); break;
            case 2: changePage(MenuPage::Test, controller); break;
        }
    } else if (direction == MenuDirection::Left && currentPage != MenuPage::Main) {
        changePage(MenuPage::Main, controller);
    }
    return action;
}

void PanelMenu::updateVisibleWindow(PageState& page, uint8_t count) {
    if (count == 0) {
        page.selectedIndex = 0;
        page.firstVisibleIndex = 0;
        return;
    }
    if (page.selectedIndex >= count) page.selectedIndex = count - 1;
    if (page.selectedIndex < page.firstVisibleIndex)
        page.firstVisibleIndex = page.selectedIndex;
    else if (page.selectedIndex >= page.firstVisibleIndex + VISIBLE_ROWS)
        page.firstVisibleIndex = page.selectedIndex - VISIBLE_ROWS + 1;
}

void PanelMenu::render(PixelFramebuffer& framebuffer, const PanelController& controller) {
    framebuffer.clear();
    const uint8_t count = itemCount(currentPage, controller);
    if (count == 0) return;

    PageState& page = state(currentPage);
    updateVisibleWindow(page, count);

    const RGB normalColor   = {20, 20, 20};
    const RGB selectedColor = {0, 100, 0};
    const RGB background    = {0, 0, 0};

    for (uint8_t line = 0; line < VISIBLE_ROWS; ++line) {
        const uint8_t index = page.firstVisibleIndex + line;
        if (index >= count) break;
        const RGB color = index == page.selectedIndex ? selectedColor : normalColor;
        gPrim.drawString(framebuffer.data(),
                         itemName(currentPage, index, controller),
                         0, 23 - line * 7, color, background);
    }

    // Draw scroll indicators once, rather than during every text row.
    if (page.firstVisibleIndex > 0) {
        framebuffer.setPixel(14, 29, 0, 50, 0);
        framebuffer.setPixel(15, 29, 0, 50, 0);
    }
    if (page.firstVisibleIndex + VISIBLE_ROWS < count) {
        framebuffer.setPixel(14, 0, 0, 50, 0);
        framebuffer.setPixel(15, 0, 0, 50, 0);
    }
}
