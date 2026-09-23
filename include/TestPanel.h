
#pragma once

#include <Arduino.h>
#include "PixelFramebuffer.h"

class TestPanel {
public:
    static void corners(PixelFramebuffer &fb);
    static void rows(PixelFramebuffer &fb, uint8_t row);
    static void columns(PixelFramebuffer &fb, uint8_t column);
    static void diagonals(PixelFramebuffer &fb);
    static void rmtBands(PixelFramebuffer &fb);
    static void colorTest(PixelFramebuffer &fb, uint8_t r, uint8_t g, uint8_t b);
    static void pixel(PixelFramebuffer &fb, uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
};