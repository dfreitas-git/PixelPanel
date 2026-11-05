
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

// Application coordinator: menu requests do not directly manipulate hardware.
void applyMenuAction(const MenuAction& action)
{
    switch (action.type) {
        case MenuActionType::SelectScene:
            panelController.selectScene(static_cast<uint8_t>(action.value));
            break;

        case MenuActionType::SelectTest:
            testScene.setPattern(static_cast<TestPattern>(action.value));
            panelController.setActiveScene(testScene);
            break;

        case MenuActionType::SetBrightness:
            panelController.setBrightnessScale(static_cast<uint8_t>(action.value));
            break;

        case MenuActionType::SetDiffuser:
            panelController.setDiffuserOffset(action.value);
            break;

        default:
            break;
    }
}

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
    panelController.addScene(shootingStarsScene);  // index0
    panelController.addScene(fireworksScene);      // index1
    panelController.addScene(checkerBoardScene);   // index2
    panelController.addScene(eyeballScene);        // index3
    panelController.addScene(pacman1Scene);        // index4

    panelController.selectScene(0);   // Start with shootingStars

    framebuffer.clear();
    pixelOutput.show( framebuffer, panelController.getBrightnessScale());

}

void loop()
{
    // Sample the physical minimum-safe-distance potentiometer
    uint32_t now = millis();
    if(now-lastDiffuserUpdate > DIFFUSER_UPDATE_TIME) {
        diffuser.sampleDiffuserPot();
        lastDiffuserUpdate = now;
    }

    WandPacket packet;
    GestureEvent event;

    while (panelRadio.getNextPacket(packet)) {
        // Button events and scene gestures share the existing event-sequence
        // deduplication. Buttons must be encoded by the wand in the gesture
        // field (see shared WandProtocol.h and integration notes).
        const bool hasEvent = panelRadio.processPacket(packet, event);

        if (hasEvent && event.gesture == WandGesture::ButtonSingle) {
            MenuAction action = panelMenu.push(panelController);
            applyMenuAction(action);
        }
        else if (hasEvent && event.gesture == WandGesture::ButtonDouble) {
            panelMenu.pop();
        }
        else if (hasEvent && panelMenu.isRunningScene()) {
            panelController.processGesture(event);
        }

        // Continuous pose goes to exactly one destination.
        if (panelMenu.isRunningScene()) {
            panelController.processPose(packet);
        } else {
            MenuAction action = panelMenu.processPose(packet, panelController);
            applyMenuAction(action);
        }
    }

    // Clearing prevents trails from an animation leaking into the menu,
    // or old menu text leaking into a newly activated animation.
    static bool wasRunningScene = false;
    const bool runningScene = panelMenu.isRunningScene();

    if (runningScene != wasRunningScene) {
        framebuffer.clear();
        wasRunningScene = runningScene;
    }

    if (runningScene) {
        panelController.renderScene(framebuffer);
    } else {
        panelMenu.render(framebuffer, panelController);
    }

    // Diffuser: menus require focus; running scenes use their requested blur + user offset.
    const int requestedDiffuserPosition = runningScene ?  panelController.getSceneDiffuserPosition() + panelController.getDiffuserOffset() : 0;

    if(DEBUG_PRINT == 2) {
        static uint32_t lastPrint = 0;
        if (millis() - lastPrint > 1000) {
            lastPrint = millis();

            Serial.printf(
              "Scene: %d  Offset: %d  Requested: %d  PotMin: %d  Applied: %d\n",
              panelController.getSceneDiffuserPosition(),
              panelController.getDiffuserOffset(),
              requestedDiffuserPosition,
              diffuser.getMinSafePosition(),
              diffuser.getAppliedPosition()
            );
        }
    }

    diffuser.setPosition(requestedDiffuserPosition);

    // Output whichever view rendered the framebuffer.
    pixelOutput.show(framebuffer, panelController.getBrightnessScale());

}
