
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
#include "graphicsPrimitives.h"

PanelRadio panelRadio;
PixelFramebuffer framebuffer;
PixelOutput pixelOutput;
TestPanel testPanel;
Diffuser diffuser;
GraphicsPrimitives gPrim;

constexpr uint8_t PANEL_MAC[6] = { 0x2C, 0xBC, 0xBB, 0x4B, 0x7C, 0x60 };
constexpr uint16_t DIFFUSER_UPDATE_TIME = 200;  // ms between reading diffuser setting pot
uint16_t lastDiffuserUpdate = 0;

void setup()
{
    Serial.begin(115200);

    // Set up the diffuser servos
    diffuser.begin();

    // Set up ESP-NOW
    if (!panelRadio.begin(PANEL_MAC)) {
        Serial.println("Fatal ESP-NOW initialization error.");

        while (true) {
            delay(1000);
        }
    }

    if (!pixelOutput.begin()) {
        Serial.println("Fatal PixelOutput initialization error.");
        while (true) {
            delay(1000);
        }
    }

}

void loop()
{
    // Manually setting diffuser distance via a potentiometer
    uint32_t now = millis();
    if(now-lastDiffuserUpdate > DIFFUSER_UPDATE_TIME) {
        diffuser.sampleDiffuserPot();
        lastDiffuserUpdate = now;
    }

    // Testing framebuffer
    //testPanel.runAllTests(pixelOutput, framebuffer);

    palette c;
    RGB *bbuf = framebuffer.data();
    framebuffer.clear();
    gPrim.drawTriangle(bbuf,{3,3},{20,15},{27,3},c.red);
    gPrim.drawTriangle(bbuf,{5,20},{10,29},{15,20},c.green);
    gPrim.drawLine(bbuf,0,15,29,15,1,c.blue);
    gPrim.drawArc(bbuf,{1,20},{15,10},{28,25}, 1, c.limegreen);
    gPrim.drawRect(bbuf,20,20,29,29,0,c.darkred);
    gPrim.drawRect(bbuf,23,23,28,25,0,c.green);
    gPrim.drawDiamond(bbuf,25,25,2,4,c.blue);
    gPrim.drawEllipse(bbuf,12,5,6,3,0,c.blue);
    gPrim.drawCircle(bbuf,5,25,3,c.blue);
    pixelOutput.show(framebuffer);
    delay(2000);

    framebuffer.clear();
    gPrim.drawDiagonalBands(bbuf,0,1,2,1,29,c.red);
    gPrim.drawDiagonalBands(bbuf,0,-1,2,2,29,c.blue);
    gPrim.drawString(bbuf,"PINE", 2,15, c.green, c.black);
    pixelOutput.show(framebuffer);
    delay(2000);

    /*
    // dlf  Need to fix this.  Right now the the ESP-NOW callback can asynchronously overwrite the packet while we are 
    // reading this.  Need to add a producer/consumer interface.  Probably adding a "getLatestPacket" or some such that
    // returns the latest complete packet and tells the caller when a new one is ready.
    const WandPacket &packet = panelRadio.getPacket();

    Serial.print("magic: ");Serial.println(packet.magic);
    Serial.print("version: ");Serial.println(packet.version);
    Serial.print("sequence: ");Serial.println(packet.sequence);
    Serial.print("timestampMs: ");Serial.println(packet.timestampMs);
    Serial.print("roll: ");Serial.println(packet.roll*180/PI);
    Serial.print("pitch: ");Serial.println(packet.pitch*180/PI);
    Serial.print("yaw: ");Serial.println(packet.yaw*180/PI);
    Serial.print("gx: ");Serial.println(packet.gx);
    Serial.print("gy: ");Serial.println(packet.gy);
    Serial.print("gz: ");Serial.println(packet.gz);
    Serial.print("vx: ");Serial.println(packet.vx);
    Serial.print("vy: ");Serial.println(packet.vy);
    Serial.print("vz: ");Serial.println(packet.vz);
    Serial.println(" ");
    delay(500);
    */
}