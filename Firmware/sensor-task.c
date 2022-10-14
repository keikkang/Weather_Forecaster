/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
//
#include "esensor.h"
//
#include "sensor-task.h"

#define	SENSOR_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )

SENSOR_DATA g_sensorData;

static void sensorTask(void *pvParam);

SENSOR_DATA* getSensorData(void)
{
	return &g_sensorData;
}

void setSensorData(SENSOR_DATA sensorData)
{
	g_sensorData = sensorData;
}

void createSensorTask(void)
{
	xTaskCreate(sensorTask,	"sensorTask", configMINIMAL_STACK_SIZE, ( void * )NULL, SENSOR_TASK_PRIORITY, NULL);
}

static void sensorTask(void *pvParam)
{
	while (1)
	{
		ESENS_Measure();
		
		vTaskDelay( ( 50 / portTICK_PERIOD_MS ) ); // 50 ms delay
	}
}
