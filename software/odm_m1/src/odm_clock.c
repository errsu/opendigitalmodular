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

#include "odm_clock.h"
#include "odm_interrupts.h"

#include "stm32f4xx.h"

#define SET_PERIPH_BYTE(addr, byte, value) \
  *((__IO uint8_t *)addr + byte) = (uint8_t)value

#define SET_PERIPH_BIT(addr, bit, value) \
  *(__IO uint32_t *)(PERIPH_BB_BASE + ((uint32_t)(addr) - PERIPH_BASE) * 32 + (bit) * 4) = (value)

#define SET_BITS_AND_DELAY(reg, bitmask) do {                   \
                                           __IO uint32_t dummy; \
                                           (reg) |= (bitmask);  \
                                           dummy = (reg);       \
                                           (void)dummy;         \
                                         } while(0)

void odm_clock_configure(void)
{
  // we are entering this with HSI as system clock, HSE is off
  // leaving power regulator voltage scale at default value of 1

  SET_BITS_AND_DELAY(RCC->APB1ENR, RCC_APB1ENR_PWREN);

  SET_PERIPH_BYTE(&RCC->CR, 2, 0x05); // HSE bypass
  while((RCC->CR & RCC_CR_HSERDY) == 0); // wait until bypassed HSE is ready

  SET_PERIPH_BIT(&RCC->CR, POSITION_VAL(RCC_CR_PLLON), 0); // disable PLL
  while(RCC->CR & RCC_CR_PLLRDY);   // wait until PLL is off

  RCC->PLLCFGR =
      RCC_PLLCFGR_PLLSRC_HSE                 // F_PLL_IN       (  8 MHz) = F_HSE bypassed from ST-LINK MCU
    |   4 << POSITION_VAL(RCC_PLLCFGR_PLLM)  // F_VCO_IN       (  2 MHz) = F_PLL_IN / PLLM
    | 180 << POSITION_VAL(RCC_PLLCFGR_PLLN)  // F_VCO_OUT      (360 MHZ) = F_VCO_IN × PLLN
    |   0 << POSITION_VAL(RCC_PLLCFGR_PLLP)  // F_SYSCLK       (180 MHz) = F_VCO_OUT / DIV, DIV = 2 * (PLLP + 1) = 2
    |   2 << POSITION_VAL(RCC_PLLCFGR_PLLQ)  // F_USB_OTG_FS   (unused)  = F_VCO_OUT / PLLQ
    |   2 << POSITION_VAL(RCC_PLLCFGR_PLLR); // F_SAI1_I2S     (unused)  = F_VCO_OUT / PLLR

  SET_PERIPH_BIT(&RCC->CR, POSITION_VAL(RCC_CR_PLLON), 1); // enable PLL
  while((RCC->CR & RCC_CR_PLLRDY) == 0); // wait until PLL is on

  SET_PERIPH_BIT(&PWR->CR, POSITION_VAL(PWR_CR_ODEN), 1); // enable overdrive (to allow for 180MHz)
  while((PWR->CSR & PWR_CSR_ODRDY) == 0); // wait for overdrive generator ready

  SET_PERIPH_BIT(&PWR->CR, POSITION_VAL(PWR_CR_ODSWEN), 1); // enable overdrive switch
  while((PWR->CSR & PWR_CSR_ODSWRDY) == 0); // wait for overdrive switch ready

  // default latency is ZERO, CPU frequency was increased => go to higher latency
  SET_PERIPH_BYTE(&FLASH->ACR, 0, FLASH_ACR_LATENCY_5WS); // assume it just works

  // system clock switch: select PLLCLK (from PLLP) as system clock
  MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

  MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_CFGR_PPRE1_DIV4); // APB1 prescaler -> PCLK1 = 45MHz
  MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, RCC_CFGR_PPRE2_DIV2); // APB2 prescaler -> PCLK2 = 90MHz

  SET_BITS_AND_DELAY(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN
                                 | RCC_AHB1ENR_GPIOBEN
                                 | RCC_AHB1ENR_GPIOCEN
                                 | RCC_AHB1ENR_GPIODEN
                                 | RCC_AHB1ENR_DMA2EN);

  SET_BITS_AND_DELAY(RCC->APB2ENR, RCC_APB2ENR_USART1EN
                                 | RCC_APB2ENR_USART6EN
                                 | RCC_APB2ENR_TIM10EN
                                 | RCC_APB2ENR_ADC1EN);

  SET_BITS_AND_DELAY(RCC->APB1ENR, RCC_APB1ENR_USART2EN
                                 | RCC_APB1ENR_DACEN);

  SysTick_Config(180000000/1000);

  NVIC_SetPriority(SysTick_IRQn,
                   ODM_INTERRUPT_PRIO(ODM_INTERRUPT_PRIOGRP_SYSTICK,   // was PRIOGRP_LINK_DMA_RX
                                      ODM_INTERRUPT_PRIOSUB_SYSTICK)); // was PRIOSUB_LINK_DMA_RX
}

static __IO uint32_t time_in_msec;

uint32_t odm_clock_time_in_msec(void)
{
  return time_in_msec;
}

void odm_clock_sleep(uint32_t duration_in_msec)
{
  uint32_t tickstart = time_in_msec;
  while((time_in_msec - tickstart) < duration_in_msec)
  {
  }
}

void odm_clock_systick_disable(void)
{
  SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

void odm_clock_systick_enable(void)
{
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}

void SysTick_Handler(void) // referred to by startup_stm32f446xx.s
{
  time_in_msec++;
}
