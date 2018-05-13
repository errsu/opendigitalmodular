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

#ifndef __ODM_LINK_H
#define __ODM_LINK_H

#include "odm_gpio.h"
#include "stm32f4xx.h"
#include <stdint.h>

#define RXBUFSIZE 8

typedef struct
{
  float   left;
  float   right;
  uint8_t data;
} odm_link_msg_t;

typedef struct
{
  uint32_t            port;
  GPIO_TypeDef*       gpio_io;
  uint32_t            gpio_rx_pin;
  uint32_t            gpio_rx_af;
  uint32_t            gpio_tx_pin;
  uint32_t            gpio_tx_af;

  USART_TypeDef*      usart_io;
  IRQn_Type           usart_irqn;

  DMA_Stream_TypeDef* dma_tx_stream_io;
  uint32_t            dma_tx_channel;
  __IO uint32_t*      dma_tx_ifcr;      // interrupt flag clear register
  uint32_t            dma_tx_ifcm;      // interrupt flag bitmask
  odm_link_msg_t      tx_buffer;

  DMA_Stream_TypeDef* dma_rx_stream_io;
  uint32_t            dma_rx_channel;
  __IO uint32_t*      dma_rx_isr;       // interrupt status register
  __IO uint32_t*      dma_rx_ifcr;      // interrupt flag clear register
  uint32_t            dma_rx_ifm;       // interrupt flag bitmask
  IRQn_Type           dma_rx_irqn;
  uint8_t             rx_sample_phase;  // see odm_timers_get_sample_phase
  uint8_t             rx_running_phase;
  uint32_t            rx_idle_count;
  uint32_t            rx_data_read_head;
  uint32_t            rx_audio_read_head;
  uint32_t            rx_receiving; // buffer index currently being filled
  odm_link_msg_t      rx_buffer[RXBUFSIZE];

} odm_link_trx_t;

extern odm_link_trx_t ODM_LINK_A;
extern odm_link_trx_t ODM_LINK_B;

extern void    odm_link_configure(odm_link_trx_t* htrx);
extern int32_t odm_link_receive_audio(odm_link_trx_t* htrx, float* left, float* right);
extern int32_t odm_link_receive_data(odm_link_trx_t* htrx, uint8_t* data);

inline void odm_link_send_frame(odm_link_trx_t* htrx, uint8_t data, float left, float right)
{
  // caller must ensure that the previous request is finished
  htrx->tx_buffer.left  = left;
  htrx->tx_buffer.right = right;
  htrx->tx_buffer.data  = data;

  *htrx->dma_tx_ifcr = htrx->dma_tx_ifcm; // clear transfer complete flag
  htrx->dma_tx_stream_io->CR |= DMA_SxCR_EN; // trigger DMA
}

#endif
