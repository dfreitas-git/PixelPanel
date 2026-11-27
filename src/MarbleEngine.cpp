
#include "MarbleEngine.h"
#include <algorithm>

    void MarbleEngine::reset() {

        marbleCount_ = 0;
        gravityX_ = 0.0f;
        gravityY_ = 0.0f;

        // Clear the board
        for (int y = 0; y < ROWS; y++) {
            for (int x = 0; x < COLUMNS; x++) {
                occupancy_[y][x] = EMPTY_CELL;
            }
        }
    }

    void MarbleEngine::clearObstacles() { }
    void MarbleEngine::setObstacle(int x, int y) { }

    bool MarbleEngine::addMarble(float x, float y, RGB color) {

        if (marbleCount_ >= MAX_MARBLES)
            return false;
    
        if (x < 0.0f || x > COLUMNS - 1 ||
            y < 0.0f || y > ROWS - 1)
            return false;
    
        int cellX = lroundf(x);
        int cellY = lroundf(y);
    
        if (occupancy_[cellY][cellX] != EMPTY_CELL)
            return false;
    
        Marble& m = marbles_[marbleCount_];
    
        m.x = x;
        m.y = y;
        m.vx = 0.0f;
        m.vy = 0.0f;
        m.color = color;
    
        occupancy_[cellY][cellX] = marbleCount_;
        marbleCount_++;
    
        return true;
    }

    bool MarbleEngine::isCellAvailable(int x, int y, int index) const {
        if (x < 0 || x >= COLUMNS ||
            y < 0 || y >= ROWS)
            return false;
    
        int16_t occupant = occupancy_[y][x];
        return occupant == EMPTY_CELL || occupant == index;
    }

    void MarbleEngine::setGravity(float gx, float gy) {
        gravityX_ = gx;
        gravityY_ = gy;
    }

    void MarbleEngine::update(float dt) {
        for (int i = 0; i < marbleCount_; i++) {
            updateMarble(i, dt);
        }
    }

    /* gravity sorting
    void MarbleEngine::update(float dt) {

        int order[MAX_MARBLES];
    
        // Initialize processing order
        for (int i = 0; i < marbleCount_; i++) {
            order[i] = i;
        }
    
        // Sort marbles from furthest downhill to furthest uphill.
        // We're using the dot product of each marble's position and 
        // the gravity vector to establish its processing priority.
        // P=xg_x+yg_y
        // The larger the priority, the further downhill the marble is.

        std::sort(order, order + marbleCount_,
            [this](int a, int b) {
    
                float priorityA = marbles_[a].x * gravityX_ +
                                  marbles_[a].y * gravityY_;
    
                float priorityB = marbles_[b].x * gravityX_ +
                                  marbles_[b].y * gravityY_;
    
                return priorityA > priorityB;
            });
    
        // Update marbles in sorted order
        for (int i = 0; i < marbleCount_; i++) {
            updateMarble(order[i], dt);
        }
    }
    */

    /*  random indexing
    void MarbleEngine::update(float dt) {

        int order[MAX_MARBLES];
    
        // Initialize processing order
        for (int i = 0; i < marbleCount_; i++) {
            order[i] = i;
        }
    
        // Randomize processing order (Fisher-Yates shuffle)
        for (int i = marbleCount_ - 1; i > 0; i--) {
            int j = random(i + 1);
            std::swap(order[i], order[j]);
        }
    
        // Update marbles in randomized order
        for (int i = 0; i < marbleCount_; i++) {
            updateMarble(order[i], dt);
        }
    }
   */

    void MarbleEngine::updateMarble(int index, float dt) {

        Marble& m = marbles_[index];
    
        // Apply acceleration
        m.vx += gravityX_ * dt;
        m.vy += gravityY_ * dt;
    
        // Apply drag
        float damping = expf(-DRAG * dt);
        m.vx *= damping;
        m.vy *= damping;
    
        // Stop very slow coasting
        if (gravityX_ == 0.0f && fabsf(m.vx) < MIN_VELOCITY)
            m.vx = 0.0f;
    
        if (gravityY_ == 0.0f && fabsf(m.vy) < MIN_VELOCITY)
            m.vy = 0.0f;
    
        // Limit velocity
        m.vx = constrain(m.vx, -MAX_VELOCITY, MAX_VELOCITY);
        m.vy = constrain(m.vy, -MAX_VELOCITY, MAX_VELOCITY);
    
        int cellX = lroundf(m.x);
        int cellY = lroundf(m.y);
    
        // -------------------------------------------------
        // Attempt X movement
        // -------------------------------------------------
    
        float nextX = m.x + m.vx * dt;
        int nextCellX = lroundf(nextX);
    
        if (nextX >= 0.0f && nextX <= COLUMNS - 1 &&
            isCellAvailable(nextCellX, cellY, index)) {
    
            if (nextCellX != cellX) {
                occupancy_[cellY][cellX] = EMPTY_CELL;
                occupancy_[cellY][nextCellX] = index;
                cellX = nextCellX;
            }
    
            m.x = nextX;
    
        } else {
            //m.vx = 0.0f;
            m.vx *= COLLISION_RETENTION;
        }
    
        // -------------------------------------------------
        // Attempt Y movement
        // -------------------------------------------------
    
        float nextY = m.y + m.vy * dt;
        int nextCellY = lroundf(nextY);
    
        if (nextY >= 0.0f && nextY <= ROWS - 1 &&
            isCellAvailable(cellX, nextCellY, index)) {
    
            if (nextCellY != cellY) {
                occupancy_[cellY][cellX] = EMPTY_CELL;
                occupancy_[nextCellY][cellX] = index;
            }
    
            m.y = nextY;
    
        } else {
            //m.vy = 0.0f;
            m.vy *= COLLISION_RETENTION;
        }
    }


    int MarbleEngine::getMarbleCount() const {
        return marbleCount_;
    }

    const MarbleEngine::Marble& MarbleEngine::getMarble(int index) const {
        return marbles_[index];
    }
