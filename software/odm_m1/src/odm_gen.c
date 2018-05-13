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

#include "odm_gen.h"
#include "odm_gpio.h"

#define SR 96000.0f

float odm_gen_calc_delta(float freq)
{
  if (freq < 0.1f)
  {
    freq = 0.1f;
  }
  else if (freq > 24000.0f)
  {
    freq = 24000.0f;
  }
  return (2.0f/SR) * freq;
}
