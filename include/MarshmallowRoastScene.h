
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
    int preferredDiffuserPosition() const override { return 10; }

private:
    enum class State { Roasting, Result, Burned };

    GraphicsPrimitives& graphics_;
    palette c_;

     struct Flame {
        int v1x,v1y;    
        int v2x,v2y;
        int v3x,v3y;
        RGB color;
    };

     struct Stick {
        float originX = 39.0;
        float originY = 17.0;    
        float endX = 15.0;
        float endY = 17.0;
        float length = 25.0;
        float angle = PI;      // radians
    };
    Stick stick_;
    RGB stickColor_ = c_.darkbrown;

    struct Marshmallow {
        float x;
        float y;
        float width;
        float height;
        float angle;      // radians
        float temperature;
        float doneness;
    };

    Marshmallow mm_ = {
        stick_.endX,
        stick_.endY,
        6.0f,
        6.0f,
        stick_.angle,      // radians
        0.0f,
        0.0f
    };
    RGB mmColor_ = c_.white;

    struct ExtraBrowning {
        int x;
        int y;
        float bf;
    };

    State state_ = State::Roasting;
    uint32_t stateStartMs_ = 0;
    uint32_t lastRenderMs_ = 0;
    uint32_t lastPhysicsMs_ = 0;
    uint32_t lastFireMs_ = 0;
    float ignitionTime_ = 0.0; // in seconds

    uint16_t const RENDER_PERIOD_MS = 20;
    uint16_t const FIRE_PERIOD_MS = 50;

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
    static constexpr  uint8_t NUMBER_OF_FLAMES = 10; 
    Flame flame_[NUMBER_OF_FLAMES];
    RGB flameColors_[2] = {c_.red,c_.yellow};
    float fireIntensity_ = 1.0;    // used for "flare-ups"

    // how much faster some marshmallow pixels brown than others
    static constexpr uint8_t EXTRA_BROWNING_PIXELS = 6;
    ExtraBrowning browningFactor_[EXTRA_BROWNING_PIXELS];   

    static constexpr float COAL_Y = 4.0;

    // Marshmallow "sprite"
    static constexpr int MM_SIZEX = 6;
    static constexpr int MM_SIZEY = 6;
    RGB mmSpritePixels_[MM_SIZEY][MM_SIZEX];

    RGB lerpColor(const RGB& a, const RGB& b, float t);
    void renderFire(PixelFramebuffer& fb);
    void renderMarshmallow(PixelFramebuffer& fb);
    void renderStick(PixelFramebuffer& fb);
    void renderScore(PixelFramebuffer& fb);
    void drawCentered(PixelFramebuffer& fb, const char* text, int y, RGB color);
    void fillRectSprite(RGB color);
    RGB  getDonenessColor(float doneness);
    void initExtraBrowning();

    // cooking constants
    static constexpr float AMBIENT_TEMP = 20.0f;   // degrees-c
    static constexpr float HEATING_RATE = 80.0f;   // degrees/sec
    static constexpr float HEAT_SCALE = 5.0f;      // pixels
    static constexpr float COOLING_RATE = 0.15f;    // 1/sec
    static constexpr float COOKING_THRESHOLD = 80.0f; 
    static constexpr float COOKING_RATE = 0.15f; 
    static constexpr float IGNITION_TEMP = 180.0f; // degrees-c
    static constexpr float IGNITION_DELAY = .75f;  // sec

};