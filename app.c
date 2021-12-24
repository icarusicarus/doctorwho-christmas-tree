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
#include "stm32f4xx_tim.h"
#include "bsp.h"
#include "cpu.h"

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
    TASK_USART,
    TASK_BT,
    TASK_LED_CTL,
    TASK_MOTOR,
    TASK_MONITOR,

    TASK_N
}task_e;

const char* messages[32] = { "Green LED On\n", "Red LED On\n", "Blue LED On\n",
                            "Green LED Off\n", "Red LED Off\n", "Blue LED Off\n",
                            "Motor Activated\n", "Motor disabled\n",
                            "App Connected\n", "Tree Stopped\n" };

typedef enum {
    GREEN_LED_ON,
    RED_LED_ON,
    BLUE_LED_ON,
    GREEN_LED_OFF,
    RED_LED_OFF,
    BLUE_LED_OFF,
    MOTOR_ACT,
    MOTOR_DIS,
    APP_CONN,
    TREE_STOP,

    MSG_N

}msg_e;

typedef struct {
    msg_e msg;
    char* disc;
} q_msg;

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
static  void  AppTaskStart(void* p_arg);
static  void  AppTaskCreate(void);
static  void  AppObjCreate(void);

static void Task_Bluetooth(void* p_arg);
static void Task_LED_Control(void* p_arg);
static void Task_Motor(void* p_arg);
static void Task_Monitor(void* p_arg);
static void AppTask_usart(void* p_arg);

static void Setup_Gpio(void);
static void USART6_Init(void);

CPU_SR_ALLOC();

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
/* ----------------- APPLICATION GLOBALS -------------- */
static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB       Task_USART_TCB;
static  OS_TCB       Task_BT_TCB;
static  OS_TCB       Task_LED_TCB;;
static  OS_TCB       Task_Motor_TCB;
static  OS_TCB       Task_MON_TCB;

static  CPU_STK  Task_USART_Stack[APP_CFG_TASK_START_STK_SIZE];
static  CPU_STK  Task_BT_Stack[APP_CFG_TASK_START_STK_SIZE];
static  CPU_STK  Task_LED_Stack[APP_CFG_TASK_START_STK_SIZE];
static  CPU_STK  Task_Motor_Stack[APP_CFG_TASK_START_STK_SIZE];
static  CPU_STK  Task_MON_Stack[APP_CFG_TASK_START_STK_SIZE];

OS_Q tree_q;
OS_Q mon_q;

int emergency = 400;

int count = 0;

task_t cyclic_tasks[TASK_N] = {
   {"Task_usart", AppTask_usart, 3, &Task_USART_Stack[0] , &Task_USART_TCB},
   {"Task_Btn" , Task_Bluetooth,  1, &Task_BT_Stack[0]  , &Task_BT_TCB},
   {"Task_LED" , Task_LED_Control,  2, &Task_LED_Stack[0]  , &Task_LED_TCB},
   {"Task_Motor" , Task_Motor,  2, &Task_Motor_Stack[0]  , &Task_Motor_TCB},
   {"Task_Mon" , Task_Monitor,  0, &Task_MON_Stack[0]  , &Task_MON_TCB},
};
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

int main(void)
{
    OS_ERR  err;

    /* Basic Init */
    RCC_DeInit();
    //    SystemCoreClockUpdate();
    Setup_Gpio();

    /* BSP Init */
    BSP_IntDisAll();                                            /* Disable all interrupts.                              */
    BSP_Init();
    CPU_Init();                                                 /* Initialize the uC/CPU Services                       */
    Mem_Init();                                                 /* Initialize Memory Management Module                  */
    Math_Init();                                                /* Initialize Mathematical Module                       */
    USART6_Init();
    /* OS Init */
    OSInit(&err);                                               /* Init uC/OS-III.                                      */

    OSQCreate(&tree_q, "Tree Queue", 10, &err);
    if (err) {
        send_string("Failed to init usart semaphore");
    }

    OSQCreate(&mon_q, "Monitor Queue", 10, &err);
    if (err) {
        send_string("Failed to init monitor queue");
    }

    OSTaskCreate((OS_TCB*)&AppTaskStartTCB,              /* Create the start task                                */
        (CPU_CHAR*)"App Task Start",
        (OS_TASK_PTR)AppTaskStart,
        (void*)0u,
        (OS_PRIO)APP_CFG_TASK_START_PRIO,
        (CPU_STK*)&AppTaskStartStk[0u],
        (CPU_STK_SIZE)AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE / 10u],
        (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE,
        (OS_MSG_QTY)0u,
        (OS_TICK)0u,
        (void*)0u,
        (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err);

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
static  void  AppTaskStart(void* p_arg)
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


static void Task_LED_Control(void* p_arg)
{
    OS_ERR  err;
    int i = 200;
    while (DEF_TRUE) {
        CPU_CRITICAL_ENTER();
        i = emergency;
        CPU_CRITICAL_EXIT();

        q_msg M = { GREEN_LED_ON, messages[GREEN_LED_ON] };
        OSQPost(&mon_q, &M, sizeof(M), 0, &err);
        GPIO_ResetBits(GPIOE, GPIO_Pin_4);

        OSTimeDlyHMSM(0u, 0u, 0u, i, OS_OPT_TIME_HMSM_STRICT, &err);
        M.msg = RED_LED_ON;
        M.disc = messages[RED_LED_ON];
        OSQPost(&mon_q, &M, sizeof(M), 0, &err);
        GPIO_ResetBits(GPIOE, GPIO_Pin_5);
        OSTimeDlyHMSM(0u, 0u, 0u, i, OS_OPT_TIME_HMSM_STRICT, &err);

        M.msg = BLUE_LED_ON;
        M.disc = messages[BLUE_LED_ON];
        OSQPost(&mon_q, &M, sizeof(M), 0, &err);
        GPIO_ResetBits(GPIOE, GPIO_Pin_6);
        OSTimeDlyHMSM(0u, 0u, 0u, i, OS_OPT_TIME_HMSM_STRICT, &err);

        M.msg = GREEN_LED_OFF;
        M.disc = messages[GREEN_LED_OFF];
        OSQPost(&mon_q, &M, sizeof(M), 0, &err);
        GPIO_SetBits(GPIOE, GPIO_Pin_4);
        OSTimeDlyHMSM(0u, 0u, 0u, i, OS_OPT_TIME_HMSM_STRICT, &err);

        M.msg = RED_LED_OFF;
        M.disc = messages[RED_LED_OFF];
        OSQPost(&mon_q, &M, sizeof(M), 0, &err);
        GPIO_SetBits(GPIOE, GPIO_Pin_5);
        OSTimeDlyHMSM(0u, 0u, 0u, i, OS_OPT_TIME_HMSM_STRICT, &err);

        M.msg = BLUE_LED_OFF;
        M.disc = messages[BLUE_LED_OFF];
        OSQPost(&mon_q, &M, sizeof(M), 0, &err);
        GPIO_SetBits(GPIOE, GPIO_Pin_6);
        OSTimeDlyHMSM(0u, 0u, 0u, i, OS_OPT_TIME_HMSM_STRICT, &err);

    }
}


static void Task_Motor(void* p_arg)
{
    OS_ERR  err;
    while (DEF_TRUE) {
        OS_MSG_SIZE size;
        q_msg* res = OSQPend(&tree_q, 0, 0, &size, NULL, &err);
        if (res->msg == APP_CONN) {
            q_msg M = { MOTOR_ACT, messages[MOTOR_ACT] };
            OSQPost(&mon_q, &M, sizeof(M), 0, &err);
            GPIO_SetBits(GPIOE, GPIO_Pin_11);
            OSTimeDlyHMSM(0u, 0u, 1u, 0u, OS_OPT_TIME_HMSM_STRICT, &err);

            M.msg = MOTOR_DIS;
            M.disc = messages[MOTOR_DIS];
            OSQPost(&mon_q, &M, sizeof(M), 0, &err);

            GPIO_ResetBits(GPIOE, GPIO_Pin_11);
            OSTimeDlyHMSM(0u, 0u, 0u, 100u, OS_OPT_TIME_HMSM_STRICT, &err);

            M.msg = APP_CONN;
            M.disc = messages[APP_CONN];
            OSQPost(&tree_q, &M, sizeof(M), 0, &err);
        }

        else if (res->msg == TREE_STOP) {
            OSQFlush(&tree_q, &err);
        }

    }
}

static void Task_Bluetooth(void* p_arg)
{
    OS_ERR  err;
    char c;
    while (DEF_TRUE) {
        while (USART_GetFlagStatus(USART6, USART_FLAG_RXNE) == RESET) {
            OSTimeDlyHMSM(0u, 0u, 0u, 1u, OS_OPT_TIME_HMSM_STRICT, &err);
        }

        c = USART_ReceiveData(USART6);
        if (c == 'C') {
            q_msg M = { APP_CONN, messages[APP_CONN] };
            OSQPost(&mon_q, &M, sizeof(M), 0, &err);
            OSQPost(&tree_q, &M, sizeof(M), 0, &err);

            CPU_CRITICAL_ENTER();
            emergency = 50;
            CPU_CRITICAL_EXIT();
        }
        else if (c == 'S') {
            q_msg M = { TREE_STOP, messages[TREE_STOP] };
            OSQPost(&mon_q, &M, sizeof(M), 0, &err);
            OSQPost(&tree_q, &M, sizeof(M), 0, &err);

            CPU_CRITICAL_ENTER();
            emergency = 400;
            CPU_CRITICAL_EXIT();
        }
        USART_SendData(Nucleo_COM1, c);

        OSTimeDlyHMSM(0u, 0u, 0u, 1u, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

static void Task_Monitor(void* p_arg)
{
    OS_ERR  err;

    while (DEF_TRUE) {
        OS_MSG_SIZE size;
        q_msg* res = OSQPend(&mon_q, 0, 0, &size, NULL, &err);
        send_string(res->disc);

        OSTimeDlyHMSM(0u, 0u, 0u, 1u,
            OS_OPT_TIME_HMSM_STRICT,
            &err);
    }
}

static void AppTask_usart(void* p_arg)
{
    OS_ERR  err;
    char c;

    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
        while (USART_GetFlagStatus(Nucleo_COM1, USART_FLAG_RXNE) == RESET) {
            OSTimeDlyHMSM(0u, 0u, 0u, 1u, OS_OPT_TIME_HMSM_STRICT, &err);
        }

        c = USART_ReceiveData(Nucleo_COM1);
        OSTimeDlyHMSM(0u, 0u, 0u, 20u, OS_OPT_TIME_HMSM_STRICT, &err);
        USART_SendData(USART6, c);

        OSTimeDlyHMSM(0u, 0u, 0u, 1u, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

static  void  AppTaskCreate(void)
{
    OS_ERR  err;

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

static  void  AppObjCreate(void)
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

void send_string_uart6(const char* str)
{
    while (*str)
    {
        while (USART_GetFlagStatus(USART6, USART_FLAG_TXE) == RESET);
        USART_SendData(USART6, *str++);
    }
}


static void Setup_Gpio(void)
{
    GPIO_InitTypeDef led_init = { 0 };
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitTypeDef button_init = { 0 };

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    RCC_AHB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    /*
        led_init.GPIO_Mode = GPIO_Mode_OUT;
        led_init.GPIO_OType = GPIO_OType_PP;
        led_init.GPIO_Speed = GPIO_Speed_2MHz;
        led_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
        led_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_7 | GPIO_Pin_14;
        GPIO_Init(GPIOB, &led_init);
    */
    led_init.GPIO_Mode = GPIO_Mode_OUT;
    led_init.GPIO_OType = GPIO_OType_PP;
    led_init.GPIO_Speed = GPIO_Speed_50MHz;
    led_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    led_init.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_Init(GPIOE, &led_init);

    led_init.GPIO_Mode = GPIO_Mode_IN;
    led_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    led_init.GPIO_Speed = GPIO_Speed_2MHz;
    led_init.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &button_init);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

static void USART6_Init() {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

    GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_USART6);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource9, GPIO_AF_USART6);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    /* Configure USART Rx as alternate function  */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    /* Enable USART */

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART6, &USART_InitStructure);
    USART_Cmd(USART6, ENABLE);
}

