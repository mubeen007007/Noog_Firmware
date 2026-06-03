#include "max17048.h"
#include "noog_debug.h"
#include "noog_power.h"

#define MAX17048_I2C_ADDRESS             (0x36U << 1)
#define MAX17048_REG_VCELL               0x02U
#define MAX17048_REG_SOC                 0x04U
#define MAX17048_REG_MODE                0x06U
#define MAX17048_REG_VERSION             0x08U
#define MAX17048_REG_CONFIG              0x0CU
#define MAX17048_REG_CRATE               0x16U
#define MAX17048_REG_STATUS              0x1AU
#define MAX17048_REG_COMMAND             0xFEU

#define MAX17048_MODE_QUICKSTART         0x4000U
#define MAX17048_COMMAND_POR             0x5400U
#define MAX17048_POWERUP_DELAY_MS        20U
#define MAX17048_RESET_DELAY_MS          10U
#define MAX17048_I2C_TIMEOUT_MS          100U

extern I2C_HandleTypeDef hi2c2;

static HAL_StatusTypeDef MAX17048_ReadRegister(uint8_t reg, uint16_t *value);
static HAL_StatusTypeDef MAX17048_WriteRegister(uint8_t reg, uint16_t value);
static float MAX17048_ConvertSignedRegister(uint16_t raw, float lsb_scale);

void MAX17048_SetPower(bool enable)
{
  (void)NOOG_Power_Set(NOOG_POWER_FUEL_GAUGE, enable);
  NOOG_LOG("MAX17048", "Fuel gauge rail %s", enable ? "enabled" : "disabled");
}

bool MAX17048_IsEnabled(void)
{
  return NOOG_Power_IsEnabled(NOOG_POWER_FUEL_GAUGE);
}

HAL_StatusTypeDef MAX17048_Init(void)
{
  uint16_t version;

  MAX17048_SetPower(true);
  HAL_Delay(MAX17048_POWERUP_DELAY_MS);

  if (MAX17048_ReadVersion(&version) != HAL_OK)
  {
    NOOG_LOG("MAX17048", "Initialization failed while reading version");
    return HAL_ERROR;
  }

  NOOG_LOG("MAX17048", "Initialized on I2C2 address 0x%02X, version=0x%04X", MAX17048_I2C_ADDRESS >> 1, version);
  return HAL_OK;
}

HAL_StatusTypeDef MAX17048_ReadVoltage(float *voltage_v)
{
  uint16_t raw;

  if ((voltage_v == NULL) || !MAX17048_IsEnabled())
  {
    NOOG_LOG("MAX17048", "ReadVoltage failed: invalid output or gauge power off");
    return HAL_ERROR;
  }

  if (MAX17048_ReadRegister(MAX17048_REG_VCELL, &raw) != HAL_OK)
  {
    NOOG_LOG("MAX17048", "Failed to read VCELL");
    return HAL_ERROR;
  }

  *voltage_v = (float)raw * 78.125e-6f;
  NOOG_LOG("MAX17048", "Voltage %.3f V", *voltage_v);
  return HAL_OK;
}

HAL_StatusTypeDef MAX17048_ReadSoc(float *soc_percent)
{
  uint16_t raw;

  if ((soc_percent == NULL) || !MAX17048_IsEnabled())
  {
    NOOG_LOG("MAX17048", "ReadSoc failed: invalid output or gauge power off");
    return HAL_ERROR;
  }

  if (MAX17048_ReadRegister(MAX17048_REG_SOC, &raw) != HAL_OK)
  {
    NOOG_LOG("MAX17048", "Failed to read SOC");
    return HAL_ERROR;
  }

  *soc_percent = (float)raw / 256.0f;
  NOOG_LOG("MAX17048", "SOC %.2f %%", *soc_percent);
  return HAL_OK;
}

HAL_StatusTypeDef MAX17048_ReadCrate(float *crate_percent_per_hour)
{
  uint16_t raw;

  if ((crate_percent_per_hour == NULL) || !MAX17048_IsEnabled())
  {
    NOOG_LOG("MAX17048", "ReadCrate failed: invalid output or gauge power off");
    return HAL_ERROR;
  }

  if (MAX17048_ReadRegister(MAX17048_REG_CRATE, &raw) != HAL_OK)
  {
    NOOG_LOG("MAX17048", "Failed to read CRATE");
    return HAL_ERROR;
  }

  *crate_percent_per_hour = MAX17048_ConvertSignedRegister(raw, 0.208f);
  NOOG_LOG("MAX17048", "Charge rate %.2f %%/hr", *crate_percent_per_hour);
  return HAL_OK;
}

HAL_StatusTypeDef MAX17048_ReadVersion(uint16_t *version)
{
  if ((version == NULL) || !MAX17048_IsEnabled())
  {
    NOOG_LOG("MAX17048", "ReadVersion failed: invalid output or gauge power off");
    return HAL_ERROR;
  }

  return MAX17048_ReadRegister(MAX17048_REG_VERSION, version);
}

HAL_StatusTypeDef MAX17048_ReadStatus(uint16_t *status)
{
  if ((status == NULL) || !MAX17048_IsEnabled())
  {
    NOOG_LOG("MAX17048", "ReadStatus failed: invalid output or gauge power off");
    return HAL_ERROR;
  }

  return MAX17048_ReadRegister(MAX17048_REG_STATUS, status);
}

HAL_StatusTypeDef MAX17048_ClearStatusBits(uint16_t bits)
{
  uint16_t status;

  if (!MAX17048_IsEnabled())
  {
    NOOG_LOG("MAX17048", "ClearStatusBits failed: gauge power off");
    return HAL_ERROR;
  }

  if (MAX17048_ReadRegister(MAX17048_REG_STATUS, &status) != HAL_OK)
  {
    NOOG_LOG("MAX17048", "Failed to read status before clear");
    return HAL_ERROR;
  }

  status &= (uint16_t)~bits;
  if (MAX17048_WriteRegister(MAX17048_REG_STATUS, status) != HAL_OK)
  {
    NOOG_LOG("MAX17048", "Failed to write status during clear");
    return HAL_ERROR;
  }

  NOOG_LOG("MAX17048", "Cleared status bits mask 0x%04X -> status 0x%04X", bits, status);
  return HAL_OK;
}

HAL_StatusTypeDef MAX17048_QuickStart(void)
{
  if (!MAX17048_IsEnabled())
  {
    NOOG_LOG("MAX17048", "QuickStart failed: gauge power off");
    return HAL_ERROR;
  }

  if (MAX17048_WriteRegister(MAX17048_REG_MODE, MAX17048_MODE_QUICKSTART) != HAL_OK)
  {
    NOOG_LOG("MAX17048", "QuickStart command failed");
    return HAL_ERROR;
  }

  NOOG_LOG("MAX17048", "QuickStart command sent");
  return HAL_OK;
}

HAL_StatusTypeDef MAX17048_Reset(void)
{
  if (!MAX17048_IsEnabled())
  {
    NOOG_LOG("MAX17048", "Reset failed: gauge power off");
    return HAL_ERROR;
  }

  if (MAX17048_WriteRegister(MAX17048_REG_COMMAND, MAX17048_COMMAND_POR) != HAL_OK)
  {
    NOOG_LOG("MAX17048", "Reset command failed");
    return HAL_ERROR;
  }

  HAL_Delay(MAX17048_RESET_DELAY_MS);
  NOOG_LOG("MAX17048", "Power-on-reset command complete");
  return HAL_OK;
}

HAL_StatusTypeDef MAX17048_ReadMeasurement(MAX17048_Measurement_t *measurement)
{
  if (measurement == NULL)
  {
    NOOG_LOG("MAX17048", "ReadMeasurement called with NULL output");
    return HAL_ERROR;
  }

  if ((MAX17048_ReadVoltage(&measurement->voltage_v) != HAL_OK) ||
      (MAX17048_ReadSoc(&measurement->soc_percent) != HAL_OK) ||
      (MAX17048_ReadCrate(&measurement->crate_percent_per_hour) != HAL_OK) ||
      (MAX17048_ReadVersion(&measurement->version) != HAL_OK) ||
      (MAX17048_ReadStatus(&measurement->status) != HAL_OK))
  {
    NOOG_LOG("MAX17048", "Composite measurement read failed");
    return HAL_ERROR;
  }

  NOOG_LOG("MAX17048", "Measurement V=%.3fV SOC=%.2f%% CRATE=%.2f%%/hr STATUS=0x%04X",
           measurement->voltage_v,
           measurement->soc_percent,
           measurement->crate_percent_per_hour,
           measurement->status);
  return HAL_OK;
}

static HAL_StatusTypeDef MAX17048_ReadRegister(uint8_t reg, uint16_t *value)
{
  uint8_t raw[2];

  if (HAL_I2C_Mem_Read(&hi2c2,
                       MAX17048_I2C_ADDRESS,
                       reg,
                       I2C_MEMADD_SIZE_8BIT,
                       raw,
                       sizeof(raw),
                       MAX17048_I2C_TIMEOUT_MS) != HAL_OK)
  {
    return HAL_ERROR;
  }

  *value = (uint16_t)(((uint16_t)raw[0] << 8) | raw[1]);
  return HAL_OK;
}

static HAL_StatusTypeDef MAX17048_WriteRegister(uint8_t reg, uint16_t value)
{
  uint8_t raw[2];

  raw[0] = (uint8_t)(value >> 8);
  raw[1] = (uint8_t)(value & 0xFFU);

  if (HAL_I2C_Mem_Write(&hi2c2,
                        MAX17048_I2C_ADDRESS,
                        reg,
                        I2C_MEMADD_SIZE_8BIT,
                        raw,
                        sizeof(raw),
                        MAX17048_I2C_TIMEOUT_MS) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

static float MAX17048_ConvertSignedRegister(uint16_t raw, float lsb_scale)
{
  return (float)((int16_t)raw) * lsb_scale;
}
