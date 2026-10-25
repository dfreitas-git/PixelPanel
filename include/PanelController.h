
#pragma once

#include <Arduino.h>
#include "Globals.h"
#include "WandProtocol.h"
#include "graphicsAnimations.h"

class PanelController {

public:
    //void processGesture(WandGesture gesture);
    void processGesture(const GestureEvent& event);
    void processPose(const WandPacket& packet);

    const ShootingStarSettings& getShootingStarSettings() const {
        return shootingStarSettings;
    }
    bool consumeSuperStarLaunch(float& direction);

private:
    ShootingStarSettings shootingStarSettings;
    bool superStarLaunchPending = false;
    float superStarDirection = 0.0f;
};
