/*
 * Copyright (c) 2021-2022, LPMS Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2021-08-29     zhangsz         first version
 */
#ifndef __LPMS_CONFIG_H__
#define __LPMS_CONFIG_H__

#include <rtthread.h>

#define LPMS_DEBUG

#ifdef LPMS_DEBUG
#define PM_PRINT(fmt, ...)  rt_kprintf(fmt, ##__VA_ARGS__)
#else
#define PM_PRINT(fmt, ...)
#endif

/* user power modules */
typedef enum {
    PM_NONE_ID = 0,
    PM_POWER_ID,
    PM_BOARD_ID,
    PM_BSP_ID,
    PM_MAIN_ID,
    PM_PMS_ID,
    PM_PMC_ID,
    PM_TASK_ID,
    PM_SPI_ID,
    PM_I2C_ID,
    PM_UART_ID,
    PM_CAN_ID,
    PM_ETH_ID,
    PM_SENSOR_ID,
    PM_LCD_ID,
    PM_KEY_ID,
    PM_TP_ID,
    PM_MODULE_MAX, /* lpms need */
}lpms_module_t;

#endif
