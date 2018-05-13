// (C) Copyright 2015-2018 Errsu
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "odm_gpio.h"

#define ODM_SET_ONE_BIT(word, pos, value) \
  word = (word & ~((uint32_t)1 << pos)) | (value << pos);

#define ODM_SET_TWO_BITS(word, pos, value) \
  word = (word & ~((uint32_t)3 << (pos * 2))) | (value << (pos * 2));

#define ODM_SET_FOUR_BITS(words, pos, value) \
  words[pos >>3] = (words[pos >> 3] & ~(0xFU << ((pos & 0x7U) * 4))) \
                                    | (value << ((pos & 0x7U) * 4));

#define INPUT        0x0 // modes
#define OUTPUT       0x1
#define ALTERNATE    0x2
#define ANALOG       0x3
#define PUSH_PULL    0x0 // output types
#define LOW_SPEED    0x0 // speeds  4MHz/100ns @50pF (STM32F446CRE)
#define MEDIUM_SPEED 0x1 //        25MHz/10ns  @50pF (STM32F446CRE)
#define FAST_SPEED   0x2 //        50MHz/6ns   @40pF (STM32F446CRE)
#define HIGH_SPEED   0x3 //       100MHz/4ns   @30pF (STM32F446CRE)
#define NO_PULL      0x0 // pull directions
#define PULL_UP      0x1
#define PULL_DOWN    0x2

void odm_gpio_configure_testpin(GPIO_TypeDef *GPIOx, uint32_t pos)
{
  ODM_SET_TWO_BITS(GPIOx->MODER,   pos, OUTPUT);
  ODM_SET_TWO_BITS(GPIOx->OSPEEDR, pos, HIGH_SPEED);
  ODM_SET_ONE_BIT (GPIOx->OTYPER,  pos, PUSH_PULL);
  ODM_SET_TWO_BITS(GPIOx->PUPDR,   pos, PULL_DOWN);
}

void odm_gpio_configure_led(GPIO_TypeDef *GPIOx, uint32_t pos)
{
  ODM_SET_TWO_BITS(GPIOx->MODER,   pos, OUTPUT);
  ODM_SET_TWO_BITS(GPIOx->OSPEEDR, pos, LOW_SPEED);
  ODM_SET_ONE_BIT (GPIOx->OTYPER,  pos, PUSH_PULL);
  ODM_SET_TWO_BITS(GPIOx->PUPDR,   pos, NO_PULL);
}

void odm_gpio_configure_switch(GPIO_TypeDef *GPIOx, uint32_t pos)
{
  ODM_SET_TWO_BITS(GPIOx->MODER,   pos, INPUT);
  ODM_SET_TWO_BITS(GPIOx->PUPDR,   pos, NO_PULL);
}

void odm_gpio_configure_adc(GPIO_TypeDef *GPIOx, uint32_t pos)
{
  ODM_SET_TWO_BITS(GPIOx->MODER,   pos, ANALOG);
  ODM_SET_TWO_BITS(GPIOx->PUPDR,   pos, NO_PULL);
}

void odm_gpio_configure_dac(GPIO_TypeDef *GPIOx, uint32_t pos)
{
  ODM_SET_TWO_BITS(GPIOx->MODER,   pos, ANALOG);
  ODM_SET_TWO_BITS(GPIOx->PUPDR,   pos, NO_PULL);
}

void odm_gpio_configure_serial(GPIO_TypeDef *GPIOx, uint32_t pos, uint32_t alternate_function)
{
  ODM_SET_FOUR_BITS(GPIOx->AFR,    pos, alternate_function);
  ODM_SET_TWO_BITS(GPIOx->MODER,   pos, ALTERNATE);
  ODM_SET_TWO_BITS(GPIOx->OSPEEDR, pos, LOW_SPEED);
  ODM_SET_ONE_BIT (GPIOx->OTYPER,  pos, PUSH_PULL);
  ODM_SET_TWO_BITS(GPIOx->PUPDR,   pos, PULL_UP);
}

void odm_gpio_configure_link(GPIO_TypeDef *GPIOx, uint32_t pos, uint32_t alternate_function)
{
  ODM_SET_FOUR_BITS(GPIOx->AFR,    pos, alternate_function);
  ODM_SET_TWO_BITS(GPIOx->MODER,   pos, ALTERNATE);
  ODM_SET_TWO_BITS(GPIOx->OSPEEDR, pos, MEDIUM_SPEED);
  ODM_SET_ONE_BIT (GPIOx->OTYPER,  pos, PUSH_PULL);
  ODM_SET_TWO_BITS(GPIOx->PUPDR,   pos, NO_PULL);
}

void odm_gpio_configure(void)
{
  odm_gpio_configure_testpin(GPIOB, 8);

  odm_gpio_configure_led(GPIOD, 2); // D1
  odm_gpio_configure_led(GPIOB, 6); // D2
  odm_gpio_configure_led(GPIOB, 9); // D3
  odm_gpio_configure_led(GPIOC, 9); // D4

  odm_gpio_configure_switch(GPIOC, 11); // T1
  odm_gpio_configure_switch(GPIOA,  8); // T2
  odm_gpio_configure_switch(GPIOA,  7); // T3
  odm_gpio_configure_switch(GPIOA,  6); // T4
}
