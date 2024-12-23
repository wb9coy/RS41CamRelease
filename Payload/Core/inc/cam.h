/*
 * cam.h
 *
 *  Created on: Dec 30, 2021
 *      Author: eswiech
 */

#ifndef INC_CAM_H_
#define INC_CAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ecc.h"
#include "stm32f1xx_hal.h"

#define CAM_READ_SIZE 56
#define CAM_DATA_LEN 42
#define CHUNCK_SIZE  (CAM_READ_SIZE*CAM_DATA_LEN)
#define CAM_HEADER_LEN 5
#define CAM_FOOTER_LEN 5

typedef enum
{
  CAM_SETUP    	 = 0x00U,
  CAM_TAKE_PIC 	 = 0x01U,
  CAM_START_XFER = 0x02U,
  CAM_DATA_XFER	 = 0x03U,
  CAM_END_XFER   = 0x04U,
} CAM_StateTypeDef;

typedef enum
{
  CAM_OK    	= 0x00U,
  CAM_FAIL	 	= 0x01U,
} CAM_StatusTypeDef;

CAM_StatusTypeDef resetCam(UART_HandleTypeDef *huart);
CAM_StatusTypeDef clearCache(UART_HandleTypeDef *huart);
CAM_StatusTypeDef stopTakingPicture(UART_HandleTypeDef *huart);
CAM_StatusTypeDef setCompression(UART_HandleTypeDef *uart);
CAM_StatusTypeDef getLenOfPhoto(UART_HandleTypeDef *huart, uint32_t *len);
CAM_StatusTypeDef takePhoto(UART_HandleTypeDef *uart);
CAM_StatusTypeDef getChuck(UART_HandleTypeDef *huart, uint8_t *chunkBuf, uint32_t totBytes);
CAM_StatusTypeDef setPhotoSize(UART_HandleTypeDef *huart);
CAM_StatusTypeDef processCAM(UART_HandleTypeDef *huart, struct rscode_driver *rsDriver, uint8_t *chunkBuf);
CAM_StatusTypeDef processTestCam();
#ifdef TEST_MODE
CAM_StatusTypeDef processTestCam(struct rscode_driver *rsDriver);
#endif


#endif /* INC_CAM_H_ */
