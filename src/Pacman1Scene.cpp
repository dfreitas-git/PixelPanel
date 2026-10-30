
#include "Pacman1Scene.h"
#include "Globals.h"

Pacman1Scene::Pacman1Scene( GraphicsAnimations& animations) : gAnim(animations)
{
}

void Pacman1Scene::begin() {
    // None for Pacman1Scene scene
}

void Pacman1Scene::processPose(const WandPacket& packet) {

    // Roll controls eyeball position
    int rollInDegrees = constrain(round(packet.roll * 180.0f / PI), -45, 45);
    settings.speed = map(rollInDegrees, -45, 45, 0, 99);

    // Pitch controls blink
    int pitchInDegrees = constrain(round(packet.pitch * 180.0f / PI), -45, 45);
    settings.vertPosition= map(pitchInDegrees, -45, 45, 0, 29);
}

void Pacman1Scene::processGesture( const GestureEvent& event)
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

void Pacman1Scene::render(
    PixelFramebuffer& framebuffer)
{
    gAnim.pacman1(framebuffer.data(), settings);
}