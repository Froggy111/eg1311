#include <Arduino.h>
#include <Servo.h>

#include "HardwareSerial.h"
#include "motor.hpp"
#include "ultrasound.hpp"

#define DEBUG

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
    Forward,
    Wall,
    Throwing,
    WallBack,
    Back,
    Stopped,
};

const u16 WALL_THRESHOLD = 75;  // need to measure this
const int FORWARD_DUTY = 128;
const int BACKWARD_DUTY = -128;
const u16 LOOP_FREQ = 100;
const u16 LOOP_DELAY = 1000 / LOOP_FREQ;
u16 STARTING_US_VAL;
bool STARTING_US_READ = false;
const u16 FORWARD_TIME = 10e3;
const u16 BACKWARD_TIME = 10e3;

const float SERVO_SPEED = 60;  // degrees per second
const u16 DELAY_PER_DEGREE_INCREMENT = 1000.0f / SERVO_SPEED;
const u8 START_ANGLE = 0;
const u8 END_ANGLE = 180;

Servo servo;
Motor motor0(pins::MOTOR0_H, pins::MOTOR0_L);
Motor motor1(pins::MOTOR1_H, pins::MOTOR1_L);
Ultrasound ultrasound(pins::ULTRASOUND_TRIG, pins::ULTRASOUND_ECHO);
State state = State::Forward;
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

unsigned long LAST_START;
bool LAST_START_INITED = false;

void loop() {
    auto start = millis();
    ultrasound.start_measurement();
    u16 dist = ultrasound.get_dist_mm();
    if (!STARTING_US_READ) {
        STARTING_US_VAL = dist;
        STARTING_US_READ = true;
    }
#ifdef DEBUG
    Serial.print("dist(mm): ");
    Serial.print(dist);
    Serial.print(", state: ");
#endif

    switch (state) {
        case State::Forward: {
#ifdef DEBUG
            Serial.println("Forward");
#endif  // DEBUG
            if (!LAST_START_INITED) {
                LAST_START = start;
                LAST_START_INITED = true;
            }
            motor0.drive(FORWARD_DUTY);
            motor1.drive(FORWARD_DUTY);

            while (millis() < LAST_START + FORWARD_TIME) {
            };
            state = State::Wall;
            break;
        }
        case State::Wall: {
#ifdef DEBUG
            Serial.println("Wall");
#endif
            if (dist <= WALL_THRESHOLD) {
                motor0.drive(0);
                motor1.drive(0);
                state = State::Throwing;
            }
            break;
        }
        case State::Throwing: {
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
            motor0.drive(BACKWARD_DUTY);
            motor1.drive(BACKWARD_DUTY);
            state = State::WallBack;
            break;
        }
        case State::WallBack: {
            if (dist >= WALL_THRESHOLD) {
                LAST_START = start;
                state = State::Back;
            }
            break;
        }
        case State::Back: {
            while (millis() < LAST_START + BACKWARD_TIME) {
            };
            motor0.drive(0);
            motor1.drive(0);
            state = State::Stopped;
            break;
        }
        case State::Stopped: {
            break;
        }
    }

    prev_dist = dist;
    while (millis() < (start + LOOP_DELAY)) {
    }
}
