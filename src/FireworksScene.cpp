
#include "FireworksScene.h"

FireworksScene::FireworksScene(GraphicsAnimations& animations)
    : gAnim(animations)
{
}

void FireworksScene::begin() {

    gAnim.initRocket();
}

void FireworksScene::processPose(const WandPacket& packet) {

    // Roll controls gravity constant (makes the explosions fall toward earth more/less)
    int rollInDegrees = constrain(round(packet.roll * 180.0f / PI), -45, 45);
    settings.gravity = float(map(rollInDegrees, -45, 45, 1, 50) / 25.0);

    // Pitch controls time before initing another launch
    int pitchInDegrees = constrain(round(packet.pitch * 180.0f / PI), -45, 45);
    settings.timeUntilInit =  map(pitchInDegrees, -45, 45, 2000, 500);
}

void FireworksScene::processGesture(const GestureEvent& event) {

    // No gesture controls yet
}

void FireworksScene::render(PixelFramebuffer& framebuffer) {

    gAnim.fireworks(framebuffer.data(), settings);
    }