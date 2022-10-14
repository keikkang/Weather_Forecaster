/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
//
#include "main-task.h"

/* Priorities at which the tasks are created. */
#define	MAIN_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

volatile static uint8_t g_u8BlinkFlag = 0;

/*
 * The tasks as described in the comments at the top of this file.
 */
static void mainTask(void *pvParam);

void createMainTask(void)
{
	xTaskCreate(mainTask,				/* The function that implements the task. */
		"mainTask", 							/* The text name assigned to the task - for debug only as it is not used by the kernel. */
		configMINIMAL_STACK_SIZE, /* The size of the stack to allocate to the task. */
		( void * )NULL, 					/* The parameter passed to the task - just to check the functionality. */
		MAIN_TASK_PRIORITY, 			/* The priority assigned to the task. */
		NULL);										/* The task handle is not required, so NULL is passed. */
}

void setBlinkFlag(uint8_t u8Value)
{
	g_u8BlinkFlag = u8Value;
}

static void mainTask(void *pvParam)
{
	while (1)
	{
		//printf("\n\nmainTask ...\n");
		//TIMER_Delay(TIMER0, 10000000);
		
		if (g_u8BlinkFlag)
			PB10 = 0;
		else
			PB10 = 1;
	}
}