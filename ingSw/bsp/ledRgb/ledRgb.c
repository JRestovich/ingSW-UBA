#include "ledRgb.h"
#include <stddef.h>

#define COLOR_MAX 255

typedef struct {
    pwm_channel red;
    pwm_channel green;
    pwm_channel blue;
} rgbChannels_t;

typedef struct {
    timer_num timer;
    rgbChannels_t channels;
} ledRgbConfig_t;

static const ledRgbConfig_t led_rgb_configs[LED_RGB_COUNT] = {
    [LED_RGB_STATUS_1] = {
        .timer = TIMER1,
        .channels = {
            .red = PWM_CHANNEL_1,
            .green = PWM_CHANNEL_2,
            .blue = PWM_CHANNEL_3,
        },
    },
};

static const ledRgbConfig_t *_getConfig(ledRgb_id id);
static pwm_polarity _connection2polarity(ledRgb_elec_conn connection);
static uint8_t _color2pcnt(uint8_t col);

bool LED_RGB_ctor(ledRgb *led, ledRgb_id id, colorSequence *colSeq,
                  ledRgb_elec_conn connection) {
    const ledRgbConfig_t *config;

    if (!led) {
        return false;
    }
    
    config = _getConfig(id);
    if (!config || !colSeq || !colSeq->seq || colSeq->stepQty == 0 ||
        connection < LED_RGB_COMMON_CATHODE ||
        connection > LED_RGB_COMMON_ANODE) {
        led->valid = false;
        return false;
    }

    led->pwm = PWM_ctor(config->timer, _connection2polarity(connection));
    if (!led->pwm ||
        !PWM_hasChannel(led->pwm, config->channels.red) ||
        !PWM_hasChannel(led->pwm, config->channels.green) ||
        !PWM_hasChannel(led->pwm, config->channels.blue)) {
        led->valid = false;
        return false;
    }

    led->colorSeq = colSeq;
    led->id = id;
    led->colorSeq->index = 0;
    led->valid = true;
    return true;
}

bool LED_RGB_init(ledRgb *self) {
    return self && self->valid && self->pwm && PWM_init(self->pwm);
}

bool LED_RGB_start(ledRgb *self) {
    const ledRgbConfig_t *config;

    if (!self || !self->valid || !self->pwm) {
        return false;
    }

    config = _getConfig(self->id);
    if (!config) {
        return false;
    }

    bool ok = true;
    ok &= PWM_start(self->pwm, config->channels.red);
    ok &= PWM_start(self->pwm, config->channels.green);
    ok &= PWM_start(self->pwm, config->channels.blue);

    if (!ok) {
        (void)LED_RGB_stop(self);
    }

    return ok;
}

bool LED_RGB_stop(ledRgb *self) {
    const ledRgbConfig_t *config;

    if (!self || !self->valid || !self->pwm) {
        return false;
    }

    config = _getConfig(self->id);
    if (!config) {
        return false;
    }
    bool ok = true;
    ok &= PWM_stop(self->pwm, config->channels.red);
    ok &= PWM_stop(self->pwm, config->channels.green);
    ok &= PWM_stop(self->pwm, config->channels.blue);

    return ok;
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

bool LED_RGB_setColor(ledRgb *self, uint8_t index) {
    const ledRgbConfig_t *config;

    if (!self || !self->valid || !self->pwm || !self->colorSeq ||
        !self->colorSeq->seq || index >= self->colorSeq->stepQty) {
        return false;
    }

    config = _getConfig(self->id);
    if (!config) {
        return false;
    }

    self->colorSeq->index = index;

    color_s color = self->colorSeq->seq[self->colorSeq->index].color;

    bool ok = true;
    ok &= PWM_setDuty(self->pwm, config->channels.red, _color2pcnt(color.red));
    ok &= PWM_setDuty(self->pwm, config->channels.green, _color2pcnt(color.green));
    ok &= PWM_setDuty(self->pwm, config->channels.blue, _color2pcnt(color.blue));

    if (!ok) {
        return LED_RGB_stop(self);
    }

    return ok;
}

bool LED_RGB_nextStep(ledRgb *self, bool cyclic) {
    uint8_t index;

    if (!self || !self->valid || !self->pwm || !self->colorSeq ||
        !self->colorSeq->seq || self->colorSeq->stepQty == 0) {
        return false;
    }

    index = self->colorSeq->index + 1U;
    if (index >= self->colorSeq->stepQty) {
        if (!cyclic) {
            return LED_RGB_stop(self);
        }
        index = 0U;
    }

    return LED_RGB_setColor(self, index);
}

static const ledRgbConfig_t *_getConfig(ledRgb_id id) {
    if ((unsigned int)id >= LED_RGB_COUNT) {
        return NULL;
    }

    return &led_rgb_configs[id];
}

static pwm_polarity _connection2polarity(ledRgb_elec_conn connection) {
    return connection == LED_RGB_COMMON_ANODE ? PWM_POLARITY_LOW
                                               : PWM_POLARITY_HIGH;
}

static uint8_t _color2pcnt(uint8_t col) {
    return col * 100 / COLOR_MAX;
}
