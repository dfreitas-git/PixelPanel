
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

I am collaborating with chatGPT in designing and coding this project.

dlf  9/22/2026
*/

#include <Arduino.h>
#include <WiFi.h>

#include "globals.h"
#include "PanelRadio.h"
#include "PixelFramebuffer.h"
#include "PixelOutput.h"
#include "TestPanel.h"
#include "Diffuser.h"
#include "PanelController.h"
#include "graphicsPrimitives.h"
#include "graphicsParticles.h"
#include "graphicsComposites.h"
#include "graphicsAnimations.h"

PanelRadio panelRadio;
PixelFramebuffer framebuffer;
PixelOutput pixelOutput;
PanelController panelController;
TestPanel testPanel;
Diffuser diffuser;
GraphicsPrimitives gPrim;
GraphicsParticles gPart;
GraphicsComposites gComp;
GraphicsAnimations gAnim;

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

    framebuffer.clear();
    pixelOutput.show(framebuffer);
    gAnim.initShootingStars();  // So we have something the first time through the loop

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

    static char text[4];
    while (panelRadio.getNextPacket(packet)) {

        // Process continuous pose/motion data
        // using packet.roll, packet.pitch, etc.
    
        if (panelRadio.processPacket(packet, event)) {
            Serial.print(F("Event # ")); Serial.println(event.eventSequence);
            panelController.processGesture(event.gesture);
            itoa(event.eventSequence, text, 10);
        }
    }

    palette c;
    RGB *bbuf = framebuffer.data();

    // Use rocket for testing
    static bool doOnce = true;
    if(doOnce) {
        gAnim.initShootingStars();
        doOnce = false;
    }
    gAnim.shootingStar(bbuf);
    gPrim.drawString(bbuf, text, 5, 5, {255,255,255}, {0,0,0});
    pixelOutput.show(framebuffer);
    
}