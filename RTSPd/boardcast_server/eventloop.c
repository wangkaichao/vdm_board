
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <assert.h>

#include "util.h"
#include "eventloop.h"

struct event_loop_st
{
    int epoll_fd;
    list_t check_list;
    list_t idle_list;
};

typedef struct event_source_interface_st
{
    int (*dispatch)(event_source_t *source,
                    struct epoll_event *ep);
    int (*remove)(event_source_t *source);
}event_source_interface_t;

struct event_source_st
{
    event_source_interface_t *interface;
    event_loop_t *loop;
    list_t link;
    void *data;
};

typedef struct event_source_fd_st
{
    event_source_t base;
    int fd;
    event_loop_fd_func_t func;
    //event_loop_fd_func_t_details func_details;
}event_source_fd_t;

static int event_source_fd_dispatch(event_source_t *source,
                            struct epoll_event *ep)
{
    event_source_fd_t *fd_source = (event_source_fd_t *) source;

    return fd_source->func(fd_source->fd, fd_source->base.data,ep);
}

static int event_source_fd_remove(event_source_t *source)
{
    event_source_fd_t *fd_source =
            (event_source_fd_t *) source;
    event_loop_t *loop = source->loop;
    int fd;

    fd = fd_source->fd;
    free(source);

    return epoll_ctl(loop->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

event_source_interface_t fd_source_interface = {
        event_source_fd_dispatch,
        event_source_fd_remove
};

event_source_t *event_loop_add_fd(event_loop_t *loop,
                     int fd, event_loop_fd_func_t func,int kind,
                     void *data)
{
    event_source_fd_t *source;
    struct epoll_event ep;

    source = malloc(sizeof *source);
    if (source == NULL)
            return NULL;

    source->base.interface = &fd_source_interface;
    source->base.loop = loop;
    list_init(&source->base.link);
    source->fd = fd;
    source->func = func;
    source->base.data = data;
	
    //EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
    //EPOLLOUT：表示对应的文件描述符可以写；
    //EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
    memset(&ep, 0, sizeof ep);
    if(kind == 0) ep.events = EPOLLIN;
    else ep.events = EPOLLIN | EPOLLET;
    ep.data.ptr = source;
	
    //EPOLL_CTL_ADD：注册新的fd到epfd中；
    //EPOLL_CTL_MOD：修改已经注册的fd的监听事件；
    //EPOLL_CTL_DEL：从epfd中删除一个fd；
    if (epoll_ctl(loop->epoll_fd, EPOLL_CTL_ADD, fd, &ep) < 0)
    {
        free(source);
        return NULL;
    }
    return &source->base;
}

int event_source_fd_update(event_source_t *source,int kind)
{
    event_source_fd_t *fd_source =
            (event_source_fd_t *) source;
    event_loop_t *loop = source->loop;
    struct epoll_event ep;
    memset(&ep, 0, sizeof ep);
    if(kind == 0) ep.events = EPOLLIN;
    else ep.events = EPOLLIN|EPOLLET;
   // ep.events |= EPOLLIN|EPOLLET;
    ep.data.ptr = source;

    return epoll_ctl(loop->epoll_fd,
                     EPOLL_CTL_MOD, fd_source->fd, &ep);
}

typedef struct event_source_timer_st
{
    event_source_t base;
    int fd;
    event_loop_timer_func_t func;
}event_source_timer_t;

static int event_source_timer_dispatch(event_source_t *source,
                               struct epoll_event *ep)
{
    event_source_timer_t *timer_source =
            (event_source_timer_t *) source;
    uint64_t expires;
    int len;

    len = read(timer_source->fd, &expires, sizeof expires);
    if (len != sizeof expires)
            /* Is there anything we can do here?  Will this ever happen? */
            fprintf(stderr, "timerfd read error: %m\n");

    return timer_source->func(timer_source->base.data);
}

static int event_source_timer_remove(event_source_t *source)
{
    event_source_timer_t *timer_source =
            (event_source_timer_t *) source;

    close(timer_source->fd);
    free(source);
    return 0;
}

event_source_interface_t timer_source_interface =
{
    event_source_timer_dispatch,
    event_source_timer_remove
};

event_source_t *event_loop_add_timer(event_loop_t *loop,
                        event_loop_timer_func_t func,int kind,
                        void *data)
{
    event_source_timer_t *source;
    struct epoll_event ep;

    source = malloc(sizeof *source);
    if (source == NULL)
        return NULL;

    source->base.interface = &timer_source_interface;
    source->base.loop = loop;
    list_init(&source->base.link);

    source->fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (source->fd < 0)
    {
        fprintf(stderr, "could not create timerfd: %m\n");
        free(source);
        return NULL;
    }

    source->func = func;
    source->base.data = data;

    memset(&ep, 0, sizeof ep);
    if(kind == 0) ep.events = EPOLLIN;
    else ep.events = EPOLLIN|EPOLLET;
    //ep.events = EPOLLIN|EPOLLET;
    ep.data.ptr = source;

    if (epoll_ctl(loop->epoll_fd, EPOLL_CTL_ADD, source->fd, &ep) < 0)
    {
        close(source->fd);
        free(source);
        return NULL;
    }

    return &source->base;
}

int event_source_timer_update(event_source_t *source, int ms_delay)
{
    event_source_timer_t *timer_source =
            (event_source_timer_t *) source;
    struct itimerspec its;

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec = ms_delay / 1000;
    its.it_value.tv_nsec = (ms_delay % 1000) * 1000 * 1000;
    if (timerfd_settime(timer_source->fd, 0, &its, NULL) < 0)
    {
        fprintf(stderr, "could not set timerfd\n: %m");
        return -1;
    }

    return 0;
}

typedef struct event_source_signal_st
{
    event_source_t base;
    int fd;
    int signal_number;
    event_loop_signal_func_t func;
}event_source_signal_t;

static int event_source_signal_dispatch(event_source_t *source,
                               struct epoll_event *ep)
{
    event_source_signal_t *signal_source =
            (event_source_signal_t *) source;
    struct signalfd_siginfo signal_info;
    int len;

    len = read(signal_source->fd, &signal_info, sizeof signal_info);
    if (len != sizeof signal_info)
        /* Is there anything we can do here?  Will this ever happen? */
        fprintf(stderr, "signalfd read error: %m\n");

    return signal_source->func(signal_source->signal_number,
                               signal_source->base.data);
}

static int event_source_signal_remove(event_source_t *source)
{
    event_source_signal_t *signal_source =
            (event_source_signal_t *) source;

    close(signal_source->fd);
    free(source);
    return 0;
}

event_source_interface_t signal_source_interface =
{
    event_source_signal_dispatch,
    event_source_signal_remove
};

event_source_t *event_loop_add_signal(event_loop_t *loop,
                        int signal_number,
                        event_loop_signal_func_t func,int kind,
                        void *data)
{
    event_source_signal_t *source;
    struct epoll_event ep;
    sigset_t mask;

    source = malloc(sizeof *source);
    if (source == NULL)
        return NULL;

    source->base.interface = &signal_source_interface;
    source->base.loop = loop;
    list_init(&source->base.link);
    source->signal_number = signal_number;

    sigemptyset(&mask);
    sigaddset(&mask, signal_number);
    source->fd = signalfd(-1, &mask, 0);
    if (source->fd < 0)
    {
        fprintf(stderr, "could not create fd to watch signal\n: %m");
        free(source);
        return NULL;
    }
    sigprocmask(SIG_BLOCK, &mask, NULL);

    source->func = func;
    source->base.data = data;

    memset(&ep, 0, sizeof ep);
    if(kind == 0) ep.events = EPOLLIN;
    else ep.events = EPOLLIN|EPOLLET;
    //ep.events = EPOLLIN;
    ep.data.ptr = source;

    if (epoll_ctl(loop->epoll_fd, EPOLL_CTL_ADD, source->fd, &ep) < 0)
    {
        close(source->fd);
        free(source);
        return NULL;
    }

    return &source->base;
}

typedef struct event_source_idle_st
{
    event_source_t base;
    event_loop_idle_func_t func;
}event_source_idle_t;

static int event_source_idle_remove(event_source_t *source)
{
    free(source);

    return 0;
}

event_source_interface_t idle_source_interface =
{
    NULL,
    event_source_idle_remove
};

event_source_t *event_loop_add_idle(event_loop_t *loop,
                       event_loop_idle_func_t func,
                       void *data)
{
    event_source_idle_t *source;

    source = malloc(sizeof *source);
    if (source == NULL)
            return NULL;

    source->base.interface = &idle_source_interface;
    source->base.loop = loop;

    source->func = func;
    source->base.data = data;

    list_insert(loop->idle_list.prev, &source->base.link);

    return &source->base;
}

void event_source_check(event_source_t *source)
{
    list_insert(source->loop->check_list.prev, &source->link);
}

int event_source_remove(event_source_t *source)
{
    if (!list_empty(&source->link))
        list_remove(&source->link);

    source->interface->remove(source);

    return 0;
}

event_loop_t *event_loop_create(void)
{
    event_loop_t *loop;

    loop = malloc(sizeof *loop);
    if (loop == NULL)
        return NULL;

    loop->epoll_fd = epoll_create(1024);
    if (loop->epoll_fd < 0)
    {
        free(loop);
        return NULL;
    }
    list_init(&loop->check_list);
    list_init(&loop->idle_list);

    return loop;
}

void event_loop_destroy(event_loop_t *loop)
{
    close(loop->epoll_fd);
    free(loop);
}

static int post_dispatch_check(event_loop_t *loop)
{
    struct epoll_event ep;
    event_source_t *source, *next;
    int n;

    ep.events = 0;
    n = 0;
    list_for_each_safe(source, next, &loop->check_list, link)
            n += source->interface->dispatch(source, &ep);

    return n;
}

static void dispatch_idle_sources(event_loop_t *loop)
{
    event_source_idle_t *source, *next;

    list_for_each_safe(source, next, &loop->idle_list, base.link)
    {
        source->func(source->base.data);
        event_source_remove(&source->base);
    }
}

int event_loop_dispatch(event_loop_t *loop, int timeout)
{
    struct epoll_event ep[32];
    event_source_t *source;
    int i, count, n;

    dispatch_idle_sources(loop);

    count = epoll_wait(loop->epoll_fd, ep, ARRAY_LENGTH(ep), timeout);
    if (count < 0)
        return -1;
    n = 0;
    for (i = 0; i < count; i++)
    {
        source = ep[i].data.ptr;
        n += source->interface->dispatch(source, &ep[i]);
    }

    while (n > 0)
        n = post_dispatch_check(loop);

    return 0;
}

int event_loop_get_fd(event_loop_t *loop)
{
    return loop->epoll_fd;
}
