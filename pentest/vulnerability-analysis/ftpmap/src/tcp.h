/* tcp.h - tcp.c header file*/
#ifndef TCP_H
#define TCP_H

#include "ftpmap.h"
#include "misc.h"

FILE * ftpmap_reconnect(ftpmap_t*,int);
FILE * ftpmap_data_tunnel(ftpmap_t*, char *mode);

#endif /* TCP_H*/
