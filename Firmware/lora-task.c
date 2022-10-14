#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
//
#include "main.h"
#include "esensor.h"
#include "loramodule.h"
//
#include "sensor-task.h"
#include "lora-task.h"

#define	LORA_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )

uint8_t g_u8LoRaTxStartFlag = 0;
uint8_t g_u8LoRaRxStartFlag = 0;

static void loraTask(void *pvParam);
void LoRa_RxDoneAction(void);
void LoRa_TxDoneAction(void);
void LoRa_TxStart(void);

void setLoRaTxStartFlag(uint8_t u8Value)
{
	g_u8LoRaTxStartFlag = u8Value;
}

void setLoRaRxStartFlag(uint8_t u8Value)
{
	g_u8LoRaRxStartFlag = u8Value;
}

uint8_t getLoRaTxStartFlag(void)
{
	return g_u8LoRaTxStartFlag;
}

uint8_t getLoRaRxStartFlag(void)
{
	return g_u8LoRaRxStartFlag;
}

void LoRa_RxDoneAction(void)
{
	unsigned char RxBuffer[LORA_PAYLOAD_SIZE];
	unsigned int BufferSize = LORA_PAYLOAD_SIZE;
	double RSSIvalue = 0.0f;
	unsigned char i;
	
	memset(RxBuffer, 0, sizeof(RxBuffer));
  LORAM_GetRxPacket(RxBuffer, (unsigned short int *)&BufferSize);
	
	if (BufferSize > 0)
	{
		printf("Received sensor data from device with LoRa [OK]\n");
		
		RSSIvalue = LORAM_GetPacketRssi();
		
		getSensorData()->u8DeviceID = RxBuffer[7] - 0x30;
		
		getSensorData()->u8Temperature[0] = RxBuffer[8];
		getSensorData()->u8Temperature[1] = RxBuffer[9];
		
		getSensorData()->u8Pressure[0] = RxBuffer[10];
		getSensorData()->u8Pressure[1] = RxBuffer[11];
		getSensorData()->u8Pressure[2] = RxBuffer[12];
		getSensorData()->u8Pressure[3] = RxBuffer[13];
		
		getSensorData()->u8Humidity[0] = RxBuffer[14];
		getSensorData()->u8Humidity[1] = RxBuffer[15];
		getSensorData()->u8Humidity[2] = RxBuffer[16];
		getSensorData()->u8Humidity[3] = RxBuffer[17];
		
		getSensorData()->u8Resistrance[0] = RxBuffer[18];
		getSensorData()->u8Resistrance[1] = RxBuffer[19];
		getSensorData()->u8Resistrance[2] = RxBuffer[20];
		getSensorData()->u8Resistrance[3] = RxBuffer[21];
		
		getSensorData()->u8AirQualityScore[0] = RxBuffer[22];
		getSensorData()->u8AirQualityScore[1] = RxBuffer[23];
		
		printf("RSSI value: %f\n", RSSIvalue);
	}
}

void LoRa_TxDoneAction(void)
{
	LORAM_RX_Init();
}

void LoRa_TxStart(void)
{
	unsigned char i;
	unsigned char TxBuffer[LORA_PAYLOAD_SIZE];
	
	printf("Send sensor data to device with LoRa [OK]\n");
	
	TxBuffer[0] = 'S';
	TxBuffer[1] = 'E';
	TxBuffer[2] = 'N';
	TxBuffer[3] = 'S';
	
	TxBuffer[4] = 'd';
	TxBuffer[5] = 'e';
	TxBuffer[6] = 'v';
	TxBuffer[7] = 0x30 + getDeviceID();
	
	TxBuffer[8] = (uint8_t)(((ESENS_Get_Temperature() & 0xFF00) >> 8) & 0xFF);
	TxBuffer[9] = (uint8_t)(((ESENS_Get_Temperature() & 0x00FF)) & 0xFF);
	
	TxBuffer[10] = (uint8_t)(((ESENS_Get_Pressure() & 0xFF000000) >> 24) & 0xFF);
	TxBuffer[11] = (uint8_t)(((ESENS_Get_Pressure() & 0x00FF0000) >> 16) & 0xFF);
	TxBuffer[12] = (uint8_t)(((ESENS_Get_Pressure() & 0x0000FF00) >> 8) & 0xFF);
	TxBuffer[13] = (uint8_t)(((ESENS_Get_Pressure() & 0x000000FF)) & 0xFF);
	
	TxBuffer[14] = (uint8_t)(((ESENS_Get_Humidity() & 0xFF000000) >> 24) & 0xFF);
	TxBuffer[15] = (uint8_t)(((ESENS_Get_Humidity() & 0x00FF0000) >> 16) & 0xFF);
	TxBuffer[16] = (uint8_t)(((ESENS_Get_Humidity() & 0x0000FF00) >> 8) & 0xFF);
	TxBuffer[17] = (uint8_t)(((ESENS_Get_Humidity() & 0x000000FF)) & 0xFF);
	
	TxBuffer[18] = (uint8_t)(((ESENS_Get_Gas_Resistance() & 0xFF000000) >> 24) & 0xFF);
	TxBuffer[19] = (uint8_t)(((ESENS_Get_Gas_Resistance() & 0x00FF0000) >> 16) & 0xFF);
	TxBuffer[20] = (uint8_t)(((ESENS_Get_Gas_Resistance() & 0x0000FF00) >> 8) & 0xFF);
	TxBuffer[21] = (uint8_t)(((ESENS_Get_Gas_Resistance() & 0x000000FF)) & 0xFF);
	
	TxBuffer[22] = (uint8_t)(((ESENS_Get_Air_Quality_Score() & 0xFF00) >> 8) & 0xFF);
	TxBuffer[23] = (uint8_t)(((ESENS_Get_Air_Quality_Score() & 0x00FF)) & 0xFF);

  LORAM_SetTxPacket(TxBuffer, LORA_PAYLOAD_SIZE);
	
  LORAM_TX_Init();
}

void createLoRaTask(void)
{
	xTaskCreate(loraTask,	"loraTask", configMINIMAL_STACK_SIZE, ( void * )NULL, LORA_TASK_PRIORITY, NULL);
}

static void loraTask(void *pvParam)
{
	if (getDeviceID() != 0x00)
	{
		vTaskDelay( ( 3000 / portTICK_PERIOD_MS ) ); // 3000 ms delay
		LoRa_TxStart();
	}
	
	while (1)
	{
		if (getDeviceID() == 0x00)
		{
			if (getLoRaRxStartFlag()/* == TRUE*/)
			{
				setLoRaRxStartFlag(0/*FALSE*/);
				LORAM_RX_Done();
				LoRa_RxDoneAction();
				LORAM_RX_Init();
			}
			
			vTaskDelay( ( 50 / portTICK_PERIOD_MS ) ); // 50 ms delay
		}
		else
		{
			if (getLoRaTxStartFlag()/* == TRUE*/)
			{
				setLoRaTxStartFlag(0/*FALSE*/);
				LORAM_TX_Done();
				LoRa_TxDoneAction();
				LoRa_TxStart();
			}
			
			vTaskDelay( ( 1000 / portTICK_PERIOD_MS ) ); // 1000 ms delay
		}
	}
}
