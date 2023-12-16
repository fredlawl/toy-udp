#pragma once

#include <stdlib.h>

#ifdef CONFIG_WITH_LOCKS
void *util_lock_init();
int util_lock_destroy(void *lock);
void util_lock(void *lock);
void util_unlock(void *lock);
#else
static inline void *util_lock_init() { return NULL; }
static inline int util_lock_destroy(void *lock) { return 0; }
static inline void util_lock(void *lock) {}
static inline void util_unlock(void *lock) {}
#endif
