#ifndef __SENSOR_TASK_H_
#define __SENSOR_TASK_H_

//
typedef struct tagSensor_Data {
	uint8_t u8DeviceID;
	uint8_t u8Temperature[2];
	uint8_t u8Pressure[4];
	uint8_t u8Humidity[4];
	uint8_t u8Resistrance[4];
	uint8_t u8AirQualityScore[2];
} SENSOR_DATA;

//
//
//
SENSOR_DATA* getSensorData(void);
void setSensorData(SENSOR_DATA sensorData);

void createSensorTask(void);

#endif //__SENSOR_TASK_H_