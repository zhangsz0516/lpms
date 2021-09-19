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

#ifdef __cplusplus
extern "C" {
#endif

#define LPMS_EOK            0
#define LPMS_ERROR          1
#define LPMS_ETIMEOUT       2
#define LPMS_EFULL          3
#define LPMS_ENOSYS         6

/* LPMS version information */
#define LPMS_VERSION_MAJOR      0   /**< major version number */
#define LPMS_VERSION_MINOR      0   /**< minor version number */
#define LPMS_VERSION_BUILD      1   /**< build version number */

/* LPMS version */
#define LPMS_VERSION            ((LPMS_VERSION_MAJOR * 10000) + \
                                 (LPMS_VERSION_MINOR * 100) + LPMS_VERSION_BUILD)

#define PM_TICK_MAX             0xffffffff
#define PM_FLAG_BUSY            0xBC
#define PM_FLAG_IDLE            0xDE

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
#define LPMS_DEFAULT_BUSY_SLEEP_MODE    PM_SLEEP_NONE
#endif

#ifndef LPMS_TICKLESS_THRESHOLD_TIME
#define LPMS_TICKLESS_THRESHOLD_TIME    6
#endif

#ifndef LPMS_TICKLESS_THRESHOLD_MODE
#define LPMS_TICKLESS_THRESHOLD_MODE    PM_SLEEP_IDLE
#endif

#ifndef LPMS_TICKLESS_DEFAULT_MODE
#define LPMS_TICKLESS_DEFAULT_MODE      PM_SLEEP_LIGHT
#endif

struct lpms_ops
{
    void (*sleep)(uint8_t sleep_mode);
    void (*set_freq)(uint8_t freq_mode);
    void (*lptim_start)(uint32_t timeout);
    void (*lptim_stop)(void);
    uint32_t (*lptim_get_tick)(void);
    uint32_t (*lptim_next_timeout)(void);
    uint32_t (*systick_get)(void);
    void (*systick_set)(uint32_t ticks);
    uint32_t (*systick_next_timeout)(void);
    uint32_t (*irq_disable)(void);
    void (*irq_enable)(uint32_t level);
};

typedef struct _lpms_s
{
    uint8_t init_flag; /* 是否初始化 */
    uint8_t enable_flag; /* pm框架使能 */
    uint8_t sleep_mode;
    uint8_t freq_mode;
    uint32_t freq_flag;
    uint32_t sleep_status[PM_SLEEP_MODE_MAX - 1][(PM_MODULE_MAX + 31) / 32];
    uint32_t freq_status[PM_FREQ_MODE_MAX - 1][(PM_MODULE_MAX + 31) / 32];
    uint32_t timeout[PM_BUSY_MODULE_MAX];
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

void lpms_init(const struct lpms_ops *ops, uint8_t freq_mode);
void lpms_enable(void);
void lpms_disable(void);

/* 睡眠模式、变频模式打印 */
void pm_sleep_mode_dump(void);
void pm_freq_mode_dump(void);

uint8_t pm_get_sleep_mode(void);
uint8_t pm_get_freq_mode(void);
uint8_t pm_is_enabled(void);

/* 【忙】状态打印 */
void pm_busy_mode_dump(void);

void pm_run_tickless(void);

void pm_idle_lock_request(uint16_t module_id);
void pm_idle_lock_release(uint16_t module_id);

uint32_t pm_irq_disable(void);
void pm_irq_enable(uint32_t level);

/* set and delete pm notify */
int32_t lpms_sleep_sethook(void (*hook)(uint8_t mode));
int32_t lpms_sleep_delhook(void (*hook)(uint8_t mode));
int32_t lpms_wakeup_sethook(void (*hook)(uint8_t mode));
int32_t lpms_wakeup_delhook(void (*hook)(uint8_t mode));
int32_t lpms_freq_sethook(void (*hook)(uint8_t mode));
int32_t lpms_freq_delhook(void (*hook)(uint8_t mode));

/* run pm notify */
void lpms_notify_sleep(uint8_t mode);
void lpms_notify_wakeup(uint8_t mode);
void lpms_notify_freq(uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif
