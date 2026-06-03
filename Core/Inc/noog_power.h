#ifndef __NOOG_POWER_H
#define __NOOG_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

typedef enum
{
  NOOG_POWER_SHT = 0,
  NOOG_POWER_W25,
  NOOG_POWER_MEM,
  NOOG_POWER_SD,
  NOOG_POWER_GPS,
  NOOG_POWER_FUEL_GAUGE,
  NOOG_POWER_RTC,
  NOOG_POWER_LORA,
  NOOG_POWER_LED,
  NOOG_POWER_SENSORS
} NOOG_PowerRail_t;

void NOOG_Power_Init(void);
HAL_StatusTypeDef NOOG_Power_Set(NOOG_PowerRail_t rail, bool enable);
bool NOOG_Power_IsEnabled(NOOG_PowerRail_t rail);
void NOOG_Power_DisableAll(void);

#ifdef __cplusplus
}
#endif

#endif /* __NOOG_POWER_H */
