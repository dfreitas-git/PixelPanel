
#include "ReflectingPondScene.h"

void ReflectingPondScene::begin() { }

void ReflectingPondScene::processGesture(const GestureEvent& event) { }

void ReflectingPondScene::onSingleClick() {
    using RenderMode = ReflectingPondCore::RenderMode;

    if (pond.getRenderMode() == RenderMode::HYBRID) {
        pond.setRenderMode(RenderMode::HEIGHT_COLOR);
        Serial.println("HEIGHT_COLOR");
    } else if (pond.getRenderMode() == RenderMode::HEIGHT_COLOR) {
        pond.setRenderMode(RenderMode::SLOPE_LIGHT);
        Serial.println("SLOPE_LIGHT");
    } else {
        pond.setRenderMode(RenderMode::HYBRID);
        Serial.println("HYBRID");
    }
}

void ReflectingPondScene::render(PixelFramebuffer& framebuffer) {
    pond.render(millis(), pixels);
    pond.setSpeed(20.0);
    pond.setIntensity(0.25);
    for (int y = 0; y < 30; ++y) {
        for (int x = 0; x < 30; ++x) {
            auto c = pixels[y][x];

            // Replace with your actual framebuffer pixel-write call:
            framebuffer.setPixel(x, y, c.r, c.g, c.b);
        }
    }
 }
