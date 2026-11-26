
#pragma once
#include <Arduino.h>
#include "Scene.h"
#include "graphicsPrimitives.h"

/* 
   Class that implements the Marshmallow Roasting game on the PixelPanel
   The game is simple, the user controls the height of a marshmallow on a stick.
   The campfire is at the bottom of the screen.  The user has to roast the  
   marshmallow to the perfect doneness without having it catch on fire and drop 
   into the fire.  Time is the score.  The lower the time the better.  User pushes 
   the click button to start a new round.  The round ends with either the marshmallow
   reaches 100% doneness or the marshmallow burns.  A game twist is that the fire can 
   occasionally flare up requiring them to pull the marshmallow up or risk setting it on fire.
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
    enum class ParticleType { Smoke, Spark };
    void updateMarble(int index, float dt);

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
        float vy;
        float width;
        float height;
        float temperature;
        float doneness;
    };

    Marshmallow mm_ = {
        stick_.endX,
        stick_.endY,
        0.0,
        6.0f,
        6.0f,
        0.0f,
        0.0f
    };
    RGB mmColor_ = c_.white;

    struct ExtraBrowning {
        int x;
        int y;
        float bf;
    };

    struct Particle {
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f;
        float vy = defaultVy_;
        uint32_t born = 0;
        uint32_t lastUpdateMs;
        uint16_t maxAge = defaultMaxAge_;
        bool  active = false;
        uint8_t decay = defaultDecay_;
        RGB   color;
    };

    static constexpr uint16_t defaultMaxAge_ = 2000;
    static constexpr float defaultVy_ = 10.0;
    static constexpr uint16_t defaultDecay_ = 4;
    static constexpr uint8_t NUM_PARTICLES = 160;
    static constexpr float IGNITION_DELAY = .75f;  // sec
    Particle sp_[NUM_PARTICLES];

    bool marshmallowFalling_ = false;  // true when the burning marshmallow is falling into the fire
    bool marshmallowBurned_ = false;

    State state_ = State::Roasting;
    uint32_t stateStartMs_ = 0;
    uint32_t lastRenderMs_ = 0;
    uint32_t lastPhysicsMs_ = 0;
    uint32_t lastFireMs_ = 0;
    uint32_t lastParticleMs_ = 0;
    uint32_t lastParticleInitMs_ = 0;
    uint32_t nextFlareMs_ = 0;
    uint32_t flareStartMs_ = 0;
    bool flareActive_ = false;

    float ignitionTime_ = 0.0; // in seconds
    bool sparksInited_ = false;
    uint32_t sparkStartMs_ = 0;
    static constexpr uint32_t SPARK_PERIOD = 500;  //ms amount of time to generate sparks
    static constexpr uint32_t SPARKS_DONE = 1500;  //ms  time after sparks start before game over.

    uint16_t const RENDER_PERIOD_MS = 20;
    uint16_t const FIRE_PERIOD_MS = 50;
    uint16_t const PARTICLE_PERIOD_MS = 30;

    uint32_t roundStartMs = 0;
    int roundTimeMs_ = 0; 
    int bestTimeMs_ = 0; // RAM only; retained while scene object exists

    void enter(State state);
    void startNewGame();
    void finishRound();

    void updateFire();
    void updateMarshmallow(float dt);
    void updateStick();
    void initParticle(ParticleType type);
    void updateParticle();
    void updateFlare(uint32_t now);
    void updateBurned(float dt);
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
    void renderBurned(PixelFramebuffer& fb);
    void renderStick(PixelFramebuffer& fb);
    void renderParticles(PixelFramebuffer& fb);
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
    static constexpr float PARTICLE_START_TEMP  =   100.0f; // degrees-c
    static constexpr float DROP_HEIGHT  =   20.0f; // pixels
    static constexpr float DROP_ACCELERATION    =   40.0f; // pixels/sec*sec

    static constexpr uint32_t FLARE_MIN_INTERVAL_MS = 1000;
    static constexpr uint32_t FLARE_MAX_INTERVAL_MS = 4000;
    static constexpr uint32_t FLARE_DURATION_MS = 800;
    static constexpr uint32_t FLARE_RECOVERY_MS = 1000;
    static constexpr float FLARE_PEAK_INTENSITY = 3.5f;

};