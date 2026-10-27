
#include "ShootingStarsScene.h"
#include "Globals.h"

ShootingStarsScene::ShootingStarsScene(
    GraphicsAnimations& animations)
    : gAnim(animations)
{
}

void ShootingStarsScene::begin() {
    superStarLaunchPending = false;
    gAnim.initShootingStars();
}

void ShootingStarsScene::processPose(const WandPacket& packet) {

    // Roll controls star count
    int rollInDegrees =
        constrain(round(packet.roll * 180.0f / PI), -45, 45);

    settings.starCount =
        map(rollInDegrees, -45, 45, 1, 5);

    // Pitch controls star speed
    int pitchInDegrees =
        constrain(round(packet.pitch * 180.0f / PI), -45, 45);

    settings.speedMultiplier =
        float(map(pitchInDegrees, -45, 45, 1, 100)) / 50.0f;
}

void ShootingStarsScene::processGesture(
    const GestureEvent& event)
{
    switch (event.gesture) {

        case WandGesture::Swipe:
            if (DEBUG_PRINT == 1) {
                Serial.print(F("Swipe gesture detected. Angle: "));
                Serial.println(event.swipeDirection * 180.0f / PI);
            }

            superStarDirection = event.swipeDirection;
            superStarLaunchPending = true;
            break;

        case WandGesture::ThrustIn:
            if (DEBUG_PRINT == 1)
                Serial.println(F("ThrustIn gesture detected"));

            break;

        case WandGesture::ThrustOut:
            if (DEBUG_PRINT == 1)
                Serial.println(F("ThrustOut gesture detected"));
            break;

        default:
            break;
    }
}

void ShootingStarsScene::render(
    PixelFramebuffer& framebuffer)
{
    if (superStarLaunchPending) {
        gAnim.initSuperStar(superStarDirection);
        superStarLaunchPending = false;
    }

    gAnim.shootingStar(framebuffer.data(), settings);
}