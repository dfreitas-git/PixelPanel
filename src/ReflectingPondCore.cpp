#include "ReflectingPondCore.h"
#include <math.h>
#include <Arduino.h>

namespace {
    //const float TWO_PI = 6.28318530718f;
    struct Wave { float wavelength, angle, amplitude, speed, phase; };
    // Speeds are in cycles per second; wavelengths are in LED pixels.
    const Wave waves[] = {
        //{23.0f, 0.28f, 0.48f,  0.065f, 0.20f},
        //{17.0f, 1.83f, 0.28f, -0.047f, 1.70f},
        //{12.5f, 2.62f, 0.16f,  0.032f, 3.10f},
        //{28.0f, 1.05f, 0.08f, -0.019f, 4.20f}
    {16.0f, 0.35f, 0.20f,  0.045f, 0.20f},
    {13.0f, 1.45f, 0.22f, -0.053f, 1.70f},
    {11.0f, 2.35f, 0.19f,  0.038f, 3.10f},
    {19.0f, 0.95f, 0.16f, -0.031f, 4.20f},
    { 8.5f, 2.85f, 0.12f,  0.062f, 2.30f}
    };
    const int WAVE_COUNT = sizeof(waves) / sizeof(waves[0]);
    // Short, weak waves whose interference creates transient crests.
    // Amplitudes sum to 1, so the resulting height is in [-1, 1].
    const Wave shimmerWaves[] = {
        {6.3f, 0.43f, 0.31f,  0.083f, 0.6f},
        {4.7f, 1.96f, 0.27f, -0.109f, 2.1f},
        {8.1f, 2.74f, 0.23f,  0.067f, 4.0f},
        {5.4f, 1.12f, 0.19f, -0.091f, 5.2f}
    };
    const int SHIMMER_WAVE_COUNT = sizeof(shimmerWaves) / sizeof(shimmerWaves[0]);
    float lerp(float a, float b, float t) { return a + (b - a) * t; }
    uint8_t toByte(float v) {
        if (v < 0.0f) v = 0.0f;
        if (v > 255.0f) v = 255.0f;
        return static_cast<uint8_t>(v + 0.5f);
    }
}

ReflectingPondCore::ReflectingPondCore()
    : mode_(HEIGHT_COLOR), speed_(1.0f), intensity_(0.75f),
      shimmerStrength_(0.65f), shimmerThreshold_(0.76f), shimmerSpeed_(1.0f),
      lastMs_(0), initialized_(false), phaseTime_(0.0f) {}

void ReflectingPondCore::setRenderMode(RenderMode mode) { mode_ = mode; }
void ReflectingPondCore::setSpeed(float speed) { speed_ = speed < 0.0f ? 0.0f : speed; }
void ReflectingPondCore::setIntensity(float intensity) { intensity_ = clamp01(intensity); }
void ReflectingPondCore::setShimmerStrength(float strength) { shimmerStrength_ = clamp01(strength); }
void ReflectingPondCore::setShimmerThreshold(float threshold) { shimmerThreshold_ = clamp01(threshold); }
void ReflectingPondCore::setShimmerSpeed(float speed) { shimmerSpeed_ = speed < 0.0f ? 0.0f : speed; }
float ReflectingPondCore::clamp01(float x) { return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x); }
void ReflectingPondCore::setShimmerCoupling(float coupling) { shimmerCoupling_ = clamp01(coupling); }

float ReflectingPondCore::heightAt(float x, float y, float t) const {
    float h = 0.0f;
    for (int i = 0; i < WAVE_COUNT; ++i) {
        const Wave &w = waves[i];
        // Precomputed direction components could replace these trig calls later.
        float distance = x * cosf(w.angle) + y * sinf(w.angle);
        float phase = TWO_PI * (distance / w.wavelength + w.speed * t) + w.phase;
        h += w.amplitude * sinf(phase);
    }
    return h;
}

float ReflectingPondCore::shimmerAt(float x, float y, float t) const {
    float h = 0.0f;
    for (int i = 0; i < SHIMMER_WAVE_COUNT; ++i) {
        const Wave &w = shimmerWaves[i];
        float distance = x * cosf(w.angle) + y * sinf(w.angle);
        float phase = TWO_PI * (distance / w.wavelength + w.speed * t) + w.phase;
        h += w.amplitude * sinf(phase);
    }
    return 0.5f + 0.5f * h; // normalized to [0, 1]
}

ReflectingPondCore::Color ReflectingPondCore::palette(float level) {
    // Deliberately subdued; adjust after looking at the real diffuser.
    const Color stops[] = { {2, 9, 22}, {3, 23, 37}, {4, 40, 49}, {10, 63, 68} };
    float s = clamp01(level) * 3.0f;
    int i = static_cast<int>(s);
    if (i > 2) i = 2;
    float f = s - i;
    Color c = {
        toByte(lerp(stops[i].r, stops[i + 1].r, f)),
        toByte(lerp(stops[i].g, stops[i + 1].g, f)),
        toByte(lerp(stops[i].b, stops[i + 1].b, f))
    };
    return c;
}

void ReflectingPondCore::render(uint32_t nowMs, Color out[HEIGHT][WIDTH]) {
    if (!initialized_) {
        lastMs_ = nowMs;
        initialized_ = true;
    }
    // Unsigned subtraction handles millis() wraparound. Clamp long pauses.
    uint32_t elapsedMs = nowMs - lastMs_;
    lastMs_ = nowMs;
    if (elapsedMs > 100) elapsedMs = 100;

    phaseTime_ += elapsedMs * 0.001f * speed_;

    // Keep phase bounded during very long runtimes.
    if (phaseTime_ > 10000.0f) phaseTime_ -= 10000.0f;

    // One-pixel halo allows centered differences on every display pixel.
    for (int y = 0; y < HEIGHT + 2; ++y)
        for (int x = 0; x < WIDTH + 2; ++x)
            heights_[y][x] = heightAt(float(x - 1), float(y - 1), phaseTime_);

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            float h = heights_[y + 1][x + 1];
            float dx = 0.5f * (heights_[y + 1][x + 2] - heights_[y + 1][x]);
            float dy = 0.5f * (heights_[y + 2][x + 1] - heights_[y][x + 1]);
            float heightLevel = clamp01(0.5f + 0.46f * h);
            // Directional light: slopes facing the light are brighter.
            float lightLevel = clamp01(0.5f + 1.4f * (0.75f * dx + 0.65f * dy));
            float level;
            switch (mode_) {
                case HEIGHT_COLOR: level = heightLevel; break;
                case SLOPE_LIGHT: level = lightLevel; break;
                default: level = clamp01(0.5f * heightLevel + 0.5f * lightLevel); break;
            }
            Color c = palette(level);
            float r = c.r * intensity_;
            float g = c.g * intensity_;
            float b = c.b * intensity_;
            if (shimmerStrength_ > 0.0f && shimmerThreshold_ < 1.0f) {
                float crest = shimmerAt(float(x), float(y), phaseTime_ * shimmerSpeed_);
                float excess = clamp01((crest - shimmerThreshold_) /
                                       (1.0f - shimmerThreshold_));
                // Cubic response: weak crests barely glow, rare high crests shine.
                float highlight = excess * excess * excess * shimmerStrength_;
             //   float couplingFactor = (1.0f - shimmerCoupling_) + shimmerCoupling_ * heightLevel;
                float couplingFactor = 1.0f + shimmerCoupling_ * (2.0f * heightLevel - 1.0f);

                highlight *= couplingFactor;

                r += 150.0f * highlight;
                g += 225.0f * highlight;
                b += 235.0f * highlight;
            }
            out[y][x] = { toByte(r), toByte(g), toByte(b) };
        }
    }
}
