
#include "ReflectingPondScene.h"

void ReflectingPondScene::begin() {
    pond.setRenderMode(ReflectingPondCore::HEIGHT_COLOR);
    pond.setSpeed(10.0f);
    pond.setIntensity(0.17f);
    pond.setShimmerStrength(1.0f);
    pond.setShimmerThreshold(0.93f);
    pond.setShimmerSpeed(1.0f);
}

void ReflectingPondScene::processGesture(const GestureEvent& event) {
    if (event.gesture == WandGesture::ThrustIn || event.gesture == WandGesture::ThrustOut) {
        pond.addDrop(float(random(5,25)), float(random(5,25)), millis());
    } else if(event.gesture == WandGesture::Swipe) {
        skipDirection = event.swipeDirection;
        skipRunning = true;
        skipIndex = 0;
        nextSkip = millis();
    }
}


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
    int32_t now = millis();
    float X, Y;
    float centerX = 15.0;
    float centerY = 15.0;
    float span = 10.0;

    if(skipRunning && (now - nextSkip > 0 )) {

        switch(skipIndex) {
            case 0:
                Y = centerY - span * sin(skipDirection);
                X = centerX - span * cos(skipDirection);
                break;

            case 1:
                Y = centerY;
                X = centerX;
                break;

            case 2:
                Y = centerY + span * sin(skipDirection);
                X = centerX + span * cos(skipDirection);
                break;

            default:
                skipRunning = false;
                return;
        }

        pond.addDrop(X, Y, now);
        skipIndex++;
        nextSkip = now + 300;   // time (mS) between skips
    }

    pond.render(millis(), pixels);

    for (int y = 0; y < 30; ++y) {
        for (int x = 0; x < 30; ++x) {
            auto c = pixels[y][x];

            framebuffer.setPixel(x, y, c.r, c.g, c.b);
        }
    }
}
