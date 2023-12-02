#pragma once

void *util_lock_init();
int util_lock_destroy(void *lock);
void util_lock(void *lock);
void util_unlock(void *lock);
