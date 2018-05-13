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

#include "odm_dac.h"
#include "odm_gpio.h"
#include "stm32f4xx.h"

static uint32_t adc_channels[] = {0, 1, 10, 11};

void odm_adc_configure(void)
{
  odm_gpio_configure_adc(GPIOA, 0); // P1
  odm_gpio_configure_adc(GPIOA, 1); // P2
  odm_gpio_configure_adc(GPIOC, 0); // P3
  odm_gpio_configure_adc(GPIOC, 1); // P4

  ADC->CCR |= (ADC_CCR_ADCPRE_0 | ADC_CCR_ADCPRE_1); // divide fPCLK2 by 8 -> 11.25MHz
  ADC1->CR2 |= ADC_CR2_ADON;

  // set channels 0, 1, 10 and 11 to 480 samples averaging -> still > 500Hz total sample rate
  ADC1->SMPR1 = (ADC_SMPR1_SMP10_0 * 7) | (ADC_SMPR1_SMP11_0 * 7);
  ADC1->SMPR2 = (ADC_SMPR2_SMP0_0 * 7) | (ADC_SMPR2_SMP1_0 * 7);
}

uint32_t odm_adc_start(uint32_t number)
{
  if (number >= sizeof(adc_channels) / sizeof(adc_channels[0]))
  {
    return 0;
  }

  ADC1->SQR3 = (adc_channels[number] & ADC_SQR3_SQ1);
  ADC1->SQR2 = 0;
  ADC1->SQR1 = 0; // L[3:0]=0 -> one conversion
  ADC1->CR2 |= ADC_CR2_SWSTART;
  return 1;
}

uint32_t odm_adc_check()
{
  if (ADC1->SR & ADC_SR_EOC)
  {
    return 1;
  }
  return 0;
}

uint32_t odm_adc_get_result()
{
  return ADC1->DR; // 12 bit right-aligned
}
