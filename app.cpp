/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/

#include "em_gpio.h"
#include "em_i2c.h"
#include "sl_i2cspm_instances.h"
#include "ustimer.h"
#include "u8g2.h"

#define SSD1306_I2C_ADDR 0x3c

uint8_t u8x8_efr32mg12_gpio_and_delay(U8X8_UNUSED u8x8_t *u8x8,
U8X8_UNUSED uint8_t msg,
                                      U8X8_UNUSED uint8_t arg_int,
                                      U8X8_UNUSED void *arg_ptr)
{
  switch (msg)
    {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
      break;
    case U8X8_MSG_DELAY_MILLI:
      USTIMER_DelayIntSafe(arg_int * 1000);
      break;
    case U8X8_MSG_DELAY_10MICRO:
      USTIMER_DelayIntSafe(10);
      break;
    default:
      return 0;
    }

  return 1;
}

static uint8_t out_buffer[32] =
  { 0 };
static uint8_t out_buffer_idx = 0;
static I2C_TransferSeq_TypeDef transfer;

uint8_t u8x8_byte_efr32mg12_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int,
                                   void *arg_ptr)
{
  switch (msg)
    {
    case U8X8_MSG_BYTE_SEND:
      {
        uint8_t *data = (uint8_t*) arg_ptr;
        while (arg_int > 0)
        {
          out_buffer[out_buffer_idx++] = *data;
          data++;
          arg_int--;
        }
      }
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
      out_buffer_idx = 0;
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
      {
        transfer.addr = (SSD1306_I2C_ADDR << 1);
        transfer.flags = I2C_FLAG_WRITE;
        transfer.buf[0].data = out_buffer;
        transfer.buf[0].len = out_buffer_idx;
        I2CSPM_Transfer(sl_i2cspm_oled, &transfer);
      }
      break;
    default:
      return 0;
    }

  return 1;
}

static u8g2_t u8g2;

void app_init(void)
{
  GPIO_PinModeSet(gpioPortF, 9, gpioModePushPull, 1);

  USTIMER_Init();
  USTIMER_DelayIntSafe(100000);

  u8g2_Setup_ssd1306_i2c_128x32_univision_1(&u8g2, U8G2_R2,
                                            u8x8_byte_efr32mg12_hw_i2c,
                                            u8x8_efr32mg12_gpio_and_delay);
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  u8g2_FirstPage(&u8g2);
  do
  {
    u8g2_SetFont(&u8g2, u8g2_font_9x15_tr);
    u8g2_DrawStr(&u8g2, 0, 20, "@alwin_wint3r");
  }
  while (u8g2_NextPage(&u8g2));
}
