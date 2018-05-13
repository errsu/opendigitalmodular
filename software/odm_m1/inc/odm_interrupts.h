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

#ifndef __ODM_INTERRUPTS_H
#define __ODM_INTERRUPTS_H

#include "stm32f4xx.h"

// using 2 bits for prio groups -> prio level is 0bxx.yy where
// xx is the prio group (lower group preemts higher group value)
// yy is the sub prio (lower subprio does what?)
#define ODM_INTERRUPT_PRIO_GROUPING          5

// sorted by prio, highest first
// none of the values must exceed 3
#define ODM_INTERRUPT_PRIOGRP_LINK_DMA_RX  0
#define ODM_INTERRUPT_PRIOSUB_LINK_DMA_RX  0
#define ODM_INTERRUPT_PRIOGRP_LINK_USART   0
#define ODM_INTERRUPT_PRIOSUB_LINK_USART   1
#define ODM_INTERRUPT_PRIOGRP_SYSTICK      1
#define ODM_INTERRUPT_PRIOSUB_SYSTICK      0
#define ODM_INTERRUPT_PRIOGRP_AUDIO_TIMER  2
#define ODM_INTERRUPT_PRIOSUB_AUDIO_TIMER  0

// use this as parameter to NVIC_SetPriority (which takes 4 bits, and shifts them by 4)
#define ODM_INTERRUPT_PRIO(grp,sub) ((grp << 2) | sub)

#endif
