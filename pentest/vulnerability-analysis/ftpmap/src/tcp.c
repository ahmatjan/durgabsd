/* tcp.c - Handle FTP-Map tcp connections 

  Copyright (c) Hypsurus <hypsurus@mail.ru>

  FTP-Map is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  FTP-Map is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "tcp.h"

FILE * ftpmap_reconnect(ftpmap_t *ftpmap, int ex) {
    struct addrinfo ai, *srv = NULL, *p = NULL;
    char hbuf[MAX_STR];

    memset(&ai, 0, sizeof(ai));

    /* ai.ai_family = AF_UNSPEC */
    ai.ai_family = AF_INET;
    ai.ai_protocol = IPPROTO_TCP;
    ai.ai_socktype = SOCK_STREAM;

    if (( getaddrinfo(ftpmap->server, ftpmap->port, &ai, &srv)) != 0 ) {
        if ( ex == 1 )
            die(1, "Connection failed.\n");
    }

    p = srv;

    getnameinfo((struct sockaddr *) p->ai_addr, p->ai_addrlen,
                        hbuf, sizeof hbuf, NULL, (size_t) 0U, NI_NUMERICHOST);

    sprintf(ftpmap->ip_addr, "%s", hbuf);

    if (( fd = socket(ai.ai_family, ai.ai_socktype, 
                    ai.ai_protocol)) < 0 ) {
        if ( ex == 1 ) 
            die(1, "Failed to create a new socket.\n");
    }

    if ( connect(fd, p->ai_addr, p->ai_addrlen) < 0 ) {
        if ( ex == 1)
            die(1, "Failed to connect\n");
		}
    freeaddrinfo(srv);
    
    return (fdopen(fd,"r+"));
}

FILE * ftpmap_data_tunnel(ftpmap_t *ftpmap, char *mode) {
    FILE *fid;
    struct sockaddr_in s;

    if (( dfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        die(1, "data_tunnel: Failed to create socket.\n");

    ftpmap_calc_data_port(ftpmap); 
    /* s.sin_family = AF_UNSPEC */
    s.sin_family = AF_INET;
    s.sin_port = htons(ftpmap->dataport);
    s.sin_addr.s_addr = inet_addr(ftpmap->ip_addr);

    if (( connect(dfd, (struct sockaddr *)&s, sizeof s)) < 0 )
        die(1, "data_tunnel: Failed to connect.\n");

    return (fdopen(dfd, mode));
}
