#include "PanelController.h"

void PanelController::processPose(const WandPacket& packet) {
    if (activeScene)
        activeScene->processPose(packet);
}

void PanelController::addScene(Scene& scene) {
    if (sceneCount < MAX_SCENES)
        scenes[sceneCount++] = &scene;
}

void PanelController::selectScene(uint8_t index)
{
    if (index >= sceneCount)
        return;

    activeSceneIndex = index;
    setActiveScene(*scenes[index]);
}

void PanelController::setActiveScene(Scene& scene) {
    if (activeScene == &scene)
        return;

    activeScene = &scene;
    activeScene->begin();
}

const char* PanelController::getSceneName(uint8_t index) const {
    return index < sceneCount ? scenes[index]->name() : "";
}

void PanelController::processGesture(const GestureEvent& event) {
    // Caller only invokes this while a scene is displayed.
    if (activeScene)
        activeScene->processGesture(event);
}

void PanelController::renderScene(PixelFramebuffer& framebuffer) {
    if (activeScene)
        activeScene->render(framebuffer);
}

int PanelController::getSceneDiffuserPosition() const {
    return activeScene
        ? activeScene->preferredDiffuserPosition()
        : 10;
}