
#ifndef _HONYAR_PLATFORM_H_
#define _HONYAR_PLATFORM_H_

#include "c_types.h"

#define HONYAR_LIBS_VERSION  "1.1.0"

#define TASK_MAX_NUM  20

#define TASK_CYCLE_TM_MS  10


typedef  void (*task_func_t)(void *parm);

void honyar_usleep(uint32_t us);

void honyar_msleep(uint32_t ms);

void honyar_sleep(uint32_t s);

void honyar_global_timer_disable(void);

uint8_t honyar_global_timer_is_disable(void);

void honyar_sys_reboot(uint32_t now);

void *honyar_malloc(uint32_t size);

void honyar_free(void *ptr);

int32_t honyar_add_task(task_func_t func, void *parm, uint32_t cycle);

int32_t honyar_del_task(task_func_t func);

int32_t honyar_pause_task(task_func_t func);

int32_t honyar_continue_task(task_func_t func);

void honyar_platform_init(void);

#endif

