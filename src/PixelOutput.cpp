
#include "PixelOutput.h"

constexpr rmt_channel_t
    PixelOutput::RMT_CHANNELS[PixelOutput::NUM_CHANNELS];
constexpr uint8_t
    PixelOutput::OUTPUT_PINS[PixelOutput::NUM_CHANNELS];

// Initialize the RMT channels for outputting WS2812 data streams
bool PixelOutput::begin()
{
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {

        rmt_config_t config{};

        config.rmt_mode = RMT_MODE_TX;
        config.channel = RMT_CHANNELS[i];
        config.gpio_num = static_cast<gpio_num_t>(OUTPUT_PINS[i]);

        config.clk_div = 4;
        config.mem_block_num = 1;

        config.tx_config.loop_en = false;
        config.tx_config.carrier_en = false;
        config.tx_config.idle_output_en = true;
        config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;

        esp_err_t result = rmt_config(&config);

        if (result != ESP_OK) {
            Serial.print("RMT config failed for channel ");
            Serial.println(i);
            return false;
        }

        result = rmt_driver_install(
            RMT_CHANNELS[i],
            0,
            0
        );

        if (result != ESP_OK) {
            Serial.print("RMT driver install failed for channel ");
            Serial.println(i);
            return false;
        }

        result = rmt_translator_init( RMT_CHANNELS[i], ws2812Translator);

        if (result != ESP_OK) {
            Serial.print("RMT translator init failed for channel ");
            Serial.println(i);
            return false;
        }
    }

    Serial.println("PixelOutput initialized.");
    return true;
}

// Maps the framebuffer pixel address to the proper RMT outputBuffer channel/index
void PixelOutput::mapPixel(
    int x,
    int y,
    uint8_t &channel,
    uint16_t &ledIndex)
{
    channel = y / ROWS_PER_CHANNEL;

    uint8_t localRow = y % ROWS_PER_CHANNEL;

    if ((localRow & 1) == 0) {
        // Even rows run left -> right.
        ledIndex = localRow * PANEL_WIDTH + x;
    }
    else {
        // Odd rows run right -> left.
        ledIndex = localRow * PANEL_WIDTH + (PANEL_WIDTH - 1 - x);
    }
}

// Send the framebuffer to the display via 5 RMT channels
void PixelOutput::show(const PixelFramebuffer &framebuffer)
{
    // Convert the Cartesian framebuffer into five physical
    // GRB byte streams, one for each RMT channel.
    for (int y = 0; y < PANEL_HEIGHT; y++) {
        for (int x = 0; x < PANEL_WIDTH; x++) {

            uint8_t channel;
            uint16_t ledIndex;

            mapPixel(x, y, channel, ledIndex);

            RGB pixel = framebuffer.getPixel(x, y);

            uint16_t byteIndex = ledIndex * 3;

            outputBuffer[channel][byteIndex + 0] = pixel.g;
            outputBuffer[channel][byteIndex + 1] = pixel.r;
            outputBuffer[channel][byteIndex + 2] = pixel.b;
        }
    }
    // Start all five channels without waiting for each
    // preceding channel to finish.
    for (uint8_t channel = 0;
         channel < NUM_CHANNELS;
         channel++) {

        esp_err_t result = rmt_write_sample(
            RMT_CHANNELS[channel],
            outputBuffer[channel],
            BYTES_PER_CHANNEL,
            false
        );

        if (result != ESP_OK) {
            Serial.print("RMT transmit failed on channel ");
            Serial.println(channel);
        }
    }

    // The output buffers must remain unchanged until their
    // asynchronous transmissions have completed.
    for (uint8_t channel = 0;
         channel < NUM_CHANNELS;
         channel++) {

        esp_err_t result = rmt_wait_tx_done(
            RMT_CHANNELS[channel],
            portMAX_DELAY
        );

        if (result != ESP_OK) {
            Serial.print("RMT wait failed on channel ");
            Serial.println(channel);
        }
    }
}

// This is code that RMT will call to translate bits in the outputBuffer into 
// RMT data (essentially timing for a "1" and "0" for our WS2812 data stream).
// This saves us a ton of memory over storing the RMT format data.  Here we can
// expand each pixel's data into RMT data on the fly trading a little CPU
// for a big saving in memory.
void IRAM_ATTR PixelOutput::ws2812Translator(
    const void *src,
    rmt_item32_t *dest,
    size_t srcSize,
    size_t wantedNum,
    size_t *translatedSize,
    size_t *itemNum)
{
    if (src == nullptr || dest == nullptr) {
        *translatedSize = 0;
        *itemNum = 0;
        return;
    }

    /*
     For RMT Timing
     80 MHz / 4 = 20 MHz  so 1 tick = 50 ns
     Reasonable WS2812 timings are:
    
        0 bit:  0.40 us high + 0.85 us low
             8 ticks       17 ticks
    
        1 bit:  0.80 us high + 0.45 us low
            16 ticks        9 ticks
    */
    constexpr uint16_t T0H = 8;   // 0.40 us
    constexpr uint16_t T0L = 17;  // 0.85 us
    constexpr uint16_t T1H = 16;  // 0.80 us
    constexpr uint16_t T1L = 9;   // 0.45 us

    const uint8_t *source =
        static_cast<const uint8_t *>(src);

    size_t sourceIndex = 0;
    size_t destinationIndex = 0;

    // Each source byte requires 8 RMT items.
    while (sourceIndex < srcSize &&
           (destinationIndex + 8) <= wantedNum) {

        uint8_t value = source[sourceIndex];

        for (uint8_t mask = 0x80;
             mask != 0;
             mask >>= 1) {

            rmt_item32_t &item =
                dest[destinationIndex++];

            if (value & mask) {
                item.level0 = 1;
                item.duration0 = T1H;
                item.level1 = 0;
                item.duration1 = T1L;
            }
            else {
                item.level0 = 1;
                item.duration0 = T0H;
                item.level1 = 0;
                item.duration1 = T0L;
            }
        }

        sourceIndex++;
    }

    *translatedSize = sourceIndex;
    *itemNum = destinationIndex;
}