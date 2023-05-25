#ifndef MIDDLEWARES_THIRD_PARTY_LWIP_PORT_ARCH_SYS_ARCH_H_
#define MIDDLEWARES_THIRD_PARTY_LWIP_PORT_ARCH_SYS_ARCH_H_

#define SYS_MBOX_NULL   NULL
#define SYS_SEM_NULL    NULL

typedef void * sys_prot_t;

typedef void * sys_sem_t;

typedef void * sys_mbox_t;

typedef void * sys_thread_t;

#else
#	warning sys_arch.h incluso
#endif
