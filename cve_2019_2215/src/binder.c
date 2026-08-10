#include <errno.h>
#include <fcntl.h>
#include <linux/android/binder.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../include/binder.h"
#include "../include/log.h"

// open binder

int open_binder(const char *device) {
  int fd = open(device, O_RDONLY);
  if (fd < 0) {
    ERROR("Failed to open binder: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  SUCCESS("binder fd: 0x%x", fd);
  binder_version(fd);
  return fd;
}

// get binder version

void binder_version(int binder_fd) {
  struct binder_version vers;
  vers.protocol_version = BINDER_CURRENT_PROTOCOL_VERSION;
  int result = ioctl(binder_fd, BINDER_VERSION, &vers);
  if (result < 0) {
    ERROR("Failed to get binder version: %s", strerror(errno));
    close(binder_fd);
    exit(EXIT_FAILURE);
  }
}

// close binder

void close_binder(int binder_fd) { close(binder_fd); }

// binder thread exit

void binder_thread_exit(int binder_fd) {
  int res = ioctl(binder_fd, BINDER_THREAD_EXIT, NULL);
  if (res < 0) {
    ERROR("Binder thread exit failed: %s", strerror(errno));
  }
  SUCCESS("Binder Thread Exit");
}
