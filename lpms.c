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
static uint32_t pm_irq_disable(void)
{
    if (_pm_sys.ops->irq_disable != NULL)
        return _pm_sys.ops->irq_disable();

    return 0;
}

/* interrupt enable */
static void pm_irq_enable(uint32_t level)
{
    if (_pm_sys.ops->irq_enable != NULL)
        _pm_sys.ops->irq_enable(level);
}

/* get os system tick */
static uint32_t pm_get_tick(void)
{
    if (_pm_sys.ops->systick_get != NULL)
        return _pm_sys.ops->systick_get();

    return 0;
}

/* set os system tick */
static void pm_set_tick(uint32_t ticks)
{
    if (_pm_sys.ops->systick_set != NULL)
        _pm_sys.ops->systick_set(ticks);
}

/* get os system timer next timeout */
static uint32_t pm_next_timeout(void)
{
    if (_pm_sys.ops->systick_next_timeout != NULL)
        return _pm_sys.ops->systick_next_timeout();

    return PM_TICK_MAX;
}

/* lptimer start */
static void lptimer_start(uint32_t timeout)
{
    if (_pm_sys.ops->lptim_start != NULL)
        _pm_sys.ops->lptim_start(timeout);
}

/* lptimer stop */
static void lptimer_stop(void)
{
    if (_pm_sys.ops->lptim_stop != NULL)
        _pm_sys.ops->lptim_stop();
}

/* lptimer get timeout tick */
static uint32_t lptimer_get_timeout(void)
{
    if (_pm_sys.ops->lptim_get_timeout != NULL)
        return _pm_sys.ops->lptim_get_timeout();

    return PM_TICK_MAX;
}

/* lpms sleep enter */
static void pm_sleep(uint8_t sleep_mode)
{
    if (_pm_sys.ops->sleep != NULL)
        _pm_sys.ops->sleep(sleep_mode);
}

/* lpms change system frequency enter */
void _pm_set_freq(uint8_t freq_mode)
{
    if (_pm_sys.ops->set_freq != NULL)
        _pm_sys.ops->set_freq(freq_mode);
}

/* 睡眠模式请求：一般向高功耗方向请求 */
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

/* 请求睡眠模式后释放：释放已请求的睡眠模式 */
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

/* 决策频率模式 */
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

    return PM_FREQ_LOW; /* 默认运行低频 */
}

/* 频率模式请求：一般向高频率方向请求 */
void pm_freq_request(uint16_t module_id, uint8_t mode)
{
    uint32_t level;
    uint8_t cur_mode;

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
    cur_mode = select_freq_mode();
    if (mode < cur_mode) /* 请求高频 */
    {
        _pm_set_freq(mode); /* 进行升频动作 */
        _pm_sys.freq_mode = mode; /* 更新系统频率模式 */
    }
    pm_irq_enable(level);
}

/* 请求频率模式后释放：释放已请求的频率模式 */
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
    pm_irq_enable(level);
}

/* 设置【忙】状态 */
void pm_busy_set(uint16_t module_id, uint32_t ticks)
{
    uint32_t level;

    if (module_id >= PM_MODULE_MAX)
    {
        return;
    }

    if ((ticks < 1) || (ticks >= PM_TICK_MAX / 2))
    {
        return;
    }

    level = pm_irq_disable();
    if (_pm_sys.timeout < pm_get_tick() + ticks)
    {
        _pm_sys.timeout = pm_get_tick() + ticks;
        _pm_sys.module_id = module_id;
    }
    pm_irq_enable(level);
}

/* 清楚【忙】状态 */
void pm_busy_clear(uint16_t module_id)
{
    uint32_t level;

    if (module_id >= PM_MODULE_MAX)
    {
        return;
    }

    level = pm_irq_disable();
    if (_pm_sys.module_id == module_id)
    {
        _pm_sys.timeout = 0;
        _pm_sys.module_id = PM_MODULE_MAX;
    }
    pm_irq_enable(level);
}

/* 决策睡眠模式 */
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

    return PM_SLEEP_DEEP;  /* 默认深睡眠 */
}

uint8_t pm_get_sleep_mode(void)
{
    return _pm_sys.sleep_mode;
}

uint8_t pm_get_freq_mode(void)
{
    return _pm_sys.freq_mode;
}

/* 检查【忙】状态 */
static uint8_t pm_check_busy(void)
{
    if (_pm_sys.timeout < pm_get_tick())
    {
        return 0x00; /* idle */
    }
    else
    {
        return 0x01; /* busy */
    }
}

/* 打印睡眠模式 */
void pm_sleep_mode_dump(void)
{
    uint8_t index;
    uint16_t len;

    PM_PRINT("| Sleep Mode |    Value     |\n");
    PM_PRINT("+------------+--------------+\n");
    for (index = 0; index < PM_SLEEP_MODE_MAX -1; index++)
    {
        for (len = 0; len < ((PM_MODULE_MAX + 31) / 32); len++)
        {
            PM_PRINT("| %d          |  0x%08x  |\n", index,
                _pm_sys.sleep_status[index][len]);
        }
    }
    PM_PRINT("+------------+--------------+\n");
}

/* 打印变频模式 */
void pm_freq_mode_dump(void)
{
    uint8_t index;
    uint16_t len;

    PM_PRINT("| Freq Mode  |    Value     |\n");
    PM_PRINT("+------------+--------------+\n");
    for (index = 0; index < PM_FREQ_MODE_MAX -1; index++)
    {
        for (len = 0; len < ((PM_MODULE_MAX + 31) / 32); len++)
        {
            PM_PRINT("| %d          |  0x%08x  |\n", index,
                _pm_sys.freq_status[index][len]);
        }
    }
    PM_PRINT("+------------+--------------+\n");
}

/* 打印【忙】状态 */
void pm_busy_mode_dump(void)
{
    PM_PRINT("|  module_id | remain ticks |\n");
    PM_PRINT("+------------+--------------+\n");
    if (_pm_sys.timeout >= pm_get_tick())
    {
        PM_PRINT("| %d          |  0x%08x  |\n", _pm_sys.module_id,
            _pm_sys.timeout - pm_get_tick());
    }
    PM_PRINT("+------------+--------------+\n");
}

void lpms_init(const struct lpms_ops *ops)
{
    if (_pm_sys.init_flag)
        return;

    _pm_sys.init_flag = 0x01;
    _pm_sys.enable_flag = 0x00; /* 默认不工作 */
    _pm_sys.sleep_mode = PM_SLEEP_DEEP;
    _pm_sys.freq_mode = PM_FREQ_LOW;
    _pm_sys.timeout = 0x00;
    _pm_sys.module_id = PM_MODULE_MAX;
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

/* 运行PM 变频机制*/
static void pm_freq_handle(void)
{
    uint8_t freq_mode;

    freq_mode = select_freq_mode();

    /* 如果当前的频率不等于决策的频率，运行决策的频率 */
    if (freq_mode != _pm_sys.freq_mode)
    {
        _pm_set_freq(freq_mode);
        _pm_sys.freq_mode = freq_mode;
    }
}

/* 运行PM Tickless 机制*/
void pm_run_tickless(void)
{
    uint32_t level;
    uint32_t timeout_tick = 0;
    uint32_t delta_tick = 0;
    uint8_t lptim_flag = 0x00;
    uint8_t sleep_mode = PM_SLEEP_DEEP;

    /* PM框架是否使能 */
    if (!pm_is_enabled())
        return;

    level = pm_irq_disable();

    /* 睡眠模式请求决策出：最大睡眠模式 */
    _pm_sys.sleep_mode = select_sleep_mode();

    /* 【忙】状态检查 */
    if (pm_check_busy())
    {
        sleep_mode = LPMS_DEFAULT_BUSY_SLEEP_MODE;
        if (sleep_mode < _pm_sys.sleep_mode)
        {
            _pm_sys.sleep_mode = sleep_mode;
        }
    }

    /* tickless threshold check */
    timeout_tick = pm_next_timeout();
    timeout_tick -= pm_get_tick();
    if (timeout_tick <= LPMS_TICKLESS_THRESHOLD_TIME)
    {
        sleep_mode = LPMS_TICKLESS_THRESHOLD_MODE;
        if (sleep_mode < _pm_sys.sleep_mode)
        {
            _pm_sys.sleep_mode = sleep_mode;
        }
    }

    /* 进入【深睡眠】处理逻辑 */
    if (_pm_sys.sleep_mode == PM_SLEEP_DEEP)
    {
        lptimer_start(timeout_tick);
        lptim_flag = 0x01;
    }

    /* 执行睡眠 */
    pm_sleep(_pm_sys.sleep_mode);

    /* 退出【深睡眠】处理逻辑 */
    if (_pm_sys.sleep_mode == PM_SLEEP_DEEP)
    {
        if (lptim_flag == 0x01)
        {
            delta_tick = lptimer_get_timeout();
            lptimer_stop(); /* 关闭lptimer */
            pm_set_tick(pm_get_tick() + delta_tick); /* 系统systick补偿 */
        }
    }

    /* 变频机制 */
    pm_freq_handle();

    pm_irq_enable(level);
}
