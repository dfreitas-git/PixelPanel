
#include "CheckerBoardScene.h"
#include "Globals.h"

CheckerBoardScene::CheckerBoardScene( GraphicsAnimations& animations) : gAnim(animations)
{
}

void CheckerBoardScene::begin() {
    // None for this scene
}

void CheckerBoardScene::processPose(const WandPacket& packet) {

    // Roll controls checkerboard size
    int rollInDegrees = constrain(round(packet.roll * 180.0f / PI), -45, 45);
    settings.squareScaler = map(rollInDegrees, -45, 45, 1, 4);

    // Pitch controls checkerboard color
    int pitchInDegrees = constrain(round(packet.pitch * 180.0f / PI), -45, 45);
    settings.colorIndex= map(pitchInDegrees, -45, 45, 0, 99);
}

void CheckerBoardScene::processGesture( const GestureEvent& event)
{
    switch (event.gesture) {

        case WandGesture::Swipe:
            if (DEBUG_PRINT == 1) {
                Serial.print(F("Swipe gesture detected. Angle: "));
                Serial.println(event.swipeDirection * 180.0f / PI);
            }
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

void CheckerBoardScene::render(
    PixelFramebuffer& framebuffer)
{
    gAnim.checker(framebuffer.data(), settings);
}