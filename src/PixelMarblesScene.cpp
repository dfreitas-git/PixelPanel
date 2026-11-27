
#include "PixelMarblesScene.h"
#include "Globals.h"

void PixelMarblesScene::begin() {
     uint32_t now = millis();
    lastRenderMs_ = now;
    lastPhysicsMs_ = now;
    marbleEngine_.reset();
    yawInitialized_ = false;
    initializeMarbles();
}

void PixelMarblesScene::onSingleClick() {
    yawInitialized_ = false;
}

void PixelMarblesScene::processPose(const WandPacket& packet) {

     if (!yawInitialized_) {
        initialYaw_ = packet.yaw;
        yawInitialized_ = true;
    }

    float relativeYaw = packet.yaw - initialYaw_;

    // Handle angular wraparound
    while (relativeYaw > PI)
        relativeYaw -= 2.0f * PI;

    while (relativeYaw < -PI)
        relativeYaw += 2.0f * PI;

    // So we don't apply board angle around neutral wand position
    float fbAngle = applyDeadband(packet.pitch);
    //float lrAngle = applyDeadband(packet.roll);
    float lrAngle = applyDeadband(relativeYaw);

    float gx = GRAVITY * sinf(lrAngle);
    float gy = GRAVITY * sinf(fbAngle);

    marbleEngine_.setGravity(gx, gy);
}

void PixelMarblesScene::initializeMarbles() {
    for (int y=0; y< ROWS; y+=2) {
        for (int x=0; x< COLUMNS; x+=2) {
            marbleEngine_.addMarble( x, y,
                marbleColors_[random( 0, sizeof(marbleColors_) / sizeof(marbleColors_[0]))]
            );

            if (marbleEngine_.getMarbleCount() >= NUM_MARBLES)
                return;
        }
    }
    Serial.print("Marble count: "); Serial.println(marbleEngine_.getMarbleCount());
}

float PixelMarblesScene::applyDeadband(float angle) const {

    angle = constrain(angle, -MAX_TILT, MAX_TILT);
    float magnitude = fabsf(angle);

    if (magnitude <= TILT_DEADBAND)
        return 0.0f;

    magnitude = (magnitude - TILT_DEADBAND) *
                MAX_TILT / (MAX_TILT - TILT_DEADBAND);

    return copysignf(magnitude, angle);
}

void PixelMarblesScene::initializeBoard() { }

void PixelMarblesScene::renderBoard(PixelFramebuffer& fb) { }

void PixelMarblesScene::renderMarbles(PixelFramebuffer& fb) { 

    for (int i = 0; i < marbleEngine_.getMarbleCount(); i++) {
        const auto& marble = marbleEngine_.getMarble(i);
        graphics_.writePixel( fb.data(), lroundf(marble.x), lroundf(marble.y), marble.color);
    }
}


void PixelMarblesScene::render(PixelFramebuffer& fb) {

    const uint32_t now = millis();
    float dt = (now - lastPhysicsMs_) / 1000.0f;

    // With occupancy-based collision detection, we need to ensure a marble can't 
    // jump over an occupied cell during updates.  Use fixed-duration physics steps.
    // At our maximum velocity of +/-25.0 pixels/sec:
    //  Physics timestep	Maximum movement per axis
    //  10 ms	0.25 pixels
    //  20 ms	0.50 pixels
    //  40 ms	1.00 pixel
    //  100 ms	2.50 pixels
    // If 25 ms have elapsed, we perform two 10 ms physics updates and retain the 
    // remaining 5 ms for the next invocation.  This guarantees a maximum movement 
    // of 0.25 pixels per axis per physics update, preventing marbles from skipping over occupied cells.

    while (now - lastPhysicsMs_ >= PHYSICS_PERIOD_MS) {
        lastPhysicsMs_ += PHYSICS_PERIOD_MS;
        marbleEngine_.update(PHYSICS_PERIOD_MS / 1000.0f);
    }

    if (now - lastRenderMs_ < RENDER_PERIOD_MS)
        return;

    lastRenderMs_ = now;
    fb.clear();
    renderMarbles(fb);
}


