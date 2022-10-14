#ifndef __BLEMODULE_H_
#define __BLEMODULE_H_

//#define BLEM_TEST // Bypass AT(UART) command for BLE Module test

void BLEM_Init(void);
void BLEM_UartRx_Handle(void);
#ifdef BLEM_TEST
void BLEM_BypassUART(void);
#endif

#endif //__BLEMODULE_H_
