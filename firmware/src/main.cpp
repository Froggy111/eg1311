#include <Arduino.h>
#include <Servo.h>

#include "motor.hpp"

namespace pins {
const uint8_t MOTOR0_H = 3;
const uint8_t MOTOR0_L = 5;
const uint8_t MOTOR1_H = 6;
const uint8_t MOTOR1_L = 9;

const uint8_t ULTRASOUND_ECHO = 2;
const uint8_t ULTRASOUND_TRIG = 8;

const uint8_t SERVO_PULSE = 4;
}  // namespace pins

Servo servo;
Motor motor0(pins::MOTOR0_H, pins::MOTOR0_L);
Motor motor1(pins::MOTOR1_H, pins::MOTOR1_L);

void setup() {
    motor0.init();
    motor1.init();
    servo.attach(pins::SERVO_PULSE);
    pinMode(pins::ULTRASOUND_ECHO, INPUT);
    pinMode(pins::ULTRASOUND_TRIG, OUTPUT);
    Serial.begin(115200);
}

const int ANGLE = 90;
const int TICK = 3;
void loop() {
    motor0.drive(128);
    motor1.drive(128);
    digitalWrite(pins::ULTRASOUND_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(pins::ULTRASOUND_TRIG, LOW);
    auto pulse_us = pulseIn(pins::ULTRASOUND_ECHO, HIGH);
    Serial.print("pulse us:");
    Serial.println(pulse_us);
    Serial.println("loop");

    servo.write(90);
    delay(1000);
    servo.write(0);
    delay(1000);
}
