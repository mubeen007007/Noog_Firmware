#ifndef __NOOG_RGB_LED_H
#define __NOOG_RGB_LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
  NOOG_LED_1 = 0,
  NOOG_LED_2,
  NOOG_LED_3
} NOOG_LedId_t;

typedef struct
{
  bool red;
  bool green;
  bool blue;
} NOOG_LedColor_t;

void NOOG_Leds_Init(void);
void NOOG_Leds_SetPower(bool enable);
void NOOG_Led_SetColor(NOOG_LedId_t led, NOOG_LedColor_t color);
void NOOG_Led_SetRgb(NOOG_LedId_t led, bool red, bool green, bool blue);
void NOOG_Led_Off(NOOG_LedId_t led);
void NOOG_Leds_AllOff(void);

#ifdef __cplusplus
}
#endif

#endif /* __NOOG_RGB_LED_H */
