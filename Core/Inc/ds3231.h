#ifndef __DS3231_H
#define __DS3231_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint8_t day_of_week;
  uint8_t day_of_month;
  uint8_t month;
  uint16_t year;
} DS3231_DateTime_t;

HAL_StatusTypeDef DS3231_Init(void);
HAL_StatusTypeDef DS3231_GetDateTime(DS3231_DateTime_t *date_time);
HAL_StatusTypeDef DS3231_SetDateTime(const DS3231_DateTime_t *date_time);
HAL_StatusTypeDef DS3231_GetTemperature(float *temperature_c);
HAL_StatusTypeDef DS3231_ReadStatus(uint8_t *status);
HAL_StatusTypeDef DS3231_ClearStatusFlags(uint8_t flags);
bool DS3231_IsEnabled(void);
void DS3231_SetPower(bool enable);

#ifdef __cplusplus
}
#endif

#endif /* __DS3231_H */
