
#include "MarshmallowRoastScene.h"

void MarshmallowRoastScene::begin() {
    startNewGame();

    uint32_t now = millis();
    lastRenderMs_ = now;
    lastPhysicsMs_ = now;
    lastFireMs_ = now;

    updateStick();
    updateFire();
}

void MarshmallowRoastScene::onSingleClick() { 
    switch (state_) {
        case State::Result :
        case State::Burned :
            startNewGame(); 
            break;

        case State::Roasting :
            finishRound();
            break;
    }
}
void MarshmallowRoastScene::processPose(const WandPacket& packet) {

    // Pitch controls the height of the marshmallow over the fire 
    const float pitch = constrain(packet.pitch, -PI / 6, PI / 6);
    stickAngle_ = PI + pitch ;  
}

void MarshmallowRoastScene::enter(State state) {
    state_ = state;
    stateStartMs_ = millis();
}

void MarshmallowRoastScene::startNewGame() {
    roundScore_ = 0;
    roundStartMs = millis();

    marshmallowColor_ = c_.white;

    temperature_ = 0.0f;
    doneness_ = 0.0f;

    enter(State::Roasting);
}

void MarshmallowRoastScene::render(PixelFramebuffer& fb) {

    const uint32_t now = millis();

    if (now - lastRenderMs_ < RENDER_PERIOD_MS)
        return;

    float dt = (now - lastPhysicsMs_) / 1000.0f;
    lastPhysicsMs_ = now;
    lastRenderMs_ = now;

    updateStick();

    if (state_ == State::Roasting) {
        updateMarshmallow(dt);
    }

    if (now - lastFireMs_ >= FIRE_PERIOD_MS) {
        lastFireMs_ = now;
        updateFire();
    }

    fb.clear();

    switch (state_) {

        case State::Roasting : 
            renderStick(fb);
            renderMarshmallow(fb);
            renderFire(fb);
            break;

        case State::Result :
            renderScore(fb);
            break;

        case State::Burned:
            renderStick(fb);
          // Implement this function once I figure out what I want the burning marshmallow to look like...
          //  renderBurned(fb);
            renderFire(fb);
            break;
    }
}

void MarshmallowRoastScene::burnMarshmallow() {
    roundScore_ = 0;
    enter(State::Burned);
}

void MarshmallowRoastScene::finishRound() {

    // Compute the score here
    // roundScore_ = computeRoundScore();
    // if(froundScore_ > bestScore_) {
    // bestScore_ = roundScore_;

    enter(State::Result);
}

void MarshmallowRoastScene::renderScore(PixelFramebuffer& fb) {
    drawCentered(fb, "SCORE", 22, c_.blue);
    char score[8];
    snprintf(score, sizeof(score), "%d", roundScore_);
    drawCentered(fb, score, 12, c_.yellow);
    snprintf(score, sizeof(score), "%d", bestScore_);
    drawCentered(fb, score, 3, c_.green);
}

int MarshmallowRoastScene::rXoff(int x, int range) {
    int newX = constrain(x + random(-range,range), 0, COLUMNS-1);
    return(newX);
}
int MarshmallowRoastScene:: rYoff(int y, int range) {
    int newY = constrain(y + random(-range,range), 0, ROWS-1);
    return(newY);
}

void MarshmallowRoastScene::updateFire() {
    for(int i=0; i<4; i++) {
        flame_[i].v1x = rXoff(i*6+3,3);
        flame_[i].v1y = 0;
        flame_[i].v2x = rXoff(i*6+6,2);
        if(i == 0 || i == 3) {
            flame_[i].v2y = rYoff(4,2);
        } else {
            flame_[i].v2y = rYoff(7,2);
        }
        flame_[i].v3x = rXoff(i*6+9,3);
        flame_[i].v3y = 0;
        flame_[i].color = flameColors_[random(2)];
    }
}

void MarshmallowRoastScene::renderFire(PixelFramebuffer& fb) {
    for(int i=0; i<4; i++) {
        graphics_.drawTriangle(fb.data(),
        {flame_[i].v1x, flame_[i].v1y},
        {flame_[i].v2x, flame_[i].v2y},
        {flame_[i].v3x, flame_[i].v3y},
        flame_[i].color);
    }
}
void MarshmallowRoastScene::updateMarshmallow(float dt) {
}
 
void MarshmallowRoastScene::renderMarshmallow(PixelFramebuffer& fb) {
    //drawRect expects rotate in degrees so need to convert from radians and the marshmallow is at 0-degrees when the stick is at 180 degrees so flip
    graphics_.drawRect(fb.data(),lroundf(stickEndX_-3), lroundf(stickEndY_-3),lroundf(stickEndX_+3),lroundf(stickEndY_+3), 180 - (stickAngle_ * 180.0 / PI),marshmallowColor_); 
}

void MarshmallowRoastScene::updateStick() {
    stickEndX_ = stickOriginX_ + stickLength_ * cosf(stickAngle_);
    stickEndY_ = stickOriginY_ - stickLength_ * sinf(stickAngle_);
}

void MarshmallowRoastScene::renderStick(PixelFramebuffer& fb) {
    graphics_.drawLine(fb.data(),lroundf(stickOriginX_),lroundf(stickOriginY_),lroundf(stickEndX_),lroundf(stickEndY_),1,c_.darkbrown);  
}

void MarshmallowRoastScene::drawCentered(PixelFramebuffer& fb, const char* text, int y, RGB color) {
    // Existing font occupies 6 columns per character.
    int len = 0;
    while (text[len] != '\0') ++len;
    graphics_.drawString(fb.data(), text, (PANEL_WIDTH - 6 * len) / 2, y, color, {0, 0, 0});
}