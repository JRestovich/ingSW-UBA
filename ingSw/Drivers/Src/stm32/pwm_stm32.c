#include "pwm.h"
#include "stm32f4xx_hal.h"

/* Valores equivalentes a los generados por CubeMX en MX_TIM1_Init(). */
#define PWM_PRESCALER        0U
#define PWM_PERIOD            41999U
#define PWM_INITIAL_PULSE     0U
#define PWM_MAX_CHANNELS      4U

typedef struct {
  pwm_channel channel;
  GPIO_TypeDef *gpio;
  uint16_t pin;
  uint32_t alternate;
  uint32_t initial_pulse;
  pwm_polarity polarity;
  bool configured;
} pwm_channel_t;

typedef struct struct_pwm_t
{
  TIM_HandleTypeDef htim;
  TIM_TypeDef *timer;
  pwm_channel_t channels[PWM_MAX_CHANNELS];
  uint8_t channel_count;
  bool valid;
} pwm_t;

static pwm_t pwms[TIMER_COUNT] = {
  [TIMER1] = {
    .timer = TIM1,
    .channels = {
      { PWM_CHANNEL_1, GPIOA, GPIO_PIN_8,  GPIO_AF1_TIM1, PWM_INITIAL_PULSE, PWM_POLARITY_LOW, true },
      { PWM_CHANNEL_2, GPIOA, GPIO_PIN_9,  GPIO_AF1_TIM1, PWM_INITIAL_PULSE, PWM_POLARITY_LOW, true },
      { PWM_CHANNEL_3, GPIOA, GPIO_PIN_10, GPIO_AF1_TIM1, PWM_INITIAL_PULSE, PWM_POLARITY_LOW, true },
    },
    .channel_count = 3,
    .valid = true,
  },
};

/* Declaraciones privadas: sus definiciones se encuentran tras la API pública. */
static void pwm_enable_timer_clock(TIM_TypeDef *timer);
static void pwm_enable_gpio_clock(GPIO_TypeDef *gpio);
static bool pwm_init_base(pwm_t *self);
static bool pwm_configure_clock_source(pwm_t *self);
static bool pwm_configure_master(pwm_t *self);
static bool pwm_configure_channel(pwm_t *self, const pwm_channel_t *channel);
static bool pwm_configure_break_dead_time(pwm_t *self);
static void pwm_configure_gpio(const pwm_channel_t *channel);
static pwm_channel_t *pwm_find_channel(pwm_t *self, pwm_channel channel);
static uint32_t pwm_hal_channel(pwm_channel channel);
static uint32_t pwm_hal_polarity(pwm_polarity polarity);

pwm_t *PWM_ctor(timer_num tim)
{
  if (tim > TIMER4 || !pwms[tim].valid) {
    return NULL;
  }

  return &pwms[tim];
}

bool PWM_hasChannel(pwm_t *self, pwm_channel channel)
{
  return pwm_find_channel(self, channel) != NULL;
}

bool PWM_init(pwm_t *self)
{
  if (self == NULL) {
    return false;
  }

  pwm_enable_timer_clock(self->timer);

  if (!pwm_init_base(self) ||
      !pwm_configure_clock_source(self) ||
      HAL_TIM_PWM_Init(&self->htim) != HAL_OK ||
      !pwm_configure_master(self) ||
      !pwm_configure_break_dead_time(self)) {
    return false;
  }

  for (uint8_t index = 0U; index < self->channel_count; index++) {
    pwm_channel_t *channel = &self->channels[index];

    if (!channel->configured) {
      continue;
    }

    pwm_enable_gpio_clock(channel->gpio);
    if (!pwm_configure_channel(self, channel)) {
      return false;
    }
    pwm_configure_gpio(channel);
  }

  return true;
}

bool PWM_start(pwm_t *self, pwm_channel channel)
{
  uint32_t hal_channel = pwm_hal_channel(channel);

  return pwm_find_channel(self, channel) != NULL &&
         HAL_TIM_PWM_Start(&self->htim, hal_channel) == HAL_OK;
}

bool PWM_stop(pwm_t *self, pwm_channel channel)
{
  uint32_t hal_channel = pwm_hal_channel(channel);

  return pwm_find_channel(self, channel) != NULL &&
         HAL_TIM_PWM_Stop(&self->htim, hal_channel) == HAL_OK;
}

bool PWM_setDuty(pwm_t *self, pwm_channel channel, uint8_t duty_pcnt)
{
  uint32_t hal_channel = pwm_hal_channel(channel);
  pwm_channel_t *configured_channel = pwm_find_channel(self, channel);

  if (configured_channel == NULL) {
    return false;
  }

  if (duty_pcnt > 100) {
    duty_pcnt = 100;
  }

  uint32_t period = __HAL_TIM_GET_AUTORELOAD(&self->htim);
  uint32_t pulse = period * duty_pcnt / 100U;

  configured_channel->initial_pulse = pulse;
  __HAL_TIM_SET_COMPARE(&self->htim, hal_channel, pulse);
  return true;
}

bool PWM_setPolarity(pwm_t *self, pwm_channel channel, pwm_polarity polarity)
{
  pwm_channel_t *configured_channel = pwm_find_channel(self, channel);

  if (configured_channel == NULL ||
      (polarity > PWM_POLARITY_LOW)) {
    return false;
  }

  configured_channel->polarity = polarity;
  return pwm_configure_channel(self, configured_channel);
}

static void pwm_enable_timer_clock(TIM_TypeDef *timer)
{
  if (timer == TIM1) {
    __HAL_RCC_TIM1_CLK_ENABLE();
  } else if (timer == TIM2) {
    __HAL_RCC_TIM2_CLK_ENABLE();
  } else if (timer == TIM3) {
    __HAL_RCC_TIM3_CLK_ENABLE();
  } else if (timer == TIM4) {
    __HAL_RCC_TIM4_CLK_ENABLE();
  }
}

static void pwm_enable_gpio_clock(GPIO_TypeDef *gpio)
{
  if (gpio == GPIOA) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  } else if (gpio == GPIOB) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  }
}

static bool pwm_init_base(pwm_t *self)
{
  self->htim.Instance = self->timer;
  self->htim.Init.Prescaler = PWM_PRESCALER;
  self->htim.Init.CounterMode = TIM_COUNTERMODE_UP;
  self->htim.Init.Period = PWM_PERIOD;
  self->htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  self->htim.Init.RepetitionCounter = 0U;
  self->htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  return HAL_TIM_Base_Init(&self->htim) == HAL_OK;
}

static bool pwm_configure_clock_source(pwm_t *self)
{
  TIM_ClockConfigTypeDef clock = {0};

  clock.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  return HAL_TIM_ConfigClockSource(&self->htim, &clock) == HAL_OK;
}

static bool pwm_configure_master(pwm_t *self)
{
  TIM_MasterConfigTypeDef master = {0};

  master.MasterOutputTrigger = TIM_TRGO_RESET;
  master.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  return HAL_TIMEx_MasterConfigSynchronization(&self->htim, &master) == HAL_OK;
}

static bool pwm_configure_channel(pwm_t *self, const pwm_channel_t *channel)
{
  TIM_OC_InitTypeDef output = {0};

  output.OCMode = TIM_OCMODE_PWM1;
  output.Pulse = channel->initial_pulse;
  output.OCPolarity = pwm_hal_polarity(channel->polarity);
  output.OCNPolarity = TIM_OCPOLARITY_HIGH;
  output.OCFastMode = TIM_OCFAST_DISABLE;
  output.OCIdleState = TIM_OCIDLESTATE_RESET;
  output.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  return HAL_TIM_PWM_ConfigChannel(&self->htim, &output,
                                   pwm_hal_channel(channel->channel)) == HAL_OK;
}

static bool pwm_configure_break_dead_time(pwm_t *self)
{
  TIM_BreakDeadTimeConfigTypeDef break_dead_time = {0};

  if (self->timer != TIM1) {
    return true;
  }

  break_dead_time.OffStateRunMode = TIM_OSSR_DISABLE;
  break_dead_time.OffStateIDLEMode = TIM_OSSI_DISABLE;
  break_dead_time.LockLevel = TIM_LOCKLEVEL_OFF;
  break_dead_time.DeadTime = 0U;
  break_dead_time.BreakState = TIM_BREAK_DISABLE;
  break_dead_time.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  break_dead_time.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;

  return HAL_TIMEx_ConfigBreakDeadTime(&self->htim, &break_dead_time) == HAL_OK;
}

static void pwm_configure_gpio(const pwm_channel_t *channel)
{
  GPIO_InitTypeDef gpio_init = {0};

  gpio_init.Pin = channel->pin;
  gpio_init.Mode = GPIO_MODE_AF_PP;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
  gpio_init.Alternate = channel->alternate;
  HAL_GPIO_Init(channel->gpio, &gpio_init);
}

static pwm_channel_t *pwm_find_channel(pwm_t *self, pwm_channel channel)
{
  if (self == NULL) {
    return NULL;
  }

  for (uint8_t index = 0U; index < self->channel_count; index++) {
    pwm_channel_t *configured_channel = &self->channels[index];

    if (configured_channel->configured && configured_channel->channel == channel) {
      return configured_channel;
    }
  }

  return NULL;
}

static uint32_t pwm_hal_channel(pwm_channel channel)
{
  switch (channel) {
    case PWM_CHANNEL_1:
      return TIM_CHANNEL_1;
    case PWM_CHANNEL_2:
      return TIM_CHANNEL_2;
    case PWM_CHANNEL_3:
      return TIM_CHANNEL_3;
    case PWM_CHANNEL_4:
      return TIM_CHANNEL_4;
    default:
      return TIM_CHANNEL_1;
  }
}

static uint32_t pwm_hal_polarity(pwm_polarity polarity)
{
  return polarity == PWM_POLARITY_LOW ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH;
}
