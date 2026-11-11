#pragma once
#include <stdint.h>

// Hardware-independent 30x30 pond renderer. C++11 compatible.
// Coordinate origin and LED mapping are left to the calling Scene.
class ReflectingPondCore {
public:
    enum RenderMode { HEIGHT_COLOR = 0, SLOPE_LIGHT = 1, HYBRID = 2 };
    struct Color { uint8_t r, g, b; };
    static const int WIDTH = 30;
    static const int HEIGHT = 30;

    ReflectingPondCore();
    void setRenderMode(RenderMode mode);
    RenderMode getRenderMode() const { return mode_; }
    void setSpeed(float speed);       // 0.0 freezes the pond; default 1.0
    void setIntensity(float intensity); // 0.0 to 1.0; default 0.75
    void setShimmerStrength(float strength); // 0 disables; 0..1
    void setShimmerThreshold(float threshold); // 0..1 normalized crest height
    void setShimmerSpeed(float speed); // relative to pond animation
    void setShimmerCoupling(float coupling);
    void render(uint32_t nowMs, Color out[HEIGHT][WIDTH]);

private:
    RenderMode mode_;
    float speed_;
    float intensity_;
    float shimmerStrength_;
    float shimmerThreshold_;
    float shimmerSpeed_;
    uint32_t lastMs_;
    bool initialized_;
    float phaseTime_;
    float heights_[HEIGHT + 2][WIDTH + 2];

    static float clamp01(float x);
    static Color palette(float level);
    float heightAt(float x, float y, float t) const;
    float shimmerAt(float x, float y, float t) const;
    float shimmerCoupling_ = 1.0f;
};
