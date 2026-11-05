
#pragma once

#include "Scene.h"
#include "TestPanel.h"

enum class TestPattern : uint8_t {
    Corners,
    Rows,
    Columns,
    Bands,
    Color,
    Pixel,
    Diagonals
};

class TestScene : public Scene {
public:
    explicit TestScene(TestPanel& tests) : testPanel(tests)
    {
    }

    const char* name() const override {
        return "TEST";
    }

    void setPattern(TestPattern pattern) {
        currentPattern = pattern;
    }

    void render(PixelFramebuffer& framebuffer) override;

    // Set the value from 0-100 (0 being closest to LEDs, 100 being most diffused - farthest from LEDs)
    int preferredDiffuserPosition() const override { return 0; }  

private:
    TestPanel& testPanel;

    TestPattern currentPattern = TestPattern::Corners;
};