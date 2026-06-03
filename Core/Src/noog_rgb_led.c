#include "noog_rgb_led.h"
#include "noog_debug.h"
#include "noog_power.h"

static void NOOG_Led_WriteChannel(GPIO_TypeDef *port, uint16_t pin, bool enabled);

void NOOG_Leds_SetPower(bool enable)
{
  (void)NOOG_Power_Set(NOOG_POWER_LED, enable);
  NOOG_LOG("LED", "LED rail %s", enable ? "enabled" : "disabled");
}

void NOOG_Leds_AllOff(void)
{
  NOOG_Led_Off(NOOG_LED_1);
  NOOG_Led_Off(NOOG_LED_2);
  NOOG_Led_Off(NOOG_LED_3);
}

void NOOG_Leds_Init(void)
{
  NOOG_Leds_SetPower(true);
  NOOG_Leds_AllOff();
  NOOG_LOG("LED", "RGB LEDs initialized");
}

void NOOG_Led_Off(NOOG_LedId_t led)
{
  NOOG_Led_SetRgb(led, false, false, false);
}

void NOOG_Led_SetColor(NOOG_LedId_t led, NOOG_LedColor_t color)
{
  NOOG_Led_SetRgb(led, color.red, color.green, color.blue);
}

void NOOG_Led_SetRgb(NOOG_LedId_t led, bool red, bool green, bool blue)
{
  switch (led)
  {
    case NOOG_LED_1:
      NOOG_Led_WriteChannel(LED1_R_GPIO_Port, LED1_R_Pin, red);
      NOOG_Led_WriteChannel(LED1_G_GPIO_Port, LED1_G_Pin, green);
      NOOG_Led_WriteChannel(LED1_B_GPIO_Port, LED1_B_Pin, blue);
      break;

    case NOOG_LED_2:
      NOOG_Led_WriteChannel(LED2_R_GPIO_Port, LED2_R_Pin, red);
      NOOG_Led_WriteChannel(LED2_G_GPIO_Port, LED2_G_Pin, green);
      NOOG_Led_WriteChannel(LED2_B_GPIO_Port, LED2_B_Pin, blue);
      break;

    case NOOG_LED_3:
      NOOG_Led_WriteChannel(LED3_R_GPIO_Port, LED3_R_Pin, red);
      NOOG_Led_WriteChannel(LED3_G_GPIO_Port, LED3_G_Pin, green);
      NOOG_Led_WriteChannel(LED3_B_GPIO_Port, LED3_B_Pin, blue);
      break;

    default:
      NOOG_LOG("LED", "Invalid LED id %d", (int)led);
      break;
  }
}

static void NOOG_Led_WriteChannel(GPIO_TypeDef *port, uint16_t pin, bool enabled)
{
  HAL_GPIO_WritePin(port, pin, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
