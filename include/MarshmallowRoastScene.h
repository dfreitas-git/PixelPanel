
#pragma once
#include <Arduino.h>
#include "Scene.h"
#include "graphicsPrimitives.h"

/* 
   Class that implements the Marshmallow Roasting game on the PixelPanel
   The game is simple, the user controls the height of a marshmallow on a stick.
   The campfire is at the bottom of the screen.  The user has to roast the  
   marshmallow to the perfect doneness without having it catch on fire and drop 
   into the fire.  Score is based on how close to perfectly done it is in the 
   minimum amount of time.   User pushes the click button to start a new round ant
   to stop roasting in the current round.  A game twist is that the fire can occasionally 
   flare up requiring them to pull the marshmallow up or risk setting it on fire.
 */

class MarshmallowRoastScene : public Scene {
public:
    explicit MarshmallowRoastScene(GraphicsPrimitives& graphics) : graphics_(graphics) {}
    const char* name() const override { return "ROAST"; }
    void begin() override;
    void onSingleClick() override;
    void processPose(const WandPacket& packet) override;
    void render(PixelFramebuffer& framebuffer) override;
    int preferredDiffuserPosition() const override { return 0; }

private:
    enum class State { Roasting, Result, Burned };

     struct Flame {
        int v1x,v1y;    
        int v2x,v2y;
        int v3x,v3y;
        RGB color;
    };

    palette c_;

    GraphicsPrimitives& graphics_;
    State state_ = State::Roasting;
    uint32_t stateStartMs_ = 0;
    uint32_t lastRenderMs_ = 0;
    uint32_t lastPhysicsMs_ = 0;
    uint32_t lastFireMs_ = 0;

    uint16_t const RENDER_PERIOD_MS = 20;
    uint16_t const FIRE_PERIOD_MS = 50;

    RGB marshmallowColor_ = c_.white;

    float temperature_ = 0.0f;
    float doneness_ = 0.0f;

    float const stickOriginX_ = 39.0;
    float const stickOriginY_ = 17.0;
    float stickEndX_ = 15.0;
    float stickEndY_ = 17.0;
    float stickLength_ = 25.0;
    float stickAngle_ = PI;

    uint32_t roundStartMs = 0;
    int roundScore_ = 0;
    int bestScore_ = 0; // RAM only; retained while scene object exists

    void enter(State state);
    void startNewGame();
    void finishRound();

    void updateFire();
    void updateMarshmallow(float dt);
    void updateStick();
    void burnMarshmallow();

    int rXoff(int x, int range);
    int rYoff(int y, int range);
    Flame flame_[4];
    RGB flameColors_[2] = {c_.red,c_.yellow};
    void renderFire(PixelFramebuffer& fb);
    void renderMarshmallow(PixelFramebuffer& fb);
    void renderStick(PixelFramebuffer& fb);
    void renderScore(PixelFramebuffer& fb);
    void drawCentered(PixelFramebuffer& fb, const char* text, int y, RGB color);

};