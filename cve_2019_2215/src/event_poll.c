#include "../include/event_poll.h"
#include "../include/log.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

int event_pool_create() {
  int epfd = epoll_create(1000);
  if (epfd < 0) {
    ERROR("Failed to epoll create: %s", strerror(errno));
    return -1;
  }
  SUCCESS("Epoll fd: 0x%x", epfd);
  return epfd;
}

void event_pool_add(int epfd, int binder_fd, struct epoll_event *event) {
  int res = epoll_ctl(epfd, EPOLL_CTL_ADD, binder_fd, event);
  if (res < 0) {
    ERROR("Failed to EPOLL_CTL_ADD: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  SUCCESS("EPOLL_CTL_ADD");
}

void event_pool_remove(int epfd, int binder_fd, struct epoll_event *event) {
  int res = epoll_ctl(epfd, EPOLL_CTL_DEL, binder_fd, event);
  if (res < 0) {
    ERROR("Failed to EPOLL_CTL_DEL: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  SUCCESS("EPOLL_CTL_DEL");
}