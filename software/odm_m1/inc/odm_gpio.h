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

#ifndef __ODM_GPIO_H
#define __ODM_GPIO_H

#include "stm32f4xx.h"
#include <stdint.h>

#define ___B_A_R_R_I_E_R___  __asm__ __volatile__("":::"memory");

void odm_gpio_configure();
void odm_gpio_configure_adc   (GPIO_TypeDef *GPIOx, uint32_t pos);
void odm_gpio_configure_dac   (GPIO_TypeDef *GPIOx, uint32_t pos);
void odm_gpio_configure_serial(GPIO_TypeDef *GPIOx, uint32_t pos, uint32_t alternate_function);
void odm_gpio_configure_link  (GPIO_TypeDef *GPIOx, uint32_t pos, uint32_t alternate_function);

#define PIN(x) (1U << (x))

#define ODM_GPIO_PA0_ON()     GPIOA->BSRR = PIN(0)
#define ODM_GPIO_PA0_TOGGLE() GPIOA->ODR  = GPIOA->ODR ^ PIN(0)
#define ODM_GPIO_PA0_OFF()    GPIOA->BSRR = PIN(0) << 16

#define ODM_GPIO_PC1_ON()     GPIOC->BSRR = PIN(1)
#define ODM_GPIO_PC1_TOGGLE() GPIOC->ODR  = GPIOC->ODR ^ PIN(1)
#define ODM_GPIO_PC1_OFF()    GPIOC->BSRR = PIN(1) << 16

#define ODM_GPIO_PB8_ON()     GPIOB->BSRR = PIN(8)
#define ODM_GPIO_PB8_TOGGLE() GPIOB->ODR  = GPIOB->ODR ^ PIN(8)
#define ODM_GPIO_PB8_OFF()    GPIOB->BSRR = PIN(8) << 16

#define ODM_GPIO_LED0_ON()    GPIOD->BSRR = PIN(2)
#define ODM_GPIO_LED0_OFF()   GPIOD->BSRR = PIN(2) << 16

#define ODM_GPIO_LED1_ON()    GPIOB->BSRR = PIN(6)
#define ODM_GPIO_LED1_OFF()   GPIOB->BSRR = PIN(6) << 16

#define ODM_GPIO_LED2_ON()    GPIOB->BSRR = PIN(9)
#define ODM_GPIO_LED2_OFF()   GPIOB->BSRR = PIN(9) << 16

#define ODM_GPIO_LED3_ON()    GPIOC->BSRR = PIN(9)
#define ODM_GPIO_LED3_OFF()   GPIOC->BSRR = PIN(9) << 16

#define ODM_GPIO_KEY0_ISSET() ((GPIOC->IDR & PIN(11)) != 0)
#define ODM_GPIO_KEY1_ISSET() ((GPIOA->IDR & PIN( 8)) != 0)
#define ODM_GPIO_KEY2_ISSET() ((GPIOA->IDR & PIN( 7)) != 0)
#define ODM_GPIO_KEY3_ISSET() ((GPIOA->IDR & PIN( 6)) != 0)

#endif
