#include <Arduino.h>
#include <Servo.h>

#include "motor.hpp"
#include "ultrasound.hpp"

// #define DEBUG

using u8 = uint8_t;

namespace pins {
const u8 MOTOR0_H = 3;
const u8 MOTOR0_L = 5;
const u8 MOTOR1_H = 6;
const u8 MOTOR1_L = 9;

const u8 ULTRASOUND_ECHO = 2;
const u8 ULTRASOUND_TRIG = 8;

const u8 SERVO_PULSE = 4;
}  // namespace pins

enum class State : u8 {
    ApproachingRampStart,
    ApproachingRampFinish,
    ApproachingWall,
    ThrowingBall,
    MovingBack,
};
const u16 RAMP_TRANSITION_START = 150;
const u16 RAMP_TRANSITION_END = 200;
const u16 WALL_THRESHOLD = 75;
const int FORWARD_DUTY = 255;
const int BACKWARD_DUTY = -255;
const u16 LOOP_FREQ = 100;
const u16 LOOP_DELAY = 1000 / LOOP_FREQ;

const float SERVO_SPEED = 60;  // degrees per second
const u16 DELAY_PER_DEGREE_INCREMENT = 1000.0f / SERVO_SPEED;
const u8 START_ANGLE = 0;
const u8 END_ANGLE = 180;

Servo servo;
Motor motor0(pins::MOTOR0_H, pins::MOTOR0_L);
Motor motor1(pins::MOTOR1_H, pins::MOTOR1_L);
Ultrasound ultrasound(pins::ULTRASOUND_TRIG, pins::ULTRASOUND_ECHO);
State state = State::ApproachingRampStart;
u16 prev_dist = 1000;

void setup() {
    motor0.init();
    motor1.init();
    ultrasound.init();
    servo.attach(pins::SERVO_PULSE);
#ifdef DEBUG
    Serial.begin(115200);
#endif
    motor0.drive(FORWARD_DUTY);
    motor1.drive(FORWARD_DUTY);
}

void loop() {
    auto start = millis();
    ultrasound.start_measurement();
    u16 dist = ultrasound.get_dist_mm();
#ifdef DEBUG
    Serial.print("dist(mm): ");
    Serial.print(dist);
    Serial.print("state: ");
#endif

    switch (state) {
        case State::ApproachingRampStart: {
#ifdef DEBUG
            Serial.println("ApproachingRampStart");
#endif
            if (dist < prev_dist && dist < RAMP_TRANSITION_START) {
                state = State::ApproachingRampFinish;
            }
            break;
        }
        case State::ApproachingRampFinish: {
#ifdef DEBUG
            Serial.println("ApproachingRampFinish");
#endif
            if (dist > RAMP_TRANSITION_END) {
                state = State::ApproachingWall;
            }
            break;
        }
        case State::ApproachingWall: {
#ifdef DEBUG
            Serial.println("ApproachingWall");
#endif
            // can do some better ramping here maybe
            // but simply stopping should be fine
            if (dist <= WALL_THRESHOLD) {
                motor0.drive(0);
                motor1.drive(0);
                state = State::ThrowingBall;
            }
            break;
        }
        case State::ThrowingBall: {
#ifdef DEBUG
            Serial.println("ThrowingBall");
#endif
            u8 angle = START_ANGLE;
            servo.write(angle);
            while (angle < END_ANGLE) {
                auto start = millis();
                angle += 1;
                servo.write(angle);
                while (millis() < (start + DELAY_PER_DEGREE_INCREMENT)) {
                }
            }
            while (angle > START_ANGLE) {
                auto start = millis();
                angle -= 1;
                servo.write(angle);
                while (millis() < (start + DELAY_PER_DEGREE_INCREMENT)) {
                }
            }
            state = State::MovingBack;
            break;
        }
        case State::MovingBack: {
#ifdef DEBUG
            Serial.println("MovingBack");
#endif
            motor0.drive(BACKWARD_DUTY);
            motor1.drive(BACKWARD_DUTY);
            break;
        }
    };

    prev_dist = dist;
    while (millis() < (start + LOOP_DELAY)) {
    }
}
