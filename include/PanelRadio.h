
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "WandProtocol.h"

class PanelRadio {
public:
    bool begin(const uint8_t panelMac[6]);

    const WandPacket &getPacket() const {
        return panelPacket;
    }

    bool getNextPacket(WandPacket& packet);

    // Returns true for a newly recognized, fresh gesture
    bool processPacket(const WandPacket& packet, GestureEvent& event);


private:
    // For the RTOS Queue
    static constexpr uint8_t QUEUE_SIZE = 8;
    QueueHandle_t packetQueue = nullptr;

    // For the ESP-NOW radio
    uint8_t peerMac[6];
    bool initialized = false;
    uint32_t lastSessionId = 0;
    uint32_t lastEventSequence = 0;
    bool sessionInitialized = false;
    const uint32_t MAX_EVENT_AGE_MS = 500;

    WandPacket panelPacket{};  // the {} says to initialize members to zero if not already explicitly initialized

    static PanelRadio *instance;

    static void OnDataRecv(
        const uint8_t *mac,
        const uint8_t *incomingData,
        int len
    );
};
