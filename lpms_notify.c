#include <lpms.h>

#define LPMS_USING_HOOK

#ifdef LPMS_USING_HOOK

/* before sleep */
#ifndef LPMS_HOOK_LIST_SLEEP_SIZE
#define LPMS_HOOK_LIST_SLEEP_SIZE     4
#endif

/* wakeup */
#ifndef LPMS_HOOK_LIST_WAKE_SIZE
#define LPMS_HOOK_LIST_WAKE_SIZE        4
#endif

/* freqency change */
#ifndef LPMS_HOOK_LIST_FREQ_SIZE
#define LPMS_HOOK_LIST_FREQ_SIZE        4
#endif

static void (*pm_hook_list_sleep[LPMS_HOOK_LIST_SLEEP_SIZE])(uint8_t mode);
static void (*pm_hook_list_wake[LPMS_HOOK_LIST_WAKE_SIZE])(uint8_t mode);
static void (*pm_hook_list_freq[LPMS_HOOK_LIST_FREQ_SIZE])(uint8_t mode);

/* set sleep hook : before sleep enter */
int32_t lpms_sleep_sethook(void (*hook)(uint8_t mode))
{
    uint32_t i;
    uint32_t level;
    int32_t ret = -LPMS_EFULL;

    /* disable interrupt */
    level = pm_irq_disable();

    for (i = 0; i < LPMS_HOOK_LIST_SLEEP_SIZE; i++)
    {
        if (pm_hook_list_sleep[i] == NULL)
        {
            pm_hook_list_sleep[i] = hook;
            ret = LPMS_EOK;
            break;
        }
    }
    /* enable interrupt */
    pm_irq_enable(level);

    return ret;
}

/* delete sleep hook : before sleep enter */
int32_t lpms_sleep_delhook(void (*hook)(uint8_t mode))
{
    uint32_t i;
    uint32_t level;
    int32_t ret = -LPMS_ENOSYS;

    /* disable interrupt */
    level = pm_irq_disable();

    for (i = 0; i < LPMS_HOOK_LIST_SLEEP_SIZE; i++)
    {
        if (pm_hook_list_sleep[i] == hook)
        {
            pm_hook_list_sleep[i] = NULL;
            ret = LPMS_EOK;
            break;
        }
    }
    /* enable interrupt */
    pm_irq_enable(level);

    return ret;
}

/* set wakeup hook : after sleep exit */
int32_t lpms_wakeup_sethook(void (*hook)(uint8_t mode))
{
    uint32_t i;
    uint32_t level;
    int32_t ret = -LPMS_EFULL;

    /* disable interrupt */
    level = pm_irq_disable();

    for (i = 0; i < LPMS_HOOK_LIST_WAKE_SIZE; i++)
    {
        if (pm_hook_list_wake[i] == NULL)
        {
            pm_hook_list_wake[i] = hook;
            ret = LPMS_EOK;
            break;
        }
    }
    /* enable interrupt */
    pm_irq_enable(level);

    return ret;
}

/* delete wakeup hook : after sleep exit */
int32_t lpms_wakeup_delhook(void (*hook)(uint8_t mode))
{
    uint32_t i;
    uint32_t level;
    int32_t ret = -LPMS_ENOSYS;

    /* disable interrupt */
    level = pm_irq_disable();

    for (i = 0; i < LPMS_HOOK_LIST_WAKE_SIZE; i++)
    {
        if (pm_hook_list_wake[i] == hook)
        {
            pm_hook_list_wake[i] = NULL;
            ret = LPMS_EOK;
            break;
        }
    }
    /* enable interrupt */
    pm_irq_enable(level);

    return ret;
}

/* set freqency change hook : after freqency change exit */
int32_t lpms_freq_sethook(void (*hook)(uint8_t mode))
{
    uint32_t i;
    uint32_t level;
    int32_t ret = -LPMS_EFULL;

    /* disable interrupt */
    level = pm_irq_disable();

    for (i = 0; i < LPMS_HOOK_LIST_FREQ_SIZE; i++)
    {
        if (pm_hook_list_freq[i] == NULL)
        {
            pm_hook_list_freq[i] = hook;
            ret = LPMS_EOK;
            break;
        }
    }
    /* enable interrupt */
    pm_irq_enable(level);

    return ret;
}

/* delete freqency change hook : after freqency change exit */
int32_t lpms_freq_delhook(void (*hook)(uint8_t mode))
{
    uint32_t i;
    uint32_t level;
    int32_t ret = -LPMS_ENOSYS;

    /* disable interrupt */
    level = pm_irq_disable();

    for (i = 0; i < LPMS_HOOK_LIST_FREQ_SIZE; i++)
    {
        if (pm_hook_list_freq[i] == hook)
        {
            pm_hook_list_freq[i] = NULL;
            ret = LPMS_EOK;
            break;
        }
    }
    /* enable interrupt */
    pm_irq_enable(level);

    return ret;
}

/* run sleep notify */
void lpms_notify_sleep(uint8_t mode)
{
    uint32_t i;

    for (i = 0; i < LPMS_HOOK_LIST_SLEEP_SIZE; i++)
    {
        if (pm_hook_list_sleep[i] != NULL)
        {
            pm_hook_list_sleep[i](mode);
        }
    }
}

/* run wakeup notify */
void lpms_notify_wakeup(uint8_t mode)
{
    uint32_t i;

    for (i = 0; i < LPMS_HOOK_LIST_WAKE_SIZE; i++)
    {
        if (pm_hook_list_wake[i] != NULL)
        {
            pm_hook_list_wake[i](mode);
        }
    }
}

/* run freqency change notify */
void lpms_notify_freq(uint8_t mode)
{
    uint32_t i;

    for (i = 0; i < LPMS_HOOK_LIST_FREQ_SIZE; i++)
    {
        if (pm_hook_list_freq[i] != NULL)
        {
            pm_hook_list_freq[i](mode);
        }
    }
}

#endif

