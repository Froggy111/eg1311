#include <Arduino.h>

const uint32_t UM_PER_US = 340e6 / 1e6 / 2;
const uint16_t TIMEOUT = 2e6 / UM_PER_US;
const uint16_t TIMEOUT_MM = (TIMEOUT * UM_PER_US) / 1000;

class Ultrasound {
    uint8_t trig;
    uint8_t echo;

   public:
    inline Ultrasound(uint8_t trig_, uint8_t echo_) {
        trig = trig_, echo = echo_;
    };
    void inline init() {
        pinMode(this->trig, OUTPUT);
        pinMode(this->echo, INPUT);
    }
    void inline start_measurement() {
        digitalWrite(this->trig, 1);
        delayMicroseconds(10);
        digitalWrite(this->trig, 0);
    }
    uint16_t inline get_pulse_us() {
        uint16_t pulse = pulseIn(this->echo, HIGH, TIMEOUT);
        if (pulse == 0) {
            pulse = TIMEOUT;
        }
        return pulse;
    }
    uint16_t inline get_dist_mm() {
        uint32_t dist_um = (uint32_t)this->get_pulse_us() * UM_PER_US;
        return dist_um / 1000;
    }
};
