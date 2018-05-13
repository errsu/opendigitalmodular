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

#ifndef __ODM_DAC_H
#define __ODM_DAC_H

#include "stm32f4xx.h"
#include <stdint.h>

void odm_dac_configure();

// Writing both channels simultaneously, this saves APB1 bus capacity
// and also ensures that both channels are updated in sync.
// Latency is 1 APB1 clock cycle (22ns) + typically 3 (max 6) us with <50pF >5k loads.
// so the max freq for large updates is given by TSettling: avg: 330kHz, min: 165kHz
// for small changes (1ULP): 1MHz -> 14bits at 48kHz (192kHz=4x oversampling) is the maximum possible.
// ch1 and ch2 are in range 0...65535, which corresponds to 0.001...4.8V
// bits [3:0] and [31:16] of each channel are ignored
#define ODM_DAC_WRITE(ch1,ch2)  DAC->DHR12LD = ((ch1) & 0xFFF0U) | ((ch2) << 16)

#endif
