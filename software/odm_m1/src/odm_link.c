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

#include "odm_gpio.h"
#include "odm_interrupts.h"
#include "odm_link.h"
#include "odm_transrec.h"
#include "odm_timers.h"
#include "stm32f4xx.h"

#include <math.h>

#define ODM_LINK_FRAME_LENGTH 9
#define RX_DEBOUNCING 96*500 // 50 ms

//--------------------------------------------------------------------------------------
// TX: PA9 aka. D8, CN10.pin21   RX: PA10 aka. D2, CN10.pin33

odm_link_trx_t ODM_LINK_A =
{
  .port             = 0,
  .gpio_io          = GPIOA,
  .gpio_rx_pin      = 10,
  .gpio_rx_af       = 7,
  .gpio_tx_pin      = 9,
  .gpio_tx_af       = 7,

  .usart_io         = USART1,
  .usart_irqn       = USART1_IRQn,

  .dma_tx_stream_io = DMA2_Stream7,
  .dma_tx_channel   = 4,
  .dma_tx_ifcr      = &(DMA2->HIFCR),
  .dma_tx_ifcm      = DMA_HISR_TCIF7,

  .dma_rx_stream_io = DMA2_Stream2,
  .dma_rx_channel   = 4,
  .dma_rx_isr       = &(DMA2->LISR),
  .dma_rx_ifcr      = &(DMA2->LIFCR),
  .dma_rx_ifm       = DMA_LISR_FEIF2
                    | DMA_LISR_DMEIF2
                    | DMA_LISR_TEIF2
                    | DMA_LISR_HTIF2
                    | DMA_LISR_TCIF2,
  .dma_rx_irqn      = DMA2_Stream2_IRQn,
};

//--------------------------------------------------------------------------------------
// TX: PC6, CN10.pin4    RX: PC7 aka. D9, CN10.pin19

odm_link_trx_t ODM_LINK_B =
{
  .port             = 1,
  .gpio_io          = GPIOC,
  .gpio_rx_pin      = 7,
  .gpio_rx_af       = 8,
  .gpio_tx_pin      = 6,
  .gpio_tx_af       = 8,

  .usart_io         = USART6,
  .usart_irqn       = USART6_IRQn,

  .dma_tx_stream_io = DMA2_Stream6,
  .dma_tx_channel   = 5,
  .dma_tx_ifcr      = &(DMA2->HIFCR),
  .dma_tx_ifcm      = DMA_HISR_TCIF6,

  .dma_rx_stream_io = DMA2_Stream1,
  .dma_rx_channel   = 5,
  .dma_rx_isr       = &(DMA2->LISR),
  .dma_rx_ifcr      = &(DMA2->LIFCR),

  .dma_rx_ifm       = DMA_LISR_FEIF1
                    | DMA_LISR_DMEIF1
                    | DMA_LISR_TEIF1
                    | DMA_LISR_HTIF1
                    | DMA_LISR_TCIF1,
  .dma_rx_irqn      = DMA2_Stream1_IRQn,
};

//--------------------------------------------------------------------------------------

static void stop_rx_dma(odm_link_trx_t* htrx)
{
  // disable DMA stream
  htrx->dma_rx_stream_io->CR &= ~DMA_SxCR_EN;
  while (htrx->dma_rx_stream_io->CR & DMA_SxCR_EN)
  {
  }

  *htrx->dma_rx_ifcr = htrx->dma_rx_ifm;

  htrx->rx_receiving = 0;
  htrx->rx_data_read_head = 0;
  htrx->rx_audio_read_head = 0;

  htrx->dma_rx_stream_io->CR &= (~DMA_SxCR_CT);
  htrx->dma_rx_stream_io->NDTR = ODM_LINK_FRAME_LENGTH;
  htrx->dma_rx_stream_io->M0AR = (uint32_t)(&htrx->rx_buffer[0]);
  htrx->dma_rx_stream_io->M1AR = (uint32_t)(&htrx->rx_buffer[1]);

  // underflow values (in "previous" buffer)
  htrx->rx_buffer[3].left  = 0.0;
  htrx->rx_buffer[3].right = 0.0;
  htrx->rx_buffer[3].data  = 0;
}

static uint32_t read_usart_sr_drop_dr(odm_link_trx_t* htrx)
{
  __IO uint32_t sr, dummy_dr;
  sr       = htrx->usart_io->SR;
  dummy_dr = htrx->usart_io->DR;
  (void)dummy_dr;
  (void)sr;
  return sr;
}

static void start_rx_dma(odm_link_trx_t* htrx)
{
  htrx->dma_rx_stream_io->CR |= DMA_SxCR_EN; // enable DMA
}

#if 0
static void print_usart_interrupt_state(odm_link_trx_t* htrx, uint32_t sr)
{
  printf("%d) S: %d IE:", (int)htrx->port, (int)htrx->rx_state);

  uint32_t cr1 = htrx->usart_io->CR1;
  uint32_t cr3 = htrx->usart_io->CR3;

  if (cr1 & USART_CR1_UE)     { printf(" UE");   }
  if (cr1 & USART_CR1_RXNEIE) { printf(" RXNE"); }
  if (cr1 & USART_CR1_IDLEIE) { printf(" IDLE"); }
  if (cr1 & USART_CR1_RWU)    { printf(" RWU");  }
  if (cr3 & USART_CR3_DMAR)   { printf(" DMAR"); }
  if (cr3 & USART_CR3_EIE)    { printf(" EIE");  }

  printf(" -- SET:");

  if (sr & USART_SR_CTS)  { printf(" CTS");  }
  if (sr & USART_SR_LBD)  { printf(" LBD");  }
  if (sr & USART_SR_TXE)  { printf(" TXE");  }
  if (sr & USART_SR_TC)   { printf(" TC");   }
  if (sr & USART_SR_RXNE) { printf(" RXNE"); }
  if (sr & USART_SR_IDLE) { printf(" IDLE"); }
  if (sr & USART_SR_ORE)  { printf(" ORE");  }
  if (sr & USART_SR_NE)   { printf(" NE");   }
  if (sr & USART_SR_FE)   { printf(" FE");   }
  if (sr & USART_SR_PE)   { printf(" PE");   }

  printf("\n");
}
#endif

#if 0

static void print_dec(const char * text, uint32_t value)
{
  printf("%s   %08d\n", text, (int)value);
}

static void print_hex(const char * text, uint32_t value)
{
  printf("%s 0x%08x\n", text, (unsigned int)value);
}

static void print_state_a(const char* text)
{
  printf("%s\n\n", text);

  print_dec("PORT", ODM_LINK_A.port);
  print_dec("STAT", ODM_LINK_A.rx_state);
  print_dec("DATA", ODM_LINK_A.rc_data_read_head);
  print_dec("AUDI", ODM_LINK_A.rc_audio_read_head);
  print_dec("RCNG", ODM_LINK_A.rx_receiving);

  print_hex("CR  ", ODM_LINK_A.dma_rx_stream_io->CR);
  print_dec("NTDR", ODM_LINK_A.dma_rx_stream_io->NDTR);
  print_hex("M0AR", ODM_LINK_A.dma_rx_stream_io->M0AR);
  print_hex("M1AR", ODM_LINK_A.dma_rx_stream_io->M1AR);
  print_hex("PAR ", ODM_LINK_A.dma_rx_stream_io->PAR);
  print_hex("FCR ", ODM_LINK_A.dma_rx_stream_io->FCR);
  print_hex("ISR",  *ODM_LINK_A.dma_rx_isr);

  print_hex("SR  ", ODM_LINK_A.usart_io->SR);
  print_dec("DR  ", ODM_LINK_A.usart_io->DR);
  print_hex("CR1 ", ODM_LINK_A.usart_io->CR1);
  print_hex("CR2 ", ODM_LINK_A.usart_io->CR2);
  print_hex("CR3 ", ODM_LINK_A.usart_io->CR3);
  print_hex("BRR ", ODM_LINK_A.usart_io->BRR);

  for(int i= 0; i < RXBUFSIZE; i++)
  {
    printf("BUF [%d] %f %f %d\n", i, (double)ODM_LINK_A.rx_buffer[i].left,
                                     (double)ODM_LINK_A.rx_buffer[i].right,
                                     (int)ODM_LINK_A.rx_buffer[i].data);
  }

  printf("\n");
}

#endif

//--------------------------------------------------------------------------------------

void odm_link_configure(odm_link_trx_t* htrx)
{
  odm_gpio_configure_link(htrx->gpio_io, htrx->gpio_rx_pin, htrx->gpio_rx_af);
  odm_gpio_configure_link(htrx->gpio_io, htrx->gpio_tx_pin, htrx->gpio_tx_af);

  htrx->dma_tx_stream_io->CR &= ~DMA_SxCR_EN; // disable DMA TX stream, will be enabled to send
  while (htrx->dma_tx_stream_io->CR & DMA_SxCR_EN);

  // all other bits are zero
  htrx->dma_tx_stream_io->CR = htrx->dma_tx_channel << POSITION_VAL(DMA_SxCR_CHSEL)
                             | 1 << POSITION_VAL(DMA_SxCR_DIR)  // memory to peripheral
                             | 1 << POSITION_VAL(DMA_SxCR_MINC) //  DMA_MINC_ENABLE
                             | 2 << POSITION_VAL(DMA_SxCR_PL);  // priority level high

  htrx->dma_tx_stream_io->FCR = 0; // direct mode, interrupt off

  htrx->dma_tx_stream_io->NDTR = ODM_LINK_FRAME_LENGTH;         // data length
  htrx->dma_tx_stream_io->PAR  = (uint32_t)&htrx->usart_io->DR; // DMA destination address
  htrx->dma_tx_stream_io->M0AR = (uint32_t)&htrx->tx_buffer;    // DMA source address

  stop_rx_dma(htrx);

  htrx->dma_rx_stream_io->PAR = (uint32_t)&htrx->usart_io->DR; // DMA source address

  htrx->dma_rx_stream_io->CR = (htrx->dma_rx_channel << POSITION_VAL(DMA_SxCR_CHSEL))
                             | DMA_SxCR_DBM     // double buffering
                             | DMA_SxCR_PL_1    // prio high
                             | DMA_SxCR_MINC    // mem inc after each transfer
                             | DMA_SxCR_CIRC    // circular mode, excludes PFCTRL
                             | DMA_SxCR_TCIE    // transfer complete interrupt only
                             // all other flags remain at their defaut (zero settings):
                             // MBURST and PBURST: single transfer
                             // Initial target: memory 0
                             // No PINC or PINCOS
                             // Memory and peripheral data size: 1 byte
                             // No peripheral address increment
                             // Peripheral-to-memory
                             // Peripheral flow controller PFCTRL: DMA
                             ;

  htrx->dma_rx_stream_io->FCR = 0; // no FIFO, using direct mode

  NVIC_SetPriority(htrx->dma_rx_irqn,
                   ODM_INTERRUPT_PRIO(ODM_INTERRUPT_PRIOGRP_LINK_DMA_RX,
                                      ODM_INTERRUPT_PRIOSUB_LINK_DMA_RX));
  NVIC_EnableIRQ(htrx->dma_rx_irqn);

  // async mode, DMA-based TX, 8N1, 11250000 baud @ 180MHz, OVERSAMPLING8, no HW flow control
  // enable UART but disable receive function
  htrx->usart_io->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_OVER8;
  htrx->usart_io->CR2 = 0;
  htrx->usart_io->CR3 = USART_CR3_DMAT;

  // USART1/6 use PCLK2
  // bitrate = fck/DIV*8, fck = fcpu/2 == 90MHz
  // DIV = (BRR[15:4] + BRR[2:0]/8), BRR[3] must be kept clear
  // BRR        DIV   bitrate@CPU=180MHz
  // 0x0010     1.0   11250000
  // 0x4650  1125.0   10000
  htrx->usart_io->BRR = 0x10; // 0x4650;

  // clear all interrupts
  read_usart_sr_drop_dr(htrx);
  htrx->usart_io->SR = 0;

  NVIC_SetPriority(htrx->usart_irqn,
                   ODM_INTERRUPT_PRIO(ODM_INTERRUPT_PRIOGRP_LINK_USART,
                                      ODM_INTERRUPT_PRIOSUB_LINK_USART));
  NVIC_EnableIRQ(htrx->usart_irqn);

  htrx->rx_idle_count = RX_DEBOUNCING;
  htrx->usart_io->CR1 |= (USART_CR1_RE | USART_CR1_IDLEIE);
}

//----------------------------------------------------------------------------------------

#define ODM_RX_IRQ_HANDLER_BODY(odm_link_x, ifcr, tcflag, stream)         \
  ifcr = tcflag; /* clear transfer complete flag */                         \
  odm_link_x.rx_sample_phase = odm_timers_get_sample_phase();             \
  odm_link_x.rx_receiving++;                                              \
  uint32_t buffer_index = (odm_link_x.rx_receiving + 1) % RXBUFSIZE;      \
  uint32_t buffer_addr = (uint32_t)(&odm_link_x.rx_buffer[buffer_index]); \
  ((__IO uint32_t*)&stream->M0AR)[buffer_index % 2] = buffer_addr

void DMA2_Stream2_IRQHandler(void)
{
  // ODM_GPIO_PB8_ON();
  // ___B_A_R_R_I_E_R___

  ODM_RX_IRQ_HANDLER_BODY(ODM_LINK_A, DMA2->LIFCR, DMA_LISR_TCIF2, DMA2_Stream2);

  // ___B_A_R_R_I_E_R___
  // ODM_GPIO_PB8_OFF();
}

void DMA2_Stream1_IRQHandler(void)
{
  ODM_RX_IRQ_HANDLER_BODY(ODM_LINK_B, DMA2->LIFCR, DMA_LISR_TCIF1, DMA2_Stream1);
}

//----------------------------------------------------------------------------------------

int32_t odm_link_receive_data(odm_link_trx_t* htrx, uint8_t* data)
{
  uint32_t rcv_head = htrx->rx_receiving;
  uint32_t read_head = htrx->rx_data_read_head;

  int32_t data_count = 0;

  // Assuming that this function is called with sample rate,
  // no more than 2 bytes of data can be pending, therefore
  // "data[]" must have room for at least 2 bytes only

  if (read_head + 2 == rcv_head)
  {
    data[data_count++] = htrx->rx_buffer[read_head++ % RXBUFSIZE].data;
  }
  if (read_head + 1 == rcv_head)
  {
    data[data_count++] = htrx->rx_buffer[read_head++ % RXBUFSIZE].data;
  }
  if (read_head != rcv_head)
  {
    read_head = rcv_head; // too much data pending - skip it
  }

  htrx->rx_data_read_head = read_head;

  return data_count;
}

//----------------------------------------------------------------------------------------

static float hermite(float x, float* y)
{
  // 4-point, 3rd-order Hermite (x-form)
  float c0 = y[1];
  float c1 = 0.5f * (y[2] - y[0]);
  float c3 = 1.5f * (y[1] - y[2]) + 0.5f * (y[3] - y[0]);
  float c2 = y[0] - y[1] + c1 - c3;
  return ((c3 * x + c2) * x + c1) * x + c0;
}

//----------------------------------------------------------------------------------------

static void interpolate_audio(
  odm_link_msg_t* rx_buffer,
  uint32_t read_head,
  uint32_t phase,
  float* left,
  float* right)
{
  float rx_left[4];
  float rx_right[4];

  for (int32_t i = 3; i >= 0; i--)
  {
    odm_link_msg_t* buffer = rx_buffer + (read_head % RXBUFSIZE);
    rx_left[i] = buffer->left;
    rx_right[i] = buffer->right;
    read_head -= 1;
  }

  float x = (float)(256 - phase) / 256.f;
  *left = hermite(x, rx_left);
  *right = hermite(x, rx_right);
}

//----------------------------------------------------------------------------------------

int32_t odm_link_receive_audio(odm_link_trx_t* htrx, float* left, float* right)
{
  // assert(__get_PRIMASK() == 0); // don't call from interrupt-disabled routines
  // __disable_irq();
  int32_t delta = (int32_t)(htrx->rx_receiving - htrx->rx_audio_read_head);
  uint8_t rcvd_phase = htrx->rx_sample_phase;
  // __enable_irq();

  uint32_t audio_read_head;
  int32_t step = 1;

  if (delta <= 1)
  {
    audio_read_head = htrx->rx_audio_read_head - 1; // underflow: use previous buffer
    step = -1;
  }
  else
  {
    htrx->rx_audio_read_head++; // default: one step per receiver SR timer routine

    int32_t phase_diff = (int32_t)(int8_t)(rcvd_phase - htrx->rx_running_phase);
    if (phase_diff > 16)
    {
      htrx->rx_running_phase += 16;
      if (htrx->rx_running_phase == 0)
      {
        htrx->rx_audio_read_head--; // wrapped backward: no steps this time
        step = 0;
      }
    }
    else if (phase_diff < -16)
    {
      if (htrx->rx_running_phase == 0)
      {
        htrx->rx_audio_read_head++; // wrapped forward: two steps instead of one
        step = 2;
      }
      htrx->rx_running_phase -= 16;
    }
    audio_read_head = htrx->rx_audio_read_head;
  }

  interpolate_audio(htrx->rx_buffer, audio_read_head, htrx->rx_running_phase, left, right);

  return step;
}

//----------------------------------------------------------------------------------------

static void odm_link_handle_usart_interrupt(odm_link_trx_t* htrx)
{
  // reading SR and DR resets all relevant interrupt flags (RXNE,FE,NE,ORE,IDLE)
  uint32_t sr = read_usart_sr_drop_dr(htrx);

  // since in each situation only a single interrupt is enabled,
  // we don't need to check which interrupt is enabled, only if
  // the one we are waiting for occurred
  if (htrx->rx_idle_count > 0)
  {
    if (sr & USART_SR_IDLE)
    {
      htrx->rx_idle_count -= 1;
      if (htrx->rx_idle_count == 0)
      {
        htrx->usart_io->CR1 &= ~USART_CR1_IDLEIE;
        htrx->usart_io->CR3 |= (USART_CR3_DMAR | USART_CR3_EIE);
        start_rx_dma(htrx);
        ODM_GPIO_PA0_ON();
      }
    }
  }
  else
  {
    if (sr & (USART_SR_FE | USART_SR_NE | USART_SR_ORE))
    {
      ODM_GPIO_PA0_OFF();
      stop_rx_dma(htrx);
      htrx->usart_io->CR3 &= (~USART_CR3_EIE & ~USART_CR3_DMAR);
      htrx->usart_io->CR1 |= USART_CR1_IDLEIE;
      htrx->rx_idle_count = RX_DEBOUNCING;
    }
  }
}

void USART1_IRQHandler(void)
{
  odm_link_handle_usart_interrupt(&ODM_LINK_A);
}

void USART6_IRQHandler(void)
{
  odm_link_handle_usart_interrupt(&ODM_LINK_B);
}
