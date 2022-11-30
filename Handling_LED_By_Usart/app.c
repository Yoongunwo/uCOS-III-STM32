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
typedef enum {
   TASK_500MS,
   TASK_1000MS,
   TASK_2000MS,

   TASK_N
}task_e;
typedef struct
{
   CPU_CHAR* name;
   OS_TASK_PTR func;
   OS_PRIO prio;
   CPU_STK* pStack;
   OS_TCB* pTcb;
}task_t;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void  AppTaskStart          (void     *p_arg);
static  void  AppTaskCreate         (void);
static  void  AppObjCreate          (void);

static void UsartTask(void *p_arg);

static void LED1Task(void *p_arg);
static void LED2Task(void *p_arg);
static void LED3Task(void *p_arg);

static void Setup_Gpio(void);


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

static  OS_TCB   LED1Task_TCB;
static  OS_TCB   LED2Task_TCB;
static  OS_TCB   LED3Task_TCB;

static  CPU_STK  LED1Task_Stack[APP_CFG_TASK_START_STK_SIZE];
static  CPU_STK  LED2Task_Stack[APP_CFG_TASK_START_STK_SIZE];
static  CPU_STK  LED3Task_Stack[APP_CFG_TASK_START_STK_SIZE];

int count=0;
task_t cyclic_tasks[TASK_N] = {
   {"LED1Task" , LED1Task,  0, &LED1Task_Stack[0] , &LED1Task_TCB},
   {"LED2Task", LED2Task, 0, &LED2Task_Stack[0], &LED2Task_TCB},
   {"LED3Task", LED3Task, 0, &LED3Task_Stack[0], &LED3Task_TCB},
};
/* ------------ FLOATING POINT TEST TASK -------------- */
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none;
* Returns     : none
*********************************************************************************************************
*/

int main(void)
{
    OS_ERR  err;

    /* Basic Init */
    RCC_DeInit();
//    SystemCoreClockUpdate();
    Setup_Gpio();

    /* BSP Init */
    BSP_IntDisAll();                                            /* Disable all interrupts.                              */

    CPU_Init();                                                 /* Initialize the uC/CPU Services                       */
    Mem_Init();                                                 /* Initialize Memory Management Module                  */
    Math_Init();                                                /* Initialize Mathematical Module                       */


    /* OS Init */
    OSInit(&err);                                               /* Init uC/OS-III.                                      */

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
*                                          AppTask_500ms
*
* Description : Example of 500mS Task
*
* Arguments   : p_arg (unused)
*
* Returns     : none
*
* Note: Long period used to measure timing in person
*********************************************************************************************************
*/
static int LED1CMD = 1;
static int LED1_T = 1;
static int LED2CMD = 1;
static int LED2_T = 1;
static int LED3CMD = 1;
static int LED3_T = 1;

static void LED1Task(void *p_arg)
{
    OS_ERR  err;
	CPU_SR_ALLOC();
    BSP_LED_On(1);
    while (DEF_TRUE) {    
		CPU_CRITICAL_ENTER();
		if (LED1CMD == 2) {
			BSP_LED_Toggle(1);
		}
		else if (LED1CMD == 1) {
			BSP_LED_Off(1);
		}else{
			BSP_LED_On(1);
		}
		CPU_CRITICAL_EXIT();
		OSTimeDlyHMSM(0u, 0u, LED1_T, 0,
		   OS_OPT_TIME_HMSM_STRICT,
		   &err);
    }
}

/*
*********************************************************************************************************
*                                          AppTask_1000ms
*
* Description : Example of 1000mS Task
*
* Arguments   : p_arg (unused)
*
* Returns     : none
*
* Note: Long period used to measure timing in person
*********************************************************************************************************
*/
static void LED2Task(void *p_arg)
{
    OS_ERR  err;
	CPU_SR_ALLOC();
    BSP_LED_On(2);
    while (DEF_TRUE) {
		CPU_CRITICAL_ENTER();
		if (LED2CMD == 2) {
			BSP_LED_Toggle(2);
		}
		else if (LED2CMD == 1) {
			BSP_LED_Off(2);
		}else{
			BSP_LED_On(2);
		}
		CPU_CRITICAL_EXIT();
	    OSTimeDlyHMSM(0u, 0u, LED2_T, 0u,
		   OS_OPT_TIME_HMSM_STRICT,
		   &err);
    }
}

/*
*********************************************************************************************************
*                                          AppTask_2000ms
*
* Description : Example of 2000mS Task
*
* Arguments   : p_arg (unused)
*
* Returns     : none
*
* Note: Long period used to measure timing in person
*********************************************************************************************************
*/
static void LED3Task(void *p_arg)
{
    OS_ERR  err;
	CPU_SR_ALLOC();
    BSP_LED_On(3);
    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */

		CPU_CRITICAL_ENTER();
		if (LED3CMD == 2) {
			BSP_LED_Toggle(3);
		}
		else if (LED3CMD == 1) {
			BSP_LED_Off(3);
		}else{
			BSP_LED_On(3);
		}
		CPU_CRITICAL_EXIT();
	   OSTimeDlyHMSM(0u, 0u, LED3_T, 0,
		   OS_OPT_TIME_HMSM_STRICT,
		   &err);
    }
}


static void UsartTask(void *p_arg){
	OS_ERR err;
	CPU_SR_ALLOC();

	while(1){
		char cmd[10];
		int index = 0;
		char c;
		int flag = 0;
		while(flag == 0){
			USART_ReceiveData(Nucleo_COM1);
			while(USART_GetFlagStatus(Nucleo_COM1, USART_FLAG_RXNE)==RESET){}

			c = USART_ReceiveData(Nucleo_COM1);
			USART_SendData(Nucleo_COM1,c);
			cmd[index++] = c;
			if(c == '.') flag = 1;
		}
		send_string("\n\r");
		CPU_CRITICAL_ENTER();
		if (Str_Cmp_N(cmd, "led1", 4) == 0) {
			if (Str_Cmp_N(cmd, "led1on", 6) == 0) {
				LED1CMD = 0;
			}else if(Str_Cmp_N(cmd, "led1off", 7) == 0) {
				LED1CMD = 1;
			}else if (Str_Cmp_N(cmd, "led1blink", 9) == 0) {
				LED1CMD = 2;
				LED1_T = cmd[9] - 48;
			}
		}else if (Str_Cmp_N(cmd, "led2", 4) == 0) {
			if (Str_Cmp_N(cmd, "led2on", 6) == 0) {
				LED2CMD = 0;
			}
			else if (Str_Cmp_N(cmd, "led2off", 7) == 0) {
				LED2CMD = 1;
			}
			else if (Str_Cmp_N(cmd, "led2blink", 9) == 0) {
				LED2CMD = 2;
				LED2_T = cmd[9] - 48;
			}
		}else if (Str_Cmp_N(cmd, "led3", 4) == 0) {
			if (Str_Cmp_N(cmd, "led3on", 6) == 0) {
				LED3CMD = 0;
			}
			else if (Str_Cmp_N(cmd, "led3off", 7) == 0) {
				LED3CMD = 1;
			}
			else if (Str_Cmp_N(cmd, "led3blink", 9) == 0) {
				LED3CMD = 2;
				LED3_T = cmd[9] - 48;
			}
		}else if (Str_Cmp_N(cmd, "reset",5) == 0) {
			LED1CMD = 1;
			LED2CMD = 1;
			LED3CMD = 1;
			LED1_T = 1;
			LED2_T = 1;
			LED3_T = 1;
		}
		CPU_CRITICAL_EXIT();
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

   OSTaskCreate((OS_TCB *)&UsartTask_TCB,
		   	   (CPU_CHAR *)"UsartTask",
		   	   (OS_TASK_PTR ) UsartTask,
			   (void *)0,
			   (OS_PRIO)2,
			   (CPU_STK *)&UsartTask_Stack[0u],
			   (CPU_STK_SIZE  )UsartTask_Stack[APP_CFG_TASK_START_STK_SIZE / 10u],
			   (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
			   (OS_MSG_QTY)0u,
			   (OS_TICK)0u,
			   (void *)0u,
			   (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
			   (OS_ERR *)&err);
   u8_t idx = 0;
   task_t* pTask_Cfg;
   for (idx = 0; idx < TASK_N; idx++)
   {
	   pTask_Cfg = &cyclic_tasks[idx];

	   OSTaskCreate(
		   pTask_Cfg->pTcb,
		   pTask_Cfg->name,
		   pTask_Cfg->func,
		   (void*)0u,
		   pTask_Cfg->prio,
		   pTask_Cfg->pStack,
		   pTask_Cfg->pStack[APP_CFG_TASK_START_STK_SIZE / 10u],
		   APP_CFG_TASK_START_STK_SIZE,
		   (OS_MSG_QTY)0u,
		   (OS_TICK)0u,
		   (void*)0u,
		   (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		   (OS_ERR*)&err
	   );
   }

   while(1){
	   OSTimeDlyHMSM(1u, 0u, 1u, 0u,
			   OS_OPT_TIME_HMSM_STRICT,
			   &err);
   }
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

/*
*********************************************************************************************************
*                                          Setup_Gpio()
*
* Description : Configure LED GPIOs directly
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     :
*              LED1 PB0
*              LED2 PB7
*              LED3 PB14
*
*********************************************************************************************************
*/
static void Setup_Gpio(void)
{
   GPIO_InitTypeDef led_init = {0};

   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
   RCC_AHB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

   led_init.GPIO_Mode   = GPIO_Mode_OUT;
   led_init.GPIO_OType  = GPIO_OType_PP;
   led_init.GPIO_Speed  = GPIO_Speed_2MHz;
   led_init.GPIO_PuPd   = GPIO_PuPd_NOPULL;
   led_init.GPIO_Pin    = GPIO_Pin_0 | GPIO_Pin_7 | GPIO_Pin_14;

   GPIO_Init(GPIOB, &led_init);
}

