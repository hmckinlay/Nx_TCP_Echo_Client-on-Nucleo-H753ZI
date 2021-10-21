/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_azure_rtos.c
  * @author  MCD Application Team
  * @brief   azure_rtos application implementation file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "app_azure_rtos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN TX_Pool_Buffer */
//SDEF_TXP //place the tx_byte_pool_buffer in D2 memory
/* USER CODE END TX_Pool_Buffer */
static UCHAR tx_byte_pool_buffer[TX_APP_MEM_POOL_SIZE];
static TX_BYTE_POOL tx_app_byte_pool;

/* USER CODE BEGIN NX_Pool_Buffer */
SDEF_NXP //place the nx_byte_pool_buffer in D2 memory
/* USER CODE END NX_Pool_Buffer */
static UCHAR  nx_byte_pool_buffer[NX_APP_MEM_POOL_SIZE];
static TX_BYTE_POOL nx_app_byte_pool;

/* USER CODE BEGIN PV */
typedef struct{
	UINT sCount,
	     fCount;
} aar_t;

aar_t aar;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Define the initial system.
  * @param  first_unused_memory : Pointer to the first unused memory
  * @retval None
  */
VOID tx_application_define(VOID *first_unused_memory)
{
  /* USER CODE BEGIN  tx_application_define */
//ThreadX offers the “tx_application_define()�? function, that is automatically called by the tx_kernel_enter() API.
//It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools…)
//It should not in any way contain a system API call (HAL or BSP).

  /* USER CODE END  tx_application_define */

  VOID *memory_ptr;

  if (tx_byte_pool_create(&tx_app_byte_pool, "Tx App memory pool", tx_byte_pool_buffer, TX_APP_MEM_POOL_SIZE) != TX_SUCCESS)
  {
    /* USER CODE BEGIN TX_Byte_Pool_Error */
  	printf("F:tx_byte_pool_create Tx App memory pool\r\n");
  	aar.fCount++;
    /* USER CODE END TX_Byte_Pool_Error */
  }
  else
  {
    /* USER CODE BEGIN TX_Byte_Pool_Success */
  	printf("S:tx_byte_pool_create Tx App memory pool\r\n");
  	aar.sCount++;
    /* USER CODE END TX_Byte_Pool_Success */

    memory_ptr = (VOID *)&tx_app_byte_pool;

    if (App_ThreadX_Init(memory_ptr) != TX_SUCCESS)
    {
      /* USER CODE BEGIN  App_ThreadX_Init_Error */
    	printf("F:App_ThreadX_Init\r\n");
    	aar.fCount++;
      /* USER CODE END  App_ThreadX_Init_Error */
    }

    /* USER CODE BEGIN  App_ThreadX_Init_Success */
    else
    {
    	printf("S:App_ThreadX_Init\r\n");
    	aar.sCount++;
    }
    /* USER CODE END  App_ThreadX_Init_Success */

  }

  if (tx_byte_pool_create(&nx_app_byte_pool, "Nx App memory pool", nx_byte_pool_buffer, NX_APP_MEM_POOL_SIZE) != TX_SUCCESS)
  {
    /* USER CODE BEGIN NX_Byte_Pool_Error */
  	printf("F:tx_byte_pool_create Nx App memory pool\r\n");
  	aar.fCount++;
    /* USER CODE END NX_Byte_Pool_Error */
  }
  else
  {
    /* USER CODE BEGIN TX_Byte_Pool_Success */
  	printf("S:tx_byte_pool_create Tx App memory pool\r\n");
  	aar.sCount++;
    /* USER CODE END TX_Byte_Pool_Success */

    memory_ptr = (VOID *)&nx_app_byte_pool;

    if (MX_NetXDuo_Init(memory_ptr) != NX_SUCCESS)
    {
      /* USER CODE BEGIN MX_NetXDuo_Init_Error */
    	printf("F:MX_NetXDuo_Init\r\n");
    	aar.fCount++;
      /* USER CODE END MX_NetXDuo_Init_Error */
    }

    /* USER CODE BEGIN MX_NetXDuo_Init_Success */
    else
    {
    	printf("S:MX_NetXDuo_Init\r\n");
    	aar.sCount++;
    }
    /* USER CODE END MX_NetXDuo_Init_Success */

  }

}

/* USER CODE BEGIN  0 */

/* USER CODE END  0 */
