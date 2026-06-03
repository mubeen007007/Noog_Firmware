#ifndef __SHT40_H
#define __SHT40_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

typedef struct
{
  float temperature_c;
  float humidity_rh;
} SHT40_Measurement_t;

HAL_StatusTypeDef SHT40_Init(void);
HAL_StatusTypeDef SHT40_SoftReset(void);
HAL_StatusTypeDef SHT40_ReadMeasurement(SHT40_Measurement_t *measurement);
bool SHT40_IsEnabled(void);
void SHT40_SetPower(bool enable);

#ifdef __cplusplus
}
#endif

#endif /* __SHT40_H */
