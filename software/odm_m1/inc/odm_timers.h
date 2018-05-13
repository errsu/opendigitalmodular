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

#ifndef __ODM_TIMERS_H
#define __ODM_TIMERS_H

#define ODM_SAMPLE_RATE 96000

typedef void(*timer_callback_t)();

void odm_timers_configure(timer_callback_t sample_rate_callback);

uint8_t odm_timers_get_sample_phase();

#endif
