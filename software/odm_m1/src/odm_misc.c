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

#include "odm_misc.h"

static uint32_t board_id = 0;

static uint32_t uuid_to_board_id [] = {
  0x00550035, 0x33345105, 0x30373031, 1,
  0x003f0029, 0x33345105, 0x30373031, 2,
};

void odm_misc_init()
{
  uint32_t u,v,w;

  u = *(uint32_t*)0x1FFF7A10;
  v = *(uint32_t*)0x1FFF7A14;
  w = *(uint32_t*)0x1FFF7A18;

  for (uint32_t i = 0; i < (sizeof(uuid_to_board_id) / sizeof(uint32_t)); i += 4)
  {
    if ( uuid_to_board_id[i + 0] == u
      && uuid_to_board_id[i + 1] == v
      && uuid_to_board_id[i + 2] == w)
    {
      board_id = uuid_to_board_id[i + 3];
    }
  }
}

uint32_t odm_board_id(void)
{
  return board_id;
}
