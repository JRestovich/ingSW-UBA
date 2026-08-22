#include "pwm.h"
#include "uart.h"
#include "stm32f4xx_hal.h"
#include "ring_buffer.h"

typedef struct struct_uart_t
{
  UART_HandleTypeDef huart;
  USART_TypeDef *instance;

  GPIO_TypeDef *gpio_rx;
  uint16_t pin_rx;

  GPIO_TypeDef *gpio_tx;
  uint16_t pin_tx;
  uint32_t alternate;

  uart_callback_t txCompleteCallback;
  uart_callback_t rxCompleteCallback;

  bool valid;
} uart_t;

static uart_t uarts[UART_COUNT] = {
  [UART_2] = {
    .instance = USART2,
    .gpio_rx = GPIOA,
    .pin_rx = GPIO_PIN_3,
    .gpio_tx = GPIOA,
    .pin_tx = GPIO_PIN_2,
    .alternate = GPIO_AF7_USART2,
    .valid = true,
  },
};

/* Declaraciones privadas: sus definiciones se encuentran tras la API pública. */
static void _uart_enable_clock(USART_TypeDef *instance);
static void _uart_enable_gpio_clock(GPIO_TypeDef *gpio);
static void _uart_configure_gpio(GPIO_TypeDef *gpio, uint16_t pin,
                                 uint32_t alternate);
static bool _uart_init_base(uart_t *self, BAUDRATE_t baud,
                            STOP_BITS_t stop_bits, PARITY_t parity);
static uint32_t _uart_hal_stop_bits(STOP_BITS_t stop_bits);
static uint32_t _uart_hal_parity(PARITY_t parity);
static void _uart_enable_irq(USART_TypeDef *instance);
static uart_t *_uart_from_handle(UART_HandleTypeDef *huart);

uart_t *UART_ctor(uart_num uartNum)
{
  uart_t *self;

  if (uartNum >= UART_COUNT) {
    return NULL;
  }

  self = &uarts[uartNum];
  if (!self->valid) {
    return NULL;
  }

  _uart_enable_clock(self->instance);
  _uart_enable_gpio_clock(self->gpio_tx);
  _uart_enable_gpio_clock(self->gpio_rx);
  _uart_configure_gpio(self->gpio_tx, self->pin_tx, self->alternate);
  _uart_configure_gpio(self->gpio_rx, self->pin_rx, self->alternate);

  return self;
}

bool UART_init(uart_t *self, BAUDRATE_t baud, STOP_BITS_t stopBits,
               PARITY_t parity)
{
  if (self == NULL || !self->valid ||
      (baud != BAUDRATE_9600 && baud != BAUDRATE_115200) ||
      stopBits > TWO_STOP_BIT || parity > EVEN) {
    return false;
  }

  return _uart_init_base(self, baud, stopBits, parity);
}

bool UART_setTxCallback(uart_t *self, uart_callback_t callback)
{
  if (self == NULL || !self->valid) {
    return false;
  }

  self->txCompleteCallback = callback;
  return true;
}

bool UART_setRxCallback(uart_t *self, uart_callback_t callback)
{
  if (self == NULL || !self->valid) {
    return false;
  }

  self->rxCompleteCallback = callback;
  return true;
}

static void _uart_enable_clock(USART_TypeDef *instance)
{
  if (instance == USART1) {
    __HAL_RCC_USART1_CLK_ENABLE();
  } else if (instance == USART2) {
    __HAL_RCC_USART2_CLK_ENABLE();
  } else if (instance == USART3) {
    __HAL_RCC_USART3_CLK_ENABLE();
  }
}

static void _uart_enable_gpio_clock(GPIO_TypeDef *gpio)
{
  if (gpio == GPIOA) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  } else if (gpio == GPIOB) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  } else if (gpio == GPIOC) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  }
}

static void _uart_configure_gpio(GPIO_TypeDef *gpio, uint16_t pin,
                                 uint32_t alternate)
{
  GPIO_InitTypeDef gpio_init = {0};

  gpio_init.Pin = pin;
  gpio_init.Mode = GPIO_MODE_AF_PP;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init.Alternate = alternate;
  HAL_GPIO_Init(gpio, &gpio_init);
}

static bool _uart_init_base(uart_t *self, BAUDRATE_t baud,
                            STOP_BITS_t stop_bits, PARITY_t parity)
{
  self->huart.Instance = self->instance;
  self->huart.Init.BaudRate = (uint32_t)baud;
  self->huart.Init.WordLength = UART_WORDLENGTH_8B;
  self->huart.Init.StopBits = _uart_hal_stop_bits(stop_bits);
  self->huart.Init.Parity = _uart_hal_parity(parity);
  self->huart.Init.Mode = UART_MODE_TX_RX;
  self->huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  self->huart.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&self->huart) != HAL_OK) {
    return false;
  }

  _uart_enable_irq(&self->instance);

  return true;
}

static uint32_t _uart_hal_stop_bits(STOP_BITS_t stop_bits)
{
  return stop_bits == TWO_STOP_BIT ? UART_STOPBITS_2 : UART_STOPBITS_1;
}

static uint32_t _uart_hal_parity(PARITY_t parity)
{
  switch (parity) {
    case ODD:
      return UART_PARITY_ODD;
    case EVEN:
      return UART_PARITY_EVEN;
    case NONE:
    default:
      return UART_PARITY_NONE;
  }
}

static void _uart_enable_irq(USART_TypeDef *instance)
{
  if (instance == USART1) {
    HAL_NVIC_SetPriority(USART_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  } else if (instance == USART2) {
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  } else if (instance == USART3) {
    HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  }
}

static uart_t *_uart_from_handle(UART_HandleTypeDef *huart)
{
  for (uart_num uart_num = UART_1; uart_num < UART_COUNT; uart_num++) {
    if (uarts[uart_num].valid && &uarts[uart_num].huart == huart) {
      return &uarts[uart_num];
    }
  }

  return NULL;
}

/* ST Callbacks */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  uart_t *self = _uart_from_handle(huart);

  if (self != NULL && self->txCompleteCallback != NULL) {
    self->txCompleteCallback();
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  uart_t *self = _uart_from_handle(huart);

  if (self != NULL && self->rxCompleteCallback != NULL) {
    self->rxCompleteCallback();
  }
}
