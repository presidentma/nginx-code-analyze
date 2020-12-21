
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONNECTION_H_INCLUDED_
#define _NGX_CONNECTION_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_listening_s  ngx_listening_t;
/* socket侦听结构 */
struct ngx_listening_s {
    ngx_socket_t        fd; /* socket文件描述符 */

    struct sockaddr    *sockaddr;   /* socket地址 */
    socklen_t           socklen;    /* size of sockaddr */
    size_t              addr_text_max_len;
    ngx_str_t           addr_text;

    int                 type;

    int                 backlog;    /* 日志 */
    int                 rcvbuf;     /* 数据接收buffer */
    int                 sndbuf;     /* 数据发送的buffer */
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                 keepidle;
    int                 keepintvl;
    int                 keepcnt;
#endif

    /* handler of accepted connection */
    /* 接收连接后的回调函数，回调方法：ngx_http_init_connection */
    ngx_connection_handler_pt   handler;

    void               *servers;  /* array of ngx_http_in_addr_t, for example */

    ngx_log_t           log;
    ngx_log_t          *logp;

    size_t              pool_size;
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    /* should be here because of the deferred accept */
    ngx_msec_t          post_accept_timeout;

    ngx_listening_t    *previous;    /* 前一个ngx_listening_t */
    ngx_connection_t   *connection;  /* 连接对象 */

    ngx_rbtree_t        rbtree;
    ngx_rbtree_node_t   sentinel;

    ngx_uint_t          worker;

    unsigned            open:1;     /* 为1表示监听句柄有效，为0表示正常关闭 */
    unsigned            remain:1;   /* 为1表示不关闭原先打开的监听端口，为0表示关闭曾经打开的监听端口 */
    unsigned            ignore:1;   /* 为1表示跳过设置当前ngx_listening_t结构体中的套接字，为0时正常初始化套接字 */

    unsigned            bound:1;       /* already bound */
    unsigned            inherited:1;   /* inherited from previous process */
    unsigned            nonblocking_accept:1;
    unsigned            listen:1;   /* 为1表示当前结构体对应的套接字已经监听 */
    unsigned            nonblocking:1;
    unsigned            shared:1;    /* shared between threads or processes */
    unsigned            addr_ntop:1;
    unsigned            wildcard:1;

#if (NGX_HAVE_INET6)
    unsigned            ipv6only:1;
#endif
    unsigned            reuseport:1;
    unsigned            add_reuseport:1;
    unsigned            keepalive:2;

    unsigned            deferred_accept:1;
    unsigned            delete_deferred:1;
    unsigned            add_deferred:1;
#if (NGX_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)
    char               *accept_filter;
#endif
#if (NGX_HAVE_SETFIB)
    int                 setfib;
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
    int                 fastopen;
#endif

};


typedef enum {
    NGX_ERROR_ALERT = 0,
    NGX_ERROR_ERR,
    NGX_ERROR_INFO,
    NGX_ERROR_IGNORE_ECONNRESET,
    NGX_ERROR_IGNORE_EINVAL
} ngx_connection_log_error_e;


typedef enum {
    NGX_TCP_NODELAY_UNSET = 0,
    NGX_TCP_NODELAY_SET,
    NGX_TCP_NODELAY_DISABLED
} ngx_connection_tcp_nodelay_e;


typedef enum {
    NGX_TCP_NOPUSH_UNSET = 0,
    NGX_TCP_NOPUSH_SET,
    NGX_TCP_NOPUSH_DISABLED
} ngx_connection_tcp_nopush_e;


#define NGX_LOWLEVEL_BUFFERED  0x0f
#define NGX_SSL_BUFFERED       0x01
#define NGX_HTTP_V2_BUFFERED   0x02

/* 存储连接有关的信息和读写事件 */
struct ngx_connection_s {
    void               *data;   /* 关联其它的 ngx_connection_s */
    ngx_event_t        *read;   /* 读取数据事件 */
    ngx_event_t        *write;  /* 写入事件*/

    ngx_socket_t        fd;     /* socket句柄 */

    ngx_recv_pt         recv;   /* 接收数据的函数指针 */
    ngx_send_pt         send;   /* 发送数据的函数指针 */
    ngx_recv_chain_pt   recv_chain; /* 批量接收数据的函数指针 */
    ngx_send_chain_pt   send_chain; /* 批量发送数据的函数指针 */

    ngx_listening_t    *listening;  /* 该连接的网络监听数据结构 */

    off_t               sent;   /* 已sent数据的偏移量 */

    ngx_log_t          *log;    /* 日志 */

    ngx_pool_t         *pool;   /* 内存池 */

    int                 type;
    /* socket的地址结构 */
    struct sockaddr    *sockaddr;
    socklen_t           socklen;
    ngx_str_t           addr_text;

    ngx_proxy_protocol_t  *proxy_protocol;

#if (NGX_SSL || NGX_COMPAT)
    ngx_ssl_connection_t  *ssl;
#endif

    ngx_udp_connection_t  *udp;
    /* 本地监听socket的地址结构 */
    struct sockaddr    *local_sockaddr;
    socklen_t           local_socklen;
    /* 用于接收和缓存客户端发来的字符流 */
    ngx_buf_t          *buffer;
    /* 该字段表示将该连接以双向链表形式添加到cycle结构体中的   cycle->free_connections*/
    ngx_queue_t         queue;
    /* 建立一条与后端服务器的连接,number+1   */
    ngx_atomic_uint_t   number;
    /* 处理请求的次数  */
    ngx_uint_t          requests;

    unsigned            buffered:8;
    /* 日志级别   */
    unsigned            log_error:3;     /* ngx_connection_log_error_e */
    /* 不期待字符流结束  */
    unsigned            timedout:1;
    /* 连接处理过程中出现错误 */
    unsigned            error:1;
    /* 标识此链接已经销毁,内存池,套接字等都不可用  */
    unsigned            destroyed:1;
    /* 连接处于空闲状态  */
    unsigned            idle:1;
    /* 连接可以重用  */
    unsigned            reusable:1;
    /* 连接关闭 */
    unsigned            close:1;
    unsigned            shared:1;
    /* 正在将文件中的数据发往另一端 */
    unsigned            sendfile:1;
    unsigned            sndlowat:1;
    /* 使用tcp的nodely特性  */
    unsigned            tcp_nodelay:2;   /* ngx_connection_tcp_nodelay_e */
    /* 使用tcp的nopush特性   */
    unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */

    unsigned            need_last_buf:1;

#if (NGX_HAVE_AIO_SENDFILE || NGX_COMPAT)
    unsigned            busy_count:2;
#endif

#if (NGX_THREADS || NGX_COMPAT)
    ngx_thread_task_t  *sendfile_task;
#endif
};


#define ngx_set_connection_log(c, l)                                         \
                                                                             \
    c->log->file = l->file;                                                  \
    c->log->next = l->next;                                                  \
    c->log->writer = l->writer;                                              \
    c->log->wdata = l->wdata;                                                \
    if (!(c->log->log_level & NGX_LOG_DEBUG_CONNECTION)) {                   \
        c->log->log_level = l->log_level;                                    \
    }


ngx_listening_t *ngx_create_listening(ngx_conf_t *cf, struct sockaddr *sockaddr,
    socklen_t socklen);
ngx_int_t ngx_clone_listening(ngx_cycle_t *cycle, ngx_listening_t *ls);
ngx_int_t ngx_set_inherited_sockets(ngx_cycle_t *cycle);
ngx_int_t ngx_open_listening_sockets(ngx_cycle_t *cycle);
void ngx_configure_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_connection(ngx_connection_t *c);
void ngx_close_idle_connections(ngx_cycle_t *cycle);
ngx_int_t ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port);
ngx_int_t ngx_tcp_nodelay(ngx_connection_t *c);
ngx_int_t ngx_connection_error(ngx_connection_t *c, ngx_err_t err, char *text);

ngx_connection_t *ngx_get_connection(ngx_socket_t s, ngx_log_t *log);
void ngx_free_connection(ngx_connection_t *c);

void ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable);

#endif /* _NGX_CONNECTION_H_INCLUDED_ */
