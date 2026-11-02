
#include "TestScene.h"

void TestScene::render(PixelFramebuffer& framebuffer)
{
    switch (currentPattern) {

        case TestPattern::Corners:
            TestPanel::corners(framebuffer);
            break;

        case TestPattern::Rows:
            TestPanel::rows(framebuffer, 15);
            break;

        case TestPattern::Columns:
            TestPanel::columns(framebuffer, 15);
            break;

        case TestPattern::Bands:
            TestPanel::rmtBands(framebuffer);
            break;

        case TestPattern::Color:
            TestPanel::colorTest(framebuffer, 32, 0, 0);
            break;

        case TestPattern::Pixel:
            TestPanel::pixel(framebuffer, 0, 0, 32, 0, 0);
            break;

        case TestPattern::Diagonals:
            TestPanel::diagonals(framebuffer);
            break;

    }
}