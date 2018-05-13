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

#include "odm_adc.h"
#include "odm_clock.h"
#include "odm_dac.h"
#include "odm_gen.h"
#include "odm_gpio.h"
#include "odm_interrupts.h"
#include "odm_link.h"
#include "odm_misc.h"
#include "odm_printf.h"
#include "odm_timers.h"

#include "stm32f4xx.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static float sin_gen_al_x;
static float sin_gen_al_delta;
static float sin_gen_ar_x;
static float sin_gen_ar_delta;
static float sin_gen_bl_x;
static float sin_gen_bl_delta;
static float sin_gen_br_x;
static float sin_gen_br_delta;

static float sin_gen_f1_x;
static float sin_gen_f1_delta;
static float sin_gen_f2_x;
static float sin_gen_f2_delta;

static float imp_gen_x;
static float imp_gen_delta;

static float envelope_gen_x;

static float al_in = 0.0;
static float ar_in = 0.0;
static float bl_in = 0.0;
static float br_in = 0.0;

static float al_out = 0.0;
static float ar_out = 0.0;
static float bl_out = 0.0;
static float br_out = 0.0;

static float freq_a = 50.0f;
static float dir_a  = 1.005f;
static float freq_b = 1000.0f;


static void update_freq_a()
{
  freq_a *= dir_a;

  if (dir_a > 1.0f && freq_a > 20000.0f)
  {
    dir_a = 0.996f;
  }
  if (dir_a < 1.0f && freq_a < 50.0f)
  {
    dir_a = 1.005f;
  }
  sin_gen_al_delta = odm_gen_calc_delta(freq_a);
  sin_gen_ar_delta = odm_gen_calc_delta(freq_a);
}

static void update_freq_b()
{
  freq_b += 1000.0f;
  if (freq_b > 20000.0f)
  {
    freq_b = 1000.0f;
  }
  sin_gen_bl_delta = odm_gen_calc_delta(freq_b);
  sin_gen_br_delta = odm_gen_calc_delta(freq_b);
}

void init_board1()
{
  sin_gen_al_x = 0.0f;
  sin_gen_ar_x = 0.0f;
  update_freq_a();
  sin_gen_bl_x = 0.0f;
  sin_gen_br_x = 0.0f;
  update_freq_b();

  imp_gen_x = 0.0f;
  imp_gen_delta = odm_gen_calc_delta(9600.0f);

  sin_gen_f1_x = 0.0f;
  sin_gen_f1_delta = odm_gen_calc_delta(300.0f);

  envelope_gen_x = 0.0f;

  odm_link_configure(&ODM_LINK_A);
  odm_link_configure(&ODM_LINK_B);
}

static uint8_t tx_sample_count = 0;
static uint32_t count_rect = 0;

void timer_callback_96000kHz_board1_signals()
{
  odm_link_send_frame(&ODM_LINK_A, tx_sample_count, al_out, ar_out);
  odm_link_send_frame(&ODM_LINK_B, tx_sample_count, bl_out, br_out);
  tx_sample_count++;

  odm_link_receive_audio(&ODM_LINK_A, &al_in, &ar_in);
  odm_link_receive_audio(&ODM_LINK_B, &bl_in, &br_in);

  al_out = odm_gen_sin_next(&sin_gen_al_x, sin_gen_al_delta);
  ar_out = odm_gen_sin_next(&sin_gen_ar_x, sin_gen_ar_delta);
  // bl_out = odm_gen_sin_next(&sin_gen_bl_x, sin_gen_bl_delta);
  // br_out = odm_gen_sin_next(&sin_gen_br_x, sin_gen_br_delta);
  // bl_out = br_out = odm_gen_impulse_next(&imp_gen_x, imp_gen_delta);
  // bl_out = br_out = odm_gen_rect_next(&imp_gen_x, imp_gen_delta);

  bl_out = br_out = (count_rect < 3) ? 1.0f : -1.0f;

  count_rect++;
  if (count_rect == 6)
  {
    count_rect = 0;
  }
}

void timer_callback_96000kHz_board1_hops()
{
  odm_link_send_frame(&ODM_LINK_A, tx_sample_count, al_out, ar_out);
  odm_link_send_frame(&ODM_LINK_B, tx_sample_count, bl_out, br_out);
  tx_sample_count++;

  odm_link_receive_audio(&ODM_LINK_A, &al_in, &ar_in);
  odm_link_receive_audio(&ODM_LINK_B, &bl_in, &br_in);

  ar_out = ar_in;
  al_out = al_in;
  br_out = br_in;
  bl_out = bl_in;
}

void timer_callback_96000kHz_board1_mix()
{
  odm_link_send_frame(&ODM_LINK_A, tx_sample_count, al_out, ar_out);
  tx_sample_count++;

  odm_link_receive_audio(&ODM_LINK_A, &al_in, &ar_in);

  float f1 = odm_gen_sin_next(&sin_gen_f1_x, sin_gen_f1_delta);
  float env = odm_gen_envelope(&envelope_gen_x);

  ar_out = (ar_in + f1 * env) * 0.9f;
  al_out = (al_in + f1) * 0.5f;
}

void timer_callback_96000kHz_board1_rs485()
{
  ODM_GPIO_PB8_TOGGLE();
  odm_link_send_frame(&ODM_LINK_A, 0xa5, -5.674839e12f, 0.0f);
}

void init_board2()
{
  sin_gen_f1_x = 0.0f;
  sin_gen_f1_delta = odm_gen_calc_delta(300.0f);

  sin_gen_f2_x = 0.0f;
  sin_gen_f2_delta = odm_gen_calc_delta(301.0f);

  odm_link_configure(&ODM_LINK_A);
  odm_link_configure(&ODM_LINK_B);
}

static uint8_t data_buffer[8];
static uint8_t last_data  = 0;

static int32_t stall_count = 0;
static int32_t jump_count = 0;

static int32_t message1 = 0;

void timer_callback_96000kHz_board2_signals()
{
  odm_link_send_frame(&ODM_LINK_A, 0, al_out, ar_out);
  odm_link_send_frame(&ODM_LINK_B, 0, bl_out, br_out);

  int32_t steps = odm_link_receive_audio(&ODM_LINK_A, &al_in, &ar_in);
  odm_link_receive_audio(&ODM_LINK_B, &bl_in, &br_in);

  if (steps == -1)
  {
    stall_count++;
  }
  if (steps == 2 || steps == 0)
  {
    jump_count++;
  }

  int32_t data_count = odm_link_receive_data(&ODM_LINK_A, data_buffer);
  for (int32_t i = 0; i < data_count; i++)
  {
    uint8_t data = data_buffer[i];
    if ((data & 0xFF) != ((last_data + 1) & 0xFF))
    {
      message1++;
    }
    last_data = data;
  }

  ODM_DAC_WRITE((uint32_t)((int)32768 + (int)(al_in * 16000.0f)),
                (uint32_t)((int)32768 + (int)(ar_in * 16000.0f)));
}

void timer_callback_96000kHz_board2_hops()
{
  odm_link_send_frame(&ODM_LINK_A, 0, al_out, ar_out);
  odm_link_send_frame(&ODM_LINK_B, 0, bl_out, br_out);

  odm_link_receive_audio(&ODM_LINK_A, &al_in, &ar_in);
  odm_link_receive_audio(&ODM_LINK_B, &bl_in, &br_in);

  float f1 = odm_gen_sin_next(&sin_gen_f1_x, sin_gen_f1_delta);

//  float f1 = (count_rect < 160) ? 1.0f : -1.0f;
//  count_rect++;
//  if (count_rect == 320)
//  {
//    count_rect = 0;
//  }

  // 7 hops
  ODM_DAC_WRITE((uint32_t)((int)32768 + (int)(bl_in * 16000.0f)),
                (uint32_t)((int)32768 + (int)(f1    * 16000.0f)));

  // 1 hop
  // ODM_DAC_WRITE((uint32_t)((int)32768 + (int)(ar_in * 16000.0f)),
  //               (uint32_t)((int)32768 + (int)(ref_sig * 16000.0f)));

  ar_out = f1;
  al_out = ar_in;
  br_out = al_in;
  bl_out = br_in;
}

#define DELAY_LEN 1024

static float delay_buffer[DELAY_LEN] = {0.0f, };
static uint32_t delay_head = 0;

void timer_callback_96000kHz_board2_mix()
{
  odm_link_send_frame(&ODM_LINK_A, 0, al_out, ar_out);

  odm_link_receive_audio(&ODM_LINK_A, &al_in, &ar_in);

  // right: 0.5*0.5 feedback output
  // left: sum of two sin gens from different CPUs
  float dac_r_out = ar_in * 0.5f;
  float dac_l_out = al_in;
  ODM_DAC_WRITE((uint32_t)((int)32768 + (int)(dac_r_out * 16000.0f)),
                (uint32_t)((int)32768 + (int)(dac_l_out * 16000.0f)));

  float f2 = odm_gen_sin_next(&sin_gen_f2_x, sin_gen_f2_delta);

  ar_out = delay_buffer[delay_head];
  delay_buffer[delay_head] = ar_in * 0.96f;
  delay_head = (delay_head + 1) % DELAY_LEN;

  al_out = f2;
}

void timer_callback_96000kHz_board2_rs485()
{
  uint8_t data[2];
  float left, right;

  int32_t step = odm_link_receive_audio(&ODM_LINK_A, &left, &right);
  odm_link_receive_data(&ODM_LINK_A, data);

  if (step < 0)
  {
    ODM_GPIO_PA0_OFF();
  }
  else
  {
    ODM_GPIO_PA0_ON();
  }

  if (data[0] == 0xa5 && left == -5.674839e12f && right == 0.0f)
  {
    ODM_GPIO_PC1_ON();
  }
  else
  {
    ODM_GPIO_PC1_OFF();
  }

  ODM_DAC_WRITE((uint32_t)((int)32768 + (int)(right * 16000.0f)),
                (uint32_t)((int)32768 + (int)(left  * 16000.0f)));
}

static void cpu_init()
{
  // enable instruction & data cache, prefetch buffer
  FLASH->ACR |= (FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_PRFTEN);
  NVIC_SetPriorityGrouping(ODM_INTERRUPT_PRIO_GROUPING);

  // enable FPU
  SCB->CPACR |= 0x00F00000; // addr = 0xE000ED88
  __DSB(); // data sync barrier
  __ISB(); // instruction sync barrier
}

int main(void)
{
  cpu_init();
  odm_clock_configure();
  odm_misc_init();

  odm_gpio_configure();
  odm_printf_configure();
  // odm_dac_configure();
  odm_adc_configure();

  printf("STM32F446 board %d\n", (int)odm_board_id());

  if (odm_board_id() == 0)
  {
    init_board1();
    odm_timers_configure(&timer_callback_96000kHz_board1_hops);
  }
  else if (odm_board_id() == 1)
  {
    init_board1();
    // odm_timers_configure(&timer_callback_96000kHz_board1_signals);
    // odm_timers_configure(&timer_callback_96000kHz_board1_hops);
    // odm_timers_configure(&timer_callback_96000kHz_board1_mix);
    // odm_timers_configure(&timer_callback_96000kHz_board1_rs485);
  }
  else if (odm_board_id() == 2)
  {
    init_board2();
    // odm_timers_configure(&timer_callback_96000kHz_board2_signals);
    // odm_timers_configure(&timer_callback_96000kHz_board2_hops);
    // odm_timers_configure(&timer_callback_96000kHz_board2_mix);
    // odm_timers_configure(&timer_callback_96000kHz_board2_rs485);
  }

  int count = 0;
//  int32_t last_stall_count = 0;
//  int32_t last_jump_count = 0;

  uint32_t last_adc_value[4] = {0, };
  uint32_t adc_num = 0;
  uint32_t adc_running = 0;

  for(;;)
  {
    update_freq_a();

    if (ODM_GPIO_KEY0_ISSET())
    {
      ODM_GPIO_LED0_ON();
    }
    else
    {
      ODM_GPIO_LED0_OFF() ;
    }

    if (ODM_GPIO_KEY1_ISSET())
    {
      ODM_GPIO_LED1_ON();
    }
    else
    {
      ODM_GPIO_LED1_OFF() ;
    }

    if (ODM_GPIO_KEY2_ISSET())
    {
      ODM_GPIO_LED2_ON();
    }
    else
    {
      ODM_GPIO_LED2_OFF() ;
    }

    if (ODM_GPIO_KEY3_ISSET())
    {
      ODM_GPIO_LED3_ON();
    }
    else
    {
      ODM_GPIO_LED3_OFF() ;
    }

//    if (count % 2048 == 0)
//    {
//      ODM_GPIO_LED3_OFF();
//      ODM_GPIO_LED0_ON();
//    }
//    else if (count % 2048 == 512)
//    {
//      ODM_GPIO_LED0_OFF();
//      ODM_GPIO_LED1_ON();
//    }
//    else if (count % 2048 == 1024)
//    {
//      ODM_GPIO_LED1_OFF();
//      ODM_GPIO_LED2_ON();
//    }
//    else if (count % 2048 == 1536)
//    {
//      ODM_GPIO_LED2_OFF();
//      ODM_GPIO_LED3_ON();
//    }
//
    if (count % 16 == 0)
    {
      if (!adc_running)
      {
        odm_adc_start(adc_num);
        adc_running = 1;
      }
      else
      {
        if (odm_adc_check())
        {
          uint32_t new_adc_value = odm_adc_get_result() / 16;
          if (new_adc_value != last_adc_value[adc_num])
          {
            last_adc_value[adc_num] = new_adc_value;
            printf("adc %03d %03d %03d %03d\n",
              (int)last_adc_value[0],
              (int)last_adc_value[1],
              (int)last_adc_value[2],
              (int)last_adc_value[3]);
          }
          adc_running = 0;
          adc_num = (adc_num + 1) % 4;
        }
      }
    }
    if (count++ == 4096)
    {
      update_freq_b();
      count = 0;
    }
//    if (last_stall_count != stall_count || last_jump_count != jump_count)
//    {
//      printf("s %d j %d\n", (int)stall_count, (int)jump_count);
//      last_stall_count = stall_count;
//      last_jump_count = jump_count;
//    }
//    if (message1 != 0)
//    {
//      printf("d %d\n", (int)message1);
//      message1 = 0;
//    }
    odm_clock_sleep(1);
  }
  exit(0);
}
