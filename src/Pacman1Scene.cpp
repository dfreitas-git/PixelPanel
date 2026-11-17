
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
    settings.speed = map(rollInDegrees, -45, 45, 25, 250);

    // Pitch controls blink
    int pitchInDegrees = constrain(round(packet.pitch * 180.0f / PI), -45, 45);
    settings.vertPosition= map(pitchInDegrees, -45, 45, 5, 25);
}

void Pacman1Scene::processGesture( const GestureEvent& event)
{
    switch (event.gesture) {

        case WandGesture::Swipe:
            break;

        case WandGesture::ThrustIn:
            break;

        case WandGesture::ThrustOut:
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