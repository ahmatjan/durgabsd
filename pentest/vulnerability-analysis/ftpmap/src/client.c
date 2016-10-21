/* client.c - FTP client stuff 
 *
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

#include "client.h"
#include "logger.h"
#include "misc.h"
#include "tcp.h"

void ftpmap_getlist(ftpmap_t *ftpmap) {
    FILE *fid;
    char buffer[MAX_STR];
    char *answer = NULL;
    
    printf(":: Getting %sLIST%s..\n\n", GREEN,END);
    fid = ftpmap_data_tunnel(ftpmap, "r");
    fprintf(gfid, "LIST %s\r\n", ftpmap->path);
    answer = ftpmap_getanswer(ftpmap);

    signal(SIGALRM, sigalrm);
    alarm(5);

    while ( (fgets(buffer, sizeof(buffer), fid)) != NULL ) {
        printf("%s", buffer);
    }
    printf("\n:: End of output\n");
}

long int ftpmap_fsize(ftpmap_t *ftpmap) {
    char *answer = NULL;
    long int size = 0;
    char code[MAX_STR];

    fprintf(gfid, "SIZE %s\r\n", ftpmap->path); 
    sscanf( ftpmap_getanswer(ftpmap), "%s %d", code, &size);

    return size;
}

void ftpmap_cat(ftpmap_t *ftpmap) {
    int fsize = ftpmap_fsize(ftpmap);
    int rsize = 0;
    FILE *fd;
    char *answer = NULL;
    char buffer[MAX_STR];

    fd = ftpmap_data_tunnel(ftpmap, "rt");

    fprintf(gfid, "TYPE A\r\n");
    answer = ftpmap_getanswer(ftpmap);
    if ( *answer == 0 )
        return;

    fprintf(gfid, "RETR %s\r\n", ftpmap->path);
    answer = ftpmap_getanswer(ftpmap);

    if ( *answer == 0 )
        return;
    printf(":-: %s", answer);
    while (( rsize = fread(buffer, 1, sizeof(buffer), fd)) > 0 ) {
        printf("%s", buffer);
    }
    putchar('\n');
}


void ftpmap_download(ftpmap_t *ftpmap) {
    int fsize = ftpmap_fsize(ftpmap);
    int dsize = 0, rsize = 0;
    FILE *fd, *file;
    char *filename = NULL;
    char *answer = NULL;
    char buffer[MAX_STR];

    filename =  (strrchr(ftpmap->path, '/'))+1;

    if (( file = fopen(filename, "w")) == NULL )
        die(1, "Failed to write %s.\n", ftpmap->path);

    fd = ftpmap_data_tunnel(ftpmap, "r");

    fprintf(gfid, "TYPE I\r\n");
    answer = ftpmap_getanswer(ftpmap);
    if ( *answer == 0 )
        return;

    fprintf(gfid, "RETR %s\r\n", ftpmap->path);
    answer = ftpmap_getanswer(ftpmap);

    if ( *answer == 0 )
        return;
    logger_write(1,ftpmap, ":-: %s", answer);
    while (( rsize = fread(buffer, 1, sizeof(buffer), fd)) > 0 ) {
         if ( buffer[rsize +1] == '\r' )
            buffer[rsize +1] = '\0';
        dsize += fwrite(buffer, 1, rsize, file);
        printf(":-: Downloading %s%s%s %s%s%s/%s%s%s ...\r",BLUE,ftpmap->path, END,YELLOW,dsize ? calc_bytes_size(dsize) : "unknown",END,
						RED,fsize ? calc_bytes_size(fsize) : "unknown",END);
        fflush(stdout);
   }
    printf("\n:-: File saved: %s\n", filename);
    fclose(file);
}

void ftpmap_upload(ftpmap_t *ftpmap) {
    FILE *lfp, *fd;
    int fsize = 0;
    int rsize = 0, dsize = 0;
    int buffer[MAX_STR];
    char *filename = "d";
    char *answer = NULL;
   
    if ( strrchr(ftpmap->path, '/'))
        filename = (strrchr(ftpmap->path, '/'))+1;
    else
        filename = ftpmap->path;
    if (( lfp = fopen(ftpmap->path, "rb")) == NULL )
        die(1, "Failed to read \'%s\' ...\n", ftpmap->path);

    fseek(lfp, 0L, SEEK_END);
    fsize += (int)ftell(lfp);
    fseek(lfp, 0L, SEEK_SET);

    fd = ftpmap_data_tunnel(ftpmap, "w");
    
    fprintf(gfid, "TYPE I\r\n");
    answer = ftpmap_getanswer(ftpmap);
    if ( *answer == 0 )
        return;

    fprintf(gfid, "STOR %s\r\n", filename);
    answer = ftpmap_getanswer(ftpmap);
    if ( *answer == 0 )
        return;

    logger_write(1,ftpmap, ":-: %s", answer);
    while (( rsize = fread(buffer, 1, sizeof(buffer), lfp)) > 0 ) {
        if ( buffer[rsize +1 ] == '\r' ) 
            buffer[rsize + 1] = '\0';
        dsize += fwrite(buffer, 1, rsize, fd);
        printf(":-: Uploading %s%s%s %s%s%s/%s%s%s bytes...\r", BLUE,filename,END, YELLOW,dsize ? calc_bytes_size(dsize) : "unknown",END, 
                RED,fsize ? calc_bytes_size(fsize) : "unknown",END);
        fflush(stdout); 
    }
    printf("\n:-: File \'%s\' Uploaded ...\n", filename);
    fclose(lfp);
}


void ftpmap_delete(ftpmap_t *ftpmap) {
    char *answer = NULL;

    fprintf(gfid, "DELE %s\r\n", ftpmap->path);
    answer = ftpmap_getanswer(ftpmap);
    if ( *answer == 0 )
        return;
    printf(":: %s", answer);
}

void ftpmap_mdtm(ftpmap_t *ftpmap) {
    char *answer = NULL;

    fprintf(gfid, "MDTM %s\r\n", ftpmap->path);
    answer = ftpmap_getanswer(ftpmap);
    if ( *answer == 0 )
        return;

    printf(":: %s",answer);
}


