#include "Diffuser.h"

bool Diffuser::begin() {
    pinMode(CONTROL_POT, INPUT);
    pwm.begin();
    pwm.setOscillatorFrequency(26000000);
    pwm.setPWMFreq(SERVO_FREQ);
    sampleDiffuserPot();  // Apply minimum limit before the first servo command
    return true;
}

// The physical pot no longer directly sets the servo target.
// It defines the minimum allowed logical diffuser distance (0..100).
void Diffuser::sampleDiffuserPot() {
    const int adc = constrain(analogRead(CONTROL_POT), 0, 4095);
    if (lastPotVal >= 0 && abs(adc - lastPotVal) <= 100) return;
    lastPotVal = adc;
    minSafePercent = map(POT_REVERSED ? 4095 - adc : adc, 0, 4095, 0, 100);
    applyPosition();
}

void Diffuser::setPosition(int percent) {
    requestedPercent = constrain(percent, 0, 100);
    applyPosition();
}

void Diffuser::applyPosition() {
    const int applied = constrain(requestedPercent, minSafePercent, 100);
    if (applied == lastAppliedPercent) return;
    lastAppliedPercent = applied;

    const int logical = PULSE_REVERSED ? 100 - applied : applied;
    const uint16_t minPulse[4] = {SERVOMIN_0, SERVOMIN_1, SERVOMIN_2, SERVOMIN_3};
    const uint16_t maxPulse[4] = {SERVOMAX_0, SERVOMAX_1, SERVOMAX_2, SERVOMAX_3};
    for (uint8_t i = 0; i < 4; ++i) {
        const uint16_t pulse = map(logical, 0, 100, minPulse[i], maxPulse[i]);
        pwm.setPWM(i, 0, pulse);
    }
}
