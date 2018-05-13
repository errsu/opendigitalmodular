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

#include "odm_interrupts.h"
#include "odm_timers.h"
#include "odm_gpio.h"
#include "stm32f4xx.h"

static timer_callback_t sample_rate_callback;

// see odm_timers_get_sample_rate_phase
static uint32_t sample_rate_phase_factor;

void odm_timers_configure(timer_callback_t callback)
{
  // this is compatible to timers 10, 11, 13, 14

  sample_rate_callback = callback;

  NVIC_SetPriority(TIM1_UP_TIM10_IRQn,
                   ODM_INTERRUPT_PRIO(ODM_INTERRUPT_PRIOGRP_AUDIO_TIMER,
                                      ODM_INTERRUPT_PRIOSUB_AUDIO_TIMER));
  NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

  // disable timer, enable update interrupt event, no clock division
  TIM10->CR1 = 0;

  // TIM10 runs at sysclk, even if APCB2 is half of that
  uint32_t divider = 180000000 / ODM_SAMPLE_RATE;

  // divider -= 1; // run faster - for test purposes only!!!

  // see odm_timers_get_sample_rate_phase
  sample_rate_phase_factor = (1 << 16) * 256 / divider;

  TIM10->ARR = divider - 1;     // auto-reload register
  TIM10->PSC = 0;               // prescaler f(CK_CNT) = fCK_PSC / (PSC + 1)
  TIM10->EGR = TIM_EGR_UG;      // Set bit 0 to 1 by software to generate update
  TIM10->DIER |= TIM_DIER_UIE;  // enable update interrupt
  // registers TIM10->CCMR1, TIM10->CCER are left at 0 - capture/compare is not used

  TIM10->CR1 |= TIM_CR1_CEN;    // enable timer
}

void TIM1_UP_TIM10_IRQHandler(void)
{
  if (TIM10->SR & TIM_SR_UIF) // check if update event interrupt is active
  {
    sample_rate_callback();
    TIM10->SR = ~(TIM_SR_UIF); // clear interrupt flag by writing 0
  }
}

uint8_t odm_timers_get_sample_phase()
{
  // the phase runs from 0...255 when the counter runs from 0...(divider - 1)
  // since division is expensive, we do multiplication with shifting
  return ((TIM10->CNT * sample_rate_phase_factor) >> 16);
}
