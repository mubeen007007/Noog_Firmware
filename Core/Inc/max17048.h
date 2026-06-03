#ifndef __MAX17048_H
#define __MAX17048_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
  float voltage_v;
  float soc_percent;
  float crate_percent_per_hour;
  uint16_t version;
  uint16_t status;
} MAX17048_Measurement_t;

HAL_StatusTypeDef MAX17048_Init(void);
HAL_StatusTypeDef MAX17048_ReadVoltage(float *voltage_v);
HAL_StatusTypeDef MAX17048_ReadSoc(float *soc_percent);
HAL_StatusTypeDef MAX17048_ReadCrate(float *crate_percent_per_hour);
HAL_StatusTypeDef MAX17048_ReadVersion(uint16_t *version);
HAL_StatusTypeDef MAX17048_ReadStatus(uint16_t *status);
HAL_StatusTypeDef MAX17048_ClearStatusBits(uint16_t bits);
HAL_StatusTypeDef MAX17048_QuickStart(void);
HAL_StatusTypeDef MAX17048_Reset(void);
HAL_StatusTypeDef MAX17048_ReadMeasurement(MAX17048_Measurement_t *measurement);
bool MAX17048_IsEnabled(void);
void MAX17048_SetPower(bool enable);

#ifdef __cplusplus
}
#endif

#endif /* __MAX17048_H */
