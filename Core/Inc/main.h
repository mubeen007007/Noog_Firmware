/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

#define LED1_R_Pin GPIO_PIN_0
#define LED1_R_GPIO_Port GPIOA
#define LED1_G_Pin GPIO_PIN_1
#define LED1_G_GPIO_Port GPIOA
#define LED1_B_Pin GPIO_PIN_2
#define LED1_B_GPIO_Port GPIOA
#define LED2_R_Pin GPIO_PIN_3
#define LED2_R_GPIO_Port GPIOA
#define LED2_G_Pin GPIO_PIN_4
#define LED2_G_GPIO_Port GPIOA
#define SPI1_SCK_MCU_Pin GPIO_PIN_5
#define SPI1_SCK_MCU_GPIO_Port GPIOA
#define SPI1_MISO_MCU_Pin GPIO_PIN_6
#define SPI1_MISO_MCU_GPIO_Port GPIOA
#define SCL_MEMS_Pin GPIO_PIN_8
#define SCL_MEMS_GPIO_Port GPIOA

#define SPI3_SCK_Pin GPIO_PIN_3
#define SPI3_SCK_GPIO_Port GPIOB
#define SPI3_MISO_Pin GPIO_PIN_4
#define SPI3_MISO_GPIO_Port GPIOB
#define SPI1_MOSI_MCU_Pin GPIO_PIN_5
#define SPI1_MOSI_MCU_GPIO_Port GPIOB
#define I2C1_SCL_SENS_Pin GPIO_PIN_6
#define I2C1_SCL_SENS_GPIO_Port GPIOB
#define I2C1_SDA_SENS_Pin GPIO_PIN_7
#define I2C1_SDA_SENS_GPIO_Port GPIOB
#define I2C2_SCL_RTC_Pin GPIO_PIN_10
#define I2C2_SCL_RTC_GPIO_Port GPIOB
#define I2C2_SDA_RTC_Pin GPIO_PIN_11
#define I2C2_SDA_RTC_GPIO_Port GPIOB
#define SPI2_SCK_MCU_Pin GPIO_PIN_13
#define SPI2_SCK_MCU_GPIO_Port GPIOB
#define RESET_STM_GPS_Pin GPIO_PIN_15
#define RESET_STM_GPS_GPIO_Port GPIOB

#define LED2_B_Pin GPIO_PIN_0
#define LED2_B_GPIO_Port GPIOC
#define LED3_R_Pin GPIO_PIN_1
#define LED3_R_GPIO_Port GPIOC
#define SPI2_MISO_MCU_Pin GPIO_PIN_2
#define SPI2_MISO_MCU_GPIO_Port GPIOC
#define SPI2_MOSI_MCU_Pin GPIO_PIN_3
#define SPI2_MOSI_MCU_GPIO_Port GPIOC
#define STM_pin_EN_SENS_Pin GPIO_PIN_5
#define STM_pin_EN_SENS_GPIO_Port GPIOC
#define STM_pin_EN_LED_Pin GPIO_PIN_6
#define STM_pin_EN_LED_GPIO_Port GPIOC
#define LED3_G_Pin GPIO_PIN_7
#define LED3_G_GPIO_Port GPIOC
#define LED3_B_Pin GPIO_PIN_8
#define LED3_B_GPIO_Port GPIOC
#define SDA_MEMS_Pin GPIO_PIN_9
#define SDA_MEMS_GPIO_Port GPIOC
#define TX_FTDI_Pin GPIO_PIN_10
#define TX_FTDI_GPIO_Port GPIOC
#define RX_FTDI_Pin GPIO_PIN_11
#define RX_FTDI_GPIO_Port GPIOC
#define SPI3_MOSI_Pin GPIO_PIN_12
#define SPI3_MOSI_GPIO_Port GPIOC
#define CS_FLASH_ISO_Pin GPIO_PIN_14
#define CS_FLASH_ISO_GPIO_Port GPIOC
#define STM_pin_EN_SD_Pin GPIO_PIN_15
#define STM_pin_EN_SD_GPIO_Port GPIOC

#define TX_GPS_Pin GPIO_PIN_5
#define TX_GPS_GPIO_Port GPIOD
#define RX_GPS_Pin GPIO_PIN_6
#define RX_GPS_GPIO_Port GPIOD
#define TX_LORA_Pin GPIO_PIN_8
#define TX_LORA_GPIO_Port GPIOD
#define RX_LORA_Pin GPIO_PIN_9
#define RX_LORA_GPIO_Port GPIOD
#define INT_MEMS_Pin GPIO_PIN_10
#define INT_MEMS_GPIO_Port GPIOD
#define CS_ICM_Pin GPIO_PIN_11
#define CS_ICM_GPIO_Port GPIOD
#define INT_ICM_Pin GPIO_PIN_12
#define INT_ICM_GPIO_Port GPIOD
#define CS_BMP_Pin GPIO_PIN_13
#define CS_BMP_GPIO_Port GPIOD
#define INT_BMP_Pin GPIO_PIN_14
#define INT_BMP_GPIO_Port GPIOD
#define STM_pin_EN_RTC_Pin GPIO_PIN_15
#define STM_pin_EN_RTC_GPIO_Port GPIOD

#define STM_pin_EN_Lora_Pin GPIO_PIN_0
#define STM_pin_EN_Lora_GPIO_Port GPIOE
#define Lora_RST_Pin GPIO_PIN_1
#define Lora_RST_GPIO_Port GPIOE
#define Alert_mcu_Pin GPIO_PIN_3
#define Alert_mcu_GPIO_Port GPIOE
#define CardDetect_Pin GPIO_PIN_4
#define CardDetect_GPIO_Port GPIOE
#define STM_pin_EN_SHT_Pin GPIO_PIN_11
#define STM_pin_EN_SHT_GPIO_Port GPIOE
#define STM_pin_EN_W25_Pin GPIO_PIN_12
#define STM_pin_EN_W25_GPIO_Port GPIOE
#define STM_pin_EN_MEM_Pin GPIO_PIN_13
#define STM_pin_EN_MEM_GPIO_Port GPIOE
#define CS_SD_Pin GPIO_PIN_14
#define CS_SD_GPIO_Port GPIOE
#define STM_pin_EN_GPS_Pin GPIO_PIN_15
#define STM_pin_EN_GPS_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
