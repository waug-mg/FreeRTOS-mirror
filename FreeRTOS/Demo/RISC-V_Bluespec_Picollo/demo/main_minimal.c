/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
 * NOTE 1:  This project provides two demo applications.  A simple blinky style
 * project, and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting in main.c is used to select
 * between the two.  See the notes on using mainCREATE_SIMPLE_BLINKY_DEMO_ONLY
 * in main.c.  This file implements the comprehensive test and demo version.
 *
 * NOTE 2:  This file only contains the source code that is specific to the
 * full demo.  Generic functions, such FreeRTOS hook functions, and functions
 * required to configure the hardware, are defined in main.c.
 *
 ******************************************************************************
 *
 * main_full() creates all the demo application tasks and software timers, then
 * starts the scheduler.  The web documentation provides more details of the
 * standard demo application tasks, which provide no particular functionality,
 * but do provide a good example of how to use the FreeRTOS API.
 *
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Reg test" tasks - These fill both the core registers with known values, then
 * check that each register maintains its expected value for the lifetime of the
 * task.  Each task uses a different set of values.  The reg test tasks execute
 * with a very low priority, so get preempted very frequently.  A register
 * containing an unexpected value is indicative of an error in the context
 * switching mechanism.
 *
 * "Check" task - The check executes every three seconds.  It checks that all
 * the standard demo tasks, and the register check tasks, are not only still
 * executing, but are executing without reporting any errors.  If the check task
 * discovers that a task has either stalled, or reported an error, then it
 * prints an error message to the UART, otherwise it prints "Pass.".
 */

/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Common demo includes. */
#include "AbortDelay.h"
#include "BlockQ.h"
#include "blocktim.h"
#include "countsem.h"
#include "death.h"
#include "dynamic.h"
// ? event groups ?
#include "integer.h"
#include "MessageBufferDemo.h"
// ? #include "mevents.h" ?
#include "PollQ.h"
#include "QPeek.h"
#include "recmutex.h"
#include "semtest.h"
// ? #include "StaticAllocation.h" ? 

/* The period after which the check timer will expire provided no errors have
been reported by any of the standard demo tasks.  ms are converted to the
equivalent in ticks using the portTICK_PERIOD_MS constant. */
#define mainCHECK_TIMER_PERIOD_MS			pdMS_TO_TICKS( 3000 )

/* A block time of zero simply means "don't block". */
#define mainDONT_BLOCK						( 0UL )

#define mainBLOCK_Q_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define mainSUICIDAL_TASK_PRIORITY		( tskIDLE_PRIORITY + 1)
#define mainINTEGER_TASK_PRIORITY		( tskIDLE_PRIORITY +1 )
#define mainQUEUE_POLL_PRIORITY			( tskIDLE_PRIORITY + 1)
#define mainSEM_TEST_PRIORITY			( tskIDLE_PRIORITY + 1)

void main_minimal( void );

/*-----------------------------------------------------------*/

/*
 * The check timer callback function, as described at the top of this file.
 */
static void prvCheckTimerCallback( TimerHandle_t xTimer );

void main_minimal( void )
{
	TimerHandle_t xCheckTimer = NULL;

	vCreateAbortDelayTasks();
	vStartBlockingQueueTasks(mainBLOCK_Q_PRIORITY);
	vCreateBlockTimeTasks();
	vStartCountingSemaphoreTasks();
	vStartDynamicPriorityTasks();
	vStartIntegerMathTasks(mainINTEGER_TASK_PRIORITY);
	vStartMessageBufferTasks(configMINIMAL_STACK_SIZE * 2);
	//vStartMultiEventTasks();
	vStartPolledQueueTasks(mainQUEUE_POLL_PRIORITY);
	vStartQueuePeekTasks();
	vStartRecursiveMutexTasks();
	vStartSemaphoreTasks(mainSEM_TEST_PRIORITY);
	//vStartStaticallyAllocatedTasks();

	/* Create the software timer that performs the 'check' functionality,
	as described at the top of this file. */
	xCheckTimer = xTimerCreate( "CheckTimer",					/* A text name, purely to help debugging. */
								( mainCHECK_TIMER_PERIOD_MS ),	/* The timer period, in this case 3000ms (3s). */
								pdTRUE,							/* This is an auto-reload timer, so xAutoReload is set to pdTRUE. */
								( void * ) 0,					/* The ID is not used, so can be set to anything. */
								prvCheckTimerCallback			/* The callback function that inspects the status of all the other tasks. */
							  );

	/* If the software timer was created successfully, start it.  It won't
	actually start running until the scheduler starts.  A block time of
	zero is used in this call, although any value could be used as the block
	time will be ignored because the scheduler has not started yet. */
	if( xCheckTimer != NULL )
	{
		xTimerStart( xCheckTimer, mainDONT_BLOCK );
	}

	// This has to be the last task created
	vCreateSuicidalTasks(mainSUICIDAL_TASK_PRIORITY);

	/* Start the kernel.  From here on, only tasks and interrupts will run. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the Idle and/or
	timer tasks to be created.  See the memory management section on the
	FreeRTOS web site for more details on the FreeRTOS heap
	http://www.freertos.org/a00111.html. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* See the description at the top of this file. */
static void prvCheckTimerCallback(__attribute__ ((unused)) TimerHandle_t xTimer )
{
static int count = 0;
unsigned long ulErrorFound = pdFALSE;

	/* Check all the demo and test tasks to ensure that they are all still
	running, and that none have detected an error. */

	if(xAreAbortDelayTestTasksStillRunning() != pdPASS)
	{
		printf("Error in xAreAbortDelayTestTasksStillRunning()\r\n");
		ulErrorFound |= ( 0x01UL << 1UL );
	}

	if(xAreBlockingQueuesStillRunning() != pdPASS)
	{
		printf("Error in xAreBlockingQueuesStillRunning()\r\n");
		ulErrorFound |= ( 0x01UL << 2UL );
	}

	if( xAreBlockTimeTestTasksStillRunning() != pdPASS )
	{
		printf("Error in xAreBlockTimeTestTasksStillRunning() \r\n");
		ulErrorFound |= ( 0x01UL << 3UL );
	}

	if( xAreCountingSemaphoreTasksStillRunning() != pdPASS )
	{
		printf("Error in xAreCountingSemaphoreTasksStillRunning() \r\n");
		ulErrorFound |= ( 0x01UL << 4UL );
	}

	if( xAreDynamicPriorityTasksStillRunning() != pdPASS )
	{
		printf("Error in xAreDynamicPriorityTasksStillRunning()\r\n");
		ulErrorFound |= ( 0x01UL << 5UL );
	}

	if( xIsCreateTaskStillRunning() != pdPASS )
	{
		printf("Error in xIsCreateTaskStillRunning() \r\n");
		ulErrorFound |= ( 0x01UL << 6UL );
	}

	if( xAreIntegerMathsTaskStillRunning() != pdPASS )
	{
		printf("Error in xAreIntegerMathsTaskStillRunning() \r\n");
		ulErrorFound |= ( 0x01UL << 7UL );
	}

	if( xAreMessageBufferTasksStillRunning() != pdPASS )
	{
		printf("Error in xAreMessageBufferTasksStillRunning() \r\n");
		ulErrorFound |= ( 0x01UL << 8UL );
	}
	/*
	if( xAreMultiEventTasksStillRunning() != pdPASS )
	{
		printf("Error in xAreMultiEventTasksStillRunning() \r\n");
		ulErrorFound |= ( 0x01UL << 9UL );
	}*/

	if( xArePollingQueuesStillRunning() != pdPASS )
	{
		printf("Error in xArePollingQueuesStillRunning() \r\n");
		ulErrorFound |= ( 0x01UL << 10UL );
	}

	if( xAreQueuePeekTasksStillRunning() != pdPASS )
	{
		printf("Error in xAreQueuePeekTasksStillRunning() \r\n");
		ulErrorFound |= ( 0x01UL << 11UL );
	}

	if( xAreRecursiveMutexTasksStillRunning() != pdPASS )
	{
		printf("Error in xAreRecursiveMutexTasksStillRunning() \r\n");
		ulErrorFound |= ( 0x01UL << 12UL );
	}

	if( xAreSemaphoreTasksStillRunning() != pdPASS )
	{
		printf("Error in xAreSemaphoreTasksStillRunning() \r\n");
		ulErrorFound |= ( 0x01UL << 13UL );
	}
	/*
	if( xAreStaticAllocationTasksStillRunning() != pdPASS )
	{
		printf("Error in xAreStaticAllocationTasksStillRunning() \r\n");
		ulErrorFound |= ( 0x01UL << 14UL );
	}*/

	if( ulErrorFound != pdFALSE )
	{
		__asm volatile("li t6, 0xbeefdead");
		printf("[%d] One or more threads has exited! \r\n", count);
	}else{
		__asm volatile("li t6, 0xdeadbeef");
		printf("[%d] All threads still alive! \r\n", count);
	}
	count++;
}