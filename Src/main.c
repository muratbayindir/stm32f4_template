/**
  ******************************************************************************
  * @file    main.c 
  * @author  
  * @version 
  * @date     
  * @brief   This file provides main program functions
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

TaskHandle_t main_Task_Handle, bk_Task_Handle;
extern uint32_t SystemCoreClock;
uint8_t * read_buffer;

FATFS USBDISKFatFs;           /* File system object for USB disk logical drive */
FIL MyFile;                   /* File object */
char USBDISKPath[4];          /* USB Host logical drive path */
USBH_HandleTypeDef hUSBHost;  /* USB Host handle */

typedef enum {
  APPLICATION_IDLE = 0,  
  APPLICATION_START,    
  APPLICATION_RUNNING,
}MSC_ApplicationTypeDef;

MSC_ApplicationTypeDef Appli_state = APPLICATION_IDLE;

static void Main_Task(void const *argument);
static void Bk_Task(void const *argument);
static void TouchUpdate(void);
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);
static void MSC_Application(void);
static void Error_Handler(void);
/* */
static void SystemClock_Config(void);


int main()
{
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
	HAL_Init();

  SystemInit();

  /* Enable the CRC Module */
  __HAL_RCC_CRC_CLK_ENABLE();
  
  /* Configure the system clock to 180 MHz */
  SystemClock_Config();

  /* Initialize STM32F429I-DISCO's LEDs */
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);

  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

  read_buffer = (uint8_t *) calloc(100, 1);

  xTaskCreate(Main_Task, "Main_Task", 
    2048, 
    ( void * ) 1,
    tskIDLE_PRIORITY,
    &main_Task_Handle );
  
  /* Start scheduler */
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
  for(;;);
}

/**
  * @brief  Toggle LED3
  * @param  thread not used
  * @retval None
  */
static void Main_Task(void const *argument)
{
  char* str = (char *) malloc(20);

  /* Initializes the SDRAM device */
  BSP_SDRAM_Init();

  /* Initialize the Touch screen */
  BSP_TS_Init(240, 320);
  
  /* Initialize emWin GUI */
  GUI_Init();  

  /* Clear Display */
  GUI_Clear();
  
  /* Change Font */
  GUI_SetFont(&GUI_Font20_1);

  xTaskCreate(Bk_Task, "bk_task", 
    512, 
    ( void * ) 1,
    tskIDLE_PRIORITY,
    &bk_Task_Handle );

  while(1)
  {
    TouchUpdate();
    GUI_Exec();
    sprintf(str, "CPU : %u%%   \0", osGetCPUUsage());
    GUI_DispStringAt(str, 150, 300);
    vTaskDelay(25);
  }
}

static void Bk_Task(void const *argument)
{
  /*##-1- Link the USB Host disk I/O driver ##################################*/
  if(FATFS_LinkDriver(&USBH_Driver, USBDISKPath) == 0)
  {
    /*##-2- Init Host Library ################################################*/
    USBH_Init(&hUSBHost, USBH_UserProcess, 0);
    
    /*##-3- Add Supported Class ##############################################*/
    USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);
    
    /*##-4- Start Host Process ###############################################*/
    USBH_Start(&hUSBHost);
    
    /*##-5- Run Application (Blocking mode) ##################################*/
    while (1)
    {
      /* USB Host Background task */
      USBH_Process(&hUSBHost);
      
      /* Mass Storage Application State Machine */
      switch(Appli_state)
      {
      case APPLICATION_START:
        MSC_Application();
        Appli_state = APPLICATION_IDLE;
        break;
        
      case APPLICATION_IDLE:
      default:
        break;      
      }
      vTaskDelay(5);
    }    
  }
}


/**
  * @brief  Main routine for Mass Storage Class
  * @param  None
  * @retval None
  */
static void MSC_Application(void)
{
  uint32_t bytesread;

  vTaskSuspendAll();

  f_mount(&USBDISKFatFs, (TCHAR const*)USBDISKPath, 0);
  f_open(&MyFile, "m.txt", FA_READ);
  f_read(&MyFile, read_buffer, 99, (void *)&bytesread);
  f_close(&MyFile);

  BSP_LED_On(LED3);

  GUI_Clear();
  GUI_DispString(read_buffer);

  xTaskResumeAll();
  
  /* Unlink the USB disk I/O driver */
  FATFS_UnLinkDriver(USBDISKPath);
}


/**
  * @brief  User Process
  * @param  phost: Host handle
  * @param  id: Host Library user message ID
  * @retval None
  */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{  
  switch(id)
  { 
  case HOST_USER_SELECT_CONFIGURATION:
    break;
    
  case HOST_USER_DISCONNECTION:
    Appli_state = APPLICATION_IDLE;
    BSP_LED_Off(LED3); 
    BSP_LED_Off(LED4);      
    f_mount(NULL, (TCHAR const*)"", 0);      
    break;
    
  case HOST_USER_CLASS_ACTIVE:
    Appli_state = APPLICATION_START;
    break;
    
  default:
    break;
  }
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  SystemCoreClock = 180000000;
}

static void Error_Handler(void)
{
  BSP_LED_On(LED4);
}

/**
  * @brief  Read the coordinate of the point touched and assign their
  *         value to the variables u32_TSXCoordinate and u32_TSYCoordinate
  * @param  None
  * @retval None
  */
static void TouchUpdate(void)
{

  static GUI_PID_STATE TS_State;
  static TS_StateTypeDef ts;

  BSP_TS_GetState(&ts);

  TS_State.Pressed = ts.TouchDetected;
  TS_State.Layer = 0;
  TS_State.x = ts.X;
  TS_State.y = ts.Y;

  GUI_TOUCH_StoreStateEx(&TS_State);
}


/*  ******************************  END OF FILE  ********************************  */
