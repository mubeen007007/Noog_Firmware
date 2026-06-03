#include "ds3231.h"
#include "noog_debug.h"
#include "noog_power.h"

#define DS3231_I2C_ADDRESS           (0x68U << 1)
#define DS3231_REG_TIME              0x00U
#define DS3231_REG_CONTROL           0x0EU
#define DS3231_REG_STATUS            0x0FU
#define DS3231_REG_TEMPERATURE_MSB   0x11U

#define DS3231_CONTROL_EOSC          0x80U
#define DS3231_CONTROL_BBSQW         0x40U
#define DS3231_CONTROL_CONV          0x20U
#define DS3231_CONTROL_INTCN         0x04U
#define DS3231_CONTROL_A2IE          0x02U
#define DS3231_CONTROL_A1IE          0x01U

#define DS3231_STATUS_OSF            0x80U
#define DS3231_STATUS_EN32KHZ        0x08U
#define DS3231_STATUS_BSY            0x04U
#define DS3231_STATUS_A2F            0x02U
#define DS3231_STATUS_A1F            0x01U

#define DS3231_POWERUP_DELAY_MS      5U
#define DS3231_I2C_TIMEOUT_MS        100U

extern I2C_HandleTypeDef hi2c1;

static HAL_StatusTypeDef DS3231_Read(uint8_t reg, uint8_t *data, uint16_t length);
static HAL_StatusTypeDef DS3231_Write(uint8_t reg, const uint8_t *data, uint16_t length);
static uint8_t DS3231_DecimalToBcd(uint8_t value);
static uint8_t DS3231_BcdToDecimal(uint8_t value);
static bool DS3231_IsValidDateTime(const DS3231_DateTime_t *date_time);

void DS3231_SetPower(bool enable)
{
  (void)NOOG_Power_Set(NOOG_POWER_RTC, enable);
  NOOG_LOG("DS3231", "RTC rail %s", enable ? "enabled" : "disabled");
}

bool DS3231_IsEnabled(void)
{
  return NOOG_Power_IsEnabled(NOOG_POWER_RTC);
}

HAL_StatusTypeDef DS3231_Init(void)
{
  uint8_t control;
  uint8_t status;

  DS3231_SetPower(true);
  HAL_Delay(DS3231_POWERUP_DELAY_MS);

  if (DS3231_Read(DS3231_REG_CONTROL, &control, 1U) != HAL_OK)
  {
    NOOG_LOG("DS3231", "Failed to read control register");
    return HAL_ERROR;
  }

  control &= (uint8_t)~(DS3231_CONTROL_EOSC | DS3231_CONTROL_BBSQW | DS3231_CONTROL_A1IE | DS3231_CONTROL_A2IE);
  control |= DS3231_CONTROL_INTCN;

  if (DS3231_Write(DS3231_REG_CONTROL, &control, 1U) != HAL_OK)
  {
    NOOG_LOG("DS3231", "Failed to write control register");
    return HAL_ERROR;
  }

  if (DS3231_Read(DS3231_REG_STATUS, &status, 1U) != HAL_OK)
  {
    NOOG_LOG("DS3231", "Failed to read status register");
    return HAL_ERROR;
  }

  status &= (uint8_t)~(DS3231_STATUS_OSF | DS3231_STATUS_A1F | DS3231_STATUS_A2F);

  if (DS3231_Write(DS3231_REG_STATUS, &status, 1U) != HAL_OK)
  {
    NOOG_LOG("DS3231", "Failed to clear status flags");
    return HAL_ERROR;
  }

  NOOG_LOG("DS3231", "Initialized on I2C1 address 0x%02X, status=0x%02X", DS3231_I2C_ADDRESS >> 1, status);
  return HAL_OK;
}

HAL_StatusTypeDef DS3231_GetDateTime(DS3231_DateTime_t *date_time)
{
  uint8_t raw[7];

  if ((date_time == NULL) || !DS3231_IsEnabled())
  {
    NOOG_LOG("DS3231", "GetDateTime failed: invalid output or RTC power off");
    return HAL_ERROR;
  }

  if (DS3231_Read(DS3231_REG_TIME, raw, sizeof(raw)) != HAL_OK)
  {
    NOOG_LOG("DS3231", "Failed to read date/time registers");
    return HAL_ERROR;
  }

  date_time->seconds = DS3231_BcdToDecimal((uint8_t)(raw[0] & 0x7FU));
  date_time->minutes = DS3231_BcdToDecimal((uint8_t)(raw[1] & 0x7FU));

  if ((raw[2] & 0x40U) != 0U)
  {
    uint8_t hours = DS3231_BcdToDecimal((uint8_t)(raw[2] & 0x1FU));
    date_time->hours = (uint8_t)((raw[2] & 0x20U) != 0U ? (hours % 12U) + 12U : (hours % 12U));
  }
  else
  {
    date_time->hours = DS3231_BcdToDecimal((uint8_t)(raw[2] & 0x3FU));
  }

  date_time->day_of_week = DS3231_BcdToDecimal((uint8_t)(raw[3] & 0x07U));
  date_time->day_of_month = DS3231_BcdToDecimal((uint8_t)(raw[4] & 0x3FU));
  date_time->month = DS3231_BcdToDecimal((uint8_t)(raw[5] & 0x1FU));
  date_time->year = (uint16_t)(2000U + (uint16_t)DS3231_BcdToDecimal(raw[6]) + (((raw[5] & 0x80U) != 0U) ? 100U : 0U));

  NOOG_LOG("DS3231", "DateTime %04u-%02u-%02u %02u:%02u:%02u",
           date_time->year,
           date_time->month,
           date_time->day_of_month,
           date_time->hours,
           date_time->minutes,
           date_time->seconds);
  return HAL_OK;
}

HAL_StatusTypeDef DS3231_SetDateTime(const DS3231_DateTime_t *date_time)
{
  uint8_t raw[7];
  uint16_t year_offset;

  if (!DS3231_IsValidDateTime(date_time) || !DS3231_IsEnabled())
  {
    NOOG_LOG("DS3231", "SetDateTime failed: invalid input or RTC power off");
    return HAL_ERROR;
  }

  year_offset = (uint16_t)(date_time->year - 2000U);

  raw[0] = DS3231_DecimalToBcd(date_time->seconds);
  raw[1] = DS3231_DecimalToBcd(date_time->minutes);
  raw[2] = DS3231_DecimalToBcd(date_time->hours);
  raw[3] = DS3231_DecimalToBcd(date_time->day_of_week);
  raw[4] = DS3231_DecimalToBcd(date_time->day_of_month);
  raw[5] = DS3231_DecimalToBcd(date_time->month);
  raw[6] = DS3231_DecimalToBcd((uint8_t)(year_offset % 100U));

  if (year_offset >= 100U)
  {
    raw[5] |= 0x80U;
  }

  if (DS3231_Write(DS3231_REG_TIME, raw, sizeof(raw)) != HAL_OK)
  {
    NOOG_LOG("DS3231", "Failed to write date/time");
    return HAL_ERROR;
  }

  NOOG_LOG("DS3231", "DateTime set to %04u-%02u-%02u %02u:%02u:%02u",
           date_time->year,
           date_time->month,
           date_time->day_of_month,
           date_time->hours,
           date_time->minutes,
           date_time->seconds);
  return HAL_OK;
}

HAL_StatusTypeDef DS3231_GetTemperature(float *temperature_c)
{
  uint8_t raw[2];
  int16_t value;

  if ((temperature_c == NULL) || !DS3231_IsEnabled())
  {
    NOOG_LOG("DS3231", "GetTemperature failed: invalid output or RTC power off");
    return HAL_ERROR;
  }

  if (DS3231_Read(DS3231_REG_TEMPERATURE_MSB, raw, sizeof(raw)) != HAL_OK)
  {
    NOOG_LOG("DS3231", "Failed to read temperature registers");
    return HAL_ERROR;
  }

  value = (int16_t)(((int16_t)raw[0] << 8) | raw[1]);
  value >>= 6;
  *temperature_c = (float)value * 0.25f;

  NOOG_LOG("DS3231", "Temperature %.2f C", *temperature_c);
  return HAL_OK;
}

HAL_StatusTypeDef DS3231_ReadStatus(uint8_t *status)
{
  if ((status == NULL) || !DS3231_IsEnabled())
  {
    NOOG_LOG("DS3231", "ReadStatus failed: invalid output or RTC power off");
    return HAL_ERROR;
  }

  return DS3231_Read(DS3231_REG_STATUS, status, 1U);
}

HAL_StatusTypeDef DS3231_ClearStatusFlags(uint8_t flags)
{
  uint8_t status;

  if (!DS3231_IsEnabled())
  {
    NOOG_LOG("DS3231", "ClearStatusFlags failed: RTC power off");
    return HAL_ERROR;
  }

  if (DS3231_Read(DS3231_REG_STATUS, &status, 1U) != HAL_OK)
  {
    NOOG_LOG("DS3231", "Failed to read status before clear");
    return HAL_ERROR;
  }

  status &= (uint8_t)~flags;
  if (DS3231_Write(DS3231_REG_STATUS, &status, 1U) != HAL_OK)
  {
    NOOG_LOG("DS3231", "Failed to write status during clear");
    return HAL_ERROR;
  }

  NOOG_LOG("DS3231", "Cleared status flags mask 0x%02X -> status 0x%02X", flags, status);
  return HAL_OK;
}

static HAL_StatusTypeDef DS3231_Read(uint8_t reg, uint8_t *data, uint16_t length)
{
  return HAL_I2C_Mem_Read(&hi2c1,
                          DS3231_I2C_ADDRESS,
                          reg,
                          I2C_MEMADD_SIZE_8BIT,
                          data,
                          length,
                          DS3231_I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef DS3231_Write(uint8_t reg, const uint8_t *data, uint16_t length)
{
  return HAL_I2C_Mem_Write(&hi2c1,
                           DS3231_I2C_ADDRESS,
                           reg,
                           I2C_MEMADD_SIZE_8BIT,
                           (uint8_t *)data,
                           length,
                           DS3231_I2C_TIMEOUT_MS);
}

static uint8_t DS3231_DecimalToBcd(uint8_t value)
{
  return (uint8_t)(((value / 10U) << 4) | (value % 10U));
}

static uint8_t DS3231_BcdToDecimal(uint8_t value)
{
  return (uint8_t)(((value >> 4) * 10U) + (value & 0x0FU));
}

static bool DS3231_IsValidDateTime(const DS3231_DateTime_t *date_time)
{
  if (date_time == NULL)
  {
    return false;
  }

  return (date_time->seconds <= 59U) &&
         (date_time->minutes <= 59U) &&
         (date_time->hours <= 23U) &&
         (date_time->day_of_week >= 1U) &&
         (date_time->day_of_week <= 7U) &&
         (date_time->day_of_month >= 1U) &&
         (date_time->day_of_month <= 31U) &&
         (date_time->month >= 1U) &&
         (date_time->month <= 12U) &&
         (date_time->year >= 2000U) &&
         (date_time->year <= 2199U);
}
