#include "PanelController.h"

void PanelController::processPose(const WandPacket& packet) {
    if (activeScene)
        activeScene->processPose(packet);
}

void PanelController::addScene(Scene& scene) {
    if (sceneCount < MAX_SCENES)
        scenes[sceneCount++] = &scene;
}

void PanelController::addGames(Scene& game) {
    if (gamesCount < MAX_GAMES)
        games[gamesCount++] = &game;
}

void PanelController::selectScene(uint8_t index)
{
    if (index >= sceneCount)
        return;

    activeSceneIndex = index;
    setActiveScene(*scenes[index]);
}

void PanelController::selectGames(uint8_t index)
{
    if (index >= gamesCount)
        return;

    activeGamesIndex = index;
    setActiveGames(*games[index]);
}

void PanelController::setActiveScene(Scene& scene) {
    if (activeScene == &scene)
        return;

    activeScene = &scene;
    activeScene->begin();
}

void PanelController::setActiveGames(Scene& game) {
    if (activeScene == &game)
        return;

    activeScene = &game;
    activeScene->begin();
}

const char* PanelController::getSceneName(uint8_t index) const {
    return index < sceneCount ? scenes[index]->name() : "";
}

const char* PanelController::getGamesName(uint8_t index) const {
    return index < gamesCount ? games[index]->name() : "";
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

void PanelController::sampleBrightnessPot() {
    int potVal = analogRead(BRIGHTNESS_POT_PIN);
    potBrightness = map(potVal, 0, 4095, 0, 100);
}

void PanelController::onSingleClick() {
    if (activeScene)
        activeScene->onSingleClick();
}