
#pragma once

#include <Arduino.h>

constexpr int PANEL_WIDTH  = 30;
constexpr int PANEL_HEIGHT = 30;

struct RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class PixelFramebuffer {
public:
    void clear();
    void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
    void setPixel(int x, int y, const RGB &color);
    RGB getPixel(int x, int y) const;

    const RGB *data() const { 
        return &pixels[0][0];
    }

private:
    RGB pixels[PANEL_HEIGHT][PANEL_WIDTH]{};
};