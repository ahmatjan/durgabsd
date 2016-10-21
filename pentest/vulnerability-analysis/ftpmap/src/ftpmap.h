/* ftpmap.h - the FTP-Map project header */

#ifndef FTPMAP_H
#define FTPMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>

#define MAX_STR 256
#define MAX_ANSWER  1024
#define FTP_DEFAULT_SERVER  "localhost"
#define FTP_DEFAULT_PORT    "21"
#define FTP_DEFAULT_USER    "anonymous"
#define FTP_DEFAULT_PASSWORD    "anonymous@localhost"

/* Databases */
#define DB_EXPLOITDB    "../db/ftp-exploit-db"
#define DB_VERSIONS    "../db/ftp-versions-db"

int fd;
int dfd;
FILE *gfid;

/* We have colors!! */
#define RED "\e[31m"
#define YELLOW "\e[33m"
#define BLUE "\e[1;34m"
#define GREEN "\e[32m"
#define END "\e[0m"

typedef struct {
    FILE *loggerfp;
    char ip_addr[MAX_STR];
    char unsoftware[MAX_STR];
    char *server;
    char *port;
    char *user;
    char *password;
    char *cmd;
    char *path;
    int dataport;
    /* Flags */
    int versiondetected;
    int fingerprinthasmatch;
    int skipfingerprint;
    int forcefingerprint;
    int nolog;
    int scan_mode;
		int islogged;
    char fversion[MAX_STR];
    char fsoftware[MAX_STR];
    char software[MAX_STR];
    char version[MAX_STR];
    char fisoftware[MAX_STR];
    char exploit[MAX_STR];
    int id;
} ftpmap_t;

void ftpmap_init(ftpmap_t *ftpmap);
void print_version(int c);
void print_usage(int ex);
void print_startup(ftpmap_t *ftpmap);
void ftpmap_detect_version_by_banner(ftpmap_t *ftpmap);
void ftpmap_findexploit(ftpmap_t *ftpmap);
int ftpmap_compar(const void *a_, const void *b_);
void ftpmap_sendcmd(ftpmap_t *ftpmap);
void ftpmap_calc_data_port(ftpmap_t *ftpmap);
char * ftpmap_getanswer(ftpmap_t*);
char * ftpmap_getanswer_long(FILE *, ftpmap_t *);
void ftpmap_scan(ftpmap_t *ftpmap,int override);
void ftpmap_brute(ftpmap_t *);

#endif /*FTPMAP_H*/
