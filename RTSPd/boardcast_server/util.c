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
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include "util.h"

void list_init(list_t *list)
{
    list->prev = list;
    list->next = list;
}

void list_insert(list_t *list, list_t *elm)
{
    elm->prev = list;
    elm->next = list->next;
    list->next = elm;
    elm->next->prev = elm;
}

void list_remove(list_t *elm)
{
    elm->prev->next = elm->next;
    elm->next->prev = elm->prev;
}

int list_length(list_t *list)
{
    list_t *e;
    int count;

    count = 0;
    e = list->next;
    while (e != list)
    {
            e = e->next;
            count++;
    }

    return count;
}

int list_empty(list_t *list)
{
    return list->next == list;
}

void array_init(array_t *array)
{
    memset(array, 0, sizeof *array);
}

void array_release(array_t *array)
{
    free(array->data);
}

void *array_add(array_t *array, int size)
{
    int alloc;
    void *data, *p;

    if (array->alloc > 0)
            alloc = array->alloc;
    else
            alloc = 16;

    while (alloc < array->size + size)
            alloc *= 2;

    if (array->alloc < alloc)
    {
        if (array->alloc > 0)
                data = realloc(array->data, alloc);
        else
                data = malloc(alloc);

        if (data == NULL)
                return 0;
        array->data = data;
        array->alloc = alloc;
    }

    p = array->data + array->size;
    array->size += size;

    return p;
}

void array_copy(array_t *array, array_t *source)
{
    array->size = 0;
    array_add(array, source->size);
    memcpy(array->data, source->data, source->size);
}

