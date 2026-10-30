
#include "EyeballScene.h"
#include "Globals.h"

EyeballScene::EyeballScene( GraphicsAnimations& animations) : gAnim(animations) { }

void EyeballScene::begin() {
    // None for EyeballScene scene
}

void EyeballScene::processPose(const WandPacket& packet) {

    // Roll controls eyeball L/R position
    int rollInDegrees = constrain(round(packet.roll * 180.0f / PI), -45, 45);
    if(rollInDegrees < -20) {
        settings.eyePosition_LR = -1;
    } else if(rollInDegrees > 20) {
        settings.eyePosition_LR = 1;
    } else {
        settings.eyePosition_LR = 0;
    }

    // Pitch controls eyeball U/D position
    int pitchInDegrees = constrain(round(packet.pitch * 180.0f / PI), -45, 45);
    if(pitchInDegrees < -20) {
        settings.eyePosition_UD = -1;
    } else if(pitchInDegrees > 20) {
        settings.eyePosition_UD = 1;
    } else {
        settings.eyePosition_UD = 0;
    }
}

void EyeballScene::processGesture( const GestureEvent& event)
{
    switch (event.gesture) {

        case WandGesture::Swipe:
            if (DEBUG_PRINT == 1) {
                Serial.print(F("Swipe gesture detected. Angle: "));
                Serial.println(event.swipeDirection * 180.0f / PI);
            }
            break;

        case WandGesture::ThrustIn:
            blinkPending = true;
            break;

        case WandGesture::ThrustOut:
            // nothing
            break;

        default:
            break;
    }
}

void EyeballScene::render(
    PixelFramebuffer& framebuffer)
{
    gAnim.eyeball( framebuffer.data(), settings, blinkPending);
    blinkPending = false;
}