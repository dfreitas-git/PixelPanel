#include "PanelMenu.h"
#include "PanelController.h"
#include <math.h>

namespace {
    // Five characters maximum per item for the 30x30 display.
    const char* const MAIN_ITEMS[]  = { "SCENE", "SETUP", "TEST" };
    // These two lists are placeholders for future action handling.
    const char* const PANEL_ITEMS[] = { "BRITE", "DIFFO" };
    const char* const TEST_ITEMS[]  = {
        "CORNR", "ROWS", "COLS", "BAND", "COLOR", "PIXEL", "DIAG"
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
    navigationArmed = false;
}

// Pitch-only navigator. Roll is ignored completely.
// Following push/pop, pitch must return to neutral before navigating again.
MenuDirection PanelMenu::processNavigationPose(const WandPacket& packet) {
    const float pitch = packet.pitch;
    const uint32_t now = millis();

    if (!navigationArmed) {
        if (fabsf(pitch) < RELEASE_ANGLE_PITCH)
            navigationArmed = true;
        return MenuDirection::None;
    }

    if (heldDirection != MenuDirection::None) {
        if (fabsf(pitch) < RELEASE_ANGLE_PITCH) {
            heldDirection = MenuDirection::None;
            repeatStarted = false;
            return MenuDirection::None;
        }

        // If the user reverses direction without first returning to neutral,
        // release the old direction, then require neutral before rearming.
        if ((heldDirection == MenuDirection::Up && pitch < 0) ||
            (heldDirection == MenuDirection::Down && pitch > 0)) {
            resetNavigation();
            return MenuDirection::None;
        }

        uint32_t interval = INITIAL_REPEAT_MS;
        if (repeatStarted) {
            interval = fabsf(pitch) > FAST_ENGAGE_ANGLE_PITCH
                ? FAST_REPEAT_MS : REPEAT_MS;
        }
        if (now - lastStepMs >= interval) {
            lastStepMs = now;
            repeatStarted = true;
            return heldDirection;
        }
        return MenuDirection::None;
    }

    if (fabsf(pitch) > ENGAGE_ANGLE_PITCH) {
        heldDirection = pitch > 0 ? MenuDirection::Up : MenuDirection::Down;
        lastStepMs = now;
        repeatStarted = false;
        return heldDirection;
    }
    return MenuDirection::None;
}

void PanelMenu::changePage(MenuPage page, const PanelController& controller) {
    currentPage = page;
    if (page == MenuPage::Scenes) {
        state(page).selectedIndex = controller.getActiveSceneIndex();
    }
    resetNavigation();
}

// One click means "enter highlighted item".
// Only entering an animation/test requests an application action.
MenuAction PanelMenu::push(const PanelController& controller) {
    MenuAction action{};
    if (currentView != MenuView::List) return action;

    const PageState& page = state(currentPage);
    if (currentPage == MenuPage::Main) {
        switch (page.selectedIndex) {
            case 0: changePage(MenuPage::Scenes, controller); break;
            case 1: changePage(MenuPage::Panel, controller); break;
            case 2: changePage(MenuPage::Test, controller); break;
        }
    } else if (currentPage == MenuPage::Scenes) {
        if (page.selectedIndex < controller.getSceneCount()) {
            action.type = MenuActionType::SelectScene;
            action.value = page.selectedIndex;
            currentView = MenuView::Scene;
            resetNavigation();
        }
    } else if (currentPage == MenuPage::Test) {
        if (page.selectedIndex < TEST_COUNT) {
            action.type = MenuActionType::SelectTest;
            action.value = page.selectedIndex;
            currentView = MenuView::Scene;
            resetNavigation();
        }
    } else if (currentPage == MenuPage::Panel) {
        if (page.selectedIndex < PANEL_COUNT) {
            currentView = MenuView::Value;
            resetNavigation();
        }
    }
    return action;
}

void PanelMenu::pop() {
    if (currentView != MenuView::List) {
        // Scene -> its list, or Value -> PANEL list.
        currentView = MenuView::List;
    } else if (currentPage != MenuPage::Main) {
        currentPage = MenuPage::Main;
    }
    resetNavigation();
}


MenuAction PanelMenu::adjustValue(MenuDirection direction)
{
    MenuAction action{};

    const int increment =
        (direction == MenuDirection::Up) ? 1 : -1;

    switch (state(MenuPage::Panel).selectedIndex) {

        case 0:  // BRITE
            brightness = constrain(brightness + increment, 0, 100);

            action.type = MenuActionType::SetBrightness;
            action.value = brightness;
            break;

        case 1:  // DIFFO
            diffuserOffset = constrain(diffuserOffset + increment, -100, 100);
            action.type = MenuActionType::SetDiffuser;
            action.value = diffuserOffset;
            break;
    }

    return action;
}















MenuAction PanelMenu::processPose(const WandPacket& packet, const PanelController& controller) {
    MenuAction action{};
    if (isRunningScene()) return action; // Pose belongs to the running scene instead.

    const MenuDirection direction = processNavigationPose(packet);
    if (direction == MenuDirection::None) return action;

    if (currentView == MenuView::Value) {
        return adjustValue(direction);
    }

    PageState& page = state(currentPage);
    const uint8_t count = itemCount(currentPage, controller);
    if (count == 0) return action;

    if (direction == MenuDirection::Down)
        page.selectedIndex = (page.selectedIndex + 1) % count;
    else
        page.selectedIndex = (page.selectedIndex + count - 1) % count;

    // Highlighting is not selection. A single-click (push) activates an item.
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

void PanelMenu::renderValue(PixelFramebuffer& framebuffer)
{
    const uint8_t selected = state(MenuPage::Panel).selectedIndex;
    const char* name = "";

    int value = 0;

    switch (selected) {

        case 0:
            name = "BRITE";
            value = brightness;
            break;

        case 1:
            name = "DIFFO";
            value = diffuserOffset;
            break;

        default:
            return;
    }

    char valueString[8];

    if (selected == 1) {
        if(value == 0) {
            snprintf(valueString, sizeof(valueString)," %d", value);  // add a space before the zero to keep it aligned
        } else {
            snprintf(valueString, sizeof(valueString), value > 0 ? "+%d" : "%d", value);
        }
    } else {
        snprintf(valueString, sizeof(valueString), "%d", value);
    }

    const RGB labelColor = {20, 20, 20};
    const RGB valueColor = {0, 100, 0};
    const RGB background = {0, 0, 0};

    gPrim.drawString(
        framebuffer.data(),
        name,
        0, 20,
        labelColor,
        background
    );

    gPrim.drawString(
        framebuffer.data(),
        valueString,
        7, 10,
        valueColor,
        background
    );
}

void PanelMenu::render(PixelFramebuffer& framebuffer, const PanelController& controller) {
    framebuffer.clear();

    if (currentView == MenuView::Scene) return; // Normally rendered by PanelController.

    if (currentView == MenuView::Value) {
        renderValue(framebuffer);
        return;
    }

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
