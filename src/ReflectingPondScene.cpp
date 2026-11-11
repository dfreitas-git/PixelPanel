
#include "ReflectingPondScene.h"

void ReflectingPondScene::begin() {
    pond.setRenderMode(ReflectingPondCore::HEIGHT_COLOR);
    pond.setSpeed(10.0f);
    pond.setIntensity(0.17f);
    pond.setShimmerStrength(1.0f);
    pond.setShimmerThreshold(0.93f);
    pond.setShimmerSpeed(1.0f);
}

void ReflectingPondScene::processGesture(const GestureEvent& event) { }

void ReflectingPondScene::onSingleClick() {
    using RenderMode = ReflectingPondCore::RenderMode;

    /*
    static float couple = 0.0;
    pond.setShimmerCoupling(couple);
    Serial.println(couple);
    couple += 0.5;
    if(couple > 1.0) {
        couple = 0.0;
    }
    */

    /*
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
        */
}

void ReflectingPondScene::render(PixelFramebuffer& framebuffer) {
    pond.render(millis(), pixels);
    for (int y = 0; y < 30; ++y) {
        for (int x = 0; x < 30; ++x) {
            auto c = pixels[y][x];

            framebuffer.setPixel(x, y, c.r, c.g, c.b);
        }
    }
 }
