// lwipopts.h - minimal example for Pico W HTTP server

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#define NO_SYS                          1
#define LWIP_SOCKET                     0
#define LWIP_NETCONN                    0
#define LWIP_TCP                        1
#define LWIP_IPV4                       1
#define MEM_LIBC_MALLOC                 1
#define MEMP_MEM_MALLOC                 1
#define PBUF_POOL_BUFSIZE               512

#endif /* __LWIPOPTS_H__ */