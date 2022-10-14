/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
//
#include "esensor.h"
#include "wifimodule.h"
//
#include "sensor-task.h"
#include "wifi-task.h"

static void wifiTask(void *pvParam);

uint8_t connectToAP(void);
uint8_t connectToServer(void);
uint8_t sendSensorData(void);
void makePacket(uint8_t* pu8Data, SENSOR_DATA* pSensorData);

#define	WIFI_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )

#define WIFI_PACKET_SIZE 			( 45 )

typedef enum {
	WS_UNKNOW = 0,
	WS_NOT_CONNECTED_TO_AP,
	WS_CONNECTED_TO_AP,
	WS_NOT_CONNECTED_TO_SERVER,
	WS_CONNECTED_TO_SERVER
} WIFI_STATUS;

WIFI_STATUS g_wifiStatus = WS_UNKNOW;

uint8_t connectToAP(void)
{
	uint8_t u8Ret = 0;
	
	printf("Connect To AP... ");
	
	//u8Ret = WIFIM_CheckReady();
	//if (u8Ret != 0)
	//	return u8Ret;
	
	WIFIM_Send("AT+CWMODE=1\r\n", 14);
	u8Ret = WIFIM_CheckOK();
	if (u8Ret != 0)
		return u8Ret;
	
	vTaskDelay( ( 1000 / portTICK_PERIOD_MS ) ); // 1000 ms delay
	
	WIFIM_Send("AT+CWJAP=\"SJ1F2G\",\"sj6800win\"\r\n", 32);
	u8Ret = WIFIM_CheckOK();
	if (u8Ret != 0)
		return u8Ret;

	printf("[Connected]\n");
	
	return 0;
}

uint8_t connectToServer(void)
{
	uint8_t u8Ret = 0;
	
	printf("Connect To Server... ");
	
	WIFIM_Send("AT+CIPSTART=\"TCP\",\"192.68.20.20\",3360\r\n", 40);
	u8Ret = WIFIM_CheckOK();
	if (u8Ret != 0)
		return u8Ret;
	
	printf("[Connected]\n");
	
	return 0;
}

void makePacket(uint8_t* pu8Data, SENSOR_DATA* pSensorData)
{
	uint8_t i;
	
	pu8Data[0] = 'S';
	pu8Data[1] = 'E';
	pu8Data[2] = 'N';
	pu8Data[3] = 'S';
	
	// size
	pu8Data[4] = 2;
	
	for (i=0; i<2; i++)
	{
		pu8Data[5 + (20 * i)] = 'd';
		pu8Data[6 + (20 * i)] = 'e';
		pu8Data[7 + (20 * i)] = 'v';
		
		if (i == 0)
		{
			pu8Data[8 + (20 * i)] = 0x30 + 0x00/*g_u8DeviceID*/;
			
			pu8Data[9 + (20 * i)] = (uint8_t)(((ESENS_Get_Temperature() & 0xFF00) >> 8) & 0xFF);
			pu8Data[10 + (20 * i)] = (uint8_t)(((ESENS_Get_Temperature() & 0x00FF)) & 0xFF);
			
			pu8Data[11 + (20 * i)] = (uint8_t)(((ESENS_Get_Pressure() & 0xFF000000) >> 24) & 0xFF);
			pu8Data[12 + (20 * i)] = (uint8_t)(((ESENS_Get_Pressure() & 0x00FF0000) >> 16) & 0xFF);
			pu8Data[13 + (20 * i)] = (uint8_t)(((ESENS_Get_Pressure() & 0x0000FF00) >> 8) & 0xFF);
			pu8Data[14 + (20 * i)] = (uint8_t)(((ESENS_Get_Pressure() & 0x000000FF)) & 0xFF);
			
			pu8Data[15 + (20 * i)] = (uint8_t)(((ESENS_Get_Humidity() & 0xFF000000) >> 24) & 0xFF);
			pu8Data[16 + (20 * i)] = (uint8_t)(((ESENS_Get_Humidity() & 0x00FF0000) >> 16) & 0xFF);
			pu8Data[17 + (20 * i)] = (uint8_t)(((ESENS_Get_Humidity() & 0x0000FF00) >> 8) & 0xFF);
			pu8Data[18 + (20 * i)] = (uint8_t)(((ESENS_Get_Humidity() & 0x000000FF)) & 0xFF);
			
			pu8Data[19 + (20 * i)] = (uint8_t)(((ESENS_Get_Gas_Resistance() & 0xFF000000) >> 24) & 0xFF);
			pu8Data[20 + (20 * i)] = (uint8_t)(((ESENS_Get_Gas_Resistance() & 0x00FF0000) >> 16) & 0xFF);
			pu8Data[21 + (20 * i)] = (uint8_t)(((ESENS_Get_Gas_Resistance() & 0x0000FF00) >> 8) & 0xFF);
			pu8Data[22 + (20 * i)] = (uint8_t)(((ESENS_Get_Gas_Resistance() & 0x000000FF)) & 0xFF);
			
			pu8Data[23 + (20 * i)] = (uint8_t)(((ESENS_Get_Air_Quality_Score() & 0xFF00) >> 8) & 0xFF);
			pu8Data[24 + (20 * i)] = (uint8_t)(((ESENS_Get_Air_Quality_Score() & 0x00FF)) & 0xFF);
		}
		else
		{
			pu8Data[8 + (20 * i)] = 0x30 + pSensorData->u8DeviceID;
			
			pu8Data[9 + (20 * i)] = pSensorData->u8Temperature[0];
			pu8Data[10 + (20 * i)] = pSensorData->u8Temperature[1];
			
			pu8Data[11 + (20 * i)] = pSensorData->u8Pressure[0];
			pu8Data[12 + (20 * i)] = pSensorData->u8Pressure[1];
			pu8Data[13 + (20 * i)] = pSensorData->u8Pressure[2];
			pu8Data[14 + (20 * i)] = pSensorData->u8Pressure[3];
			
			pu8Data[15 + (20 * i)] = pSensorData->u8Humidity[0];
			pu8Data[16 + (20 * i)] = pSensorData->u8Humidity[1];
			pu8Data[17 + (20 * i)] = pSensorData->u8Humidity[2];
			pu8Data[18 + (20 * i)] = pSensorData->u8Humidity[3];
			
			pu8Data[19 + (20 * i)] = pSensorData->u8Resistrance[0];
			pu8Data[20 + (20 * i)] = pSensorData->u8Resistrance[1];
			pu8Data[21 + (20 * i)] = pSensorData->u8Resistrance[2];
			pu8Data[22 + (20 * i)] = pSensorData->u8Resistrance[3];
			
			pu8Data[23 + (20 * i)] = pSensorData->u8AirQualityScore[0];
			pu8Data[24 + (20 * i)] = pSensorData->u8AirQualityScore[1];
		}
	}
}

uint8_t sendSensorData(void)
{
	uint8_t u8Ret = 0;
	uint8_t u8PacketData[WIFI_PACKET_SIZE];
	
	printf("Send Sensor Data ");
	
	makePacket(&u8PacketData[0], getSensorData());
	
	WIFIM_Send("AT+CIPSEND=45\r\n", 15);
	u8Ret = WIFIM_CheckOK();
	if (u8Ret != 0)
		return u8Ret;
	
	WIFIM_Send(&u8PacketData[0], WIFI_PACKET_SIZE);
	u8Ret = WIFIM_CheckOK();
	if (u8Ret != 0)
		return u8Ret;
	
	printf("[Done] \n");
	
	return 0;
}

void createWiFiTask(void)
{
	xTaskCreate(wifiTask,	"sensorTask", configMINIMAL_STACK_SIZE, ( void * )NULL, WIFI_TASK_PRIORITY, NULL);
}

static void wifiTask(void *pvParam)
{
	uint8_t u8Ret = 0;
	
	while (1)
	{
		switch (g_wifiStatus)
		{
			case WS_UNKNOW:
				g_wifiStatus = WS_NOT_CONNECTED_TO_AP;
				WIFIM_QueueClear();
				WIFIM_Reset();
				vTaskDelay( ( 50 / portTICK_PERIOD_MS ) ); // 50 ms delay
				break;
			case WS_NOT_CONNECTED_TO_AP:
				u8Ret = connectToAP();
				if (u8Ret == 0)
				{
					g_wifiStatus = WS_CONNECTED_TO_AP;
				}
				else if (u8Ret == 1)
				{
					g_wifiStatus = WS_NOT_CONNECTED_TO_AP;
				}
				else
				{
					g_wifiStatus = WS_NOT_CONNECTED_TO_AP;
				}
				vTaskDelay( ( 3000 / portTICK_PERIOD_MS ) ); // 3000 ms delay
				break;
			case WS_CONNECTED_TO_AP:
			case WS_NOT_CONNECTED_TO_SERVER:
				if (connectToServer() == 0)
				{
					g_wifiStatus = WS_CONNECTED_TO_SERVER;
				}
				else
				{
					g_wifiStatus = WS_NOT_CONNECTED_TO_SERVER;
				}
				vTaskDelay( ( 3000 / portTICK_PERIOD_MS ) ); // 3000 ms delay
				break;
			case WS_CONNECTED_TO_SERVER:
				if (sendSensorData() == 2)
					g_wifiStatus = WS_NOT_CONNECTED_TO_SERVER;
				
				vTaskDelay( ( 3000 / portTICK_PERIOD_MS ) ); // 3000 ms delay
				break;
			default:
				g_wifiStatus = WS_UNKNOW;
				vTaskDelay( ( 50 / portTICK_PERIOD_MS ) ); // 50 ms delay
				break;
		}
	}
}
