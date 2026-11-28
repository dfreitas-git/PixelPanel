#pragma once
#include <Arduino.h>
#include "Scene.h"
#include "graphicsPrimitives.h"
#include "MarbleEngine.h"

/*
    PixelMarblesScene and MarbleEngine implement a board of marbles that roll with 
    the tilting of the board.  Some boards have obstacles that the marbles have 
    to roll around.  The following is what each class takes care of:

    PixelMarblesScene                      MarbleEngine	                                 
    -----------------                      ------------                                 
    Define obstacle patterns               Store obstacle locations	                    
    Select/initialize active pattern       Prevent movement into obstacles	             
	Choose obstacle color                  Prevent marble initialization inside obstacle
    Render marbles and obstacles           Clear and set obstacles	                     
    Handle button clicks                   Provide obstacle query method	               
    Wand pose interpretation               Compute marble velocities, locations         
    Physics update scheduling

    We maintain an occupancy map as our single source of truth for both marbles and obstacles

*/

class PixelMarblesScene : public Scene {
public:
    explicit PixelMarblesScene(GraphicsPrimitives& graphics) : graphics_(graphics) {}
    const char* name() const override { return "MARBL"; }
    void begin() override;
    void onSingleClick() override;
    void processPose(const WandPacket& packet) override;
    void render(PixelFramebuffer& framebuffer) override;
    int preferredDiffuserPosition() const override { return 0; }

private:
    palette c_;
    GraphicsPrimitives& graphics_;
    MarbleEngine marbleEngine_;

    enum class Boards { NoObstacles, Diamond, Pegs, Baffles };

    bool yawInitialized_ = false;
    float initialYaw_ = 0.0f;

    // Desired number of marbles for this scene.
    // Must not exceed MarbleEngine MAX_MARBLES storage capacity.
    static constexpr int NUM_MARBLES = 120;
    static_assert(NUM_MARBLES <= MarbleEngine::getMaxMarbles(),
              "Scene marble count exceeds engine capacity");

    RGB marbleColors_[5] = {c_.red, c_.green, c_.blue, c_.yellow, c_.purple};
    static constexpr float MAX_TILT = PI / 6.0f;
    static constexpr float TILT_DEADBAND = 3.0f * PI / 180.0f;
    float applyDeadband(float angle) const;
    static constexpr float GRAVITY = 120.0f;
    uint16_t const RENDER_PERIOD_MS = 20;
    static constexpr uint32_t PHYSICS_PERIOD_MS = 10;
    static_assert( MarbleEngine::getMaxVelocity() * (PHYSICS_PERIOD_MS / 1000.0f) <= 1.0f,
               "Max marble displacement exceeds one pixel per physics cycle");
    uint32_t lastPhysicsMs_ = 0;
    uint32_t lastRenderMs_ = 0;
    Boards currentBoard_ = Boards::NoObstacles;
    void initializeBoard();
    void initializeMarbles();
    void drawRectObstacle(int llx, int lly, int urx, int ury);
    void renderBoard(PixelFramebuffer& framebuffer);
    void renderMarbles(PixelFramebuffer& framebuffer);
    void initDiamond();
    void initBaffles();
    void initPegs();
};
