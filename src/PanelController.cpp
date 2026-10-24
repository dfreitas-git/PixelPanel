#include "PanelController.h"

void PanelController::processGesture(WandGesture gesture) {
    switch (gesture) {

        case WandGesture::ThrustIn:
            Serial.println("ThrustIn gesture detected");
            superStarLaunchPending = true;
            break;

        case WandGesture::ThrustOut:
            Serial.println("ThrustOut gesture detected");
            break;

        default:
            break;
    }
}

void PanelController::processPose(const WandPacket& packet) {

    // For shootingStars anim, control the number of stars with the wand roll 
    int rollInDegrees = constrain(round(packet.roll * 180.0/PI), -45, 45);
    int pitchInDegrees = constrain(round(packet.pitch * 180.0/PI), -45, 45);
    shootingStarSettings.starCount =  map(rollInDegrees, -45, 45, 1, 5);
    shootingStarSettings.speedMultiplier = float(map(pitchInDegrees, -45, 45, 1, 100))/50.0;
      
    // Add specific pose related settings for other animations
    // as we add more
}

bool PanelController::consumeSuperStarLaunch() {
    // if nothing pending just return
    if (!superStarLaunchPending)
        return false;

    // if launch pending, reset the pending and acknowledge we saw it to the caller
    // This is how we only execute the launch once per pending request
    superStarLaunchPending = false;
    return true;
}
