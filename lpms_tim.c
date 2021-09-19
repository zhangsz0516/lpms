/*
 * Copyright (c) 2021-2022, LPMS Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2021-08-29     zhangsz         first version
 */
#include <board.h>
#include <pms.h>

static LPTIM_HandleTypeDef LptimHandle;

void LPTIM1_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_LPTIM_IRQHandler(&LptimHandle);
    rt_interrupt_leave();
}

void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
    /* enter interrupt */
    rt_interrupt_enter();

    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * This function get current count value of LPTIM
 *
 * @return the count vlaue
 */
rt_uint32_t stm32_lptim_get_current_tick(void)
{
    return HAL_LPTIM_ReadCounter(&LptimHandle);
}

/**
 * This function get the max value that LPTIM can count
 *
 * @return the max count
 */
rt_uint32_t stm32_lptim_get_tick_max(void)
{
    return (0xFFFF);
}

/**
 * This function start LPTIM with reload value
 *
 * @param reload The value that LPTIM count down from
 *
 * @return RT_EOK
 */
rt_err_t stm32_lptim_start(rt_uint32_t reload)
{
    HAL_LPTIM_TimeOut_Start_IT(&LptimHandle, reload, 0xff);

    return (RT_EOK);
}

/**
 * This function stop LPTIM
 */
void stm32_lptim_stop(void)
{
    HAL_LPTIM_TimeOut_Stop_IT(&LptimHandle);
}

/**
 * This function get the count clock of LPTIM
 *
 * @return the count clock frequency in Hz
 */
rt_uint32_t stm32_lptim_get_countfreq(void)
{
    return 32768 / 32;
}

/**
 * This function initialize the lptim
 */
int stm32_hw_lptim_init(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct = {0};

    __HAL_RCC_LPTIM1_CLK_ENABLE();

    /* Enable LSI clock */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* Select the LSI clock as LPTIM peripheral clock */
    RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
    RCC_PeriphCLKInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSI;
    HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

    LptimHandle.Instance = LPTIM1;
    LptimHandle.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    LptimHandle.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
    LptimHandle.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
    //LptimHandle.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
    LptimHandle.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
    LptimHandle.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
    if (HAL_LPTIM_Init(&LptimHandle) != HAL_OK)
    {
        return -1;
    }

    NVIC_ClearPendingIRQ(LPTIM1_IRQn);
    //HAL_LPTIM_TimeOut_Interrupt_enable(&LptimHandle);
    NVIC_SetPriority(LPTIM1_IRQn, 0);
    NVIC_EnableIRQ(LPTIM1_IRQn);

    return 0;
}

INIT_DEVICE_EXPORT(stm32_hw_lptim_init);
