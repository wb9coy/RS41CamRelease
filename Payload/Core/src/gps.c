/*
 * gps.c
 *
 *  Created on: Feb 5, 2022
 *      Author: eswiech
 */

#include <string.h>
#include "radio.h"
#include "config.h"
#include "gps.h"
#include "packetDefs.h"

GPS_StatusTypeDef processGPS(struct rscode_driver *rsDriver,UART_HandleTypeDef *huart)
{
	HAL_StatusTypeDef HAL_Status;
	GPS_StatusTypeDef gpsStatus  = GPS_OK;
	uint16_t uartRxLen;
	int indx = 0;
	uint8_t gpsSentence[GPS_UART_BUF_DATA_SIZE];
	uint8_t gpsUARTBuf[UART_DATA_SIZE];
	uint8_t txBuf[MTU_SIZE];

	static int sendGGA 		= 0;
	static int sendRMC 		= 0;
	static uint16_t ggaLen  = 0;
	static uint16_t rmcLen  = 0;
	static uint8_t GPGGASentence[GPS_UART_BUF_DATA_SIZE+1];
	static uint8_t GPRMCSentence[GPS_UART_BUF_DATA_SIZE+1];
	struct HABPacketGPSDataType HABPacketGPSData;

	HAL_Status = HAL_UARTEx_ReceiveToIdle(huart, (uint8_t *)&gpsUARTBuf, UART_DATA_SIZE, &uartRxLen, 3000);
	HAL_Delay(100);
	//strncpy(gpsUARTBuf,"$GPGGA,184052.00,3254.58594,N,11655.81393,W,2,10,0.95,10167.3,M,-33.4,M,,0000*62\r\n",82);
	//uartRxLen = 82;
	if(HAL_Status == HAL_OK)
	{
		for(int i=0;i<uartRxLen;i++)
		{
			if(indx > GPS_UART_BUF_DATA_SIZE-1)
			{
				indx = 0;
				memset(gpsSentence, '\0', GPS_UART_BUF_DATA_SIZE);
			}
			else
			{
				gpsSentence[indx] = gpsUARTBuf[i];
				if( gpsUARTBuf[i] == '\n')
				{
					if(gpsSentence[0] == '$')
					{
						if(strncmp("GGA",(const char *)&gpsSentence[3],3) == 0)
						{
							memset(GPGGASentence, '\0', GPS_UART_BUF_DATA_SIZE+1);
							memcpy(GPGGASentence,gpsSentence,indx+1);
							sendGGA = 1;
						}
						if(strncmp("RMC",(const char *)&gpsSentence[3],3) == 0)
						{
							memset(GPRMCSentence, '\0', GPS_UART_BUF_DATA_SIZE+1);
							memcpy(GPRMCSentence,gpsSentence,indx+1);
							sendRMC = 1;
						}
					}
					indx = 0;
				}
				else
				{
					indx++;
				}
			}
		}
	}
	else
	{
	  HAL_Delay(1);
	}

	if(sendGGA == 1 && sendRMC ==1)
	{
		//strncpy((const char *)GPGGASentence,"$GPGGA,184052.00,3254.58594,N,11655.81393,W,2,10,0.95,10167.3,M,-33.4,M,,0000*62\r\n",82);
		rmcLen = strlen((const char *)GPRMCSentence);
		ggaLen = strlen((const char *)GPGGASentence);

		memset(&HABPacketGPSData, ' ', sizeof(HABPacketGPSData));
		if(ggaLen > GPS_BUF_DATA_SIZE)
		{
			HABPacketGPSData.packetType = GPS_GGA_1;
			HABPacketGPSData.gpsDataLen = GPS_BUF_DATA_SIZE;
		}
		else
		{
			HABPacketGPSData.packetType = GPS_GGA;
			HABPacketGPSData.gpsDataLen = ggaLen;
		}
		memcpy(HABPacketGPSData.gpsData,GPGGASentence,HABPacketGPSData.gpsDataLen);
		rscode_encode(rsDriver, (unsigned char *)&HABPacketGPSData, sizeof(HABPacketGPSData)-NPAR , (unsigned char *)&HABPacketGPSData);
		memcpy(txBuf,&HABPacketGPSData,sizeof(HABPacketGPSData));
		HAL_Status =  radioTxData(txBuf,sizeof(HABPacketGPSData));
		//HAL_Delay(PROTOCOL_DELAY);

		if(ggaLen >  GPS_BUF_DATA_SIZE)
		{
			memset(&HABPacketGPSData, ' ', sizeof(HABPacketGPSData));
			HABPacketGPSData.packetType = GPS_GGA_2;
			HABPacketGPSData.gpsDataLen = ggaLen - GPS_BUF_DATA_SIZE;
			memcpy(HABPacketGPSData.gpsData,&GPGGASentence[GPS_BUF_DATA_SIZE],HABPacketGPSData.gpsDataLen);
			rscode_encode(rsDriver, (unsigned char *)&HABPacketGPSData, sizeof(HABPacketGPSData)-NPAR , (unsigned char *)&HABPacketGPSData);
			memcpy(txBuf,&HABPacketGPSData,sizeof(HABPacketGPSData));
			HAL_Status =  radioTxData(txBuf,sizeof(HABPacketGPSData));
			//HAL_Delay(PROTOCOL_DELAY);
		}
		///////////////////////////////////////////

		memset(&HABPacketGPSData, ' ', sizeof(HABPacketGPSData));
		if(rmcLen >  GPS_BUF_DATA_SIZE)
		{
			HABPacketGPSData.packetType = GPS_RMC_1;
			HABPacketGPSData.gpsDataLen = GPS_BUF_DATA_SIZE;
		}
		else
		{
			HABPacketGPSData.packetType = GPS_RMC;
			HABPacketGPSData.gpsDataLen = rmcLen;
		}

		memcpy(HABPacketGPSData.gpsData,GPRMCSentence,HABPacketGPSData.gpsDataLen);
		rscode_encode(rsDriver, (unsigned char *)&HABPacketGPSData, sizeof(HABPacketGPSData)-NPAR, (unsigned char *)&HABPacketGPSData);
		memcpy(txBuf,&HABPacketGPSData,sizeof(HABPacketGPSData));
		HAL_Status =  radioTxData(txBuf,sizeof(HABPacketGPSData));
		//HAL_Delay(PROTOCOL_DELAY);
		if(rmcLen >  GPS_BUF_DATA_SIZE)
		{
			memset(&HABPacketGPSData, ' ', sizeof(HABPacketGPSData));
			HABPacketGPSData.packetType = GPS_RMC_2;
			HABPacketGPSData.gpsDataLen = rmcLen - GPS_BUF_DATA_SIZE;
			memcpy(HABPacketGPSData.gpsData,&GPRMCSentence[GPS_BUF_DATA_SIZE],HABPacketGPSData.gpsDataLen);
			rscode_encode(rsDriver, (unsigned char *)&HABPacketGPSData, sizeof(HABPacketGPSData)-NPAR, (unsigned char *)&HABPacketGPSData);
			memcpy(txBuf,&HABPacketGPSData,sizeof(HABPacketGPSData));
			HAL_Status =  radioTxData(txBuf,sizeof(HABPacketGPSData));
			//HAL_Delay(PROTOCOL_DELAY);
		}

		sendGGA 		= 0;
		sendRMC 		= 0;
	}

	return gpsStatus;
}

