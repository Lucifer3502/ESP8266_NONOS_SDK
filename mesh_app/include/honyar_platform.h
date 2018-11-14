
#ifndef _HONYAR_PLATFORM_H_
#define _HONYAR_PLATFORM_H_

#include "c_types.h"

#define TASK_MAX_NUM  20

#define TASK_CYCLE_TM_MS  10

#define honyar_malloc os_malloc
#define honyar_free os_free

typedef  void (*task_func_t)(void *parm);

int32_t honyar_add_task(task_func_t func, void *parm, uint32_t cycle);

int32_t honyar_del_task(task_func_t func);

int32_t honyar_pause_task(task_func_t func);

int32_t honyar_continue_task(task_func_t func);

void honyar_platform_init(void);

#endif

