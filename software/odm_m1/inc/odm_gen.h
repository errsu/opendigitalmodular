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

#ifndef __ODM_GEN_H
#define __ODM_GEN_H

// x:     phase (-1.0 ... 1.0, corresponds to -Pi ... Pi)
// delta: increment of x per sample (0.00000208 ... 0.41666)
// freq:  frequency in Hz (0.1Hz ... 20kHz)
//
// CPU load is 117ns@180MHz per sample

extern float odm_gen_calc_delta(float freq);

static inline float odm_gen_sin_next(float* x, float delta)
{
  *x += delta;
  if (*x > 1.0f)
  {
    *x -= 2.0f;
  }

  float xSqrd = *x * *x;

  float result = 0.433645f * xSqrd - 2.428288f;
  result = (result * xSqrd) + 5.133625f;
  result = (result * xSqrd) - 3.138982f;
  result *= *x;
  return result;
}

static inline float odm_gen_rect_next(float* x, float delta)
{
  *x += delta;
  if (*x > 1.0f)
  {
    *x -= 2.0f;
  }

  if (*x >= 0.0f)
  {
    return 1.0f;
  }
  else
  {
    return -1.0f;
  }
}

static inline float odm_gen_impulse_next(float* x, float delta)
{
  *x += delta;
  if (*x > 1.0f)
  {
    *x -= 2.0f;
    return 1.0f;
  }

  return 0.0f;
}

static inline float odm_gen_envelope(float* x)
{
  *x += 1.0f;
  if (*x >= 24000.f)
  {
    *x = 0.0f;
  }

  if (*x < 2400.0f)
  {
    return 1.0f;
  }
  else
  {
    return 0.0f;
  }
}

#endif
