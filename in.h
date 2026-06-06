#ifndef _ARPA_INET_H_
#define _ARPA_INET_H_

#include <features.h>
#include "types.h"
/* #include __SYSINC__(in.h)
#include __SYSINC__(un.h) */

typedef __u32 ipaddr_t;		/* ip address in network byte order*/

#ifdef __GNUC__
#define ntohs(x)	__builtin_bswap16(x)
#define ntohl(x)	__builtin_bswap32(x)

#else

#define ntohs(x)	( (((x) >> 8) & 0xff) | ((unsigned char)(x) << 8) )
#define ntohl(x)	( ((((unsigned long)x) >> 24) & 0xff)	| \
			  ((((unsigned long)x) >> 8 ) & 0xff00)		| \
			  ((((unsigned long)x) & 0xff00) << 8)		| \
			  ((((unsigned long)x) & 0xff) << 24) )
#endif

#define htons(x)	ntohs(x)
#define	htonl(x)	ntohl(x)

#endif /* _ARPA_INET_H_ */

#ifndef __LINUXMT_IN_H
#define __LINUXMT_IN_H

struct in_addr {
    unsigned long s_addr;
};

struct sockaddr_in {
    unsigned short sin_family;	/* AF_INET */
    unsigned short sin_port;	/* port in network order */
    struct in_addr sin_addr;
};

#define INADDR_ANY      ((unsigned long) 0x00000000)
#define PORT_ANY        ((unsigned short) 0x0000)
#define INADDR_NONE     ((unsigned long) 0xffffffff)
#define INADDR_LOOPBACK ((unsigned long) 0x7f000001)

#endif
