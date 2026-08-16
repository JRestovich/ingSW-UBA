#include "ledRgb.h"

#define COLOR_MAX 255

uint8_t color2pcnt(uint8_t col);

bool LED_RGB_ctor(ledRgb *led, timer_num tim, colorSequence *colSeq) {

    if (!led) {
        return false;
    }
    
    if (!colSeq || !colSeq->seq || colSeq->stepQty == 0) {
        led->valid = false;
        return false;
    }

    led->pwm = PWM_ctor(tim);
    if (!led->pwm) {
        led->valid = false;
        return false;
    }

    led->colorSeq = colSeq;
    led->colorSeq->index = 0;
    led->valid = true;
    return true;
}

bool LED_RGB_init(ledRgb *self) {
    return self && self->valid && self->pwm && PWM_init(self->pwm);
}

bool LED_RGB_start(ledRgb *self) {
    static const pwm_channel rgb_channels[] = {
        PWM_CHANNEL_1,
        PWM_CHANNEL_2,
        PWM_CHANNEL_3,
    };

    if (!self || !self->valid || !self->pwm) {
        return false;
    }

    for (uint8_t i = 0; i < sizeof(rgb_channels) / sizeof(rgb_channels[0]); ++i) {
        if (!PWM_start(self->pwm, rgb_channels[i])) {
            while (i > 0) {
                --i;
                (void)PWM_stop(self->pwm, rgb_channels[i]);
            }
            return false;
        }
    }

    return true;
}

bool LED_RGB_stop(ledRgb *self) {
    static const pwm_channel rgb_channels[] = {
        PWM_CHANNEL_1,
        PWM_CHANNEL_2,
        PWM_CHANNEL_3,
    };
    bool stopped = true;

    if (!self || !self->valid || !self->pwm) {
        return false;
    }

    for (uint8_t i = 0; i < sizeof(rgb_channels) / sizeof(rgb_channels[0]); ++i) {
        if (!PWM_stop(self->pwm, rgb_channels[i])) {
            stopped = false;
        }
    }

    return stopped;
}

bool LED_RGB_setColorSequence(ledRgb *self, colorSequence *colSeq) {
    if (!self || !self->pwm) {
        return false;
    }

    if (!colSeq || !colSeq->seq || colSeq->stepQty == 0) {
        return false;
    }

    self->colorSeq = colSeq;
    self->colorSeq->index = 0;
    self->valid = true;
    return true;
}

bool LED_RGB_nextStep(ledRgb *self, bool cyclic) {
    if (!self || !self->valid || !self->pwm || !self->colorSeq ||
        !self->colorSeq->seq || self->colorSeq->stepQty == 0) {
        return false;
    }

    self->colorSeq->index++;
    if (self->colorSeq->index >= self->colorSeq->stepQty) {
        if (!cyclic) {
            return LED_RGB_stop(self);
        }
        self->colorSeq->index = 0;
    }

    color_s color = self->colorSeq->seq[self->colorSeq->index].color;
    bool duty_set = PWM_setDuty(self->pwm, PWM_CHANNEL_1, color2pcnt(color.red));
    duty_set = PWM_setDuty(self->pwm, PWM_CHANNEL_2, color2pcnt(color.green)) && duty_set;
    duty_set = PWM_setDuty(self->pwm, PWM_CHANNEL_3, color2pcnt(color.blue)) && duty_set;
    if (!duty_set) {
        return LED_RGB_stop(self);
    }

    return true;
}

uint8_t color2pcnt(uint8_t col) {
    return col * 100 / COLOR_MAX;
}
