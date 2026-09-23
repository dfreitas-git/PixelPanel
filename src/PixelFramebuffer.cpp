
#include "PixelFramebuffer.h"

// Clear the entire framebuffer
void PixelFramebuffer::clear()
{
    memset(pixels, 0, sizeof(pixels));
}


// Set one pixel with a explicit RGB (255/255/255) value
void PixelFramebuffer::setPixel(
    int x,
    int y,
    uint8_t r,
    uint8_t g,
    uint8_t b)
{
    if (x < 0 || x >= PANEL_WIDTH ||
        y < 0 || y >= PANEL_HEIGHT) {
        return;
    }

    pixels[y][x].r = r;
    pixels[y][x].g = g;
    pixels[y][x].b = b;
}


// Set one pixel with a preset RGB value
void PixelFramebuffer::setPixel(
    int x,
    int y,
    const RGB &color)
{
    setPixel(x, y, color.r, color.g, color.b);
}


// Read what value is in framebuffer x/y location
RGB PixelFramebuffer::getPixel(int x, int y) const
{
    if (x < 0 || x >= PANEL_WIDTH ||
        y < 0 || y >= PANEL_HEIGHT) {

        return RGB{0, 0, 0};
    }

    return pixels[y][x];
}