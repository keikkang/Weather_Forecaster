#ifndef __LORA_TASK_H_
#define __LORA_TASK_H_

//
//
//
void setLoRaTxStartFlag(uint8_t u8Value);
void setLoRaRxStartFlag(uint8_t u8Value);
uint8_t getLoRaTxStartFlag(void);
uint8_t getLoRaRxStartFlag(void);

void createLoRaTask(void);

#endif //__LORA_TASK_H_
