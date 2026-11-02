
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

private:
    TestPanel& testPanel;

    TestPattern currentPattern = TestPattern::Corners;
};