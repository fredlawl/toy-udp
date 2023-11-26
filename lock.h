#ifndef __LOCK_H
#define __LOCK_H
void *util_lock_init();
int util_lock_destroy(void *lock);
void util_lock(void *lock);
void util_unlock(void *lock);
#endif