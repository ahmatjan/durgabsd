/* client.h - client.c Headers */
#ifndef CLIENT_H
#define CLIENT_H

#include "ftpmap.h"

void ftpmap_getlist(ftpmap_t *ftpmap);
void ftpmap_delete(ftpmap_t *ftpmap);
void ftpmap_mdtm(ftpmap_t *ftpmap);
void ftpmap_download(ftpmap_t *ftpmap);
void ftpmap_upload(ftpmap_t *ftpmap);
void ftpmap_cat(ftpmap_t *ftpmap);
long int ftpmap_fsize(ftpmap_t *ftpmap);

#endif /*CLIENT_H*/
