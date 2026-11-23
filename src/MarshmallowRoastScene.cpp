
#include "MarshmallowRoastScene.h"
#include "Globals.h"


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
    stick_.angle = PI + pitch ;  
}

void MarshmallowRoastScene::enter(State state) {
    state_ = state;
    stateStartMs_ = millis();
}

void MarshmallowRoastScene::startNewGame() {
    roundScore_ = 0;
    roundStartMs = millis();
    ignitionTime_ = 0.0f;
    mm_.doneness = 0.0f;
    mm_.temperature = AMBIENT_TEMP;
    mmColor_ = c_.white;
    initExtraBrowning();
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
    Serial.println("Marshmallow Burned!");
    enter(State::Burned);
}

void MarshmallowRoastScene::finishRound() {

    /*  Doneness scoring scale
    Doneness	Interpretation
    0	 Raw, white
    25	 Lightly toasted
    50	 Golden
    75	 Nicely browned
    100	 Perfectly roasted
    125	 Overdone
    150+ Charred
    */

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
    for(int i=0; i<NUMBER_OF_FLAMES; i++) {
        flame_[i].v1x = rXoff(i*3-1,2);
        flame_[i].v1y = 0;
        flame_[i].v2x = rXoff(i*3+1,2);
        if(i == 3 || i == 5 || i == 6 || i == 7) {
            flame_[i].v2y = rYoff(8,3);
        } else {
            flame_[i].v2y = rYoff(5,2);
        }
        flame_[i].v3x = rXoff(i*3+3,2);
        flame_[i].v3y = 0;
        flame_[i].color = flameColors_[random(2)];
    }
}

void MarshmallowRoastScene::renderFire(PixelFramebuffer& fb) {
    for(int i=0; i<NUMBER_OF_FLAMES; i++) {
        graphics_.drawTriangle(fb.data(),
        {flame_[i].v1x, flame_[i].v1y},
        {flame_[i].v2x, flame_[i].v2y},
        {flame_[i].v3x, flame_[i].v3y},
        flame_[i].color);

         // make the flames tips bright
        if(flame_[i].v2y >= 6) {
            graphics_.drawLine(fb.data(), flame_[i].v2x, flame_[i].v2y - 1, flame_[i].v2x, flame_[i].v2y, 1, c_.flameblue);
        }
    }
}

void MarshmallowRoastScene::updateMarshmallow(float dt) {

    if(state_ == State::Roasting) {
        mm_.x = stick_.endX;
        mm_.y = stick_.endY;
    }

    /*
    We defined:
    H(d)=1/(1+(d/k)^2)

    where:
    - d is the marshmallow's distance above the fire's coal bed, in pixels.
    - k controls how quickly heat decreases with distance.
    - H(d) is dimensionless, ranging from 0 to 1.

    A simple temperature with respect to time equation is:
    dT/dt = A * H(d)-B(T-T_ambient)

    where:
    - T is marshmallow temperature.
    - A is the heating rate at maximum fire intensity, in degrees per second.
    - B is the cooling coefficient, in inverse seconds.
    - T_ambient is ambient temperature.

     If marshmallow is cooler than ambient, then it heats
    */
    
    float d = max(0.0f, mm_.y - COAL_Y);
    float normalized = d / HEAT_SCALE;
    float heat = (1.0f / (1.0f + normalized * normalized)) * fireIntensity_;
    float heating = HEATING_RATE * heat;
    float cooling = COOLING_RATE * (mm_.temperature - AMBIENT_TEMP);

    mm_.temperature += (heating - cooling) * dt;

    float cookingTemp = max(0.0f, mm_.temperature - COOKING_THRESHOLD);
    mm_.doneness += cookingTemp * COOKING_RATE * dt;

    if (mm_.temperature > IGNITION_TEMP) {
        ignitionTime_ += dt;
    } else {
        ignitionTime_ = max(0.0f, ignitionTime_ - dt * 2.0f);
    }
    if (ignitionTime_ >= IGNITION_DELAY) {
        burnMarshmallow();
    }
    if(DEBUG_PRINT == 4) {
        Serial.print(F("dt: ")); Serial.print(dt); 
        Serial.print(F("  distance: ")); Serial.print(d); 
        Serial.print(F("  heat: ")); Serial.print(heat); 
        Serial.print(F("  heating: ")); Serial.print(heating); 
        Serial.print(F("  cooling: ")); Serial.print(cooling); 
        Serial.print(F("  mm_.temperature: ")); Serial.print(mm_.temperature); 
        Serial.print(F("  ignitionTime_: ")); Serial.print(ignitionTime_);
        Serial.print(F("  mm_.doneness: ")); Serial.println(mm_.doneness);
    }
}
 
RGB MarshmallowRoastScene::lerpColor(const RGB& a, const RGB& b, float t) {
    t = constrain(t, 0.0f, 1.0f);

    return {
        static_cast<uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<uint8_t>(a.b + (b.b - a.b) * t)
    };
}

RGB MarshmallowRoastScene::getDonenessColor(float doneness) {
    RGB lerpedColor;

    if (doneness < 50.0f) {
        lerpedColor = lerpColor(c_.white, c_.mmGolden, doneness / 50.0f);
    }
    else if (doneness < 100.0f) {
        lerpedColor = lerpColor(c_.mmGolden, c_.mmToasted, (doneness - 50.0f) / 50.0f);
    }
    else {
        lerpedColor = lerpColor(c_.mmToasted, c_.mmCharred, (doneness - 100.0f) / 50.0f);
    }
    return lerpedColor;
}

void MarshmallowRoastScene::initExtraBrowning() {
    for (int i = 0; i < EXTRA_BROWNING_PIXELS; i++) {
        int x, y;
        bool duplicate;

        do {
            // keep the extraBrowning on the interior of the marshmallow
            x = random(1, 5);
            y = random(1, 5);

            duplicate = false;

            for (int j = 0; j < i; j++) {
                if (browningFactor_[j].x == x &&
                    browningFactor_[j].y == y) {
                    duplicate = true;
                    break;
                }
            }
        } while (duplicate);

        browningFactor_[i].x = x;
        browningFactor_[i].y = y;
        browningFactor_[i].bf = random(150, 200) / 100.0f;
    }
}

void MarshmallowRoastScene::fillRectSprite(RGB color) {
    for (int y = 0; y < MM_SIZEY; y++) {
        for (int x = 0; x < MM_SIZEX; x++) {
            mmSpritePixels_[y][x] = color;
        }
    }
}

void MarshmallowRoastScene::renderMarshmallow(PixelFramebuffer& fb) {

    // if the marshmallow burns, it falls away from the stick, use marshmallow coords else we can follow the stick
    bool falling = (state_ == State::Burned);
    float x = falling ? mm_.x : stick_.endX;
    float y = falling ? mm_.y : stick_.endY;
    float angle = falling ? mm_.angle : stick_.angle;

    mmColor_ = getDonenessColor(mm_.doneness);

    fillRectSprite(mmColor_);

    // Now pixels from the extraBrowning set that get an accelerated doneness
    for(int i=0; i<EXTRA_BROWNING_PIXELS; i++) {
        mmSpritePixels_[browningFactor_[i].y] [browningFactor_[i].x] =
            getDonenessColor(mm_.doneness * browningFactor_[i].bf);
    }

    // need to send in degrees (not radians) as that's what the library expects
    graphics_.drawTexturedRect( fb.data(), x, y, MM_SIZEX, MM_SIZEY, 180.0f - angle * 180.0f / PI, &mmSpritePixels_[0][0]);
}

void MarshmallowRoastScene::updateStick() {
    stick_.endX = stick_.originX + stick_.length * cosf(stick_.angle);
    stick_.endY = stick_.originY - stick_.length * sinf(stick_.angle);
}

void MarshmallowRoastScene::renderStick(PixelFramebuffer& fb) {
    graphics_.drawLine(fb.data(),lroundf(stick_.originX),lroundf(stick_.originY),lroundf(stick_.endX),lroundf(stick_.endY),1,stickColor_);  
}

void MarshmallowRoastScene::drawCentered(PixelFramebuffer& fb, const char* text, int y, RGB color) {
    // Existing font occupies 6 columns per character.
    int len = 0;
    while (text[len] != '\0') ++len;
    graphics_.drawString(fb.data(), text, (PANEL_WIDTH - 6 * len) / 2, y, color, {0, 0, 0});
}