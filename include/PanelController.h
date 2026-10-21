
#pragma once

#include <Arduino.h>
#include "WandProtocol.h"

class PanelController {

public:
    void processGesture(WandGesture gesture);
};