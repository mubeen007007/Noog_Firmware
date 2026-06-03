#ifndef __MAX_M10S_H
#define __MAX_M10S_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_M10S_UBX_CLASS_NAV      0x01U
#define MAX_M10S_UBX_ID_NAV_PVT     0x07U
#define MAX_M10S_UBX_CLASS_MON      0x0AU
#define MAX_M10S_UBX_ID_MON_VER     0x04U

typedef struct
{
  uint32_t i_tow_ms;
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t valid;
  uint8_t fix_type;
  uint8_t flags;
  uint8_t flags2;
  uint8_t num_sv;
  int32_t longitude_deg_e7;
  int32_t latitude_deg_e7;
  int32_t height_msl_mm;
  uint32_t horizontal_accuracy_mm;
  uint32_t vertical_accuracy_mm;
  int32_t ground_speed_mm_s;
  uint32_t speed_accuracy_mm_s;
  uint16_t p_dop;
  bool valid_fix;
} MAXM10S_Pvt_t;

HAL_StatusTypeDef MAXM10S_Init(void);
HAL_StatusTypeDef MAXM10S_ResetHardware(void);
HAL_StatusTypeDef MAXM10S_StartReception(void);
HAL_StatusTypeDef MAXM10S_PollNavPvt(void);
HAL_StatusTypeDef MAXM10S_PollVersion(void);
HAL_StatusTypeDef MAXM10S_SendUbx(uint8_t msg_class, uint8_t msg_id, const uint8_t *payload, uint16_t payload_length);
bool MAXM10S_GetLastPvt(MAXM10S_Pvt_t *pvt);
bool MAXM10S_HasFix(void);
const char *MAXM10S_GetVersionString(void);
bool MAXM10S_IsEnabled(void);
void MAXM10S_SetPower(bool enable);

#ifdef __cplusplus
}
#endif

#endif /* __MAX_M10S_H */
