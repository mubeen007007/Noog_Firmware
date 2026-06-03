#include "max_m10s.h"
#include "noog_debug.h"
#include "noog_power.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>

#define MAX_M10S_UART_BAUDRATE_DEFAULT         9600U
#define MAX_M10S_UART_TIMEOUT_MS               200U
#define MAX_M10S_RESET_ASSERT_MS               10U
#define MAX_M10S_RESET_RECOVERY_MS             200U
#define MAX_M10S_POWERUP_DELAY_MS              50U
#define MAX_M10S_VERSION_STRING_MAX_LEN        96U
#define MAX_M10S_UBX_SYNC_CHAR_1               0xB5U
#define MAX_M10S_UBX_SYNC_CHAR_2               0x62U
#define MAX_M10S_UBX_NAV_PVT_PAYLOAD_LEN       92U
#define MAX_M10S_UBX_MON_VER_MIN_PAYLOAD_LEN   40U

extern UART_HandleTypeDef huart2;

typedef enum
{
  MAXM10S_RX_SYNC1 = 0,
  MAXM10S_RX_SYNC2,
  MAXM10S_RX_CLASS,
  MAXM10S_RX_ID,
  MAXM10S_RX_LEN1,
  MAXM10S_RX_LEN2,
  MAXM10S_RX_PAYLOAD,
  MAXM10S_RX_CK_A,
  MAXM10S_RX_CK_B
} MAXM10S_RxState_t;

typedef struct
{
  MAXM10S_RxState_t state;
  uint8_t msg_class;
  uint8_t msg_id;
  uint16_t payload_length;
  uint16_t payload_index;
  uint8_t ck_a;
  uint8_t ck_b;
  uint8_t received_ck_a;
  uint8_t payload[128];
} MAXM10S_RxParser_t;

static HAL_StatusTypeDef MAXM10S_ConfigureUart(uint32_t baudrate);
static void MAXM10S_RxParser_Reset(void);
static void MAXM10S_RxParser_ProcessByte(uint8_t byte);
static void MAXM10S_RxParser_UpdateChecksum(uint8_t byte);
static void MAXM10S_HandleUbxMessage(uint8_t msg_class, uint8_t msg_id, const uint8_t *payload, uint16_t length);
static void MAXM10S_HandleNavPvt(const uint8_t *payload, uint16_t length);
static void MAXM10S_HandleMonVer(const uint8_t *payload, uint16_t length);
static uint16_t MAXM10S_ReadU2(const uint8_t *buffer);
static uint32_t MAXM10S_ReadU4(const uint8_t *buffer);
static int32_t MAXM10S_ReadI4(const uint8_t *buffer);

static volatile uint8_t max_m10s_rx_byte;
static volatile bool max_m10s_pvt_valid;
static MAXM10S_Pvt_t max_m10s_last_pvt;
static char max_m10s_version_string[MAX_M10S_VERSION_STRING_MAX_LEN];
static MAXM10S_RxParser_t max_m10s_rx_parser;
static volatile uint32_t max_m10s_rx_oversize_count;
static volatile uint32_t max_m10s_rx_checksum_error_count;
static volatile uint32_t max_m10s_rx_unknown_message_count;
static volatile uint32_t max_m10s_rx_short_nav_pvt_count;
static volatile uint32_t max_m10s_rx_short_mon_ver_count;
static volatile uint32_t max_m10s_uart_error_count;

void MAXM10S_SetPower(bool enable)
{
  (void)NOOG_Power_Set(NOOG_POWER_GPS, enable);
  NOOG_LOG("MAX-M10S", "GPS rail %s", enable ? "enabled" : "disabled");
}

bool MAXM10S_IsEnabled(void)
{
  return NOOG_Power_IsEnabled(NOOG_POWER_GPS);
}

HAL_StatusTypeDef MAXM10S_Init(void)
{
  MAXM10S_SetPower(true);
  HAL_Delay(MAX_M10S_POWERUP_DELAY_MS);

  if (MAXM10S_ConfigureUart(MAX_M10S_UART_BAUDRATE_DEFAULT) != HAL_OK)
  {
    NOOG_LOG("MAX-M10S", "UART2 configuration failed");
    return HAL_ERROR;
  }

  if (MAXM10S_ResetHardware() != HAL_OK)
  {
    NOOG_LOG("MAX-M10S", "Hardware reset failed");
    return HAL_ERROR;
  }

  if (MAXM10S_StartReception() != HAL_OK)
  {
    NOOG_LOG("MAX-M10S", "Receive start failed");
    return HAL_ERROR;
  }

  (void)MAXM10S_PollVersion();
  (void)MAXM10S_PollNavPvt();

  NOOG_LOG("MAX-M10S", "Initialized on USART2 PD5/PD6 at %lu baud with reset on PB15",
           (unsigned long)MAX_M10S_UART_BAUDRATE_DEFAULT);
  return HAL_OK;
}

HAL_StatusTypeDef MAXM10S_ResetHardware(void)
{
  if (!MAXM10S_IsEnabled())
  {
    NOOG_LOG("MAX-M10S", "Reset requested while GPS power is off");
    return HAL_ERROR;
  }

  HAL_GPIO_WritePin(RESET_STM_GPS_GPIO_Port, RESET_STM_GPS_Pin, GPIO_PIN_RESET);
  HAL_Delay(MAX_M10S_RESET_ASSERT_MS);
  HAL_GPIO_WritePin(RESET_STM_GPS_GPIO_Port, RESET_STM_GPS_Pin, GPIO_PIN_SET);
  HAL_Delay(MAX_M10S_RESET_RECOVERY_MS);

  NOOG_LOG("MAX-M10S", "Hardware reset complete");
  return HAL_OK;
}

HAL_StatusTypeDef MAXM10S_StartReception(void)
{
  MAXM10S_RxParser_Reset();

  if (HAL_UART_Receive_IT(&huart2, (uint8_t *)&max_m10s_rx_byte, 1U) != HAL_OK)
  {
    NOOG_LOG("MAX-M10S", "HAL_UART_Receive_IT failed");
    return HAL_ERROR;
  }

  NOOG_LOG("MAX-M10S", "UART reception started");
  return HAL_OK;
}

HAL_StatusTypeDef MAXM10S_PollNavPvt(void)
{
  NOOG_LOG("MAX-M10S", "Polling UBX-NAV-PVT");
  return MAXM10S_SendUbx(MAX_M10S_UBX_CLASS_NAV, MAX_M10S_UBX_ID_NAV_PVT, NULL, 0U);
}

HAL_StatusTypeDef MAXM10S_PollVersion(void)
{
  NOOG_LOG("MAX-M10S", "Polling UBX-MON-VER");
  return MAXM10S_SendUbx(MAX_M10S_UBX_CLASS_MON, MAX_M10S_UBX_ID_MON_VER, NULL, 0U);
}

HAL_StatusTypeDef MAXM10S_SendUbx(uint8_t msg_class, uint8_t msg_id, const uint8_t *payload, uint16_t payload_length)
{
  uint8_t frame[8U + 128U];
  uint16_t index = 0U;
  uint8_t ck_a = 0U;
  uint8_t ck_b = 0U;
  uint16_t payload_index;

  if (!MAXM10S_IsEnabled() ||
      (payload_length > 128U) ||
      ((payload_length > 0U) && (payload == NULL)))
  {
    NOOG_LOG("MAX-M10S", "SendUbx failed: invalid state or payload length %u", payload_length);
    return HAL_ERROR;
  }

  frame[index++] = MAX_M10S_UBX_SYNC_CHAR_1;
  frame[index++] = MAX_M10S_UBX_SYNC_CHAR_2;
  frame[index++] = msg_class;
  frame[index++] = msg_id;
  frame[index++] = (uint8_t)(payload_length & 0xFFU);
  frame[index++] = (uint8_t)(payload_length >> 8);

  ck_a += msg_class; ck_b += ck_a;
  ck_a += msg_id; ck_b += ck_a;
  ck_a += frame[4]; ck_b += ck_a;
  ck_a += frame[5]; ck_b += ck_a;

  for (payload_index = 0U; payload_index < payload_length; payload_index++)
  {
    frame[index++] = payload[payload_index];
    ck_a = (uint8_t)(ck_a + payload[payload_index]);
    ck_b = (uint8_t)(ck_b + ck_a);
  }

  frame[index++] = ck_a;
  frame[index++] = ck_b;

  if (HAL_UART_Transmit(&huart2, frame, index, MAX_M10S_UART_TIMEOUT_MS) != HAL_OK)
  {
    NOOG_LOG("MAX-M10S", "UART transmit failed for UBX %02X %02X", msg_class, msg_id);
    return HAL_ERROR;
  }

  return HAL_OK;
}

bool MAXM10S_GetLastPvt(MAXM10S_Pvt_t *pvt)
{
  if ((pvt == NULL) || !max_m10s_pvt_valid)
  {
    return false;
  }

  taskENTER_CRITICAL();
  *pvt = max_m10s_last_pvt;
  taskEXIT_CRITICAL();
  return true;
}

bool MAXM10S_HasFix(void)
{
  return max_m10s_pvt_valid && max_m10s_last_pvt.valid_fix;
}

const char *MAXM10S_GetVersionString(void)
{
  return max_m10s_version_string;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    MAXM10S_RxParser_ProcessByte(max_m10s_rx_byte);
    (void)HAL_UART_Receive_IT(&huart2, (uint8_t *)&max_m10s_rx_byte, 1U);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    max_m10s_uart_error_count++;
    (void)HAL_UART_Receive_IT(&huart2, (uint8_t *)&max_m10s_rx_byte, 1U);
  }
}

static HAL_StatusTypeDef MAXM10S_ConfigureUart(uint32_t baudrate)
{
  huart2.Init.BaudRate = baudrate;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;

  return HAL_UART_Init(&huart2);
}

static void MAXM10S_RxParser_Reset(void)
{
  memset(&max_m10s_rx_parser, 0, sizeof(max_m10s_rx_parser));
  max_m10s_rx_parser.state = MAXM10S_RX_SYNC1;
}

static void MAXM10S_RxParser_ProcessByte(uint8_t byte)
{
  switch (max_m10s_rx_parser.state)
  {
    case MAXM10S_RX_SYNC1:
      if (byte == MAX_M10S_UBX_SYNC_CHAR_1)
      {
        max_m10s_rx_parser.state = MAXM10S_RX_SYNC2;
      }
      break;

    case MAXM10S_RX_SYNC2:
      if (byte == MAX_M10S_UBX_SYNC_CHAR_2)
      {
        max_m10s_rx_parser.state = MAXM10S_RX_CLASS;
        max_m10s_rx_parser.ck_a = 0U;
        max_m10s_rx_parser.ck_b = 0U;
      }
      else
      {
        max_m10s_rx_parser.state = MAXM10S_RX_SYNC1;
      }
      break;

    case MAXM10S_RX_CLASS:
      max_m10s_rx_parser.msg_class = byte;
      MAXM10S_RxParser_UpdateChecksum(byte);
      max_m10s_rx_parser.state = MAXM10S_RX_ID;
      break;

    case MAXM10S_RX_ID:
      max_m10s_rx_parser.msg_id = byte;
      MAXM10S_RxParser_UpdateChecksum(byte);
      max_m10s_rx_parser.state = MAXM10S_RX_LEN1;
      break;

    case MAXM10S_RX_LEN1:
      max_m10s_rx_parser.payload_length = byte;
      MAXM10S_RxParser_UpdateChecksum(byte);
      max_m10s_rx_parser.state = MAXM10S_RX_LEN2;
      break;

    case MAXM10S_RX_LEN2:
      max_m10s_rx_parser.payload_length |= (uint16_t)((uint16_t)byte << 8);
      MAXM10S_RxParser_UpdateChecksum(byte);
      max_m10s_rx_parser.payload_index = 0U;

      if (max_m10s_rx_parser.payload_length > sizeof(max_m10s_rx_parser.payload))
      {
        max_m10s_rx_oversize_count++;
        MAXM10S_RxParser_Reset();
      }
      else if (max_m10s_rx_parser.payload_length == 0U)
      {
        max_m10s_rx_parser.state = MAXM10S_RX_CK_A;
      }
      else
      {
        max_m10s_rx_parser.state = MAXM10S_RX_PAYLOAD;
      }
      break;

    case MAXM10S_RX_PAYLOAD:
      max_m10s_rx_parser.payload[max_m10s_rx_parser.payload_index++] = byte;
      MAXM10S_RxParser_UpdateChecksum(byte);
      if (max_m10s_rx_parser.payload_index >= max_m10s_rx_parser.payload_length)
      {
        max_m10s_rx_parser.state = MAXM10S_RX_CK_A;
      }
      break;

    case MAXM10S_RX_CK_A:
      max_m10s_rx_parser.received_ck_a = byte;
      max_m10s_rx_parser.state = MAXM10S_RX_CK_B;
      break;

    case MAXM10S_RX_CK_B:
      if ((max_m10s_rx_parser.received_ck_a == max_m10s_rx_parser.ck_a) &&
          (byte == max_m10s_rx_parser.ck_b))
      {
        MAXM10S_HandleUbxMessage(max_m10s_rx_parser.msg_class,
                                 max_m10s_rx_parser.msg_id,
                                 max_m10s_rx_parser.payload,
                                 max_m10s_rx_parser.payload_length);
      }
      else
      {
        max_m10s_rx_checksum_error_count++;
      }
      MAXM10S_RxParser_Reset();
      break;

    default:
      MAXM10S_RxParser_Reset();
      break;
  }
}

static void MAXM10S_RxParser_UpdateChecksum(uint8_t byte)
{
  max_m10s_rx_parser.ck_a = (uint8_t)(max_m10s_rx_parser.ck_a + byte);
  max_m10s_rx_parser.ck_b = (uint8_t)(max_m10s_rx_parser.ck_b + max_m10s_rx_parser.ck_a);
}

static void MAXM10S_HandleUbxMessage(uint8_t msg_class, uint8_t msg_id, const uint8_t *payload, uint16_t length)
{
  if ((msg_class == MAX_M10S_UBX_CLASS_NAV) && (msg_id == MAX_M10S_UBX_ID_NAV_PVT))
  {
    MAXM10S_HandleNavPvt(payload, length);
  }
  else if ((msg_class == MAX_M10S_UBX_CLASS_MON) && (msg_id == MAX_M10S_UBX_ID_MON_VER))
  {
    MAXM10S_HandleMonVer(payload, length);
  }
  else
  {
    max_m10s_rx_unknown_message_count++;
  }
}

static void MAXM10S_HandleNavPvt(const uint8_t *payload, uint16_t length)
{
  if (length < MAX_M10S_UBX_NAV_PVT_PAYLOAD_LEN)
  {
    max_m10s_rx_short_nav_pvt_count++;
    return;
  }

  max_m10s_last_pvt.i_tow_ms = MAXM10S_ReadU4(&payload[0]);
  max_m10s_last_pvt.year = MAXM10S_ReadU2(&payload[4]);
  max_m10s_last_pvt.month = payload[6];
  max_m10s_last_pvt.day = payload[7];
  max_m10s_last_pvt.hour = payload[8];
  max_m10s_last_pvt.minute = payload[9];
  max_m10s_last_pvt.second = payload[10];
  max_m10s_last_pvt.valid = payload[11];
  max_m10s_last_pvt.fix_type = payload[20];
  max_m10s_last_pvt.flags = payload[21];
  max_m10s_last_pvt.flags2 = payload[22];
  max_m10s_last_pvt.num_sv = payload[23];
  max_m10s_last_pvt.longitude_deg_e7 = MAXM10S_ReadI4(&payload[24]);
  max_m10s_last_pvt.latitude_deg_e7 = MAXM10S_ReadI4(&payload[28]);
  max_m10s_last_pvt.height_msl_mm = MAXM10S_ReadI4(&payload[36]);
  max_m10s_last_pvt.horizontal_accuracy_mm = MAXM10S_ReadU4(&payload[40]);
  max_m10s_last_pvt.vertical_accuracy_mm = MAXM10S_ReadU4(&payload[44]);
  max_m10s_last_pvt.ground_speed_mm_s = MAXM10S_ReadI4(&payload[60]);
  max_m10s_last_pvt.speed_accuracy_mm_s = MAXM10S_ReadU4(&payload[68]);
  max_m10s_last_pvt.p_dop = MAXM10S_ReadU2(&payload[76]);
  max_m10s_last_pvt.valid_fix = ((max_m10s_last_pvt.flags & 0x01U) != 0U) &&
                                (max_m10s_last_pvt.fix_type >= 2U);
  max_m10s_pvt_valid = true;

}

static void MAXM10S_HandleMonVer(const uint8_t *payload, uint16_t length)
{
  uint16_t copy_length;

  if (length < MAX_M10S_UBX_MON_VER_MIN_PAYLOAD_LEN)
  {
    max_m10s_rx_short_mon_ver_count++;
    return;
  }

  copy_length = (uint16_t)((length < (MAX_M10S_VERSION_STRING_MAX_LEN - 1U)) ? length : (MAX_M10S_VERSION_STRING_MAX_LEN - 1U));
  memcpy(max_m10s_version_string, payload, copy_length);
  max_m10s_version_string[copy_length] = '\0';

  for (copy_length = 0U; copy_length < (MAX_M10S_VERSION_STRING_MAX_LEN - 1U); copy_length++)
  {
    if (max_m10s_version_string[copy_length] == '\0')
    {
      break;
    }
    if ((max_m10s_version_string[copy_length] == '\r') || (max_m10s_version_string[copy_length] == '\n'))
    {
      max_m10s_version_string[copy_length] = ' ';
    }
  }

}

static uint16_t MAXM10S_ReadU2(const uint8_t *buffer)
{
  return (uint16_t)((uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8));
}

static uint32_t MAXM10S_ReadU4(const uint8_t *buffer)
{
  return (uint32_t)buffer[0] |
         ((uint32_t)buffer[1] << 8) |
         ((uint32_t)buffer[2] << 16) |
         ((uint32_t)buffer[3] << 24);
}

static int32_t MAXM10S_ReadI4(const uint8_t *buffer)
{
  return (int32_t)MAXM10S_ReadU4(buffer);
}
