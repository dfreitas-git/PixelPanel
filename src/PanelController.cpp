#include "PanelController.h"

void PanelController::processPose(const WandPacket& packet) {
    currentMode = packet.mode;
    if (currentMode == WandMode::Scene && activeScene)
        activeScene->processPose(packet);
}

void PanelController::addScene(Scene& scene) {
    if (sceneCount < MAX_SCENES)
        scenes[sceneCount++] = &scene;
}

void PanelController::selectScene(uint8_t index) {
    if (index >= sceneCount) return;
    if (activeScene != nullptr && activeSceneIndex == index) return;

    activeSceneIndex = index;
    activeScene = scenes[index];
    activeScene->begin();
}

const char* PanelController::getSceneName(uint8_t index) const {
    return index < sceneCount ? scenes[index]->name() : "";
}

void PanelController::processGesture(const GestureEvent& event) {
    // Menu gestures are deliberately not routed here. PanelMenu owns them.
    if (event.mode == WandMode::Scene && activeScene)
        activeScene->processGesture(event);
}

void PanelController::renderScene(PixelFramebuffer& framebuffer) {
    if (activeScene)
        activeScene->render(framebuffer);
}
