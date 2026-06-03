#include "sht40.h"
#include "noog_power.h"

#define SHT40_I2C_ADDRESS         (0x44U << 1)
#define SHT40_CMD_SOFT_RESET      0x94U
#define SHT40_CMD_MEASURE_HIGH    0xFDU
#define SHT40_POWERUP_DELAY_MS    5U
#define SHT40_RESET_DELAY_MS      2U
#define SHT40_MEASURE_DELAY_MS    10U
#define SHT40_TX_TIMEOUT_MS       100U
#define SHT40_RX_TIMEOUT_MS       100U

extern I2C_HandleTypeDef hi2c1;

static uint8_t SHT40_Crc8(const uint8_t *data, uint8_t length);

void SHT40_SetPower(bool enable)
{
  (void)NOOG_Power_Set(NOOG_POWER_SHT, enable);
}

bool SHT40_IsEnabled(void)
{
  return NOOG_Power_IsEnabled(NOOG_POWER_SHT);
}

HAL_StatusTypeDef SHT40_SoftReset(void)
{
  uint8_t cmd = SHT40_CMD_SOFT_RESET;

  if (!SHT40_IsEnabled())
  {
    return HAL_ERROR;
  }

  if (HAL_I2C_Master_Transmit(&hi2c1, SHT40_I2C_ADDRESS, &cmd, 1U, SHT40_TX_TIMEOUT_MS) != HAL_OK)
  {
    return HAL_ERROR;
  }

  HAL_Delay(SHT40_RESET_DELAY_MS);
  return HAL_OK;
}

HAL_StatusTypeDef SHT40_Init(void)
{
  SHT40_SetPower(true);
  HAL_Delay(SHT40_POWERUP_DELAY_MS);
  return SHT40_SoftReset();
}

HAL_StatusTypeDef SHT40_ReadMeasurement(SHT40_Measurement_t *measurement)
{
  uint8_t cmd = SHT40_CMD_MEASURE_HIGH;
  uint8_t raw_data[6];
  uint16_t raw_temperature;
  uint16_t raw_humidity;

  if (measurement == NULL)
  {
    return HAL_ERROR;
  }

  if (!SHT40_IsEnabled())
  {
    return HAL_ERROR;
  }

  if (HAL_I2C_Master_Transmit(&hi2c1, SHT40_I2C_ADDRESS, &cmd, 1U, SHT40_TX_TIMEOUT_MS) != HAL_OK)
  {
    return HAL_ERROR;
  }

  HAL_Delay(SHT40_MEASURE_DELAY_MS);

  if (HAL_I2C_Master_Receive(&hi2c1, SHT40_I2C_ADDRESS, raw_data, sizeof(raw_data), SHT40_RX_TIMEOUT_MS) != HAL_OK)
  {
    return HAL_ERROR;
  }

  if ((SHT40_Crc8(&raw_data[0], 2U) != raw_data[2]) || (SHT40_Crc8(&raw_data[3], 2U) != raw_data[5]))
  {
    return HAL_ERROR;
  }

  raw_temperature = (uint16_t)((raw_data[0] << 8) | raw_data[1]);
  raw_humidity = (uint16_t)((raw_data[3] << 8) | raw_data[4]);

  measurement->temperature_c = -45.0f + (175.0f * (float)raw_temperature / 65535.0f);
  measurement->humidity_rh = -6.0f + (125.0f * (float)raw_humidity / 65535.0f);

  if (measurement->humidity_rh < 0.0f)
  {
    measurement->humidity_rh = 0.0f;
  }
  else if (measurement->humidity_rh > 100.0f)
  {
    measurement->humidity_rh = 100.0f;
  }

  return HAL_OK;
}

static uint8_t SHT40_Crc8(const uint8_t *data, uint8_t length)
{
  uint8_t crc = 0xFFU;
  uint8_t index;
  uint8_t bit;

  for (index = 0; index < length; index++)
  {
    crc ^= data[index];
    for (bit = 0; bit < 8U; bit++)
    {
      if ((crc & 0x80U) != 0U)
      {
        crc = (uint8_t)((crc << 1) ^ 0x31U);
      }
      else
      {
        crc <<= 1;
      }
    }
  }

  return crc;
}
