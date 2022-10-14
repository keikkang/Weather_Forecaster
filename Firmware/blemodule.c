#include <stdio.h>
#include <NuMicro.h>
#include "blemodule.h"

// Driver: MDBT42Q-PAT(Slave)

#define BLEM_UARTPU		(PE0 = 0)
#define BLEM_UARTPD		(PE0 = 1)

#define BLEM_RX_BUFFER_SIZE		128

volatile static uint8_t g_u8BleRxBuf[BLEM_RX_BUFFER_SIZE] = { 0, };
volatile static int16_t g_s16BleRxHead  = 0;
volatile static int16_t g_s16BleRxTail  = 0;

uint8_t BLEM_QueueEmpty(void);
uint8_t BLEM_QueueFull(void);
void BLEM_QueueClear(void);
void BLEM_QueuePush(uint8_t x);
uint8_t BLEM_QueuePop(void);
int16_t BLEM_QueueSize(void);

uint8_t BLEM_QueueEmpty(void)
{
	return (g_s16BleRxHead == g_s16BleRxTail);
}

uint8_t BLEM_QueueFull(void)
{
	return (((g_s16BleRxTail + 1) % BLEM_RX_BUFFER_SIZE) == g_s16BleRxHead);
}

void BLEM_QueueClear(void)
{
	g_s16BleRxHead = g_s16BleRxTail;
}

void BLEM_QueuePush(uint8_t x)
{
	if (BLEM_QueueFull())
	{
	}
	else
	{
		g_s16BleRxTail = ((g_s16BleRxTail + 1) % BLEM_RX_BUFFER_SIZE);
		g_u8BleRxBuf[g_s16BleRxTail] = x;
	}
}

uint8_t BLEM_QueuePop(void)
{
	if (BLEM_QueueEmpty())
	{
	}
	else
	{
		g_s16BleRxHead = ((g_s16BleRxHead + 1) % BLEM_RX_BUFFER_SIZE);
		return g_u8BleRxBuf[g_s16BleRxHead];
	}

	return 0;
}

int16_t BLEM_QueueSize(void)
{
	if (g_s16BleRxHead > g_s16BleRxTail)
		return (g_s16BleRxHead - g_s16BleRxTail);
	else if (g_s16BleRxHead < g_s16BleRxTail)
		return (g_s16BleRxTail - g_s16BleRxHead);
	
	return 0;
}

void BLEM_Init(void)
{
	// Unlock protected registers
  SYS_UnlockReg();
	
	// Enable UART3 clock
	CLK->APBCLK0 |= CLK_APBCLK0_UART3CKEN_Msk;
	
	// Switch UART3 clock source to HIRC
  CLK->CLKSEL3 = (CLK->CLKSEL3 & (~CLK_CLKSEL3_UART3SEL_Msk)) | CLK_CLKSEL3_UART3SEL_HIRC;
	
	// Set PD multi-function pins for UART3
	SYS->GPD_MFPL = (SYS->GPD_MFPL & ~(SYS_GPD_MFPL_PD0MFP_Msk | SYS_GPD_MFPL_PD1MFP_Msk | SYS_GPD_MFPL_PD2MFP_Msk | SYS_GPD_MFPL_PD3MFP_Msk)) | 
		(SYS_GPD_MFPL_PD0MFP_UART3_RXD | SYS_GPD_MFPL_PD1MFP_UART3_TXD | SYS_GPD_MFPL_PD2MFP_UART3_nCTS | SYS_GPD_MFPL_PD3MFP_UART3_nRTS);
	
	// Set PE multi-function pin for UARTPD(Please refer MDBT42Q-PAT datasheet)
	SYS->GPE_MFPL = (SYS->GPE_MFPL & ~(SYS_GPE_MFPL_PE0MFP_Msk)) | (SYS_GPE_MFPL_PE0MFP_GPIO);
	
	// Set PH multi-function pin for Wakeup(Please refer MDBT42Q-PAT datasheet)
	SYS->GPH_MFPH = (SYS->GPH_MFPH & ~(SYS_GPH_MFPH_PH8MFP_Msk)) | (SYS_GPH_MFPH_PH8MFP_GPIO);
	
	// UARTPD set output
	GPIO_SetMode(PE, BIT0, GPIO_MODE_OUTPUT);
	BLEM_UARTPD; // UARTPD set high
	
	// Lock protected registers
  SYS_LockReg();
	
	UART_Open(UART3, 9600);
	
	//
	memset(&g_u8BleRxBuf[0], 0, sizeof(g_u8BleRxBuf));
	g_s16BleRxHead  = 0;
	g_s16BleRxTail  = 0;
	
	// Enable RX interrupt
	UART_EnableInt(UART3, (UART_INTEN_RDAIEN_Msk));
}

void BLEM_UartRx_Handle(void)
{
    uint8_t u8InChar = 0xFF;
    uint32_t u32IntSts = UART3->INTSTS;

    // Receive Data Available Interrupt Handle    
    if(u32IntSts & UART_INTSTS_RDAINT_Msk)
    {
        // Get all the input characters
        while(UART_IS_RX_READY(UART3))
        {          
            // Receive Line Status Error Handle 
            if(u32IntSts & UART_INTSTS_RLSINT_Msk)
            {                
                // Clear Receive Line Status Interrupt
                UART_ClearIntFlag(UART3, UART_INTSTS_RLSINT_Msk);   
            }              
            
            // Get the character from UART Buffer
            u8InChar =  UART_READ(UART3);

            BLEM_QueuePush(u8InChar);
        }
    }

    // Buffer Error Interrupt Handle    
    if (u32IntSts & UART_INTSTS_BUFERRINT_Msk)   
    {
        // Clear Buffer Error Interrupt
        UART_ClearIntFlag(UART3, UART_INTSTS_BUFERRINT_Msk);             
    }
}

#ifdef BLEM_TEST
void BLEM_BypassUART(void)
{
	uint8_t u8Buffer;
	
	// UART0: PC
	// UART3: BLE Module
	BLEM_UARTPU; // UARTPD set low
	if (UART_Read(UART0, &u8Buffer, 1))
		UART_Write(UART3, &u8Buffer, 1);
	if (BLEM_QueueSize() > 0)
	{
		u8Buffer = BLEM_QueuePop();
		UART_Write(UART0, &u8Buffer, 1);
	}
}
#endif
