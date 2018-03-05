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

// pins 5-8
#define GPIO_PORT1 1
#define GPIO_PIN1 24
#define GPIO_PORT2 1
#define GPIO_PIN2 23
#define GPIO_PORT3 1
#define GPIO_PIN3 20
#define GPIO_PORT4 0
#define GPIO_PIN4 21

static void second_callback_isr() {
	//Special semaphore function to be used inside ISR's
	long task_woken;
	xSemaphoreGiveFromISR(second_isr_semaphore, &task_woken);
	if(task_woken) {
		Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT2, GPIO_PIN2, 1);
		Board_LED_Toggle(3);
		Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT2, GPIO_PIN2, 0);
		taskYIELD();
	}
}

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_PORT1, GPIO_PIN1, IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, GPIO_PORT1, GPIO_PIN1);

	Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_PORT2, GPIO_PIN2, IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, GPIO_PORT2, GPIO_PIN2);

	Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_PORT3, GPIO_PIN3, IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, GPIO_PORT3, GPIO_PIN3);

	Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_PORT4, GPIO_PIN4, IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, GPIO_PORT4, GPIO_PIN4);

	ms_timer_init (second_callback_isr);


}

/* Semaphore task. It is given a semaphore each second after a interrupt. */
static void sem_task(void *pvParameters) {
	while (1) {
		if(xSemaphoreTake(second_isr_semaphore, portMAX_DELAY)) {
			Board_LED_Toggle(1);
		}
	}
}

/* Low priority tasks... Tries to take a lot of CPU time*/
static void lowpri_task(void *pvParameters) {
	while (1) {
		Board_LED_Toggle(2);
		Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT1, GPIO_PIN1, 1);
		for(long i = 0; i <6000000; i++);
		Chip_GPIO_WritePortBit(LPC_GPIO, GPIO_PORT1, GPIO_PIN1, 0);
		Board_LED_Toggle(2);
		vTaskDelay(configTICK_RATE_HZ / 20); //~10Hz
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
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2UL),
				(xTaskHandle *) NULL);
	xTaskCreate(lowpri_task, (signed char *) "lowpri_task",
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
