
// Class to control the servos that set the spacing between the LEDs and the diffuser screen.
// We use this to create different blur effects for different scenes
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>


class Diffuser {
public:

    bool begin();
    void sampleDiffuserPot();
    void setPosition(int requestedPercent);  // 0=closest, 100=farthest (logical)
    int getMinSafePosition() const { return minSafePercent; }
    int getAppliedPosition() const { return lastAppliedPercent; }

private:

    Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

    int lastPotVal = -1;    // ADC-change filter
    int minSafePercent = 0; // Potentiometer sets minimum allowed distance
    int requestedPercent = 0;
    int lastAppliedPercent = -1; // Avoid redundant I2C writes

    void applyPosition();

    // IMPORTANT: verify these directions against your actual rack/servo motion.
    // When false, pulse=SERVOMIN is assumed closest to LEDs on all four servos.
    // If a servo is mechanically reversed, use its own endpoints appropriately.
    static constexpr bool POT_REVERSED = false;
    static constexpr bool PULSE_REVERSED = true; // smallest pulse is the largest diffuser distance

    static constexpr uint8_t CONTROL_POT =  34 ;

    // Depending on your servo make, the pulse width min and max may vary, you  want 
    // these to be as small/large as possible without hitting the hard stop for max range. 
    //You'll have to tweak them as necessary to match the servos you have!
    static constexpr  uint16_t SERVOMIN_0 = 200; // This is the 'minimum' pulse length count (out of 4096)
    static constexpr  uint16_t SERVOMAX_0 = 500; // This is the 'maximum' pulse length count (out of 4096)
    static constexpr  uint16_t SERVOMIN_1 = 200; 
    static constexpr  uint16_t SERVOMAX_1 = 500; 
    static constexpr  uint16_t SERVOMIN_2 = 200;
    static constexpr  uint16_t SERVOMAX_2 = 500;
    static constexpr  uint16_t SERVOMIN_3 = 200;
    static constexpr  uint16_t SERVOMAX_3 = 500;
    static constexpr  uint8_t  SERVO_FREQ = 50; // Analog servos run at ~50 Hz updates


};

