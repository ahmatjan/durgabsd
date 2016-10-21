/* misc.c  - FTP-Map misc
 
  Copyright 2015 (c) by Hypsurus <hypsurus@mail.ru>

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


#include "misc.h"
#include "logger.h"

char * calc_bytes_size(int size) {
    float KB = 1024;
    float MB = 1024 * 1024;
    float GB = 1024 * 1024 * 1024;
    char *ret = NULL;
    /*int TB = 1024 * 1024 * 1024 * 1024; */
    
    if ( size >= KB ) {
        ret = fret("%.2f%s", size / KB, KB_PREFIX);
    }
    if ( size >= MB ) {
        ret = fret("%.2f%s", size / MB, MB_PREFIX);
    }
    if ( size >= GB ) {
        ret = fret("%.2f%s", size / GB, GB_PREFIX);
   }

    return ret;
}

char * fret(char *format, ...) {
    static char ret[MAX_STR];
    va_list li;

    va_start(li, format);
    vsprintf(ret, format, li);
    va_end(li);

    return ret;
}

void misc_check(char *buffer) {
    int i = 0;
    
    if ( buffer ) {
        for ( i = 0; buffer[i] != 0;i++ ) {
            if ( i >= MAX_STR-6 )
                die(1, "You cannont pass this string.\n");
        }
    }
}

void die(int stat, char *format, ...) {
        va_list li;
        char m[MAX_STR];

        va_start(li, format);
        vsnprintf(m, sizeof(m), format, li);
        va_end(li);

        if ( stat == 1 ) {
            fprintf(stderr, "Error: %s", m);
            exit(EXIT_FAILURE);
        }
}

void ftpmap_draw(int ch, int times) {
    int i = 0;

    printf("+");
    for ( i = 0; i <= times; i++ ) {
        putchar(ch);
    }
    printf("+\n");
}

void ftpmap_draw_extable(ftpmap_t *ftpmap, int id, char *exploit) {
        ftpmap_draw(0x2d, strlen(exploit));
        logger_write(1, ftpmap,"|%8s|\n", exploit);
        ftpmap_draw(0x2d, strlen(exploit));
        logger_write(1,ftpmap,"|http://exploit-db.com/download/%d|\n", id); 
        ftpmap_draw(0x2d, 35);
        putchar(0x0a);
}

void sigalrm(int dummy) {
    (void) dummy;
    close(fd);
    fd = -1;
}

void sigalint(int dummy) {
	(void) dummy;
	printf("\n:-: %s Interrupt ): ..%s\n", RED,END);
	exit(1);
}

