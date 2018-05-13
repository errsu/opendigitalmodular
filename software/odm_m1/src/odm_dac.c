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

void odm_dac_configure(void)
{
  odm_gpio_configure_dac(GPIOA, 4); // DAC1
  odm_gpio_configure_dac(GPIOA, 5); // DAC2

  // DAC output buffers are enabled by default
  DAC->CR = (DAC_CR_EN1 | DAC_CR_EN2); // enable 2 channels
}
