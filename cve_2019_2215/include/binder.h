#ifndef BINDER
#define BINDER

#include <linux/android/binder.h>
#include <stdbool.h>
#include <stddef.h>

#define BINDER_DEVICE "/dev/binder"
#define BINDER_MAPSIZE (128 * 1024)

int open_binder(const char *device);
void binder_version(int binder_fd);
void binder_thread_exit(int binder_fd);
void close_binder(int binder_fd);

#endif