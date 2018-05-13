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

#include "odm_printf.h"
#include "odm_gpio.h"
#include "stm32f4xx.h"

void odm_printf_configure(void)
{
  odm_gpio_configure_serial(GPIOA, 2, 7); // TX
  odm_gpio_configure_serial(GPIOA, 3, 7); // RX

  USART2->CR1 &=  ~USART_CR1_UE; // disable UART

  // async mode, no DMA, 8N1, OVERSAMPLING16, no HW flow control
  USART2->CR1 = USART_CR1_TE | USART_CR1_RE;
  USART2->CR2 = 0;
  USART2->CR3 = 0;

  // USART2 uses PCLK1
  // bitrate = fck/DIV*16, fck = fcpu/4 == 45MHz
  // DIV = (BRR[15:0]/16) => BRR = 45Mhz/bitrate
  // BRR    DIV   bitrate@CPU=180MHz
  // 4688  293.0    9600 (9598.98)
  //  781   48.8   57600 (57618.4)
  USART2->BRR = 4688;

  USART2->CR1 |=  USART_CR1_UE; // enable UART
}

#ifdef __GNUC__
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif
{
  if (ch == '\n')
  {
    USART2->DR = (uint32_t)'\r';
    while((USART2->SR & USART_SR_TXE) == 0)
    {
    }
  }
  USART2->DR = ch & 0xFFU;
  while((USART2->SR & USART_SR_TXE) == 0)
  {
  }
  return ch;
}
