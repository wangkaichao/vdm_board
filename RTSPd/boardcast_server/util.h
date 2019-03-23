/****************************************************************************
**
** Copyright (C) 2011 好视网络科技有限公司.
** All rights reserved.
**
** author:  wangyuman
** mail:    wangym@gvtv.com.cn
** date:    2011-09-30
** des:     链表，数组处理相关函数
**
****************************************************************************/
#ifndef UTIL_H
#define UTIL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

/*linked list*/
typedef struct list_st
{
    struct list_st *prev;
    struct list_st *next;
}list_t;

void list_init(list_t *list);
void list_insert(list_t *list, list_t *elm);
void list_remove(list_t *elm);
int list_length(list_t *list);
int list_empty(list_t *list);

#ifndef offsetof
#define offsetof(type, member) ((size_t) &((type *)0)->member)
#endif
#define list_entry(ptr,type,member) ((type *)((char *)(ptr) - offsetof(type, member)))

#define __container_of(ptr, sample, member)				\
        (void *)((char *)(ptr)	-					\
                 ((char *)&(sample)->member - (char *)(sample)))

#define list_for_each(pos, head, member)				\
        for (pos = 0, pos = __container_of((head)->next, pos, member);	\
             &pos->member != (head);					\
             pos = __container_of(pos->member.next, pos, member))

#define list_for_each_safe(pos, tmp, head, member)			\
        for (pos = 0, tmp = 0, 						\
             pos = __container_of((head)->next, pos, member),		\
             tmp = __container_of((pos)->member.next, tmp, member);	\
             &pos->member != (head);					\
             pos = tmp,							\
             tmp = __container_of(pos->member.next, tmp, member))

#define list_for_each_reverse(pos, head, member)			\
        for (pos = 0, pos = __container_of((head)->prev, pos, member);	\
             &pos->member != (head);					\
             pos = __container_of(pos->member.prev, pos, member))


/*array*/
typedef struct array_st
{
    uint32_t size;
    uint32_t alloc;
    void *data;
}array_t;

void array_init(array_t *array);
void array_release(array_t *array);
void *array_add(array_t *array, int size);
void array_copy(array_t *array, array_t *source);

#ifdef  __cplusplus
}
#endif

#endif
