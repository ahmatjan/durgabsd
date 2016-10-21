AC_PREREQ(2.50)
AC_INIT([ftpmap], [0.15], [hypsurus@mail.ru])
AC_CONFIG_SRCDIR(src/ftpmap.c)
AM_INIT_AUTOMAKE

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_AIX
AC_ISC_POSIX
AC_PROG_CC_STDC
CFLAGS="$CFLAGS -D_GNU_SOURCE=1 "

AC_CANONICAL_HOST

dnl Checks for header files

AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/param.h ioctl.h sys/ioctl.h)
AC_CHECK_HEADERS(getopt.h stddef.h)
AC_CHECK_HEADERS(netinet/in_systm.h)
AC_CHECK_HEADERS(fcntl.h sys/fcntl.h)

dnl Checks for types

AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_TYPE_OFF_T

AC_CHECK_TYPE(socklen_t, , [AC_DEFINE(socklen_t, int)],
[
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
])

AC_CHECK_TYPE(sig_atomic_t, , [AC_DEFINE(sig_atomic_t, signed char)],
[
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#include <signal.h>
])

dnl Compiler characteristics

AC_PROG_GCC_TRADITIONAL
AC_C_CONST
AC_C_INLINE

dnl Options

AM_WITH_DMALLOC

dnl Checks

AC_MSG_CHECKING([whether __ss_family is defined])
AC_TRY_COMPILE([
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
],
[
do {
 struct sockaddr_storage a;
 (void) a.__ss_family;
} while(0)
],
[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE___SS_FAMILY)
],
[
  AC_MSG_RESULT(no)
])  


AC_MSG_CHECKING([whether ss_len is defined])
AC_TRY_COMPILE([
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
],
[
do {
 struct sockaddr_storage a;
 (void) a.ss_len;
} while(0)
],
[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_SS_LEN)
],
[
  AC_MSG_RESULT(no)
])  



AC_MSG_CHECKING([whether __ss_len is defined])
AC_TRY_COMPILE([
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
],
[
do {
 struct sockaddr_storage a;
 (void) a.__ss_len;
} while(0)
],
[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE___SS_LEN)
],
[
  AC_MSG_RESULT(no)
])  

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_VPRINTF

AC_CHECK_FUNCS(getopt_long)

dnl Socket things

AC_CHECK_FUNC(connect, , [AC_CHECK_LIB(socket, connect)])

AC_CHECK_FUNC(gethostbyname, , [AC_CHECK_LIB(resolv, gethostbyname)])

AC_CHECK_FUNC(gethostbyname, , [AC_CHECK_LIB(nsl, gethostbyname)])

if test "$ac_cv_lib_nsl_gethostbyname" != "yes" -a "$ac_cv_func_gethostbyname" != "yes"; then
  AC_CHECK_FUNC(gethostbyname, , [AC_CHECK_LIB(socket, gethostbyname)])
fi

if test "$ac_cv_lib_nsl_gethostbyname" = "$ac_cv_func_gethostbyname"; then
  AC_MSG_CHECKING([if we can include libnsl + libsocket])
  LIBS="-lnsl -lsocket $LIBS"
  AC_TRY_LINK( , [(void) gethostbyname], my_ac_link_result=yes,
             my_ac_link_result=no )
  if test "$my_ac_link_result" = "no"; then
    AC_MSG_RESULT([failure])
    AC_MSG_ERROR([unable to use gethostbyname()])
  else
    AC_MSG_RESULT([success])
  fi
fi


AC_CONFIG_FILES(Makefile src/Makefile)

AC_OUTPUT

