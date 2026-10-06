#include <Arduino.h>

class Motor {
    uint8_t p0;
    uint8_t p1;

   public:
    inline Motor(uint8_t pin0, uint8_t pin1) { p0 = pin0, p1 = pin1; };
    inline void init() {
        pinMode(p0, OUTPUT);
        pinMode(p1, OUTPUT);
    }
    inline void drive(int drive) {
        if (drive > 0) {
            analogWrite(p1, 0);
            analogWrite(p0, drive);
        } else {
            analogWrite(p0, 0);
            analogWrite(p1, -drive);
        }
    }
};
