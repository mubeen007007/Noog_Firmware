#include "noog_power.h"

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
} NOOG_PowerRailConfig_t;

static const NOOG_PowerRailConfig_t noog_power_rail_map[] = {
  [NOOG_POWER_SHT] = {STM_pin_EN_SHT_GPIO_Port, STM_pin_EN_SHT_Pin},
  [NOOG_POWER_W25] = {STM_pin_EN_W25_GPIO_Port, STM_pin_EN_W25_Pin},
  [NOOG_POWER_MEM] = {STM_pin_EN_MEM_GPIO_Port, STM_pin_EN_MEM_Pin},
  [NOOG_POWER_SD] = {STM_pin_EN_SD_GPIO_Port, STM_pin_EN_SD_Pin},
  [NOOG_POWER_GPS] = {STM_pin_EN_GPS_GPIO_Port, STM_pin_EN_GPS_Pin},
  [NOOG_POWER_FUEL_GAUGE] = {STM_pin_EN_FG_GPIO_Port, STM_pin_EN_FG_Pin},
  [NOOG_POWER_RTC] = {STM_pin_EN_RTC_GPIO_Port, STM_pin_EN_RTC_Pin},
  [NOOG_POWER_LORA] = {STM_pin_EN_Lora_GPIO_Port, STM_pin_EN_Lora_Pin},
  [NOOG_POWER_LED] = {STM_pin_EN_LED_GPIO_Port, STM_pin_EN_LED_Pin},
  [NOOG_POWER_SENSORS] = {STM_pin_EN_SENS_GPIO_Port, STM_pin_EN_SENS_Pin}
};

static bool NOOG_Power_IsValidRail(NOOG_PowerRail_t rail);

void NOOG_Power_Init(void)
{
  NOOG_Power_DisableAll();
}

HAL_StatusTypeDef NOOG_Power_Set(NOOG_PowerRail_t rail, bool enable)
{
  if (!NOOG_Power_IsValidRail(rail))
  {
    return HAL_ERROR;
  }

  /* Keep EN lines actively driven low when off to avoid accidental rail enable. */
  HAL_GPIO_WritePin(noog_power_rail_map[rail].port,
                    noog_power_rail_map[rail].pin,
                    enable ? GPIO_PIN_SET : GPIO_PIN_RESET);

  return HAL_OK;
}

bool NOOG_Power_IsEnabled(NOOG_PowerRail_t rail)
{
  if (!NOOG_Power_IsValidRail(rail))
  {
    return false;
  }

  return HAL_GPIO_ReadPin(noog_power_rail_map[rail].port,
                          noog_power_rail_map[rail].pin) == GPIO_PIN_SET;
}

void NOOG_Power_DisableAll(void)
{
  uint32_t rail;

  for (rail = 0; rail < (sizeof(noog_power_rail_map) / sizeof(noog_power_rail_map[0])); rail++)
  {
    (void)NOOG_Power_Set((NOOG_PowerRail_t)rail, false);
  }
}

static bool NOOG_Power_IsValidRail(NOOG_PowerRail_t rail)
{
  return (rail >= NOOG_POWER_SHT) &&
         ((uint32_t)rail < (sizeof(noog_power_rail_map) / sizeof(noog_power_rail_map[0])));
}
