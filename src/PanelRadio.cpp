
#include "PanelRadio.h"

PanelRadio *PanelRadio::instance = nullptr;

bool PanelRadio::begin(const uint8_t panelMac[6])
{
    memcpy(peerMac, panelMac, 6);

    // ESP-NOW can operate through the station interface.
    WiFi.mode(WIFI_STA);

    //Serial.print("PixelPanel WiFi MAC: ");
    //Serial.println(WiFi.macAddress());

    // Initialize ESP-NOW.
    if (esp_now_init() != ESP_OK) {
        Serial.println(F("ESP-NOW initialization failed."));
        return false;
    }

    // Describe the PixelPanel as our peer.
    esp_now_peer_info_t peerInfo{};
    
    memcpy(peerInfo.peer_addr, peerMac, 6);

    // Channel 0 means use the current WiFi channel.
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println(F("Failed to add PixelPanel ESP-NOW peer."));
        return false;
    }

    // We use a RTOS queue to manage the packets being sent from the wand
    packetQueue = xQueueCreate( QUEUE_SIZE, sizeof(WandPacket));

    if (packetQueue == nullptr) {
        Serial.println(F("Failed to create packet queue"));
        return false;
    }

    instance = this;

    if (esp_now_register_recv_cb(OnDataRecv) != ESP_OK) {
        Serial.println(F("ESP-NOW receive callback registration failed."));
        return false;
    }

    initialized = true;
    return true;
}


// Callback when data is received
void PanelRadio::OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

    if (instance == nullptr ||
        instance->packetQueue == nullptr) {
        return;
    }

    if (len != sizeof(WandPacket)) {
        return;
    }

    WandPacket newPacket;

    memcpy( &newPacket, incomingData, sizeof(WandPacket));

    if (newPacket.magic != WAND_PROTOCOL_MAGIC ||
        newPacket.version != WAND_PROTOCOL_VERSION) {
        return;
    }

    // Packet has been validated.  Send to the non-blocking enqueue
    xQueueSend(instance->packetQueue, &newPacket, 0);

}


bool PanelRadio::processPacket( const WandPacket& packet, GestureEvent& event) {
    if (!sessionInitialized ||
        packet.sessionId != lastSessionId) {

        lastSessionId = packet.sessionId;
        lastEventSequence = 0;
        sessionInitialized = true;
    }

    if (packet.eventSequence == 0 ||
        packet.gesture == WandGesture::None) {
        return false;
    }

   // Serial.print("eS = "); Serial.print(packet.eventSequence); Serial.print("  lEs = "); Serial.println(lastEventSequence);
    if (packet.eventSequence == lastEventSequence) {
        return false;
    }

    lastEventSequence = packet.eventSequence;

    uint32_t eventAge = packet.packetTimestampMs - packet.eventTimestampMs;

    if (eventAge > MAX_EVENT_AGE_MS) {
        return false;
    }

    event.gesture = packet.gesture;
    event.eventSequence = packet.eventSequence;
    event.eventTimestampMs = packet.eventTimestampMs;

    return true;
}


bool PanelRadio::getNextPacket(WandPacket& packet)
{
    if (packetQueue == nullptr) {
        return false;
    }

    return xQueueReceive( packetQueue, &packet, 0) == pdTRUE;
}