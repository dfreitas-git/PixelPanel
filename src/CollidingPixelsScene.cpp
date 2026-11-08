#include "CollidingPixelsScene.h"
#include <math.h>
#include <stdio.h>

void CollidingPixelsScene::enter(State state, uint32_t now) {
    state_ = state;
    stateStartMs_ = now;
    thrustPending_ = false;
}

void CollidingPixelsScene::begin() { startNewGame(); }

void CollidingPixelsScene::onSingleClick() { 
    // We allow the user to retry the level if they singleClick a miss within a few seconds
    if(state_ == State::Retry) {
        state_ = State::Ready;
        roundScore_ = 0;
        stateStartMs_ = millis();
    } else {
        startNewGame(); 
    }
}

void CollidingPixelsScene::startNewGame() {
    roundNumber_ = 1;
    totalScore_ = 0;
    roundScore_ = 0;
    roundSucceeded_ = false;
    setupRound();
    enter(State::Ready, millis());
}

// For computing the trajectory of a pixel with no bouncing off a wall
CollidingPixelsScene::Trajectory CollidingPixelsScene::makeTrajectory(
    float startX, float startY,
    float targetX, float targetY,
    uint32_t collisionTimeMs)
{
    float t = collisionTimeMs / 1000.0f;

    return {
        startX, startY,
        (targetX - startX) / t,
        (targetY - startY) / t
    };
}

/* For computing the trajectory of a pixel that bounces off the wall(s)
    We are using a technique known as the method-of-images or unfolding.
    The mirror numbering works like this:
    - 0: The real panel (no reflection).
    - 1: The mirrored panel immediately to the right or above.
    - -1: The mirrored panel immediately to the left or below.
    - 2, -2, etc.: Additional mirrored panels.
*/
CollidingPixelsScene::Trajectory CollidingPixelsScene::makeReflectedTrajectory(
    float startX, float startY,
    float collisionX, float collisionY,
    int mirrorX, int mirrorY,
    uint32_t collisionTimeMs)
{
    constexpr float MAX_X = PANEL_WIDTH - 1.0f;
    constexpr float MAX_Y = PANEL_HEIGHT - 1.0f;

    // Each virtual panel alternates between normal
    // and mirrored orientation.
    auto virtualTarget = [](float target,
                            float maxCoord,
                            int mirror) -> float
    {
        if (mirror % 2 == 0)
            return mirror * maxCoord + target;

        return (mirror + 1) * maxCoord - target;
    };

    float targetX = virtualTarget(
        collisionX, MAX_X, mirrorX);

    float targetY = virtualTarget(
        collisionY, MAX_Y, mirrorY);

    return makeTrajectory(
        startX, startY,
        targetX, targetY,
        collisionTimeMs);
}

void CollidingPixelsScene::setupRound() {
    // All three trajectories meet at (14.5, 14.5) at collisionTimeMs.
    // Velocities are in pixels per second.
    switch (roundNumber_) {

        // pixels not reflecting off any wall cases
        case 1:
            // arguments: starting x/y, collision x,y, collision-time mS
            round_.a = makeTrajectory( 0, 15, 14.5, 15, 3000);
            round_.b = makeTrajectory( 29, 15, 14.5, 15, 3000);
            round_.collisionTimeMs = 3000;
            break;

        case 2:  // Vertical
            round_.a = makeTrajectory( 15, 0, 15, 14.5, 2000);
            round_.b = makeTrajectory( 15, 29, 15, 14.5, 2000);
            round_.collisionTimeMs = 2000;
            break;

        case 3:  // Diagonal
            round_.a = makeTrajectory( 0, 0, 14.5, 14.5, 1150);
            round_.b = makeTrajectory( 29, 29, 14.5, 14.5, 1150);
            round_.collisionTimeMs = 1150;
            break;

        // pixels reflecting off walls cases
        case 4: {
            constexpr uint32_t T = 3000;  // time before collision (in mS)
            // arguments: starting x/y, collision x,y, mirror spec +/-X, +/-Y, collision-time mS
            round_.a = makeReflectedTrajectory( 7, 2, 15, 20, 1, 0, T);
            round_.b = makeReflectedTrajectory( 2, 10, 15, 20, 0, 0, T);
            round_.collisionTimeMs = T;
            break;
        }

        case 5: {
            constexpr uint32_t T = 3000;  // time before collision (in mS)
            round_.a = makeReflectedTrajectory(10, 10, 8, 5, -1, 0, 3000);
            round_.b = makeReflectedTrajectory(20, 20, 8, 5, 0, 1, 3000);
            round_.collisionTimeMs = T;
            break;
        }
        case 6: {
            constexpr uint32_t T = 3000;  // time before collision (in mS)
            round_.a = makeReflectedTrajectory(10, 10, 18, 0, -2, -2, 3000);
            round_.b = makeReflectedTrajectory(20, 20, 18, 0, 1, 1, 3000);
            round_.collisionTimeMs = 3000;
            break;
        }
    }
    positionAt(0);
}

// Used when folding virtual mirror-grids back to the physical grid
float CollidingPixelsScene::reflected(float position, float maximum) {
    const float period = 2.0f * maximum;
    float p = fmodf(position, period);
    if (p < 0) p += period;
    return p <= maximum ? p : period - p;
}

void CollidingPixelsScene::positionAt(uint32_t elapsedMs) {
    const float t = elapsedMs * 0.001f;
    a_.x = reflected(round_.a.x0 + round_.a.vx * t, PANEL_WIDTH - 1);
    a_.y = reflected(round_.a.y0 + round_.a.vy * t, PANEL_HEIGHT - 1);
    b_.x = reflected(round_.b.x0 + round_.b.vx * t, PANEL_WIDTH - 1);
    b_.y = reflected(round_.b.y0 + round_.b.vy * t, PANEL_HEIGHT - 1);
}

float CollidingPixelsScene::distance() const {
    return hypotf(a_.x - b_.x, a_.y - b_.y);
}

void CollidingPixelsScene::finishRound(bool attempted) {
    const float d = distance();
    roundScore_ = 0;
    if (attempted) {
        if (d <= 0.5f) roundScore_ = 100;
        else if (d <= 1.5f) roundScore_ = 75;
        else if (d <= 2.5f) roundScore_ = 50;
        else if (d <= 3.5f) roundScore_ = 25;
    }
    roundSucceeded_ = roundScore_ > 0;
    totalScore_ += roundScore_;
    enter(State::Result, millis());
}

void CollidingPixelsScene::processGesture(const GestureEvent& event) {
    if (event.gesture == WandGesture::ThrustIn && state_ == State::Playing && !thrustPending_) {
        thrustMs_ = millis();
        thrustPending_ = true;
    }
}

void CollidingPixelsScene::drawCentered(PixelFramebuffer& fb, const char* text, int y, RGB color) {
    // Existing font occupies 6 columns per character.
    int len = 0;
    while (text[len] != '\0') ++len;
    graphics_.drawString(fb.data(), text, (PANEL_WIDTH - 6 * len) / 2, y, color, {0, 0, 0});
}

void CollidingPixelsScene::render(PixelFramebuffer& fb) {
    const uint32_t now = millis();
    const uint32_t elapsed = now - stateStartMs_;
    fb.clear();

    switch (state_) {
        case State::Ready: {
            positionAt(0);
            if (elapsed >= COUNTDOWN_MS) {
                enter(State::Playing, now);
                break;
            }
            const char digit = char('3' - elapsed / 1000);
            char label[] = {digit, '\0'};
            drawCentered(fb, label, 12, {200, 200, 200});
            break;
        }
        case State::Playing: {
            if (thrustPending_) {
                const uint32_t shotElapsed = thrustMs_ - stateStartMs_;

                // Late packet processing must not extend the scoring window.
                const bool withinWindow = shotElapsed <= round_.collisionTimeMs + GRACE_MS;
                positionAt(shotElapsed);
                finishRound(withinWindow);
            } else {
                positionAt(elapsed);

                // if they wait too long (i.e. pixels cross), game over!
                if (elapsed > round_.collisionTimeMs + GRACE_MS)
                    finishRound(false);
            }
            break;
        }
        case State::Result:
            if (elapsed >= RESULT_MS) {
                if (!roundSucceeded_ || roundNumber_ >= FINAL_ROUND) {
                    if (totalScore_ > bestScore_) bestScore_ = totalScore_;
                    if(roundSucceeded_) {
                        enter(State::GameOver, now);
                    } else {
                        enter(State::Retry, now);
                    }
                } else {
                    ++roundNumber_;
                    setupRound();
                    enter(State::Ready, now);
                }
            }
            break;

        case State::Retry:
            // if they single-click before RETRY_MS, then we restart the level (see onSingleClick())
            if (elapsed >= RETRY_MS) {
                enter(State::GameOver, now);
            }
            break;

        case State::GameOver:
            break;
    }

    if (state_ == State::Playing || state_ == State::Result) {
        fb.setPixel(lroundf(a_.x), lroundf(a_.y), a_.color);
        fb.setPixel(lroundf(b_.x), lroundf(b_.y), b_.color);
    }
    if (state_ == State::Result) {
        char score[8];
        if(roundScore_ == 0) {
            snprintf(score, sizeof(score), "MISS!");
        } else {
            snprintf(score, sizeof(score), "%d", roundScore_);
        }
        drawCentered(fb, score, 20, roundSucceeded_ ? RGB{40, 255, 60} : RGB{255, 40, 40});

    } else if (state_ == State::Retry) {
        char level[5];
        snprintf(level, sizeof(level), "LEV%d", roundNumber_);
        drawCentered(fb, level, 22, {128, 0, 0});
        drawCentered(fb, "CLICK", 15, {60, 160, 255});
        drawCentered(fb, "TO", 8, {60, 160, 255});
        drawCentered(fb, "RETRY", 1, {60, 160, 255});

    } else if (state_ == State::GameOver) {
        char score[8];
        drawCentered(fb, "END", 23, {255, 70, 50});
        snprintf(score, sizeof(score), "%d", totalScore_);
        drawCentered(fb, score, 13, {255, 255, 255});
        snprintf(score, sizeof(score), "%d", bestScore_);
        drawCentered(fb, score, 3, {60, 160, 255});
    }
}
