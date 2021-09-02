/*
 * Copyright (c) 2021-2022, LPMS Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2021-08-29     zhangsz         first version
 */
#include <lpms.h>
#include <lpms_drv.h>
#include <lpms_tim.h>

#include <rtthread.h>
#include <rthw.h>
#include <stdlib.h>
#include <board.h>

#ifndef LPMS_DEFAULT_SLEEP_MODE
#define LPMS_DEFAULT_SLEEP_MODE     PM_SLEEP_NONE
#endif

#ifndef LPMS_DEFAULT_FREQ_MODE
#define LPMS_DEFAULT_FREQ_MODE      PM_FREQ_NORMAL
#endif

/* 【命令】睡眠模式请求 */
static void pms_sleep_request(int argc, char **argv)
{
    int module = 0;
    int mode = 0;

    if (argc >= 3)
    {
        module = atoi(argv[1]);
        mode = atoi(argv[2]);
        pm_sleep_request(module, mode);
    }
}
MSH_CMD_EXPORT(pms_sleep_request, pms_sleep_request module sleep_mode);

/* 【命令】睡眠模式释放 */
static void pms_sleep_release(int argc, char **argv)
{
    int module = 0;
    int mode = 0;

    if (argc >= 3)
    {
        module = atoi(argv[1]);
        mode = atoi(argv[2]);
        pm_sleep_release(module, mode);
    }
}
MSH_CMD_EXPORT(pms_sleep_release, pms_sleep_release module sleep_mode);

/* 【命令】变频模式请求 */
static void pms_freq_request(int argc, char **argv)
{
    int module = 0;
    int mode = 0;

    if (argc >= 3)
    {
        module = atoi(argv[1]);
        mode = atoi(argv[2]);
        pm_freq_request(module, mode);
    }
}
MSH_CMD_EXPORT(pms_freq_request, pms_freq_request module freq_mode);

/* 【命令】变频模式请求 */
static void pms_freq_release(int argc, char **argv)
{
    int module = 0;
    int mode = 0;

    if (argc >= 3)
    {
        module = atoi(argv[1]);
        mode = atoi(argv[2]);
        pm_freq_release(module, mode);
    }
}
MSH_CMD_EXPORT(pms_freq_release, pms_freq_release module freq_mode);

/* 【命令】设置【忙】状态 */
static void pms_busy_set(int argc, char **argv)
{
    int module = 0;
    int time = 0;

    if (argc >= 3)
    {
        module = atoi(argv[1]);
        time = atoi(argv[2]);
        pm_busy_set(module, time);
    }
}
MSH_CMD_EXPORT(pms_busy_set, pms_busy_set module ticks);

/* 【命令】清楚【忙】状态 */
static void pms_busy_clear(int argc, char **argv)
{
    int module = 0;

    if (argc >= 2)
    {
        module = atoi(argv[1]);
        pm_busy_clear(module);
    }
}
MSH_CMD_EXPORT(pms_busy_clear, pms_busy_clear module_id);

/* 【命令】使能与禁能PM框架 */
static void pms_enable_ctrl(int argc, char **argv)
{
    int flag = 0;

    if (argc >= 2)
    {
        flag = atoi(argv[1]);
        if (flag)
        {
            lpms_enable();
        }
        else
        {
            lpms_disable();
        }
    }
}
MSH_CMD_EXPORT(pms_enable_ctrl, pms_enable flag);

/* 【命令】打印睡眠模式 */
static void pms_dump_sleep_mode(void)
{
    pm_sleep_mode_dump();
}
MSH_CMD_EXPORT(pms_dump_sleep_mode, pms dump sleep_mode);

/* 【命令】打印变频模式 */
static void pms_dump_freq_mode(void)
{
    pm_freq_mode_dump();
}
MSH_CMD_EXPORT(pms_dump_freq_mode, pms dump freq_mode);

/* 【命令】打印【忙】状态 */
static void pms_dump_busy_mode(void)
{
    pm_busy_mode_dump();
}
MSH_CMD_EXPORT(pms_dump_busy_mode, pms dump busy status);

static void pms_dump_mode(void)
{
    rt_kprintf("sleep_mode=%d\n", pm_get_sleep_mode());
    rt_kprintf("freq_mode=%d\n", pm_get_freq_mode());
}
MSH_CMD_EXPORT(pms_dump_mode, pms dump sleep and freq mode);
/* PM管理入口 */
void lpms_enter(void)
{
    pm_run_tickless();
}

/* Disable SysTick */
void HAL_SysTickDisable(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void HAL_SysTick_IRQ_Disable(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

void HAL_SysTick_IRQ_Enable(void)
{
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}

/* Enable SysTick */
void HAL_SysTickEnable(void)
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void pm_bsp_enter_idle(void)
{
    __WFI();
}

void pm_bsp_enter_light(void)
{
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void pm_bsp_enter_deepsleep(void)
{
    /* Enter STOP 2 mode  */
    HAL_SysTick_IRQ_Disable();
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
    HAL_RCC_DeInit(); /* TODO: system clock reconfig */
    SystemClock_Config();
    HAL_SysTick_IRQ_Enable();
}

void pm_bsp_enter_standby(void)
{
    /* Enter STANDBY mode */
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    HAL_PWR_EnterSTANDBYMode();
}

void pm_bsp_enter_shutdown(void)
{
    /* Enter SHUTDOWNN mode */
    HAL_PWREx_EnterSHUTDOWNMode();
}

/**
 * This function will put MCU into sleep mode.
 *
 * @param mode pm_sleep_mode_t sleep mode
 */
static void sleep(uint8_t mode)
{
    switch (mode)
    {
    case PM_SLEEP_NONE:
        break;

    case PM_SLEEP_IDLE:
        pm_bsp_enter_idle();
        break;

    case PM_SLEEP_LIGHT:
        pm_bsp_enter_light();
        break;

    case PM_SLEEP_DEEP:
        pm_bsp_enter_deepsleep();
        break;

    default:
        RT_ASSERT(0);
        break;
    }
}

static uint32_t pms_irq_disable(void)
{
    return rt_hw_interrupt_disable();
}

static void pms_irq_enable(uint32_t level)
{
    rt_hw_interrupt_enable(level);
}

/**
 * This function caculate the PM tick from OS tick
 *
 * @param tick OS tick
 *
 * @return the PM tick
 */
static rt_tick_t stm32_pm_tick_from_os_tick(rt_tick_t tick)
{
    rt_uint32_t freq = stm32_lptim_get_countfreq();

    return (freq * tick / RT_TICK_PER_SECOND);
}

/**
 * This function caculate the OS tick from PM tick
 *
 * @param tick PM tick
 *
 * @return the OS tick
 */
static rt_tick_t stm32_os_tick_from_pm_tick(rt_uint32_t tick)
{
    static rt_uint32_t os_tick_remain = 0;
    rt_uint32_t ret, freq;

    freq = stm32_lptim_get_countfreq();
    ret = (tick * RT_TICK_PER_SECOND + os_tick_remain) / freq;

    os_tick_remain += (tick * RT_TICK_PER_SECOND);
    os_tick_remain %= freq;

    return ret;
}

/**
 * This function start the timer of pm
 *
 * @param pm Pointer to power manage structure
 * @param timeout How many OS Ticks that MCU can sleep
 */
static void pm_timer_start(rt_uint32_t timeout)
{
    RT_ASSERT(timeout > 0);

    if (timeout != RT_TICK_MAX)
    {
        /* Convert OS Tick to pmtimer timeout value */
        timeout = stm32_pm_tick_from_os_tick(timeout);
        if (timeout >= stm32_lptim_get_tick_max())
        {
            timeout = stm32_lptim_get_tick_max();
        }
        stm32_lptim_start(timeout);
    }
    else
    {
        //rt_kprintf("no timer!\n");
    }
}

/**
 * This function stop the timer of pm
 *
 * @param pm Pointer to power manage structure
 */
static void pm_timer_stop()
{
    /* Reset pmtimer status */
    stm32_lptim_stop();
}

/**
 * This function calculate how many OS Ticks that MCU have suspended
 *
 * @param pm Pointer to power manage structure
 *
 * @return OS Ticks
 */
static rt_tick_t pm_timer_get_tick()
{
    rt_uint32_t timer_tick;

    timer_tick = stm32_lptim_get_current_tick();

    return stm32_os_tick_from_pm_tick(timer_tick);
}

static void pm_tick_set(uint32_t ticks)
{
    rt_tick_set(ticks);
    SET_BIT(SCB->ICSR, SCB_ICSR_PENDSTSET_Msk);
}

void lpms_drv_init(void)
{
    static const struct lpms_ops pm_ops =
    {
        .sleep = sleep,
        .set_freq = NULL,
        .lptim_start = pm_timer_start,
        .lptim_stop = pm_timer_stop,
        .lptim_get_timeout = pm_timer_get_tick,
        .systick_get = rt_tick_get,
        .systick_set = pm_tick_set,
        .systick_next_timeout = rt_timer_next_timeout_tick,
        .irq_disable = pms_irq_disable,
        .irq_enable = pms_irq_enable,
    };

    lpms_init(&pm_ops, LPMS_DEFAULT_SLEEP_MODE, LPMS_DEFAULT_FREQ_MODE);
    rt_thread_idle_sethook(lpms_enter);
}
