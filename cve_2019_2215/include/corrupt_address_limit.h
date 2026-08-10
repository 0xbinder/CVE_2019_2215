#ifndef CORRUPT_ADDRESS_LIMIT
#define CORRUPT_ADDRESS_LIMIT

void corrupt_address_limit(int binder_fd, struct task_struct **leak_task_struct,
                           void **mapped_memory);

#endif