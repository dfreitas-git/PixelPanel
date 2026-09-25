
#include "Diffuser.h"
 
// called this way, it uses the default address 0x40
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);
// you can also call it with a different address and I2C interface
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, Wire);

bool Diffuser::begin() {

     pinMode(CONTROL_POT, INPUT);
     pwm.begin();
     pwm.setOscillatorFrequency(26000000);
     pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates
     return true;
}

// Check the potentiometer to see what spacing to set between the 
// LEDs and the diffuser screen (controlled by 4-servos).
void Diffuser::sampleDiffuserPot() {
    int potVal = analogRead(CONTROL_POT);
    if(abs(potVal - lastPotVal) > 100) {
        int pulseLen0 = map(potVal,0,4095,SERVOMIN_0,SERVOMAX_0);
        int pulseLen1 = map(potVal,0,4095,SERVOMIN_1,SERVOMAX_1);
        int pulseLen2 = map(potVal,0,4095,SERVOMIN_2,SERVOMAX_2);
        int pulseLen3 = map(potVal,0,4095,SERVOMIN_3,SERVOMAX_3);
        pwm.setPWM(0, 0, pulseLen0);
        pwm.setPWM(1, 0, pulseLen1);
        pwm.setPWM(2, 0, pulseLen2);
        pwm.setPWM(3, 0, pulseLen3);
        lastPotVal = potVal;
        Serial.printf("PotVal: %d   Pulse: %d\n",potVal,pulseLen0);
    }
};