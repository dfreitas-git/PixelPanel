
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "WandProtocol.h"

class PanelRadio {
public:
    bool begin(const uint8_t panelMac[6]);

    const WandPacket &getPacket() const {
        return panelPacket;
    }

private:
    uint8_t peerMac[6];
    bool initialized = false;

    WandPacket panelPacket{};

    static PanelRadio *instance;

    static void OnDataRecv(
        const uint8_t *mac,
        const uint8_t *incomingData,
        int len
    );
};