#include "CollidingPixelsScene.h"
#include <math.h>
#include <stdio.h>

void CollidingPixelsScene::enter(State state, uint32_t now) {
    state_ = state;
    stateStartMs_ = now;
    thrustPending_ = false;
}

void CollidingPixelsScene::begin() { startNewGame(); }
void CollidingPixelsScene::onSingleClick() { startNewGame(); }

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
    uint32_t collisionTimeMs)
{
    float t = collisionTimeMs / 1000.0f;

    return {
        startX, startY,
        (targetX - startX) / t,
        (targetY - startY) / t
    };
}

void CollidingPixelsScene::setupRound() {
    // All three trajectories meet at (14.5, 14.5) at collisionTimeMs.
    // Velocities are in pixels per second.
    switch (roundNumber_) {
        case 1:
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
    }
    positionAt(0);
}

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
                if (elapsed > round_.collisionTimeMs + GRACE_MS)
                    finishRound(false);
            }
            break;
        }
        case State::Result:
            if (elapsed >= RESULT_MS) {
                if (!roundSucceeded_ || roundNumber_ >= FINAL_ROUND) {
                    if (totalScore_ > bestScore_) bestScore_ = totalScore_;
                    enter(State::GameOver, now);
                } else {
                    ++roundNumber_;
                    setupRound();
                    enter(State::Ready, now);
                }
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
        snprintf(score, sizeof(score), "%d", roundScore_);
        drawCentered(fb, score, 20, roundSucceeded_ ? RGB{40, 255, 60} : RGB{255, 40, 40});
    } else if (state_ == State::GameOver) {
        char score[8];
        drawCentered(fb, "END", 23, {255, 70, 50});
        snprintf(score, sizeof(score), "%d", totalScore_);
        drawCentered(fb, score, 13, {255, 255, 255});
        snprintf(score, sizeof(score), "%d", bestScore_);
        drawCentered(fb, score, 3, {60, 160, 255});
    }
}
