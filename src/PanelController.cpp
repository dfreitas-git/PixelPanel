#include "PanelController.h"

//void PanelController::processGesture(WandGesture gesture) {
void PanelController::processGesture(const GestureEvent& event) {
    switch (event.gesture) {

        case WandGesture::ThrustIn:
            if(DEBUG_PRINT == 1) { 
                Serial.println(F("ThrustIn gesture detected"));
                superStarLaunchPending = true;
            }
            break;

        case WandGesture::ThrustOut:
            if(DEBUG_PRINT == 1) { 
                Serial.println(F("ThrustOut gesture detected"));
            }
            break;

        case WandGesture::Swipe:
            if(DEBUG_PRINT == 1) { 
                Serial.print(F("Swipe gesture detected.  Swipe Angle: "));
                Serial.println(event.swipeDirection * 180 / PI);
            }
            superStarLaunchPending = true;
            superStarDirection = event.swipeDirection;
            break;

        default:
            break;
    }
}

void PanelController::processPose(const WandPacket& packet) {

    // assign which mode, scene or panel, we are in
    currentMode = packet.mode;

    // For shootingStars anim, control the number of stars with the wand roll 
    int rollInDegrees = constrain(round(packet.roll * 180.0/PI), -45, 45);
    int pitchInDegrees = constrain(round(packet.pitch * 180.0/PI), -45, 45);
    shootingStarSettings.starCount =  map(rollInDegrees, -45, 45, 1, 5);
    shootingStarSettings.speedMultiplier = float(map(pitchInDegrees, -45, 45, 1, 100))/50.0;
      
    // Add specific pose related settings for other animations
    // as we add more
}

bool PanelController::consumeSuperStarLaunch(float& direction) {
    if (!superStarLaunchPending)
        return false;

    direction = superStarDirection;

    // if launch pending, reset the pending and acknowledge we saw it to the caller
    // This is how we only execute the launch once per pending request
    superStarLaunchPending = false;
    return true;
}