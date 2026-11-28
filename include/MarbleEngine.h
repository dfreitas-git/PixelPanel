
#pragma once
#include <Arduino.h>
#include "graphicsPrimitives.h"

/*
    MarbleEngine
    Physical environment and simulation
    Marble storage and active count
    Occupancy map
    Obstacle storage
    Gravity and acceleration
    Velocity and position integration
    aaBoundary and collision resolution
*/

class MarbleEngine {
public:

    struct Marble {
        float x = 0;
        float y = 0;
        float vx = 0;
        float vy = 0;
        RGB color;
    };

    void reset();
    void clearObstacles();
    void clearMarbles();
    bool setObstacle(int x, int y);
    bool isObstacle(int x, int y) const;

    bool addMarble(float x, float y, RGB color);
    void setGravity(float gx, float gy);
    void update(float dt);
    int getMarbleCount() const ;
    const Marble& getMarble(int index) const;
    static constexpr int getMaxMarbles() {
        return MAX_MARBLES;
    }
    static constexpr float getMaxVelocity() {
        return MAX_VELOCITY;
    }


private:

    // Maximum storage capacity of the physics engine.
    static constexpr int MAX_MARBLES = 120;

    Marble marbles_[MAX_MARBLES];
    int marbleCount_ = 0;

    static constexpr float DRAG = 0.5f;
    static constexpr float MIN_VELOCITY = 0.1f;
    static constexpr float MAX_VELOCITY = 75.0f;
    float gravityX_ = 0.0f;
    float gravityY_ = 0.0f;
    int16_t occupancy_[ROWS][COLUMNS];
    static constexpr float COLLISION_RETENTION = 0.8f;

    void updateMarble(int index, float dt);

    // Occupancy map for collision avoidance
    static constexpr int16_t EMPTY_CELL = -1;
    static constexpr int16_t OBSTACLE_CELL = -2;
    bool isCellAvailable(int x, int y, int index) const;
};