
#include "PixelMarblesScene.h"
#include "Globals.h"

void PixelMarblesScene::begin() {
     uint32_t now = millis();
    lastRenderMs_ = now;
    lastPhysicsMs_ = now;
    marbleEngine_.reset();
    //yawInitialized_ = false;
    initializeBoard();
    initializeMarbles();
}

void PixelMarblesScene::onSingleClick() {
    //yawInitialized_ = false;
    marbleEngine_.clearObstacles();
    marbleEngine_.clearMarbles();
    
    switch (currentBoard_) {
        case Boards::NoObstacles :
            currentBoard_ = Boards:: Diamond;
            initDiamond();
        break;

        case Boards:: Diamond :
            currentBoard_ = Boards:: Pegs;
            initPegs();
        break;

        case Boards:: Pegs :
            currentBoard_ = Boards:: Baffles;
            initBaffles();
        break;

        case Boards::Baffles :
            currentBoard_ = Boards:: Funnel;
            initFunnel();
        break;

        case Boards::Funnel :
            currentBoard_ = Boards:: NoObstacles;
        break;
    }
    initializeMarbles();
}

void PixelMarblesScene::initDiamond() {
  int centerX = COLUMNS/2;
  int centerY = ROWS/2;
  int extentX = 7;
  int extentY = 7;
  int plusMinus;
  int leftX = centerX - extentX;
  int leftY = centerY;
  int rightX = centerX + extentX;
  int rightY = centerY;
  int topX = centerX;
  int topY = centerY - extentY;

  // Draw the left half of the diamond with the positive y=mx 
  for (int col = leftX; col <= centerX; col++) {
    plusMinus = (float(leftY-topY)/float(topX-leftX)) * (col-leftX);   // Offset X to run from 0 to centerX
    for(int row = centerY - plusMinus; row <= centerY + plusMinus; row++) {
      marbleEngine_.setObstacle(col, row);
    }
  }
  // Draw the right half of the diamond with the negative y=mx
  for (int col = topX; col <= rightX; col++) {
    plusMinus = (float(rightY-topY)/float(rightX-topX)) * (rightX-col);   // Offset X to run rightX to 0
    for(int row = centerY - plusMinus; row <= centerY + plusMinus; row++) {
      marbleEngine_.setObstacle(col, row);
    }
  }
}

void PixelMarblesScene::initBaffles() {
    drawRectObstacle(0, 23, 20, 24);
    drawRectObstacle(8, 15, COLUMNS-1, 16);
    drawRectObstacle(0, 6, 20, 7);
}

void PixelMarblesScene::drawRectObstacle(int llx, int lly, int urx, int ury) {
    for(int col=llx; col<=urx; col++) {
        for(int row=lly; row<= ury; row++) {
            marbleEngine_.setObstacle(col,row);
        }
    }
}

void PixelMarblesScene::initPegs() {
    for(int y=0; y<3; y++) {
        for(int x=0; x<3; x++) {
            //drawRectObstacle(x*8+5, y*8+5, x*8+8, y*8+8);
            drawRectObstacle(y%2 ? x*8+4 : x*8+8, y*8+5, y%2 ? x*8+7 : x*8+11, y*8+8);
        }
    }
}

void PixelMarblesScene::initFunnel() {
    for(int y=10; y<27; y++) {
        int x = (y-10) + 1;
        marbleEngine_.setObstacle(15+x, y);
        marbleEngine_.setObstacle(15-x, y);
    }
    for(int y=8; y<10; y++) {
        int x = 1;
        marbleEngine_.setObstacle(15+x, y);
        marbleEngine_.setObstacle(15-x, y);
    }
}

void PixelMarblesScene::processPose(const WandPacket& packet) {

    /*
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
    */

    // So we don't apply board angle around neutral wand position
    float fbAngle = applyDeadband(packet.pitch);
    float lrAngle = applyDeadband(packet.roll);
    //float lrAngle = applyDeadband(relativeYaw);

    float gx = GRAVITY * sinf(lrAngle);
    float gy = GRAVITY * sinf(fbAngle);

    marbleEngine_.setGravity(gx, gy);
}

void PixelMarblesScene::initializeMarbles() {

    // fill from the top so we can drop marbles over any obstacles
    for (int y=ROWS-1; y>= 0; y--) {
        for (int x=0; x< COLUMNS; x++) {
            marbleEngine_.addMarble( x, y,
                marbleColors_[random( 0, sizeof(marbleColors_) / sizeof(marbleColors_[0]))]
            );

            if (marbleEngine_.getMarbleCount() >= NUM_MARBLES)
                return;
        }
    }
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

void PixelMarblesScene::initializeBoard() { 
    currentBoard_ = Boards:: NoObstacles;
    marbleEngine_.clearObstacles();
}

void PixelMarblesScene::renderBoard(PixelFramebuffer& fb) { 
    for (int y = 0; y < ROWS; y++) {
        for (int x=0; x< COLUMNS; x++) {
            if(marbleEngine_.isObstacle(x,y)) {
                graphics_.writePixel( fb.data(), x, y, c_.darkgray);
            }
        }
    }
}

void PixelMarblesScene::renderMarbles(PixelFramebuffer& fb) { 

    for (int i = 0; i < marbleEngine_.getMarbleCount(); i++) {
        const auto& marble = marbleEngine_.getMarble(i);
        graphics_.writePixel( fb.data(), lroundf(marble.x), lroundf(marble.y), marble.color);
    }
}

void PixelMarblesScene::render(PixelFramebuffer& fb) {

    const uint32_t now = millis();

    while (now - lastPhysicsMs_ >= PHYSICS_PERIOD_MS) {
        lastPhysicsMs_ += PHYSICS_PERIOD_MS;
        marbleEngine_.update(PHYSICS_PERIOD_MS / 1000.0f);
    }

    if (now - lastRenderMs_ < RENDER_PERIOD_MS)
        return;

    lastRenderMs_ = now;
    fb.clear();
    renderMarbles(fb);
    renderBoard(fb);
}


