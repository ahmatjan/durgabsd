# OpenVAS
# $Id$
# Description: CMakefile for the OpenVAS CLI program omp
#
# Authors:
# Matthew Mundell <matthew.mundell@intevation.de>
#
# Copyright:
# Copyright (C) 2009, 2010 Greenbone Networks GmbH
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.

## Config

include_directories (${GLIB_INCLUDE_DIRS} ${GNUTLS_INCLUDE_DIRS}
                     ${LIBOPENVAS_OMP_INCLUDE_DIRS}
                     ${LIBOPENVAS_BASE_INCLUDE_DIRS}
                     ${LIBOPENVAS_MISC_INCLUDE_DIRS})

## Program

# FIXME: Find out how to install to nagios plugins directory.
add_executable (check_omp check_omp.c)

set_target_properties (check_omp PROPERTIES LINKER_LANGUAGE C)

if (OPENVASCLI_VERSION)
  add_definitions (-DOPENVASCLI_VERSION=\\\"${OPENVASCLI_VERSION}\\\")
endif (OPENVASCLI_VERSION)

add_definitions (-DOPENVAS_OS_NAME=\\\"${CMAKE_SYSTEM}\\\")
add_definitions (-DPREFIX=\\\"${CMAKE_INSTALL_PREFIX}\\\")

if (OPENVAS_LOG_DIR)
  add_definitions (-DOPENVAS_LOG_DIR=\\\"${OPENVAS_LOG_DIR}\\\")
endif (OPENVAS_LOG_DIR)

if (MINGW)
  set (GLIB_LDFLAGS "-L${CROSS_ENV}/lib")
  set (OPENVAS_LDFLAGS "-L${CMAKE_INSTALL_PREFIX}/lib")

  set_target_properties (check_omp PROPERTIES LINK_FLAGS
                         "${GLIB_LDFLAGS} ${OPENVAS_LDFLAGS}")

  set (GLIB_CFLAGS "-I${CROSS_ENV}include")
  set (GLIB_CFLAGS "${GLIB_CFLAGS} -I${CROSS_ENV}include/glib-2.0")
  set (GLIB_CFLAGS "${GLIB_CFLAGS} -I${CROSS_ENV}lib/glib-2.0/include")

  set_target_properties (check_omp PROPERTIES COMPILE_FLAGS
                         "-I${CMAKE_INSTALL_PREFIX}/include ${GLIB_CFLAGS}")

  target_link_libraries (check_omp "${GLIB_LDFLAGS} ${OPENVAS_LDFLAGS} -lglib-2.0 -lgnutls -lws2_32 -lgcrypt -lintl -lgpg-error -liconv -lz -lole32 -lopenvas_omp -lopenvas_misc")
else (MINGW)
  if (BUILD_STATIC_NAGIOS)
    set(CMAKE_EXE_LINKER_FLAGS -static)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
    set(CMAKE_SHARED_LIBRARY_C_FLAGS)
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
    set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

    target_link_libraries (check_omp
                           ${GNUTLS_STATIC_LDFLAGS}
                           ${GLIB_STATIC_LDFLAGS}
                           ${GCRYPT_LDFLAGS}
                           ${LIBOPENVAS_OMP_STATIC_LDFLAGS}
                           ${LIBOPENVAS_BASE_STATIC_LDFLAGS}
                           ${LIBOPENVAS_MISC_STATIC_LDFLAGS}
                           ${GPGME_LDFLAGS})
  else (BUILD_STATIC_NAGIOS)
    target_link_libraries (check_omp
                           ${GNUTLS_LDFLAGS}
                           ${GLIB_LDFLAGS}
                           ${LIBOPENVAS_OMP_LDFLAGS}
                           ${LIBOPENVAS_BASE_LDFLAGS}
                           ${LIBOPENVAS_MISC_LDFLAGS}
                           ${LINKER_HARDENING_FLAGS})
  endif (BUILD_STATIC_NAGIOS)
endif (MINGW)

## Install

install (TARGETS check_omp
         RUNTIME DESTINATION ${BINDIR})

# FIXME: etags/ctags/tags support

