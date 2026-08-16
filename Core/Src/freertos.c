/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    freertos.c
  * @brief   Code for freertos applications
  * @version V1.0.0
  * @date    15-August-2026
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "string.h"
#include "queue.h"
#include "semphr.h"
#include "frame_protocol.h"
#include "flash_operations.h"
#include "bootloader_config.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define CBC 1
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern DMA_HandleTypeDef hdma_usart6_rx;

// Buffer definitions
uint8_t rx_buffer[RX_BUF_SIZE];
uint8_t tx_buffer[TX_BUF_SIZE];

// RX info structure definition (must be visible to all files)
//struct rxINFO {
//    uint8_t Sizebuffer;
//    uint8_t Databuffer[RX_BUF_SIZE];
//};


// RTOS objects
QueueHandle_t xQueueDMA;
SemaphoreHandle_t xMutex;
SemaphoreHandle_t xTX_Semaphore;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void xTaskLed1(void *pvParameters);
void xTaskReceivequeueDMA(void *pvParameters);

/* USER CODE END FunctionPrototypes */

void MX_FREERTOS_Init(void);

/* ============================================================================
   FreeRTOS Initialization
   ============================================================================ */
void MX_FREERTOS_Init(void) {
    /* USER CODE BEGIN Init */
    // Start UART DMA reception
    HAL_UART_Receive_DMA(&huart6, rx_buffer, RX_BUF_SIZE);

    // Create LED task
    if ((xTaskCreate(xTaskLed1, "led 1 Task", 128, NULL, 1, NULL)) != pdTRUE) {
        HAL_UART_Transmit(&huart6, (uint8_t*)"Error during task 01 Creation\n", 25, 1000);
    }

    // Create DMA receive task
    if ((xTaskCreate(xTaskReceivequeueDMA, "Receive DMA Task", 128 * 4, NULL, 1, NULL)) != pdTRUE) {
        HAL_UART_Transmit(&huart6, (uint8_t*)"Error during Receive DMA Creation\n", 35, 1000);
    }
    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    // Create Mutex
    xMutex = xSemaphoreCreateMutex();
    if (xMutex == NULL) {
        HAL_UART_Transmit(&huart6, (uint8_t*)"Error during xMutex Creation\n", 30, 1000);
    }

    // Create Binary Semaphore for TX
    xTX_Semaphore = xSemaphoreCreateBinary();
    if (xTX_Semaphore == NULL) {
        HAL_UART_Transmit(&huart6, (uint8_t*)"Error during xTX_Semaphore Creation\n", 42, 1000);
    }
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    // Create Queue for DMA data
    if ((xQueueDMA = xQueueCreate(10, sizeof(struct rxINFO *))) == NULL) {
        HAL_UART_Transmit(&huart6, (uint8_t*)"Error during xQueueCreate Creation\n", 35, 1000);
    }
    /* USER CODE END RTOS_QUEUES */

    /* USER CODE BEGIN RTOS_THREADS */
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_EVENTS */
    // Start scheduler
    vTaskStartScheduler();
    /* USER CODE END RTOS_EVENTS */
}

/* ============================================================================
   Tasks Implementation
   ============================================================================ */

/**
  * @brief  LED blinking task
  */
void xTaskLed1(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

/**
  * @brief  DMA receive queue processing task
  */
void xTaskReceivequeueDMA(void *pvParameters) {
    (void)pvParameters;
    struct rxINFO *pxInforx;

    for (;;) {
        if (xQueueReceive(xQueueDMA, &(pxInforx), portMAX_DELAY) == pdPASS) {
            // Process received frame with mutual exclusion
            xSemaphoreTake(xMutex, portMAX_DELAY);
            ReceiveRX(pxInforx);
            xSemaphoreGive(xMutex);
        }
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

/* ============================================================================
   UART Communication Functions
   ============================================================================ */

/**
  * @brief  Transmit data via UART DMA
  */
uint8_t Uart_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, TickType_t timeout) {
    (void)huart;

    // Copy data to TX buffer
    memcpy(tx_buffer, pData, Size);

    // Start DMA transmission
    if (HAL_UART_Transmit_DMA(&huart6, (uint8_t*)&tx_buffer, Size) != HAL_OK) {
        return pdFALSE;
    }

    // Wait for transmission complete
    if (xSemaphoreTake(xTX_Semaphore, timeout) == pdTRUE) {
        return pdTRUE;
    }
    return pdFALSE;
}

/* ============================================================================
   UART Callbacks
   ============================================================================ */

/**
  * @brief  UART TX complete callback (ISR context)
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (USART6 == huart6.Instance) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(xTX_Semaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/**
  * @brief  UART IDLE line callback (ISR context)
  * @note   This handles frame reception via DMA
  */
void UART_IDleCallBack(UART_HandleTypeDef *huart) {
    struct rxINFO *xInforx = NULL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (USART6 == huart6.Instance) {
        if (__HAL_UART_GET_FLAG(&huart6, UART_FLAG_IDLE) != RESET) {
            __HAL_UART_CLEAR_IDLEFLAG(&huart6);

            // Stop DMA to get received data
            HAL_UART_DMAStop(&huart6);

            // Calculate received data length
            uint8_t data_length = RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart6_rx);

            // Allocate memory for RX info (using static pool for ISR safety)
            static struct rxINFO rxInfoPool[10];
            static uint8_t poolIndex = 0;
            xInforx = &rxInfoPool[poolIndex++ % 10];

            if (xInforx != NULL) {
                xInforx->Sizebuffer = data_length;
                memcpy(xInforx->Databuffer, rx_buffer, data_length);

                // Send to queue from ISR
                xQueueSendFromISR(xQueueDMA, &xInforx, &xHigherPriorityTaskWoken);
            }

            // Restart DMA reception
            HAL_UART_Receive_DMA(&huart6, rx_buffer, RX_BUF_SIZE);

            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/* USER CODE END Application */
