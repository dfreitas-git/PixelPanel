
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
    } else if (event.gesture == WandGesture::Swipe) {
        if (swipeEffect == BUBBLES) {
            bubbleDirection = event.swipeDirection;
            bubbleStartMs = millis();
            nextBubbleStep = 0;
            bubbleRunning = true;
            skipRunning = false;
        } else {
            skipDirection = event.swipeDirection;
            skipRunning = true;
            bubbleRunning = false;
            skipIndex = 0;
            nextSkip = millis();
        }
    }
}


void ReflectingPondScene::onSingleClick() {
    swipeEffect = (swipeEffect == SKIP) ? BUBBLES : SKIP;
    skipRunning = false;  // Cancel any pending skip when switching modes.
    Serial.println(swipeEffect == SKIP ? "POND: SKIP" : "POND: BUBBLES");
}


void ReflectingPondScene::render(PixelFramebuffer& framebuffer) {
    uint32_t now = millis();
    float X, Y;
    float centerX = 14.5;
    float centerY = 14.5;
    float span = 10.0;

    if (skipRunning && (int32_t)(now - nextSkip) >= 0) {

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

        }
        pond.addDrop(X, Y, now);
        skipIndex++;

        if (skipIndex >= 3)
            skipRunning = false;
        else
            nextSkip = now + 300;   // time (mS) between skips

    }

    pond.render(millis(), pixels);

    // A bubble field created in the direction of the swipe.  Bubbles are created and dissipate with 
    // different lifetimes for a nice random effect of the tail disappearing.
    if (bubbleRunning) {
        const int STEPS = 40;
        const uint32_t STEP_MS = 17;

        uint32_t elapsed = now - bubbleStartMs;

        while (nextBubbleStep < STEPS && elapsed >= uint32_t(nextBubbleStep) * STEP_MS) {

            // Always send the bubbles through the center of the screen and adjust the start/stop
            // so the line of bubbles travels at  the angle and direction the wand was swiped.
            float position = -20.0f + nextBubbleStep;
            float x = 14.5f + position * cosf(bubbleDirection);
            float y = 14.5f + position * sinf(bubbleDirection);

            addBubbleCluster(x, y, now);
            nextBubbleStep++;
        }

        if (nextBubbleStep >= STEPS)
            bubbleRunning = false;
    }
    
    renderBubbles(now);

    for (int y = 0; y < 30; ++y) {
        for (int x = 0; x < 30; ++x) {
            auto c = pixels[y][x];

            framebuffer.setPixel(x, y, c.r, c.g, c.b);
        }
    }
}

void ReflectingPondScene::addBubbleCluster(
    float x, float y, uint32_t now)
{
    const int CLUSTER_SIZE = 4;

    for (int i = 0; i < CLUSTER_SIZE; ++i) {
        Bubble &b = bubbles[nextBubble];
        nextBubble = (nextBubble + 1) % MAX_BUBBLES;

        b.x = x + random(-15, 16) * 0.1f;
        b.y = y + random(-15, 16) * 0.1f;

        b.birthMs = now;
        b.lifetimeMs = random(400, 1400);
        b.brightness = random(60, 220);
        b.active = true;
    }
}

void ReflectingPondScene::renderBubbles(uint32_t now)
{
    for (int i = 0; i < MAX_BUBBLES; ++i) {
        Bubble &b = bubbles[i];
        if (!b.active) continue;

        uint32_t age = now - b.birthMs;

        if (age >= b.lifetimeMs) {
            b.active = false;
            continue;
        }

        float life = float(age) / b.lifetimeMs;

        // Gradual fade with a little sparkle variation.
        float fade = 1.0f - life;
        float sparkle = random(50, 101) * 0.01f;

        float brightness = b.brightness * fade * sparkle;

        int x = int(b.x + 0.5f);
        int y = int(b.y + 0.5f);

        if (x < 0 || x >= 30 || y < 0 || y >= 30)
            continue;

        auto &p = pixels[y][x];

        // Additive, slightly cyan-white bubbles.
        p.r = uint8_t(fminf(255.0f, p.r + brightness * 0.55f));
        p.g = uint8_t(fminf(255.0f, p.g + brightness * 0.90f));
        p.b = uint8_t(fminf(255.0f, p.b + brightness));
    }
}
