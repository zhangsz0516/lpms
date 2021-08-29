/*
 * Copyright (c) 2021-2022, LPMS Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2021-08-29     zhangsz         first version
 */
#ifndef __LPMS_H__
#define __LPMS_H__

/* Low-Power Management System */
#include <stdint.h>
#include <lpms_config.h>

#define PM_TICK_MAX    0xffffffff

/* sleep modes */
typedef enum
{
    PM_SLEEP_NONE = 0,
    PM_SLEEP_IDLE,
    PM_SLEEP_LIGHT,
    PM_SLEEP_DEEP, /* default */
    PM_SLEEP_MODE_MAX,
}pm_sleep_mode_t;

/* freq modes */
typedef enum
{
    PM_FREQ_HIGH = 0,
    PM_FREQ_NORMAL,
    PM_FREQ_LOW, /* default */
    PM_FREQ_MODE_MAX,
}pm_freq_mode_t;

#ifndef LPMS_DEFAULT_BUSY_SLEEP_MODE
#define LPMS_DEFAULT_BUSY_SLEEP_MODE    PM_SLEEP_IDLE
#endif

#ifndef LPMS_TICKLESS_THRESHOLD_TIME
#define LPMS_TICKLESS_THRESHOLD_TIME    5
#endif

#ifndef LPMS_TICKLESS_THRESHOLD_MODE
#define LPMS_TICKLESS_THRESHOLD_MODE    PM_SLEEP_IDLE
#endif

struct lpms_ops
{
    void (*sleep)(uint8_t sleep_mode);
    void (*set_freq)(uint8_t freq_mode);
    void (*lptim_start)(uint32_t timeout);
    void (*lptim_stop)(void);
    uint32_t (*lptim_get_timeout)(void);
    uint32_t (*systick_get)(void);
    void (*systick_set)(uint32_t ticks);
    uint32_t (*systick_next_timeout)(void);
    uint32_t (*irq_disable)(void);
    void (*irq_enable)(uint32_t level);
};

typedef struct
{
    uint8_t init_flag; /* 是否初始化 */
    uint8_t enable_flag; /* pm框架使能 */
    uint8_t sleep_mode;
    uint8_t freq_mode;
    uint16_t module_id;
    uint32_t timeout;
    uint32_t sleep_status[PM_SLEEP_MODE_MAX - 1][(PM_MODULE_MAX + 31) / 32];
    uint32_t freq_status[PM_FREQ_MODE_MAX - 1][(PM_MODULE_MAX + 31) / 32];
    const struct lpms_ops *ops;
}lpms_sys_t;

/* 睡眠模式请求与释放 */
void pm_sleep_request(uint16_t module_id, uint8_t mode);
void pm_sleep_release(uint16_t module_id, uint8_t mode);

/* 变频模式请求与释放 */
void pm_freq_request(uint16_t module_id, uint8_t mode);
void pm_freq_release(uint16_t module_id, uint8_t mode);

/* 【忙】状态延时后再进入睡眠 */
void pm_busy_set(uint16_t module_id, uint32_t ticks);
void pm_busy_clear(uint16_t module_id);

void lpms_init(const struct lpms_ops *ops, uint8_t sleep_mode, uint8_t freq_mode);
void lpms_enable(void);
void lpms_disable(void);

/* 睡眠模式、变频模式打印 */
void pm_sleep_mode_dump(void);
void pm_freq_mode_dump(void);

uint8_t pm_get_sleep_mode(void);
uint8_t pm_get_freq_mode(void);

/* 【忙】状态打印 */
void pm_busy_mode_dump(void);

void pm_run_tickless(void);

#endif
