#include <stdio.h>
#include <string.h>
#include <NuMicro.h>
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
//
#include "esensor.h"
#include "wifimodule.h"
#include "loramodule.h"
//
#include "main-task.h"
#include "sensor-task.h"
#include "wifi-task.h"
#include "lora-task.h"

#include "main.h"

//
//
//
void TMR1_IRQHandler(void);
void UART4_IRQHandler(void);
void GPF_IRQHandler(void);

void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
void vApplicationTickHook(void);

void SYS_Init(void);
void DEBUG_Init(void);
void TIMER_Init(void);

void readDeviceID(void);

//
//
//
static uint8_t g_u8DeviceID = 0x00;

void TMR1_IRQHandler(void)
{
	static uint8_t u8Value = 0;
	
	if (TIMER_GetIntFlag(TIMER1) == 1)
	{
		/* Clear Timer1 time-out interrupt flag */
		TIMER_ClearIntFlag(TIMER1);
		
		u8Value = !u8Value;
		setBlinkFlag(u8Value);
  }
}

void UART4_IRQHandler(void)
{
	WIFIM_UartRx_Handle();
}

void GPF_IRQHandler(void)
{
	/* To check if PF.7 interrupt occurred */
	if(GPIO_GET_INT_FLAG(PF, BIT7))
	{
		GPIO_CLR_INT_FLAG(PF, BIT7);
		//printf("PF.7 INT occurred.\n");
		
		if (SX1276LoRaGetRFState() == RFLR_STATE_RX_RUNNING)
    {
			setLoRaRxStartFlag(1/*TRUE*/);
		}
		else if (SX1276LoRaGetRFState() == RFLR_STATE_TX_RUNNING)
		{
			setLoRaTxStartFlag(1/*TRUE*/);
		}
	}
	else
	{
		/* Un-expected interrupt. Just clear all PE interrupts */
		PF->INTSRC = PF->INTSRC;
		//printf("Un-expected interrupts.\n");
  }
}

void SYS_Init(void)
{
	SYS_UnlockReg();

	// Enable HIRC clock (Internal RC 48MHz)
  CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
	// Wait for HIRC clock ready
  CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
	
	// Select HCLK clock source as HIRC and HCLK source divider as 1
  CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));
	
	CLK_SetSysTickClockSrc(CLK_CLKSEL0_HCLKSEL_HIRC);
	SysTick_Config(SystemCoreClock / 1);
	
	// Enable UART0 clock
  CLK_EnableModuleClock(UART0_MODULE);

  // Switch UART0 clock source to HIRC
  CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));
	
	// Enable TIMER module clock for delay
  CLK_EnableModuleClock(TMR0_MODULE);
  CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);
	
	// Enable TIMER module clock for count
  CLK_EnableModuleClock(TMR1_MODULE);
  CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_HIRC, 0);
	
	// Update System Core Clock
	SystemCoreClockUpdate();
	
	// Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13
	SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk)) | (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);
	
	// Set PB multi-function pins for GPIO(Device id)
	SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB7MFP_Msk | SYS_GPB_MFPL_PB6MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk | SYS_GPB_MFPL_PB4MFP_Msk)) | 
		(SYS_GPB_MFPL_PB7MFP_GPIO | SYS_GPB_MFPL_PB6MFP_GPIO | SYS_GPB_MFPL_PB5MFP_GPIO | SYS_GPB_MFPL_PB4MFP_GPIO);
	GPIO_SetMode(PB, BIT7, GPIO_MODE_INPUT);
	GPIO_SetMode(PB, BIT6, GPIO_MODE_INPUT);
	GPIO_SetMode(PB, BIT5, GPIO_MODE_INPUT);
	GPIO_SetMode(PB, BIT4, GPIO_MODE_INPUT);
	
	GPIO_SetMode(PB, BIT10, GPIO_MODE_OUTPUT);
	PB10 = 0;
	
	// Lock protected registers
  SYS_LockReg();
}

void DEBUG_Init(void)
{
	// Init UART0 to 115200-8n1 for print message
	UART_Open(UART0, 115200);
}

void TIMER_Init(void)
{
	TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1);
	TIMER_EnableInt(TIMER1);
	NVIC_EnableIRQ(TMR1_IRQn);
	TIMER_Start(TIMER1);
}

void readDeviceID(void)
{
	g_u8DeviceID |= (PB7 << 3) | (PB6 << 2) | (PB5 << 1) | (PB4 << 0);
}

uint8_t getDeviceID(void)
{
	return g_u8DeviceID;
}

int main(void)
{
	SYS_Init();
	DEBUG_Init();
	TIMER_Init();
	
	readDeviceID();
	
	ESENS_Init();
	
	printf("\n\nHello world! NuMaker IoT M263A with FreeRTOS\n");
	printf("CPU @ %d Hz\n", SystemCoreClock);
	printf("Device ID: 0x%02x\n", getDeviceID());
	printf("Environment sensor chip ID: 0x%02x\n", ESENS_ReadChipID());
	ESENS_Get_Calib_Data();
	ESENS_Measure_Init();
	
	LORAM_Init();
	if (getDeviceID() == 0x00)
	{
		// Receiver
		LORAM_RX_Init();
		WIFIM_Init();
	}

	createMainTask();
	createSensorTask();
	if (getDeviceID() == 0x00)
		createWiFiTask();
	createLoRaTask();
	
	/* Start the tasks and timer running. */
	vTaskStartScheduler();
	
	while (1);

	return 0;
}

//
//
//
void vApplicationMallocFailedHook(void)
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

void vApplicationIdleHook(void)
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
  __BKPT();
  printf("Stack overflow task name=%s\n", pcTaskName);

	for( ;; );
}

void vApplicationTickHook(void)
{
	/* This function will be called by each tick interrupt if
	configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	added here, but the tick hook is called from an interrupt context, so
	code must not attempt to block, and only the interrupt safe FreeRTOS API
	functions can be used (those that end in FromISR()).  The code in this
	tick hook implementation is for demonstration only - it has no real
	purpose.  It just gives a semaphore every 50ms.  The semaphore unblocks a
	task that then toggles an LED.  Additionally, the call to
	vQueueSetAccessQueueSetFromISR() is part of the "standard demo tasks"
	functionality. */
}
