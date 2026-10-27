
#include "PanelController.h"

void PanelController::processPose(const WandPacket& packet) {

    currentMode = packet.mode;

    if (currentMode == WandMode::Scene) {
        if (activeScene)
            activeScene->processPose(packet);
    }
    else {
        processPanelPose(packet);
    }
}

void PanelController::setActiveScene(Scene& scene) {
    activeScene = &scene;
    activeScene->begin();
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

void PanelController::processPanelGesture(
    const GestureEvent& event)
{
    // Future panel-wide gesture controls
}