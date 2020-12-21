
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2


typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);

struct ngx_shm_zone_s {
    void                     *data;
    ngx_shm_t                 shm;
    ngx_shm_zone_init_pt      init;
    void                     *tag;
    void                     *sync;
    ngx_uint_t                noreuse;  /* unsigned  noreuse:1; */
};

/* 全局变量cycle，此为全运行周期依赖结构 */
struct ngx_cycle_s {
    void                  ****conf_ctx;     /* 配置文件 上下文的数组，每个模块的配置信息*/
    ngx_pool_t               *pool;         /* 内存池地址 */

    ngx_log_t                *log;          /* 日志*/
    ngx_log_t                 new_log;

    ngx_uint_t                log_use_stderr;  /* unsigned  log_use_stderr:1; */

    ngx_connection_t        **files;           /* 连接文件句柄 */
    ngx_connection_t         *free_connections;/* 空闲连接 */
    ngx_uint_t                free_connection_n;/* 空闲连接个数 */

    ngx_module_t            **modules;          /* 模块数组 */
    ngx_uint_t                modules_n;        /* 模块个数 */
    ngx_uint_t                modules_used;    /*已使用模块 unsigned  modules_used:1; */

    ngx_queue_t               reusable_connections_queue;
    ngx_uint_t                reusable_connections_n;

    ngx_array_t               listening;        /* 监听数组（socket） */
    ngx_array_t               paths;            /* 路径数组 */

    ngx_array_t               config_dump;
    ngx_rbtree_t              config_dump_rbtree;   
    ngx_rbtree_node_t         config_dump_sentinel;

    ngx_list_t                open_files;       /* 打开的文件 */
    ngx_list_t                shared_memory;    /* 共享内存链表*/

    ngx_uint_t                connection_n;     /* 连接的个数*/
    ngx_uint_t                files_n;          /* 打开文件的个数 */

    ngx_connection_t         *connections;      /* 连接事件*/
    ngx_event_t              *read_events;      /* 读取事件*/
    ngx_event_t              *write_events;     /* 写入事件*/

    ngx_cycle_t              *old_cycle;

    ngx_str_t                 conf_file;        /* 配置文件（path） */
    ngx_str_t                 conf_param;       /* 配置参数 */
    ngx_str_t                 conf_prefix;      /* 配置文件前缀*/
    ngx_str_t                 prefix;           /* 前缀*/
    ngx_str_t                 lock_file;        /* 锁文件*/
    ngx_str_t                 hostname;         /* 主机名称*/
};

/* nginx.conf的核心配置文件 */
typedef struct {
    ngx_flag_t                daemon;           /* 守护进程flag */
    ngx_flag_t                master;           /* 主进程flag */

    ngx_msec_t                timer_resolution; /* 计时器精度 */
    ngx_msec_t                shutdown_timeout; /* 关闭超时时间 */

    ngx_int_t                 worker_processes; /* 工作进程个数 */
    ngx_int_t                 debug_points;     /* gebug点 */

    ngx_int_t                 rlimit_nofile;
    off_t                     rlimit_core;

    int                       priority;         /* 优先级 */

    ngx_uint_t                cpu_affinity_auto;
    ngx_uint_t                cpu_affinity_n;
    ngx_cpuset_t             *cpu_affinity;

    char                     *username;         /* 运行用户名 */
    ngx_uid_t                 user;             /* 运行用户uid */
    ngx_gid_t                 group;            /* 运行用户gid */

    ngx_str_t                 working_directory;/* 工作目录 */
    ngx_str_t                 lock_file;        /* 锁文件 */

    ngx_str_t                 pid;              /* 主进程运行pid */
    ngx_str_t                 oldpid;           /* 主进程运行old pid */

    ngx_array_t               env;              /* 环境变量数组 */
    char                    **environment;      /* 环境变量 */

    ngx_uint_t                transparent;  /* 透明代理unsigned  transparent:1; */
} ngx_core_conf_t;


#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)


ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);
ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log);
void ngx_delete_pidfile(ngx_cycle_t *cycle);
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig);
void ngx_reopen_files(ngx_cycle_t *cycle, ngx_uid_t user);
char **ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last);
ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char *const *argv);
ngx_cpuset_t *ngx_get_cpu_affinity(ngx_uint_t n);
ngx_shm_zone_t *ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name,
    size_t size, void *tag);
void ngx_set_shutdown_timer(ngx_cycle_t *cycle);


extern volatile ngx_cycle_t  *ngx_cycle;
extern ngx_array_t            ngx_old_cycles;
extern ngx_module_t           ngx_core_module;
extern ngx_uint_t             ngx_test_config;
extern ngx_uint_t             ngx_dump_config;
extern ngx_uint_t             ngx_quiet_mode;


#endif /* _NGX_CYCLE_H_INCLUDED_ */
