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

binder_ctx *open_binder(const char *device) {
  binder_ctx *ctx;

  ctx = malloc(sizeof(*ctx));

  if (!ctx) {
    ERROR("Failed to allocate memory");
    exit(1);
  }

  ctx->fd = open(device, O_RDWR | O_CLOEXEC, 0);

  if (ctx->fd < 0) {
    ERROR("Failed to open binder");
    free(ctx);
    exit(1);
  }

  binder_version(ctx->fd);

  ctx->mapsize = 2 * BINDER_MAPSIZE;
  ctx->mapped =
      mmap((void *)0x100000000ul, ctx->mapsize, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (ctx->mapped == MAP_FAILED) {
    ERROR("mmap failed");
    close(ctx->fd);
    free(ctx);
    exit(1);
  }

  INFO("Memory mapped at %p", ctx->mapped);

  return ctx;
}

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

void close_binder(binder_ctx *ctx) {
  if (ctx != NULL) {
    munmap(ctx->mapped, ctx->mapsize);
    close(ctx->fd);
    free(ctx);
  }
}

void binder_thread_exit(int binder_fd) {
  int res = ioctl(binder_fd, BINDER_THREAD_EXIT, NULL);
  if (res < 0) {
    ERROR("Binder thread exit failed: %s", strerror(errno));
  }
  SUCCESS("Binder Thread Exit");
}
