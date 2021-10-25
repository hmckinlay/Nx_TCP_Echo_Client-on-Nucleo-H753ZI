/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
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
#include "app_netxduo.h"

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
/* USER CODE BEGIN PV */
typedef struct{
	UINT sCount,
	     fCount;
	TX_THREAD AppMainThread,
	 	 	 	 	  AppTCPThread;
	TX_SEMAPHORE Semaphore;
	NX_PACKET_POOL AppPool;
	ULONG IpAddress,NetMask;
	NX_IP IpInstance;
	NX_DHCP DHCPClient;
	NX_TCP_SOCKET TCPSocket;
	CHAR *pointer;
} nx_t;

nx_t nx;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID App_Main_Thread_Entry(ULONG thread_input);
static VOID App_TCP_Thread_Entry(ULONG thread_input);
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);
/* USER CODE END PFP */
/**
  * @brief  Application NetXDuo Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN MX_NetXDuo_MEM_POOL */

  /* USER CODE END MX_NetXDuo_MEM_POOL */

  /* USER CODE BEGIN MX_NetXDuo_Init */
  //Avoid HAL and Thread calls in this function, wait for thread creation
  //TX_BYTE_POOL *byte_pool is the 'ThreadX Byte Pool' defined in CubeMX
  //NX_PACKET_POOL AppPool is the 'NetXDuo Memory Pool' defined in CubeMX
  printf("NX_PACKET_POOL_SIZE=%u\r\n",NX_PACKET_POOL_SIZE);

  //Allocate the memory for packet_pool.
  if (tx_byte_allocate(byte_pool, (VOID **) &nx.pointer,  NX_PACKET_POOL_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  //Create the Packet pool to be used for packet allocation
  ret = nx_packet_pool_create(&nx.AppPool, "Main Packet Pool", PAYLOAD_SIZE, nx.pointer, NX_PACKET_POOL_SIZE);
  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  //Allocate the memory for Ip_Instance
  if (tx_byte_allocate(byte_pool, (VOID **) &nx.pointer,   2 * DEFAULT_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  //Create the main NX_IP instance
  ret = nx_ip_create(&nx.IpInstance, "Main Ip instance", NULL_ADDRESS, NULL_ADDRESS, &nx.AppPool,nx_stm32_eth_driver,nx.pointer, 2 * DEFAULT_MEMORY_SIZE, DEFAULT_PRIORITY);
  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  // Allocate the memory for ARP
  if (tx_byte_allocate(byte_pool, (VOID **) &nx.pointer, DEFAULT_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  //Enable the ARP protocol and provide the ARP cache size for the IP instance
  ret = nx_arp_enable(&nx.IpInstance, (VOID *)nx.pointer, DEFAULT_MEMORY_SIZE);
  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  //Enable the ICMP
  ret = nx_icmp_enable(&nx.IpInstance);
  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  // Enable the UDP protocol required for  DHCP communication
  ret = nx_udp_enable(&nx.IpInstance);
  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  // Enable the TCP protocol
  ret = nx_tcp_enable(&nx.IpInstance);
  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  // Allocate the memory for main thread
  if (tx_byte_allocate(byte_pool, (VOID **) &nx.pointer,2 *  DEFAULT_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  // Create the main thread
  ret = tx_thread_create(&nx.AppMainThread, "App Main thread", App_Main_Thread_Entry, 0, nx.pointer, 2 * DEFAULT_MEMORY_SIZE,DEFAULT_PRIORITY, DEFAULT_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  // Allocate the memory for TCP server thread
  if (tx_byte_allocate(byte_pool, (VOID **) &nx.pointer,2 *  DEFAULT_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  // create the TCP server thread
  ret = tx_thread_create(&nx.AppTCPThread, "App TCP Thread", App_TCP_Thread_Entry, 0, nx.pointer, 2 * DEFAULT_MEMORY_SIZE,DEFAULT_PRIORITY, DEFAULT_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);
  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  // create the DHCP client
  ret = nx_dhcp_create(&nx.DHCPClient, &nx.IpInstance, "DHCP Client");
  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  tx_semaphore_create(&nx.Semaphore, "App Semaphore", 0); // set DHCP notification callback
  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 1 */
/**
* @brief  Main thread entry.
* @param thread_input: ULONG user argument used by the thread entry
* @retval none
*/
static VOID App_Main_Thread_Entry(ULONG thread_input)
{
	UINT ret;

	ret = nx_ip_address_change_notify(&nx.IpInstance, ip_address_change_notify_callback, NULL);
	if (ret != NX_SUCCESS)
	{
		printf("F:nx_ip_address_change_notify\r\n");
		nx.fCount++;
	}
 	else
 	{
 		printf("S:nx_ip_address_change_notify\r\n");
 		nx.sCount++;
 	}

	ret = nx_dhcp_start(&nx.DHCPClient);
  if (ret != NX_SUCCESS)
  {
  	printf("F:nx_dhcp_start\r\n");
  	nx.fCount++;
  }
  else
  {
  	printf("S:nx_dhcp_start\r\n");
  	nx.sCount++;
  }

  printf("I:tx_semaphore_get\r\n");
  if(tx_semaphore_get(&nx.Semaphore, TX_WAIT_FOREVER) != TX_SUCCESS) // wait until an IP address is ready
  {
  	printf("F:tx_semaphore_get\r\n");
  	nx.fCount++;
  }
  else
  {
  	printf("S:tx_semaphore_get\r\n");
  	nx.sCount++;
  }

  ret = nx_ip_address_get(&nx.IpInstance, &nx.IpAddress, &nx.NetMask);
  if (ret != TX_SUCCESS)
  {
  	printf("F:nx_ip_address_get\r\n");
  	nx.fCount++;
  }
  else
  {
  	printf("S:nx_ip_address_get\r\n");
  	nx.sCount++;
  }

  /* the network is correctly initialized, start the TCP server thread */
  tx_thread_resume(&nx.AppTCPThread);
  /* this thread is not needed any more, relinquish it */
  tx_thread_relinquish();
  printf("I:***END OF App_Main_Thread_Entry***\r\n");
  return;
}

/**
* @brief  IP Address change callback.
* @param ip_instance: NX_IP instance registered for this callback.
* @param ptr: VOID* user data pointer
* @retval none
*/
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr)
{
	printf("I:ip_address_change_notify_callback\r\n");
  tx_semaphore_put(&nx.Semaphore);
  printf("S:ip_address_change_notify_callback\r\n");
}

/**
* @brief  TCP thread entry.
* @param thread_input: thread user data
* @retval none
*/

static VOID App_TCP_Thread_Entry(ULONG thread_input)
{
	printf("I:App_TCP_Thread_Entry\r\n");


  while(1) //Ping responses will now work.
  {
  	HAL_GPIO_TogglePin(GPO_LED_RED_GPIO_Port, GPO_LED_RED_Pin);
    PRINT_IP_ADDRESS(nx.IpAddress);
    tx_thread_sleep(100); // Thread sleep for 1s
  }

/*
  UINT ret;
  UINT count = 0;

  ULONG bytes_read;
  UCHAR data_buffer[512];

  ULONG source_ip_address;
  UINT source_port;

  NX_PACKET *server_packet;
  NX_PACKET *data_packet;

  // create the TCP socket
  ret = nx_tcp_socket_create(&IpInstance, &TCPSocket, "TCP Server Socket", NX_IP_NORMAL, NX_FRAGMENT_OKAY,NX_IP_TIME_TO_LIVE, WINDOW_SIZE, NX_NULL, NX_NULL);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  // bind the client socket for the DEFAULT_PORT
  ret =  nx_tcp_client_socket_bind(&TCPSocket, DEFAULT_PORT, NX_WAIT_FOREVER);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  // connect to the remote server on the specified port
  ret = nx_tcp_client_socket_connect(&TCPSocket, TCP_SERVER_ADDRESS, TCP_SERVER_PORT, NX_WAIT_FOREVER);

  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  while(count++ < MAX_PACKET_COUNT)
  {
    TX_MEMSET(data_buffer, '\0', sizeof(data_buffer));

    ret = nx_packet_allocate(&AppPool, &data_packet, NX_UDP_PACKET, TX_WAIT_FOREVER); // allocate the packet to send over the TCP socket
    if (ret != NX_SUCCESS)
    {
      break;
    }

    ret = nx_packet_data_append(data_packet, (VOID *)DEFAULT_MESSAGE, sizeof(DEFAULT_MESSAGE), &AppPool, TX_WAIT_FOREVER); // append the message to send into the packet

    if (ret != NX_SUCCESS)
    {
      nx_packet_release(data_packet);
      break;
    }

    // send the packet over the TCP socket
    ret = nx_tcp_socket_send(&TCPSocket, data_packet, DEFAULT_TIMEOUT);
    if (ret != NX_SUCCESS)
    {
      break;
    }

    // wait for the server response
    ret = nx_tcp_socket_receive(&TCPSocket, &server_packet, DEFAULT_TIMEOUT);
    if (ret == NX_SUCCESS)
    {
      nx_udp_source_extract(server_packet, &source_ip_address, &source_port); // get the server IP address and  port
      nx_packet_data_retrieve(server_packet, data_buffer, &bytes_read); // retrieve the data sent by the server
      PRINT_DATA(source_ip_address, source_port, data_buffer); // print the received data
      nx_packet_release(server_packet);  // release the server packet
      HAL_GPIO_TogglePin(GPO_LED_RED_GPIO_Port, GPO_LED_RED_Pin);  // toggle the green led on success
    }
    else
    {
      break;  // no message received exit the loop
    }
  }

  nx_packet_release(server_packet); // release the allocated packets
  nx_tcp_socket_disconnect(&TCPSocket, DEFAULT_TIMEOUT); // disconnect the socket
  nx_tcp_client_socket_unbind(&TCPSocket); // unbind the socket
  nx_tcp_socket_delete(&TCPSocket); // delete the socket


  if (count == MAX_PACKET_COUNT + 1) // print test summary on the UART
  {
    printf("\n-------------------------------------\n\tSUCCESS : %u / %u packets sent\n-------------------------------------\n", count - 1, MAX_PACKET_COUNT);
    //Success_Handler();
  }
  else
  {
    printf("\n-------------------------------------\n\tFAIL : %u / %u packets sent\n-------------------------------------\n", count - 1, MAX_PACKET_COUNT);
    //Error_Handler();
  }
*/
}

/* USER CODE END 1 */
