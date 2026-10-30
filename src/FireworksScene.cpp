
#include "FireworksScene.h"

FireworksScene::FireworksScene(GraphicsAnimations& animations)
    : gAnim(animations)
{
}

void FireworksScene::begin() {

    gAnim.initRocket();
}

void FireworksScene::processPose(const WandPacket& packet) {

    // No pose controls yet
}

void FireworksScene::processGesture(const GestureEvent& event) {

    // No gesture controls yet
}

void FireworksScene::render(PixelFramebuffer& framebuffer) {

    gAnim.fireworks(framebuffer.data());
}