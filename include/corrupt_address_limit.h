#ifndef CORRUPT_ADDRESS_LIMIT
#define CORRUPT_ADDRESS_LIMIT

#include "binder.h"
void corrupt_address_limit(binder_ctx *ctx,
                           struct task_struct **leak_task_struct);

#endif