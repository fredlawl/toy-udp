#include "lock.h"
#include <pthread.h>
#include <stdlib.h>

void *util_lock_init() {
  pthread_mutex_t *lock;
  lock = malloc(sizeof(*lock));
  if (!lock) {
    return NULL;
  }
  pthread_mutex_init(lock, NULL);
  return lock;
}

int util_lock_destroy(void *lock) {
  int ret = pthread_mutex_destroy((pthread_mutex_t *)lock);
  free(lock);
  return ret;
}

void util_lock(void *lock) { pthread_mutex_lock((pthread_mutex_t *)lock); }

void util_unlock(void *lock) { pthread_mutex_unlock((pthread_mutex_t *)lock); }