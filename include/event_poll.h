#ifndef EVENT_POLL_H
#define EVENT_POLL_H

#include <sys/epoll.h>

int event_pool_create();
void event_pool_add(int epfd, int binder_fd, struct epoll_event *event);
void event_pool_remove(int epfd, int binder_fd, struct epoll_event *event);

#endif