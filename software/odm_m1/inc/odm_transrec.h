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

#ifndef __ODM_TRANSREC_H
#define __ODM_TRANSREC_H

#include <stdint.h>

#define ODM_TRANSREC_STACKSIZE 16
#define ODM_TRANSREC_PARAMS    3
#define ODM_TRANSREC_ENTRIES   8

extern volatile uint32_t odm_transrec_stack[ODM_TRANSREC_STACKSIZE];
extern volatile uint32_t odm_transrec_sp;
extern volatile uint32_t odm_transrec_data[ODM_TRANSREC_ENTRIES][ODM_TRANSREC_PARAMS + 1];
extern volatile uint32_t odm_transrec_count;

static inline void odm_transrec_enter(uint32_t routine)
{
  odm_transrec_stack[odm_transrec_sp++] = routine;
}

static inline void odm_transrec_exit()
{
  --odm_transrec_sp;
}

static inline void odm_transrec_param(uint32_t param, uint32_t value)
{
  odm_transrec_data[odm_transrec_count][param] = value;
}

extern void odm_transrec_commit();

#endif
