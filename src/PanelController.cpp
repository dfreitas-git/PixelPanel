
#include "PanelController.h"


// Either process the packet for the scene or for the panel base on what mode the packet is intended for
void PanelController::processPose(const WandPacket& packet) {

    currentMode = packet.mode;

    if (currentMode == WandMode::Scene) {
        if (activeScene)
            activeScene->processPose(packet);

    } else {
        processPanelPose(packet);
    }
}

void PanelController::addScene(Scene& scene) {

    if (sceneCount >= MAX_SCENES)
        return;

    scenes[sceneCount++] = &scene;
}

void PanelController::selectMain(uint8_t index) {

    if (index >= mainCount)
        return;

    activeMainIndex = index;
}

void PanelController::selectScene(uint8_t index) {

    if (index >= sceneCount)
        return;

    if (activeSceneIndex == index && activeScene != nullptr)
        return;

    activeSceneIndex = index;
    activeScene = scenes[index];

    activeScene->begin();
}

const char* PanelController::getSceneName(uint8_t index) const {
    if (index >= sceneCount) return "";
    return scenes[index]->name();
}

void PanelController::processGesture(
    const GestureEvent& event)
{
    if (event.mode == WandMode::Scene) {
        if (activeScene)
            activeScene->processGesture(event);
    }
    else {
        processPanelGesture(event);
    }
}

void PanelController::renderScene(
    PixelFramebuffer& framebuffer)
{
    if (activeScene)
        activeScene->render(framebuffer);
}

void PanelController::processPanelPose(
    const WandPacket& packet)
{
    // Future panel-wide pose controls
}


// Panel-mode Gestures are for panel setup, scene selection, etc.
void PanelController::processPanelGesture( const GestureEvent& event)
{
    switch (event.gesture) {
        case WandGesture::Swipe:
            if(DEBUG_PRINT == 1) {
                Serial.println(F("Process Panel Swipe"));
            } 
            break;

        case WandGesture::ThrustIn:
            if(DEBUG_PRINT == 1) {
                Serial.println(F("Process Panel ThrustIn"));
            } 
            // if(main[activeMainIndex] == "SCENE") {
            //     currentPage = (MenuPage::Scenes);
            // }
            break;

        case WandGesture::ThrustOut:
            if(DEBUG_PRINT == 1) {
                Serial.println(F("Process Panel ThrustOut"));
            } 
            // currentPage = (MenuPage::Main);
            break;
    }
}