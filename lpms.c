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

/* lpms control block */
static lpms_sys_t _pm_sys = { 0 };

/* interrupt disable */
uint32_t pm_irq_disable(void)
{
    if (_pm_sys.ops == NULL)
        return 0;

    if (_pm_sys.ops->irq_disable != NULL)
        return _pm_sys.ops->irq_disable();

    return 0;
}

/* interrupt enable */
void pm_irq_enable(uint32_t level)
{
    if (_pm_sys.ops == NULL)
        return;

    if (_pm_sys.ops->irq_enable != NULL)
        _pm_sys.ops->irq_enable(level);
}

/* get os system tick */
static uint32_t pm_get_tick(void)
{
    if (_pm_sys.ops == NULL)
        return 0;

    if (_pm_sys.ops->systick_get != NULL)
        return _pm_sys.ops->systick_get();

    return 0;
}

/* set os system tick */
static void pm_set_tick(uint32_t ticks)
{
    if (_pm_sys.ops == NULL)
        return;

    if (_pm_sys.ops->systick_set != NULL)
        _pm_sys.ops->systick_set(ticks);
}

/* get os system timer next timeout */
static uint32_t pm_next_timeout(void)
{
    if (_pm_sys.ops == NULL)
        return PM_TICK_MAX;

    if (_pm_sys.ops->systick_next_timeout != NULL)
        return _pm_sys.ops->systick_next_timeout();

    return PM_TICK_MAX;
}

/* get lptimer timer next timeout */
static uint32_t lptimer_next_timeout(void)
{
    if (_pm_sys.ops == NULL)
        return PM_TICK_MAX;

    if (_pm_sys.ops->lptim_next_timeout != NULL)
        return _pm_sys.ops->lptim_next_timeout();

    return PM_TICK_MAX;
}

/* lptimer start */
static void lptimer_start(uint32_t timeout)
{
    if (_pm_sys.ops == NULL)
        return;

    if (_pm_sys.ops->lptim_start != NULL)
        _pm_sys.ops->lptim_start(timeout);
}

/* lptimer stop */
static void lptimer_stop(void)
{
    if (_pm_sys.ops == NULL)
        return;

    if (_pm_sys.ops->lptim_stop != NULL)
        _pm_sys.ops->lptim_stop();
}

/* lptimer get timeout tick */
static uint32_t lptimer_get_timeout(void)
{
    if (_pm_sys.ops == NULL)
        return PM_TICK_MAX;

    if (_pm_sys.ops->lptim_get_tick != NULL)
        return _pm_sys.ops->lptim_get_tick();

    return PM_TICK_MAX;
}

/* lpms sleep enter */
static void pm_sleep(uint8_t sleep_mode)
{
    if (_pm_sys.ops == NULL)
        return;

    if (_pm_sys.ops->sleep != NULL)
        _pm_sys.ops->sleep(sleep_mode);
}

/* lpms change system frequency enter */
void _pm_set_freq(uint8_t freq_mode)
{
    if (_pm_sys.ops == NULL)
        return;

    if (_pm_sys.ops->set_freq != NULL)
        _pm_sys.ops->set_freq(freq_mode);
}

/* ??????????????????????????????????????????????????? */
void pm_sleep_request(uint16_t module_id, uint8_t mode)
{
    uint32_t level;

    if (module_id >= PM_MODULE_MAX)
    {
        return;
    }

    if (mode >= (PM_SLEEP_MODE_MAX - 1))
    {
        return;
    }

    level = pm_irq_disable();
    _pm_sys.sleep_status[mode][module_id / 32] |= 1 << (module_id % 32);
    pm_irq_enable(level);
}

void pm_idle_lock_request(uint16_t module_id)
{
    pm_sleep_request(module_id, PM_SLEEP_IDLE);
}

/* ???????????????????????????????????????????????????????????? */
void pm_sleep_release(uint16_t module_id, uint8_t mode)
{
    uint32_t level;

    if (module_id >= PM_MODULE_MAX)
    {
        return;
    }

    if (mode >= (PM_SLEEP_MODE_MAX - 1))
    {
        return;
    }

    level = pm_irq_disable();
    _pm_sys.sleep_status[mode][module_id / 32] &= ~(1 << (module_id % 32));
    pm_irq_enable(level);
}

void pm_idle_lock_release(uint16_t module_id)
{
    pm_sleep_release(module_id, PM_SLEEP_IDLE);
}

/* ?????????????????? */
uint8_t select_freq_mode(void)
{
    uint8_t index;
    uint16_t len;

    for (index = 0; index < PM_FREQ_MODE_MAX -1; index++)
    {
        for (len = 0; len < ((PM_MODULE_MAX + 31) / 32); len++)
        {
            if (_pm_sys.freq_status[index][len] != 0x00)
                return index;
        }
    }

    return PM_FREQ_LOW; /* ?????????????????? */
}

/* ??????????????????????????????????????????????????? */
void pm_freq_request(uint16_t module_id, uint8_t mode)
{
    uint32_t level;

    if (module_id >= PM_MODULE_MAX)
    {
        return;
    }

    if (mode >= (PM_FREQ_MODE_MAX - 1))
    {
        return;
    }

    level = pm_irq_disable();
    _pm_sys.freq_status[mode][module_id / 32] |= 1 << (module_id % 32);
    if (mode < _pm_sys.freq_mode) /* ???????????? */
    {
        _pm_set_freq(mode); /* ?????????????????? */
        _pm_sys.freq_mode = mode; /* ???????????????????????? */
    }
    pm_irq_enable(level);
}

/* ???????????????????????????????????????????????????????????? */
void pm_freq_release(uint16_t module_id, uint8_t mode)
{
    uint32_t level;

    if (module_id >= PM_MODULE_MAX)
    {
        return;
    }

    if (mode >= (PM_FREQ_MODE_MAX - 1))
    {
        return;
    }

    level = pm_irq_disable();
    _pm_sys.freq_status[mode][module_id / 32] &= ~(1 << (module_id % 32));
    _pm_sys.freq_flag = 0x01; /* idle?????????????????? */
    pm_irq_enable(level);
}

/* ????????????????????? */
void pm_busy_set(uint16_t module_id, uint32_t ticks)
{
    uint32_t level;

    if (module_id >= PM_BUSY_MODULE_MAX)
    {
        return;
    }

    if ((ticks < 1) || (ticks >= PM_TICK_MAX / 2))
    {
        return;
    }

    level = pm_irq_disable();
    _pm_sys.timeout[module_id] = pm_get_tick() + ticks;
    pm_irq_enable(level);
}

/* ????????????????????? */
void pm_busy_clear(uint16_t module_id)
{
    uint32_t level;

    if (module_id >= PM_BUSY_MODULE_MAX)
    {
        return;
    }

    level = pm_irq_disable();
    _pm_sys.timeout[module_id] = 0;
    pm_irq_enable(level);
}

/* ?????????????????? */
static uint8_t select_sleep_mode(void)
{
    uint8_t index;
    uint16_t len;

    for (index = 0; index < PM_SLEEP_MODE_MAX -1; index++)
    {
        for (len = 0; len < ((PM_MODULE_MAX + 31) / 32); len++)
        {
            if (_pm_sys.sleep_status[index][len] != 0x00)
                return index;
        }
    }

    return PM_SLEEP_DEEP;  /* ??????????????? */
}

uint8_t pm_get_sleep_mode(void)
{
    return _pm_sys.sleep_mode;
}

uint8_t pm_get_freq_mode(void)
{
    return _pm_sys.freq_mode;
}

/* ????????????????????? */
static uint8_t pm_check_busy(void)
{
    uint32_t index = 0;

    for (index = 0; index < PM_BUSY_MODULE_MAX; index++)
    {
        if (_pm_sys.timeout[index] > pm_get_tick())
        {
            return PM_FLAG_BUSY; /* ???????????????timeout????????? */
        }
    }

    return PM_FLAG_IDLE; /* ???????????????????????????????????? */
}

/* ?????????????????? */
void pm_sleep_mode_dump(void)
{
    uint8_t index;
    uint16_t len;

    PM_PRINT("+-------------+--------------+\n");
    PM_PRINT("| Sleep Mode  |    Value     |\n");
    PM_PRINT("+-------------+--------------+\n");
    for (index = 0; index < PM_SLEEP_MODE_MAX -1; index++)
    {
        for (len = 0; len < ((PM_MODULE_MAX + 31) / 32); len++)
        {
            PM_PRINT("| Mode[%d] : %d |  0x%08x  |\n", index, len,
                _pm_sys.sleep_status[index][len]);
        }
    }
    PM_PRINT("+-------------+--------------+\n");
}

/* ?????????????????? */
void pm_freq_mode_dump(void)
{
    uint8_t index;
    uint16_t len;

    PM_PRINT("+-------------+--------------+\n");
    PM_PRINT("|  Freq Mode  |    Value     |\n");
    PM_PRINT("+-------------+--------------+\n");
    for (index = 0; index < PM_FREQ_MODE_MAX -1; index++)
    {
        for (len = 0; len < ((PM_MODULE_MAX + 31) / 32); len++)
        {
            PM_PRINT("| Mode[%d] : %d |  0x%08x  |\n", index, len,
                _pm_sys.freq_status[index][len]);
        }
    }
    PM_PRINT("+-------------+--------------+\n");
}

/* ????????????????????? */
void pm_busy_mode_dump(void)
{
    uint32_t index = 0;

    PM_PRINT("+-------------+--------------+\n");
    PM_PRINT("|  module_id  | remain ticks |\n");
    PM_PRINT("+-------------+--------------+\n");
    for (index = 0; index < PM_BUSY_MODULE_MAX; index++)
    {
        if (_pm_sys.timeout[index] > pm_get_tick())
        {
            PM_PRINT("|  %3d        |  0x%08x  |\n", index,
                _pm_sys.timeout[index] - pm_get_tick());
        }
    }
    PM_PRINT("+-------------+--------------+\n");
}

void lpms_init(const struct lpms_ops *ops, uint8_t freq_mode)
{
    if (_pm_sys.init_flag)
        return;

    _pm_sys.init_flag = 0x01;
    _pm_sys.enable_flag = 0x00; /* ??????????????? */
    _pm_sys.sleep_mode = PM_SLEEP_DEEP;
    _pm_sys.freq_mode = freq_mode;
    _pm_sys.ops = ops;
}

void lpms_enable(void)
{
    _pm_sys.enable_flag = 0x01;
}

void lpms_disable(void)
{
    _pm_sys.enable_flag = 0x00;
}

uint8_t pm_is_enabled(void)
{
    return _pm_sys.enable_flag;
}

/* ??????PM ????????????*/
static void pm_freq_handle(void)
{
    uint8_t freq_mode;

    if (_pm_sys.freq_flag == 0x01)
    {
        _pm_sys.freq_flag = 0x00;
        freq_mode = select_freq_mode();

        /* ????????????????????????????????????????????????????????????????????? */
        if (freq_mode != _pm_sys.freq_mode)
        {
            _pm_set_freq(freq_mode);
            _pm_sys.freq_mode = freq_mode;
            /* ??????????????? */
            lpms_notify_freq(_pm_sys.freq_mode);
        }
    }
}

/* ??????PM Tickless ??????*/
void pm_run_tickless(void)
{
    uint32_t level;
    uint32_t timeout_tick = 0;
    uint32_t delta_tick = 0;
    uint8_t lptim_flag = 0x00;
    uint8_t sleep_mode = PM_SLEEP_DEEP;

    /* PM?????????????????? */
    if (!pm_is_enabled())
        return;

    level = pm_irq_disable();

    /* ???????????????????????????????????????????????? */
    _pm_sys.sleep_mode = select_sleep_mode();

    /* ????????????????????? */
    if (pm_check_busy() == PM_FLAG_BUSY)
    {
        sleep_mode = LPMS_DEFAULT_BUSY_SLEEP_MODE;
        if (sleep_mode < _pm_sys.sleep_mode)
        {
            _pm_sys.sleep_mode = sleep_mode; /* ??????BUSY????????????????????? */
        }
    }

    /* tickless threshold check */
    timeout_tick = pm_next_timeout(); /* system-timer timeout */
    timeout_tick -= pm_get_tick();
    if (timeout_tick <= LPMS_TICKLESS_THRESHOLD_TIME)
    {
        sleep_mode = LPMS_TICKLESS_THRESHOLD_MODE;
        if (sleep_mode < _pm_sys.sleep_mode)
        {
            _pm_sys.sleep_mode = sleep_mode;
        }
    }

    /* ????????????????????????????????? */
    if (_pm_sys.sleep_mode >= LPMS_TICKLESS_DEFAULT_MODE)
    {
        timeout_tick = lptimer_next_timeout();
        timeout_tick -= pm_get_tick();
        lptimer_start(timeout_tick);
        lptim_flag = 0x01;
    }

    /* ??????????????? */
    lpms_notify_sleep(_pm_sys.sleep_mode);

    /* ???????????? */
    pm_sleep(_pm_sys.sleep_mode);

    /* ??????????????? */
    lpms_notify_wakeup(_pm_sys.sleep_mode);

    /* ????????????????????????????????? */
    if (_pm_sys.sleep_mode >= LPMS_TICKLESS_DEFAULT_MODE)
    {
        if (lptim_flag == 0x01)
        {
            delta_tick = lptimer_get_timeout();
            lptimer_stop(); /* ??????lptimer */
            pm_set_tick(pm_get_tick() + delta_tick); /* ??????systick?????? */
            lptim_flag = 0x00;
        }
    }

    /* ???????????? */
    pm_freq_handle();

    pm_irq_enable(level);
}
