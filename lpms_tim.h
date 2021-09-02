/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2019-05-06     Zero-Free       first version
 */

#ifndef  __LPMS_TIM_H__
#define  __LPMS_TIM_H__

#include <rtthread.h>

rt_uint32_t stm32_lptim_get_countfreq(void);
rt_uint32_t stm32_lptim_get_tick_max(void);
rt_uint32_t stm32_lptim_get_current_tick(void);

rt_err_t stm32_lptim_start(rt_uint32_t load);
void stm32_lptim_stop(void);

#endif /* __LPMS_TIM_H__ */
