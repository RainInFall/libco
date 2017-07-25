#ifndef __CO_THREAD_H__
#define __CO_THREAD_H__

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct co_thread_t co_thread_t;

typedef void (*co_thread_func_t)(void*);

int co_thread_init(void);

size_t co_thread_size(size_t stack_size);

int co_thread_create(co_thread_t* thread,
                     size_t stack_size,
                     co_thread_func_t entry,
                     void* data);

bool co_thread_is_running(co_thread_t* thread);

co_thread_t* co_thread_current(void);

int co_thread_switch(co_thread_t* next_thread);
/*Should not call in main function*/
int co_thread_yield(void);

void co_thread_join(co_thread_t* thread);

#ifdef __cplusplus
}
#endif

#endif /* __CO_THREAD_H__ */
