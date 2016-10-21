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

include_directories (${GLIB_INCLUDE_DIRS} ${GNUTLS_INCLUDE_DIRS}
                     ${LIBOPENVAS_OMP_INCLUDE_DIRS}
                     ${LIBOPENVAS_BASE_INCLUDE_DIRS}
                     ${LIBOPENVAS_MISC_INCLUDE_DIRS})

## Program

add_executable (omp omp.c)

set_target_properties (omp PROPERTIES LINKER_LANGUAGE C)

if (OPENVASCLI_VERSION)
  add_definitions (-DOPENVASCLI_VERSION=\\\"${OPENVASCLI_VERSION}\\\")
endif (OPENVASCLI_VERSION)

add_definitions (-DOPENVAS_OS_NAME=\\\"${CMAKE_SYSTEM}\\\")
add_definitions (-DPREFIX=\\\"${CMAKE_INSTALL_PREFIX}\\\")

if (OPENVAS_LOG_DIR)
  add_definitions (-DOPENVAS_LOG_DIR=\\\"${OPENVAS_LOG_DIR}\\\")
endif (OPENVAS_LOG_DIR)

set (OPENVAS_STATE_DIR    "${LOCALSTATEDIR}/lib/openvas")
set (OPENVAS_CLIENT_CERTIFICATE  "${OPENVAS_STATE_DIR}/CA/clientcert.pem")
set (OPENVAS_CLIENT_KEY          "${OPENVAS_STATE_DIR}/private/CA/clientkey.pem")
set (OPENVAS_CA_CERTIFICATE      "${OPENVAS_STATE_DIR}/CA/cacert.pem")
add_definitions (-DCACERT=\\\"${OPENVAS_CA_CERTIFICATE}\\\")
add_definitions (-DCLIENTCERT=\\\"${OPENVAS_CLIENT_CERTIFICATE}\\\")
add_definitions (-DCLIENTKEY=\\\"${OPENVAS_CLIENT_KEY}\\\")

if (MINGW)
  set (GLIB_LDFLAGS "-L${CROSS_ENV}/lib")
  set (OPENVAS_LDFLAGS "-L${CMAKE_INSTALL_PREFIX}/lib")

  set_target_properties (omp PROPERTIES LINK_FLAGS
                         "${GLIB_LDFLAGS} ${OPENVAS_LDFLAGS}")

  set (GLIB_CFLAGS "-I${CROSS_ENV}include")
  set (GLIB_CFLAGS "${GLIB_CFLAGS} -I${CROSS_ENV}include/glib-2.0")
  set (GLIB_CFLAGS "${GLIB_CFLAGS} -I${CROSS_ENV}lib/glib-2.0/include")

  set_target_properties (omp PROPERTIES COMPILE_FLAGS
                         "-I${CMAKE_INSTALL_PREFIX}/include ${GLIB_CFLAGS}")

  target_link_libraries (omp "${GLIB_LDFLAGS} ${OPENVAS_LDFLAGS} -lopenvas_omp -lopenvas_misc -lglib-2.0 -lgthread-2.0 -lgnutls -ltasn1 -lkdewin -lws2_32 -lgcrypt -lintl -lgpg-error -liconv -lz -lole32 -lwinmm")
else (MINGW)
  if (BUILD_STATIC_OMP)
    set(CMAKE_EXE_LINKER_FLAGS -static)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
    set(CMAKE_SHARED_LIBRARY_C_FLAGS)
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
    set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

    target_link_libraries (omp
                           ${GNUTLS_STATIC_LDFLAGS}
                           ${GLIB_STATIC_LDFLAGS}
                           ${GCRYPT_LDFLAGS}
                           ${LIBOPENVAS_OMP_STATIC_LDFLAGS}
                           ${LIBOPENVAS_BASE_STATIC_LDFLAGS}
                           ${LIBOPENVAS_MISC_STATIC_LDFLAGS}
                           ${GPGME_LDFLAGS})
  else (BUILD_STATIC_OMP)
    target_link_libraries (omp
                           ${GNUTLS_LDFLAGS}
                           ${GLIB_LDFLAGS}
                           ${LIBOPENVAS_OMP_LDFLAGS}
                           ${LIBOPENVAS_BASE_LDFLAGS}
                           ${LIBOPENVAS_MISC_LDFLAGS}
                           ${LINKER_HARDENING_FLAGS})
  endif (BUILD_STATIC_OMP)
endif (MINGW)

## Install

install (TARGETS omp
         RUNTIME DESTINATION ${BINDIR})


## Tag files

set (C_FILES "${CMAKE_CURRENT_SOURCE_DIR}/omp.c")
add_custom_target (etags COMMENT "Building TAGS..."
                   COMMAND etags ${C_FILES})
add_custom_target (ctags COMMENT "Building tags..."
                   COMMAND ctags ${C_FILES})
add_custom_target (tags COMMENT "Building tags files...")
add_dependencies (tags etags ctags)

## End
