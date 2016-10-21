/* misc.h - Header file */
#ifndef MISC_H
#define MISC_H

#include "ftpmap.h"

#define KB_PREFIX   "KB"
#define MB_PREFIX   "MB"
#define GB_PREFIX   "GB"
#define TB_PREFIX   "TB"


/* prototypes */
void die(int,char*, ...);
void ftpmap_draw(int,int);
void ftpmap_draw_extable(ftpmap_t *,int,char*);
void ftpmap_genchars(int,char*,int);
void sigalrm(int);
void sigalint(int);
void misc_check(char *);
char * calc_bytes_size(int); 
char * fret(char *, ...);

#endif /*MISC_H*/
