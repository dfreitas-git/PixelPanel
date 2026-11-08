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


CollidingPixelsScene::Trajectory CollidingPixelsScene::makeTrajectory(
    float startX, float startY,
    float targetX, float targetY,
    uint32_t durationMs)
{
    float t = durationMs / 1000.0f;

    return {
        startX, startY,
        (targetX - startX) / t,
        (targetY - startY) / t,
        0.0f,       // amplitude: no sine modulation
        0.0f,       // cycles: no sine modulation
        durationMs  // trajectory duration
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

CollidingPixelsScene::Trajectory CollidingPixelsScene::makeSineTrajectory(
    float startX, float startY,
    float targetX, float targetY,
    uint32_t durationMs,
    float amplitude, float cycles)
{
    Trajectory tr = makeTrajectory(
        startX, startY,
        targetX, targetY,
        durationMs);

    tr.amplitude = amplitude;
    tr.cycles = cycles;

    return tr;
}

void CollidingPixelsScene::evaluateTrajectory(
    const Trajectory& trajectory,
    uint32_t elapsedMs,
    float& x,
    float& y)
{
    float t = elapsedMs / 1000.0f;

    // Original straight-line trajectory
    x = trajectory.x0 + trajectory.vx * t;
    y = trajectory.y0 + trajectory.vy * t;

    // Optional sinusoidal displacement
    if (trajectory.amplitude != 0.0f) {

        float length = sqrtf(
            trajectory.vx * trajectory.vx +
            trajectory.vy * trajectory.vy);

        if (length > 0.0001f) {
            float nx = -trajectory.vy / length;
            float ny =  trajectory.vx / length;
            float u = (float)elapsedMs / trajectory.durationMs;
            float offset = trajectory.amplitude * sinf(2.0f * PI * trajectory.cycles * u);

            x += nx * offset;
            y += ny * offset;
        }
    }

    // Fold virtual coordinates into physical panel
    x = reflected(x, PANEL_WIDTH - 1);
    y = reflected(y, PANEL_HEIGHT - 1);
}

void CollidingPixelsScene::setupRound() {
    // All three trajectories meet at (14.5, 14.5) at collisionTimeMs.
    // Velocities are in pixels per second.
    switch (roundNumber_) {

        // pixels not reflecting off any wall cases
        case 1:
            // arguments: starting x/y, collision x,y, durationMs
            round_.a = makeTrajectory( 0, 15, 14.5, 15, 3000);
            round_.b = makeTrajectory( 29, 15, 14.5, 15, 3000);
            round_.collisionTimeMs = 3000;
            break;
        
        case 2:  // Vertical
            // arguments: starting x/y, collision x,y, durationMs
            round_.a = makeTrajectory( 15, 0, 15, 14.5, 2000);
            round_.b = makeTrajectory( 15, 29, 15, 14.5, 2000);
            round_.collisionTimeMs = 2000;
            break;

        case 3:  // Diagonal
            // arguments: starting x/y, collision x,y, durationMs
            round_.a = makeTrajectory( 0, 0, 14.5, 14.5, 1150);
            round_.b = makeTrajectory( 29, 29, 14.5, 14.5, 1150);
            round_.collisionTimeMs = 1150;
            break;

        case 4:
            // arguments: starting x/y, collision x,y, durationMs
            round_.a = makeTrajectory(0, 15, 15, 15, 1000);
            round_.b = makeTrajectory(15, 29, 15, 15, 1000);
            round_.collisionTimeMs = 1000;
            break;
        
        // pixels reflecting off walls cases
        case 5: {
            constexpr uint32_t T = 3000;  // time before collision (in mS)
            // arguments: starting x/y, collision x,y, mirror spec +/-X, +/-Y, durationMs
            round_.a = makeReflectedTrajectory( 7, 2, 15, 20, 1, 0, T);
            round_.b = makeReflectedTrajectory( 2, 10, 15, 20, 0, 0, T);
            round_.collisionTimeMs = T;
            break;
        }

        case 6: {
            //python3 colliding_pixels_generator.py  --a 10 10 --b 20 20 --mirror-a -1 0 --mirror-b 0 1  --time 3000 --top 10
            constexpr uint32_t T = 3000;  // time before collision (in mS)
            round_.a = makeReflectedTrajectory(10, 10, 8, 5, -1, 0, 3000);
            round_.b = makeReflectedTrajectory(20, 20, 8, 5, 0, 1, 3000);
            round_.collisionTimeMs = T;
            break;
        }
        case 7:
            // python3 colliding_pixels_generator.py  --a 10 10 --b 20 20 --mirror-a -3 3 --mirror-b 2 -2  --time 4000 --top 1 
            round_.a = makeReflectedTrajectory(10, 10, 19, 0, -3, 3, 4000);
            round_.b = makeReflectedTrajectory(20, 20, 19, 0, 2, -2, 4000);
            round_.collisionTimeMs = 4000;
            break;
        
        case 8: {
            // python3 colliding_pixels_generator.py --a 10 10 --b 20 20 --mirror-a -2 -2 --mirror-b 1 1  --time 3000 --top 10
            constexpr uint32_t T = 3000;  // time before collision (in mS)
            round_.a = makeReflectedTrajectory(10, 10, 18, 0, -2, -2, 3000);
            round_.b = makeReflectedTrajectory(20, 20, 18, 0, 1, 1, 3000);
            round_.collisionTimeMs = 3000;
            break;
        }
        // sine wave paths
        case 9:
            // sine wave path
            round_.a = makeSineTrajectory( 2, 2, 29, 29, 3000,7,4);
            round_.b = makeTrajectory( 29, 2, 0, 15, 3000);
            // Need to specify the actual collision time since the endpoint 
            // of the sine path may not be where the two pixels actually cross
            round_.collisionTimeMs = 1446;  
            break;

        case 10:
            // sine wave path
            // python3 colliding_pixels_curves.py --a 0 15 --target-a 29 15 --duration-a 3000 --b 15 29 --target-b 15 0 --duration-b 4000 --sine-a 7 4
            round_.a = makeSineTrajectory(0, 15, 29, 15, 3000, 7, 4);
            round_.b = makeTrajectory(15, 29, 15, 0, 4000);
            round_.collisionTimeMs = 1549;
            break;

        case 11:
            // sine wave path
            //python3 colliding_pixels_curves.py --a 5 5 --target-a 25 25 --duration-a 3000 --sine-a 7 2 --b 5 25 --target-b 25 5  --duration-b 3000 --sine-b 7 2
            round_.a = makeSineTrajectory(5, 5, 25, 25, 3000, 7, 2);
            round_.b = makeSineTrajectory(5, 25, 25, 5, 3000, 7, 2);
            round_.collisionTimeMs = 1500;
            break;

        case 12:
            // sine wave path
            //  python3 colliding_pixels_curves.py --a 0 15 --target-a 29 15 --duration-a 3000 --sine-a 14 2  --b 15 0 --target-b 15 29 --duration-b 3000 --sine-b 10 3
            round_.a = makeSineTrajectory(0, 15, 29, 15, 3000, 14, 2);
            round_.b = makeSineTrajectory(15, 0, 15, 29, 3000, 10, 3);
            round_.collisionTimeMs = 1490;
            break;

        case 13:
            // sine wave path
            // python3 colliding_pixels_curves.py --a 0 15 --target-a 29 15 --duration-a 3000 --sine-a 7 4 --b 15 29 --target-b 15 0 --duration-b 3000 --sine-b 7 4
            round_.a = makeSineTrajectory(0, 15, 29, 15, 3000, 7, 4);
            round_.b = makeSineTrajectory(15, 29, 15, 0, 3000, 7, 4);
            round_.collisionTimeMs = 1492;
            break;

        case 14:
            // sine wave with reflections
            // python3 colliding_pixels_curves.py --a 0 0 --target-a 29 29 --duration-a 3000 --sine-a 14 2 --b 15 0 --target-b 15 29 --duration-b 3000 --sine-b 10 3  --tolerance .21
            round_.a = makeSineTrajectory(0, 0, 29, 29, 3000, 14, 2);
            round_.b = makeSineTrajectory(15, 0, 15, 29, 3000, 10, 3);
            round_.collisionTimeMs = 1496;
            break;

        case 15:
            // python3 colliding_pixels_curves.py --a 0 15 --target-a 29 15 --duration-a 3000 --sine-a 14 2 --b 15 0 --target-b 15 29 --duration-b 3000 --sine-b 10 3 
            round_.a = makeSineTrajectory(29, 0, 0, 29, 5000, 14, 2);
            round_.b = makeSineTrajectory(0, 29, 29, 0, 5000, 14, 3);
            round_.collisionTimeMs = 2500;
            break;

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

void CollidingPixelsScene::positionAt(uint32_t elapsedMs)
{
    evaluateTrajectory(
        round_.a, elapsedMs,
        a_.x, a_.y);

    evaluateTrajectory(
        round_.b, elapsedMs,
        b_.x, b_.y);
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
        char level[6];
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
