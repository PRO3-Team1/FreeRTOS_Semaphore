/*
 * @ Semaphore interrupt example.
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "ms_timer.h"

xSemaphoreHandle second_isr_semaphore;

static void second_callback_isr() {
	//Special semaphore function to be used inside ISR's
	xSemaphoreGiveFromISR(second_isr_semaphore,NULL);
}

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	ms_timer_init (second_callback_isr);
}

/* Semaphore task. It is given a semaphore each second after a interrupt. */
static void sem_task(void *pvParameters) {
	int tickCnt = 0;

	while (1) {
		if(xSemaphoreTake(second_isr_semaphore, portMAX_DELAY)) {
			DEBUGOUT("Tick: %d\r\n", tickCnt);
			tickCnt++;
		}
	}
}

/**
 * @brief	main routine for Semaphore example
 * @return	Nothing, function should not exit
 */
int main(void)
{
	//Setup the semaphore
	vSemaphoreCreateBinary(second_isr_semaphore);
	prvSetupHardware();

	// Semaphore tasks. Prints ticks every second after receiving semaphore.
	xTaskCreate(sem_task, (signed char *) "sem_task",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

/**
 * @}
 */
