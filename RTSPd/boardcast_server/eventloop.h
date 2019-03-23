/****************************************************************************
**
** Copyright (C) 2011 好视网络科技有限公司.
** All rights reserved.
**
** author:  wangyuman
** mail:    wangym@gvtv.com.cn
** date:    2011-09-30
** des:     事件循环处理相关函数
**
****************************************************************************/
#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "util.h"

//typedef int (*event_loop_fd_func_t_details)(int fd, void *data,struct epoll_event *ep);
typedef int (*event_loop_fd_func_t)(int fd, void *data,struct epoll_event *ep);
typedef int (*event_loop_timer_func_t)(void *data);
typedef int (*event_loop_signal_func_t)(int signal_number, void *data);
typedef void (*event_loop_idle_func_t)(void *data);

typedef struct event_loop_st event_loop_t;
typedef struct event_source_st event_source_t;

event_loop_t *event_loop_create(void);
void event_loop_destroy(event_loop_t *loop);
event_source_t *event_loop_add_fd(event_loop_t *loop, int fd,event_loop_fd_func_t func, int kind,void *data);//kind 0 lt,1 et
//event_source_t *event_loop_add_fd_details(event_loop_t *loop, int fd,event_loop_fd_func_t_details func, void *data);
int event_source_fd_update(event_source_t *source,int kind);
event_source_t *event_loop_add_timer(event_loop_t *loop,
                                                event_loop_timer_func_t func,int kind,
                                                void *data);
int event_source_timer_update(event_source_t *source, int ms_delay);

event_source_t *event_loop_add_signal(event_loop_t *loop,
                        int signal_number,
                        event_loop_signal_func_t func,int kind,
                        void *data);

int event_source_remove(event_source_t *source);
void event_source_check(event_source_t *source);

int event_loop_dispatch(event_loop_t *loop, int timeout);
event_source_t *event_loop_add_idle(event_loop_t *loop,
                                               event_loop_idle_func_t func,
                                               void *data);
int event_loop_get_fd(event_loop_t *loop);

#ifdef  __cplusplus
}
#endif

#endif
