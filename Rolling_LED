/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
*                                       IAR Development Kits
*                                              on the
*
*                                    STM32F429II-SK KICKSTART KIT
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : YS
*                 DC
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <includes.h>
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx.h"
#include "lib_str.h"
/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  APP_TASK_EQ_0_ITERATION_NBR              16u
/*
*********************************************************************************************************
*                                            TYPES DEFINITIONS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void  AppTaskStart          (void     *p_arg);
static  void  AppTaskCreate         (void);
static  void  AppObjCreate          (void);

static void UsartTask(void *p_arg);

static void Rolling_LED_Task(void* p_arg);

static void ButtonTask(void *p_arg);

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
/* ----------------- APPLICATION GLOBALS -------------- */
static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB   UsartTask_TCB;
static  CPU_STK	 UsartTask_Stack[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB	 Rolling_LED_Task_TCB;
static  CPU_STK  Rolling_LED_Task_Stack[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB	 ButtonTask_TCB;
static  CPU_STK  ButtonTask_Stack[APP_CFG_TASK_START_STK_SIZE];

/* ------------ FLOATING POINT TEST TASK -------------- */
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/
OS_SEM MySem;

int main(void)
{
    OS_ERR  err;

    /* Basic Init */
    RCC_DeInit();
//    SystemCoreClockUpdate();

    /* BSP Init */
    BSP_IntDisAll();                                            /* Disable all interrupts.                              */

    CPU_Init();                                                 /* Initialize the uC/CPU Services                       */
    Mem_Init();                                                 /* Initialize Memory Management Module                  */
    Math_Init();                                                /* Initialize Mathematical Module                       */


    /* OS Init */
    OSInit(&err);                                               /* Init uC/OS-III.                                      */

    OSSemCreate(&MySem,
    			"My Semaphore",
				1,
				&err);

    OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,              /* Create the start task                                */
                 (CPU_CHAR     *)"App Task Start",
                 (OS_TASK_PTR   )AppTaskStart,
                 (void         *)0u,
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO,
                 (CPU_STK      *)&AppTaskStartStk[0u],
                 (CPU_STK_SIZE  )AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE / 10u],
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY    )0u,
                 (OS_TICK       )0u,
                 (void         *)0u,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

   OSStart(&err);   /* Start multitasking (i.e. give control to uC/OS-III). */

   (void)&err;

   return (0u);
}
/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
static  void  AppTaskStart (void *p_arg)
{
    OS_ERR  err;

   (void)p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                             */
    CPU_Init();

    BSP_Tick_Init();                                            /* Initialize Tick Services.                            */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

   // BSP_LED_Off(0u);                                            /* Turn Off LEDs after initialization                   */

   APP_TRACE_DBG(("Creating Application Kernel Objects\n\r"));
   AppObjCreate();                                             /* Create Applicaiton kernel objects                    */

   APP_TRACE_DBG(("Creating Application Tasks\n\r"));
   AppTaskCreate();                                            /* Create Application tasks                             */
}



/*
*********************************************************************************************************
*/


char cmd[10];
int solve;
static void UsartTask(void* p_arg) {
	OS_ERR err;
	CPU_TS ts;

	while (DEF_TRUE) {
		char c;
		int index = 0;

		if (!solve) {
			OSSemPend(&MySem,
				0,
				OS_OPT_PEND_BLOCKING,
				&ts,
				&err);
			send_string("\n\rchoose LEDx\n\r");
			while (DEF_TRUE) {
				USART_ReceiveData(Nucleo_COM1);
				while (USART_GetFlagStatus(Nucleo_COM1, USART_FLAG_RXNE) == RESET) {}

				c = USART_ReceiveData(Nucleo_COM1);
				USART_SendData(Nucleo_COM1, c);
				cmd[index] = c;
				index++;

				if (index == 4) {
					if (Str_Cmp_N("led", cmd, 3) == 0) {
						break;
					}
					else {
						index = 0;
						send_string("Retry\n\r");
					}
				}
			}
			send_string("\n\r");
			OSSemPost(&MySem,
				OS_OPT_POST_1,
				&err);
			solve = 1;

			OSTimeDlyHMSM(0u, 0u, 1u, 0u,
				OS_OPT_TIME_HMSM_STRICT,
				&err);
		}
	}
}


/*
*********************************************************************************************************
*/


int led;
static void Rolling_LED_Task(void* p_arg){
    OS_ERR  err;
	CPU_TS ts;

    led = 1;

    while (DEF_TRUE) {  /* Task body, always written as an infinite loop.       */
		OSSemPend(&MySem,
			0,
			OS_OPT_PEND_BLOCKING,
			&ts,
			&err);
		if(led > 3){
    		led = 1;
    	}
    	
		BSP_LED_On(led);
		
		OSSemPost(&MySem,
			OS_OPT_POST_1,
			&err);
    	
		OSTimeDlyHMSM(0u, 0u, 0u, 500u,
            OS_OPT_TIME_HMSM_STRICT,
            &err);
    	
		BSP_LED_Off(led);
    	led++;
    }
}


/*
*********************************************************************************************************
*/


static void ButtonTask(void *p_arg){
    OS_ERR  err;
 	CPU_TS ts;
	solve = 0;

    int button = 0;
    while (DEF_TRUE) {
    	while (solve) {
			button = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);

			while (button == 1) {
				OSSemPend(&MySem,
					0,
					OS_OPT_PEND_BLOCKING,
					&ts,
					&err);

				if (led == cmd[3] - 48) {
					BSP_LED_On(0);
					send_string("\n\rCorrect!!");
					solve = 0;
				}
				else {
					BSP_LED_Off(0);
					send_string("\n\rWrong and Retry!!");
				}
				OSSemPost(&MySem,
					OS_OPT_POST_1,
					&err);
				button = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);
			}
			OSTimeDlyHMSM(0u, 0u, 0u, 500u,
				OS_OPT_TIME_HMSM_STRICT,
				&err);
		}
		
		OSTimeDlyHMSM(0u, 0u, 0u, 500u,
				OS_OPT_TIME_HMSM_STRICT,
	            &err);
    }
}


/*
*********************************************************************************************************
*                                          AppTaskCreate()
*
* Description : Create application tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
	OS_ERR  err;

	OSTaskCreate((OS_TCB*)&UsartTask_TCB,
		(CPU_CHAR*)"UsartTask",
		(OS_TASK_PTR)UsartTask,
		(void*)0u,
		(OS_PRIO)2u,
		(CPU_STK*)&UsartTask_Stack[0u],
		(CPU_STK_SIZE)UsartTask_Stack[APP_CFG_TASK_START_STK_SIZE / 10u],
		(CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE,
		(OS_MSG_QTY)0u,
		(OS_TICK)0u,
		(void*)0u,
		(OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		(OS_ERR*)&err);


	OSTaskCreate((OS_TCB*)&ButtonTask_TCB,
		(CPU_CHAR*)"ButtonTask",
		(OS_TASK_PTR)ButtonTask,
		(void*)0u,
		(OS_PRIO)1u,
		(CPU_STK*)&ButtonTask_Stack[0u],
		(CPU_STK_SIZE)ButtonTask_Stack[APP_CFG_TASK_START_STK_SIZE / 10u],
		(CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE,
		(OS_MSG_QTY)0u,
		(OS_TICK)0u,
		(void*)0u,
		(OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		(OS_ERR*)&err);


	OSTaskCreate((OS_TCB*)&Rolling_LED_Task_TCB,
		(CPU_CHAR*)"RollingLED",
		(OS_TASK_PTR)Rolling_LED_Task,
		(void*)0u,
		(OS_PRIO)0u,
		(CPU_STK*)&Rolling_LED_Task_Stack[0u],
		(CPU_STK_SIZE)Rolling_LED_Task_Stack[APP_CFG_TASK_START_STK_SIZE / 10u],
		(CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE,
		(OS_MSG_QTY)0u,
		(OS_TICK)0u,
		(void*)0u,
		(OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		(OS_ERR*)&err);

}


/*
*********************************************************************************************************
*                                          AppObjCreate()
*
* Description : Create application kernel objects tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppObjCreate (void)
{

}
