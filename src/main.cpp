
/* PixelPanel : ESP32 code written in the Arduino PlatformIO IDE.

The code controls a 30x30 ws2812 pixel array (made up of five strips of 180 LED's each).
The pixels are addressed as row-0 at the bottom, row-29 at the top, column-0 at the left, 
column 29 at the right.  So like quadrant-0 of a cartesian coordinate system.

The PixelPanel communicates with a separate wand via ESP-NOW wireless protocol.  The wand
will be used to control what scenes are displayed on the panel, and to interact with the 
scenes (the wand will transmit orientation, acceleration so gestures may be used to control
scene parameters).

The PixelPanel has a software controlled diffuser screen which may be moved closer/farther 
from the LEDs to produce different visual blurring effects.

To add a new scene do the following:
    1) Create  SceneName.h (or copy an existing one as a template)
        - Edit/add class name
        - instantiate private settings{}
        - Update name() method with menu name
    2) Create ScemeName.cpp (or copy an existing one as a template)
        - Edit/add processPose code,
        - Edit/add processGesture code
        - Edit/add render call
    3) Edit the graphicsAnimation function to add settings structure, modify the animation function to use the settings
    4) Update main.cpp
       - #include SceneName.h
       - instantiate scene object
       - register scene with addScene in setup


I am collaborating with chatGPT in designing and coding this project.

dlf  9/22/2026
*/

#include <Arduino.h>
#include <WiFi.h>

#include "Globals.h"
#include "PanelRadio.h"
#include "PixelFramebuffer.h"
#include "PixelOutput.h"
#include "TestPanel.h"
#include "PanelMenu.h"
#include "Diffuser.h"
#include "PanelController.h"
#include "graphicsPrimitives.h"
#include "graphicsParticles.h"
#include "graphicsComposites.h"
#include "graphicsAnimations.h"
#include "ShootingStarsScene.h"
#include "FireworksScene.h"
#include "CheckerBoardScene.h"
#include "EyeballScene.h"
#include "Pacman1Scene.h"
#include "TestScene.h"


PanelRadio panelRadio;

PixelFramebuffer framebuffer;
PixelOutput pixelOutput;

TestPanel testPanel;
Diffuser diffuser;

// Instantiate the graphics objects
GraphicsPrimitives gPrim;
GraphicsParticles gPart;
GraphicsComposites gComp;
GraphicsAnimations gAnim;

// The scenes use our existing animation engine from MyGraphics
ShootingStarsScene shootingStarsScene(gAnim);
FireworksScene fireworksScene(gAnim);
CheckerBoardScene checkerBoardScene(gAnim);
EyeballScene eyeballScene(gAnim);
Pacman1Scene pacman1Scene(gAnim);
TestScene testScene(testPanel);

PanelController panelController;
PanelMenu panelMenu(gPrim);

constexpr uint16_t DIFFUSER_UPDATE_TIME = 200;  // ms between reading diffuser setting pot
uint16_t lastDiffuserUpdate = 0;

void setup()
{
    Serial.begin(115200);

     // Wait for USB serial connection (min 2 seconds, max 5) so serial prints show up...
    while ((!Serial || millis() < 2000) && millis() < 5000) {
        delay(10);
    }

    // Set up the diffuser servos
    diffuser.begin();

    // Set up ESP-NOW
    if (!panelRadio.begin(WAND_MAC)) {
        Serial.println(F("Fatal ESP-NOW initialization error."));

        while (true) {
            delay(1000);
        }
    }

    if (!pixelOutput.begin()) {
        Serial.println(F("Fatal PixelOutput initialization error."));
        while (true) {
            delay(1000);
        }
    }

    // Register the scenes and select the default
    panelController.addScene(shootingStarsScene);
    panelController.addScene(fireworksScene);
    panelController.addScene(checkerBoardScene);
    panelController.addScene(eyeballScene);
    panelController.addScene(pacman1Scene);

    panelController.selectScene(0);

    framebuffer.clear();
    pixelOutput.show(framebuffer);

}

void loop()
{
    // Manually setting diffuser distance via a potentiometer
    uint32_t now = millis();
    if(now-lastDiffuserUpdate > DIFFUSER_UPDATE_TIME) {
        diffuser.sampleDiffuserPot();
        lastDiffuserUpdate = now;
    }

    WandPacket packet;
    GestureEvent event;

    while (panelRadio.getNextPacket(packet)) {

        WandMode previousMode = panelController.getMode();

        // Process continuous pose/motion data using packet.roll, packet.pitch, etc.
        panelController.processPose(packet);

        // When we go into panel mode, reset navigation direction state in case we exited
        // panel mode last time while in a L/R/U/D state
        if (previousMode != panelController.getMode() && panelController.getMode() == WandMode::Panel) {
            panelMenu.resetNavigation();
        }

        // PanelMenu owns navigation and hierarchy. Execute only its requests.
        if (packet.mode == WandMode::Panel) {
            MenuAction action = panelMenu.processPose(packet, panelController);

            switch (action.type) {
            case MenuActionType::SelectScene:
                panelController.selectScene(static_cast<uint8_t>(action.value));
                break;

            case MenuActionType::SelectTest:
                testScene.setPattern( static_cast<TestPattern>(action.value));
                panelController.setActiveScene(testScene);
                break;
            }
        }

        // Process any gestures
        if (panelRadio.processPacket(packet, event)) {
            panelController.processGesture(event);
        }
    }

    // Clear the framebuffer so we don't have residual bits from a previous scene
    // Technically we would only need to do this if the last scene and current scene
    // changed, but this is simple and safe since we always switch modes when changing a scene
    static WandMode lastMode = WandMode::Scene;
    WandMode mode = panelController.getMode();
    if (mode != lastMode) {
        framebuffer.clear();
        lastMode = mode;
    }

    // Scene mode
    if (panelController.getMode() == WandMode::Scene) {
        panelController.renderScene(framebuffer);
    }
    // Panel mode
    else {
        panelMenu.render(framebuffer, panelController);
    }

    // Output whichever mode rendered the framebuffer
    pixelOutput.show(framebuffer);

}
