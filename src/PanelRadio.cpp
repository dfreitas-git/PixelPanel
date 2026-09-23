
#include "PanelRadio.h"

PanelRadio *PanelRadio::instance = nullptr;

bool PanelRadio::begin(const uint8_t panelMac[6])
{
    memcpy(peerMac, panelMac, 6);

    // ESP-NOW can operate through the station interface.
    WiFi.mode(WIFI_STA);

    Serial.print("PixelPanel WiFi MAC: ");
    Serial.println(WiFi.macAddress());

    // Initialize ESP-NOW.
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW initialization failed.");
        return false;
    }

    // Describe the PixelPanel as our peer.
    esp_now_peer_info_t peerInfo{};
    
    memcpy(peerInfo.peer_addr, peerMac, 6);

    // Channel 0 means use the current WiFi channel.
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add PixelPanel ESP-NOW peer.");
        return false;
    }

    instance = this;

    if (esp_now_register_recv_cb(OnDataRecv) != ESP_OK) {
        Serial.println("ESP-NOW receive callback registration failed.");
        return false;
    }

    initialized = true;
    Serial.println("ESP-NOW initialized.");

    return true;
}


// Callback when data is received
void PanelRadio::OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

    if (instance == nullptr) {
        return;
    }

    if (len != sizeof(WandPacket)) {
        return;
    }

    WandPacket newPacket;

    memcpy( &newPacket, incomingData, sizeof(WandPacket));

    if (newPacket.magic != WAND_PROTOCOL_MAGIC) {
        return;
    }

    if (newPacket.version != WAND_PROTOCOL_VERSION) {
        return;
    }

    // Packet has passed all validation.
    instance->panelPacket = newPacket;

}
