
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LIST_H_INCLUDED_
#define _NGX_LIST_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_list_part_s  ngx_list_part_t;
/**
 * 链表节点  每个节点大小 = size * nelts
 * 节点元素用完后，每次就会分配一个新的节点
 */
struct ngx_list_part_s {
    void             *elts;     /* 节点的内存起始位置 */
    ngx_uint_t        nelts;    /* 已经使用的元素*/
    ngx_list_part_t  *next;     /* 指向下一个链表节点*/
};

/**
 * 链表结构
 */
typedef struct {
    ngx_list_part_t  *last;     /* 指向最新的链表节点*/
    ngx_list_part_t   part;     /* 第一个链表节点*/
    size_t            size;     /* 这个链表默认的每个元素大小 */
    ngx_uint_t        nalloc;   /* 每个节点part 可以支持多少个元素*/
    ngx_pool_t       *pool;     /* 内存池 */
} ngx_list_t;


ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);

static ngx_inline ngx_int_t
ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    /* 分配一个链表节点的内存块 大小  n * size*/
    list->part.elts = ngx_palloc(pool, n * size);
    if (list->part.elts == NULL) {
        return NGX_ERROR;
    }

    list->part.nelts = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return NGX_OK;
}


/*
 *
 *  the iteration through the list:
 *
 *  part = &list.part;
 *  data = part->elts;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->nelts) {
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          part = part->next;
 *          data = part->elts;
 *          i = 0;
 *      }
 *
 *      ...  data[i] ...
 *
 *  }
 */


void *ngx_list_push(ngx_list_t *list);


#endif /* _NGX_LIST_H_INCLUDED_ */
