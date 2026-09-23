
#pragma once

#include <Arduino.h>
#include "PixelFramebuffer.h"
#include <driver/rmt.h>

class PixelOutput {
public:
    static constexpr uint8_t NUM_CHANNELS = 5;
    static constexpr uint8_t ROWS_PER_CHANNEL = 6;
    static constexpr uint16_t LEDS_PER_CHANNEL = PANEL_WIDTH * ROWS_PER_CHANNEL;          // 180

    bool begin();
    void show(const PixelFramebuffer &framebuffer);

private:
    static constexpr uint8_t OUTPUT_PINS[NUM_CHANNELS] = { 18, 19, 25, 26, 27 };
    static constexpr uint16_t BITS_PER_LED = 24;
    static constexpr uint16_t RMT_ITEMS_PER_CHANNEL = LEDS_PER_CHANNEL * BITS_PER_LED;
    static constexpr rmt_channel_t RMT_CHANNELS[NUM_CHANNELS] = {
        RMT_CHANNEL_0,
        RMT_CHANNEL_1,
        RMT_CHANNEL_2,
        RMT_CHANNEL_3,
        RMT_CHANNEL_4
    };

    static constexpr uint16_t BYTES_PER_CHANNEL = LEDS_PER_CHANNEL * 3;
    uint8_t outputBuffer[NUM_CHANNELS][BYTES_PER_CHANNEL];

    static void IRAM_ATTR ws2812Translator(
        const void *src,
        rmt_item32_t *dest,
        size_t srcSize,
        size_t wantedNum,
        size_t *translatedSize,
        size_t *itemNum
    );

    // Convert framebuffer x/y to channel and LED position within
    // that channel's 180-LED physical stream.
    static void mapPixel(
        int x,
        int y,
        uint8_t &channel,
        uint16_t &ledIndex
    );

};