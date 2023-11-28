#include <stdlib.h>

#include "lock.h"

inline void *util_lock_init() { return NULL; }

inline int util_lock_destroy(void *lock) { return 0; }

inline void util_lock(void *lock) {}

inline void util_unlock(void *lock) {}