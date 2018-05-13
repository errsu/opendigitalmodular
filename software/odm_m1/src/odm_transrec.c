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

#include "odm_transrec.h"
#include <stdio.h>

volatile uint32_t odm_transrec_stack[ODM_TRANSREC_STACKSIZE] = {0, };
volatile uint32_t odm_transrec_sp = 0;
volatile uint32_t odm_transrec_data[ODM_TRANSREC_ENTRIES][ODM_TRANSREC_PARAMS+1] = {{0, 0, 0, 0}, };
volatile uint32_t odm_transrec_count = 0;

void odm_transrec_commit()
{
  odm_transrec_data[odm_transrec_count++][ODM_TRANSREC_PARAMS] = odm_transrec_stack[odm_transrec_sp - 1];

  if (odm_transrec_count == ODM_TRANSREC_ENTRIES)
  {
    printf("----------------------------------------------------\n");
    for (uint32_t i = 0; i < ODM_TRANSREC_ENTRIES; i++)
    {
      printf("%02ld: ", (unsigned long)odm_transrec_data[i][ODM_TRANSREC_PARAMS]);
      odm_transrec_data[i][ODM_TRANSREC_PARAMS] = 0;
      for (uint32_t p = 0; p < ODM_TRANSREC_PARAMS; p++)
      {
        printf("0x%08lx ", (unsigned long)odm_transrec_data[i][p]);
        odm_transrec_data[i][p] = 0;
      }
      printf("\n");
    }
    odm_transrec_count = 0;
  }
}
