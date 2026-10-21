#include "PanelController.h"

    void PanelController::processGesture(WandGesture gesture) {
        switch (gesture) {

            case WandGesture::ThrustIn:
                Serial.println("ThrustIn gesture detected");
                break;

            case WandGesture::ThrustOut:
                Serial.println("ThrustOut gesture detected");
                break;

            default:
                break;
        }
    }