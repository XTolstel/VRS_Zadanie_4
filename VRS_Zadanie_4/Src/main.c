/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);


/* Function processing DMA Rx data. Counts how many capital and small letters are in sentence.
 * Result is supposed to be stored in global variable of type "letter_count_" that is defined in "main.h"
 *
 * @param1 - received sign
 */
void proccesDmaData(uint8_t sign);


/* Space for your global variables. */

	#define STRING_LIMIT 35U
#define RESULT_QUEUE_SIZE 64U
#define REPORT_PERIOD_MS 2000U

typedef struct
{
    char text[STRING_LIMIT + 1U];
    letter_count_ letters;
} received_string_;

volatile uint32_t system_ms = 0U;
letter_count_ letter_count = {0U, 0U};
static volatile received_string_ results[RESULT_QUEUE_SIZE];
static volatile uint8_t result_write = 0U;
static volatile uint8_t result_read = 0U;
volatile uint32_t dropped_strings = 0U; /* Debug counter if PC floods TX. */


int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  USART2_RegisterCallback(proccesDmaData);
  MX_USART2_UART_Init();

  /* Space for your local variables, callback registration ...*/

  	  uint32_t last_report = system_ms;
      uint16_t occupied;
      uint16_t load_tenths;
      uint8_t index;
      uint8_t i;
      int length;
      received_string_ result;
      static char tx_buffer[160]; /* Must remain valid during DMA TX. */

  while (1)
  {
	  /* Periodic transmission of information about DMA Rx buffer state.
	   * Transmission frequency - 0.5Hz.
	   * Message format - "Buffer capacity: %d bytes, occupied memory: %d bytes, load [in %]: %f%"
	   * Example message (what I wish to see in terminal) - Buffer capacity: 1000 bytes, occupied memory: 231 bytes, load [in %]: 23.1%
	   */

	  /* Valid text string information transmission.
	   * Transmission frequency - when new valid string is received.
	   * Message format - "Valid string: %s, lower-case: %d, upper-case: %d"
	   * Example message (what I wish to see in terminal) - Valid string: Platn15uborZnakov, lower-case: 13, upper-case: 2
	   */

  	  	  	  /* One persistent TX buffer: never rewrite it while DMA is using it. */
          if (USART2_TxBusy() == 0U)
          {
              if ((uint32_t)(system_ms - last_report) >= REPORT_PERIOD_MS)
              {
                  last_report += REPORT_PERIOD_MS;
                  occupied = USART2_GetRxOccupied();
                  /* Integer tenths give 23.1% without printf float support. */
                  load_tenths = (occupied * 1000U + DMA_USART2_BUFFER_SIZE / 2U)
                                / DMA_USART2_BUFFER_SIZE;
                  length = snprintf(tx_buffer, sizeof(tx_buffer),
                      "Buffer capacity: %u bytes, occupied memory: %u bytes, load [in %%]: %u.%u%%\r\n",
                      (unsigned int)DMA_USART2_BUFFER_SIZE,
                      (unsigned int)occupied,
                      (unsigned int)(load_tenths / 10U),
                      (unsigned int)(load_tenths % 10U));
                  if ((length > 0) && (length < (int)sizeof(tx_buffer)))
                  {
                      USART2_PutBuffer((uint8_t *)tx_buffer, (uint8_t)length);
                  }
              }
              else if (result_read != result_write)
              {
                  /* Only main advances read; only RX callback advances write.
                   * The producer cannot overwrite this slot until it is freed. */
                  index = result_read;
                  __DMB();
                  for (i = 0U; i <= STRING_LIMIT; ++i)
                  {
                      result.text[i] = results[index].text[i];
                  }
                  result.letters.small_letter = results[index].letters.small_letter;
                  result.letters.capital_letter = results[index].letters.capital_letter;
                  __DMB();
                  result_read = (uint8_t)((index + 1U) % RESULT_QUEUE_SIZE);

                  length = snprintf(tx_buffer, sizeof(tx_buffer),
                      "Valid string: %s, lower-case: %u, upper-case: %u\r\n",
                      result.text,
                      (unsigned int)result.letters.small_letter,
                      (unsigned int)result.letters.capital_letter);
                  if ((length > 0) && (length < (int)sizeof(tx_buffer)))
                  {
                      USART2_PutBuffer((uint8_t *)tx_buffer, (uint8_t)length);
                  }
              }
          }
  }
  /* USER CODE END 3 */
}


void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
  Error_Handler();  
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {
    
  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
  
  }
  LL_Init1msTick(8000000);
  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  LL_SetSystemCoreClock(8000000);
  NVIC_SetPriority(SysTick_IRQn, 2U);
  LL_SYSTICK_EnableIT();
}

/*
 * Implementation of function processing data received via USART.
 */
void proccesDmaData(uint8_t sign)
{
    static uint8_t receiving = 0U;
    static uint8_t length = 0U;
    static char text[STRING_LIMIT + 1U];
    static letter_count_ current = {0U, 0U};
    uint8_t next;
    uint8_t i;

    if (receiving == 0U)
    {
        if (sign == '#')
        {
            receiving = 1U;
            length = 0U;
            current.small_letter = 0U;
            current.capital_letter = 0U;
        }
        return;
    }

    /* End marker is valid within the next 35 characters after the start.
     * State is preserved between separate DMA reception events. */
    if (sign == '$')
    {
        text[length] = '\0';
        letter_count = current;
        next = (uint8_t)((result_write + 1U) % RESULT_QUEUE_SIZE);
        if (next != result_read)
        {
            for (i = 0U; i <= length; ++i)
            {
                results[result_write].text[i] = text[i];
            }
            results[result_write].letters.small_letter = current.small_letter;
            results[result_write].letters.capital_letter = current.capital_letter;
            __DMB();
            result_write = next;
        }
        else
        {
            ++dropped_strings;
        }
        receiving = 0U;
        return;
    }

    text[length++] = (char)sign;
    if ((sign >= 'a') && (sign <= 'z'))
    {
        ++current.small_letter;
    }
    else if ((sign >= 'A') && (sign <= 'Z'))
    {
        ++current.capital_letter;
    }

    if (length >= STRING_LIMIT)
    {
        /* No end marker within 35 characters: discard this frame. */
        receiving = 0U;
        length = 0U;
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
