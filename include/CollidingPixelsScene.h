#pragma once
#include <Arduino.h>
#include "Scene.h"
#include "graphicsPrimitives.h"

/* class that implements the CollitingPixel game on the PixelPanel
   The game is simple, watch two pixels travel around the PixelPanel.  When
   the pixel paths intersect, thrust the wand forward.  The closer they are 
   together at the thrust moment, the higher the score.  If they are too far
   apart, you score zero and the game is over.  You get only one chance at 
   the first intersection point.  If you succeed, you move on to the next (harder)
   level.  Final total score is displayed at the end.
   
   You do have a chance to replay a level if you miss.  Within five seconds, pressing
   the wand buton will restart that level.   Single press on the button any other time 
   will restart the game from the beginning 
 */

class CollidingPixelsScene : public Scene {
public:
    explicit CollidingPixelsScene(GraphicsPrimitives& graphics) : graphics_(graphics) {}
    const char* name() const override { return "COLID"; }
    void begin() override;
    void onSingleClick() override;
    void processGesture(const GestureEvent& event) override;
    void render(PixelFramebuffer& framebuffer) override;
    int preferredDiffuserPosition() const override { return 0; }

private:
    enum class State { Ready, Playing, Result, GameOver };

    struct Trajectory { 
        float x0, y0, vx, vy; 
    };

    struct Pixel { 
        float x, y; 
        RGB color; 
    };

    struct Round { 
        Trajectory a, b; 
        uint32_t collisionTimeMs; 
    };

    Trajectory makeTrajectory(
        float startX, float startY,
        float targetX, float targetY,
        uint32_t collisionTimeMs);

    Trajectory makeReflectedTrajectory(
        float startX, float startY,
        float collisionX, float collisionY,
        int mirrorX, int mirrorY,
        uint32_t collisionTimeMs
    );

    static constexpr uint32_t COUNTDOWN_MS = 3000;
    static constexpr uint32_t RESULT_MS = 1700;
    static constexpr uint32_t GRACE_MS = 180;
    static constexpr int FINAL_ROUND = 6;

    GraphicsPrimitives& graphics_;
    State state_ = State::Ready;
    uint32_t stateStartMs_ = 0;
    uint32_t thrustMs_ = 0;
    bool thrustPending_ = false;
    bool roundSucceeded_ = false;
    int roundNumber_ = 1;
    int roundScore_ = 0;
    int totalScore_ = 0;
    int bestScore_ = 0; // RAM only; retained while scene object exists
    Round round_ {};
    Pixel a_ {0, 0, {255, 45, 30}};
    Pixel b_ {0, 0, {30, 110, 255}};

    void enter(State state, uint32_t now);
    void startNewGame();
    void setupRound();
    void positionAt(uint32_t elapsedMs);
    static float reflected(float position, float maximum);
    float distance() const;
    void finishRound(bool attempted);
    void drawCentered(PixelFramebuffer& fb, const char* text, int y, RGB color);
};
