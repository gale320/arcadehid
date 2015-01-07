#include "system_config.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_hw_config.h"

#include "usb_arcade.h"
#include "usb_pwr.h"

ErrorStatus HSEStartUpStatus;
/* Extern variables ----------------------------------------------------------*/
volatile uint8_t kb_tx_complete = 1;
volatile uint8_t mouse_tx_complete = 1;

uint8_t kb_led_state = 0;

static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len);

void USB_Cable_Config(FunctionalState NewState) {
#ifdef CONFIG_HY_TEST_BOARD
  if (NewState != DISABLE) {
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
  } else {
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
  }
#endif
}

void USB_ARC_KB_tx(usb_kb_report *report)
{
  // byte 0:   modifiers
  // byte 1:   reserved (0x00)
  // byte 2-x: keypresses
  report->reserved = 0;

  uint32_t spoon_guard = 1000000;
  while(kb_tx_complete==0 && --spoon_guard);
  ASSERT(spoon_guard > 0);

  /* Reset the control token to inform upper layer that a transfer is ongoing */
  kb_tx_complete = 0;

  /* Copy keyboard vector info in ENDP1 Tx Packet Memory Area*/
  USB_SIL_Write(EP1_IN, report->raw, sizeof(report->raw));

  /* Enable endpoint for transmission */
  SetEPTxValid(ENDP1);

}

void USB_ARC_MOUSE_tx(usb_mouse_report *report)
{
  uint32_t spoon_guard = 1000000;
  while(mouse_tx_complete==0 && --spoon_guard);
  ASSERT(spoon_guard > 0);

  /* Reset the control token to inform upper layer that a transfer is ongoing */
  mouse_tx_complete = 0;

  /* Copy mouse position info in ENDP2 Tx Packet Memory Area*/
  USB_SIL_Write(EP2_IN, report->raw, sizeof(report->raw));

  /* Enable endpoint for transmission */
  SetEPTxValid(ENDP2);

}

void Get_SerialNum(void)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

  Device_Serial0 = *(uint32_t*)ID1;
  Device_Serial1 = *(uint32_t*)ID2;
  Device_Serial2 = *(uint32_t*)ID3;

  Device_Serial0 += Device_Serial2;

  if (Device_Serial0 != 0)
  {
    IntToUnicode (Device_Serial0, &ARC_string_serial[2] , 8);
    IntToUnicode (Device_Serial1, &ARC_string_serial[18], 4);
  }
}

static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len) {
  uint8_t idx = 0;

  for (idx = 0; idx < len; idx++) {
    if (((value >> 28)) < 0xA) {
      pbuf[2 * idx] = (value >> 28) + '0';
    } else {
      pbuf[2 * idx] = (value >> 28) + 'A' - 10;
    }

    value = value << 4;

    pbuf[2 * idx + 1] = 0;
  }
}

void USB_ARC_init(void) {
  USB_Init();
}

