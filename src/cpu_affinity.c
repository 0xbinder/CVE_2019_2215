#define _GNU_SOURCE 1
#include <errno.h>
#include <sched.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../include/log.h"

void pin_cpu(int cpu) {
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(cpu, &set);
  int res = sched_setaffinity(0, sizeof(set), &set);
  if (res < 0) {
    ERROR("[-] Sched set affinity failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  SUCCESS("Binding to %dth core", cpu);
}
