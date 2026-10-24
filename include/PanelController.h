
#pragma once

#include <Arduino.h>
#include "WandProtocol.h"
#include "graphicsAnimations.h"

class PanelController {

public:
    void processGesture(WandGesture gesture);
    void processPose(const WandPacket& packet);

    const ShootingStarSettings& getShootingStarSettings() const {
        return shootingStarSettings;
    }
    bool consumeSuperStarLaunch();

private:
    ShootingStarSettings shootingStarSettings;
    bool superStarLaunchPending = false;
};
