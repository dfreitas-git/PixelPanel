#include "PanelMenu.h"

// Code to present a menu for selecting scenes, controlling panel parameters (diffuser, brightness, etc.)

MenuDirection PanelMenu::processNavigationPose(
    const WandPacket& packet)
{
    float roll = packet.roll;
    float pitch = packet.pitch;

    uint32_t now = millis();

    // If a direction is already held, retain it until
    // its axis returns near neutral.
    if (heldDirection != MenuDirection::None) {

        bool released = false;

        switch (heldDirection) {

            case MenuDirection::Right:
            case MenuDirection::Left:
                released = fabsf(roll) < RELEASE_ANGLE_ROLL;
                break;

            case MenuDirection::Up:
            case MenuDirection::Down:
                released = fabsf(pitch) < RELEASE_ANGLE_PITCH;
                break;

            default:
                break;
        }

        if (released) {
            heldDirection = MenuDirection::None;
            repeatStarted = false;
            return MenuDirection::None;
        }

        uint32_t interval = repeatStarted
            ? REPEAT_MS
            : INITIAL_REPEAT_MS;

        if (now - lastStepMs >= interval) {
            lastStepMs = now;
            repeatStarted = true;
            return heldDirection;
        }

        return MenuDirection::None;
    }

    // Nothing currently held: detect a new direction.
    if (fabsf(roll) > ENGAGE_ANGLE_ROLL &&
        fabsf(roll) >= fabsf(pitch)) {

        heldDirection = roll > 0
            ? MenuDirection::Right
            : MenuDirection::Left;
    }
    else if (fabsf(pitch) > ENGAGE_ANGLE_PITCH) {

        heldDirection = pitch > 0
            ? MenuDirection::Up
            : MenuDirection::Down;
    }

    if (heldDirection != MenuDirection::None) {
        lastStepMs = now;
        repeatStarted = false;
        return heldDirection;  // Immediate first movement
    }

    return MenuDirection::None;
}

void PanelMenu::resetNavigation() {
    heldDirection = MenuDirection::None;
    repeatStarted = false;
    lastStepMs = 0;
}

void PanelMenu::updateVisibleWindow(uint8_t selectedIndex)
{
    // Selected item is above the visible window
    if (selectedIndex < firstVisibleIndex) {
        firstVisibleIndex = selectedIndex;
    }

    // Selected item is below the visible window
    else if (selectedIndex >= firstVisibleIndex + VISIBLE_ROWS) {
        firstVisibleIndex = selectedIndex - VISIBLE_ROWS + 1;
    }
}

void PanelMenu::render( PixelFramebuffer& framebuffer, const PanelController& controller)
{
    framebuffer.clear();

    uint8_t selectedIndex;
    uint8_t itemCount;
    const char* itemName;

    switch(controller.getCurrentPage()) {
        case MenuPage::Scenes:
            itemCount = controller.getSceneCount();
            if (itemCount == 0)
                return;

            selectedIndex = controller.getActiveSceneIndex();
            break;

        case MenuPage::Main:
            itemCount = controller.getMainCount();
            if (itemCount == 0)
                return;

            selectedIndex = controller.getActiveMainIndex();
            break;
    }

    updateVisibleWindow(selectedIndex);

    // Menu colors
    const RGB normalColor   = {20, 20, 20};  // dark white
    const RGB selectedColor = {0, 100, 0};   // green
    const RGB background    = {0, 0, 0};     // black

    for (uint8_t line = 0; line < VISIBLE_ROWS; line++) {
        uint8_t itemIndex = firstVisibleIndex + line;

        if (itemIndex >= itemCount)
            break;
        if(controller.getCurrentPage() == MenuPage::Main) 
            itemName = controller.getMainName(itemIndex);
        if(controller.getCurrentPage() == MenuPage::Scenes) 
            itemName = controller.getSceneName(itemIndex);

        RGB color = (itemIndex == selectedIndex)
            ? selectedColor
            : normalColor;

        // line 0 is top of display
        int y = 23 - (line * 7);

        gPrim.drawString(
            framebuffer.data(),
            itemName,
            0,
            y,
            color,
            background
        );

        // Additional menu items above the visible window
        if (firstVisibleIndex > 0) {
            framebuffer.setPixel(14, 29, 0, 50, 0);
            framebuffer.setPixel(15, 29, 0, 50, 0);
        }
        
        // Additional menu items below the visible window
        if (firstVisibleIndex + VISIBLE_ROWS < itemCount) {
            framebuffer.setPixel(14, 0, 0, 50, 0);
            framebuffer.setPixel(15, 0, 0, 50, 0);
        }
    }
}