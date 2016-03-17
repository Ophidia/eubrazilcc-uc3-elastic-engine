#
#    EUBrazilCC UC3 Elastic Engine
#    Copyright 2014-2015 EUBrazilCC (EU‐Brazil Cloud Connect)
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

AC_PREREQ([2.60])
AC_INIT([eubrazilcc-uc3-elastic-engine], [1.2], [ophidia-info@lists.cmcc.it])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_CC_STDC
AM_PROG_CC_C_O

LT_INIT([disable-static])

# Checks for libraries.
AC_CHECK_LIB([m],[cos])
AX_PTHREAD(AC_MSG_NOTICE("pthread found"),AC_MSG_ERROR("pthread not found"))

# Checks for header files.
AX_LIB_MYSQL

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADERS([stddef.h stdint.h unistd.h stdio.h errno.h signal.h ftw.h ctype.h])
AC_HEADER_STDBOOL
AC_HEADER_DIRENT
AC_HEADER_STAT
AC_HEADER_STDC
AC_HEADER_TIME
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_FUNC_STRTOD
AC_CHECK_FUNCS([memmove memset mkdir strcasecmp strchr strdup strerror strstr strtol])

# Checks for library.
AC_CHECK_LIB(pthread, pthread_create)
if test "$ac_cv_lib_pthread_pthread_create" = yes ; then
   CFLAGS="$CFLAGS -D_REENTRANT"
else
   AC_CHECK_LIB(pthreads, pthread_create)
   if test "$ac_cv_lib_pthreads_pthread_create" = yes ; then
      CFLAGS="$CFLAGS -D_REENTRANT"
   fi
fi

AC_ARG_WITH(mysql_header_path,
	   [  --with-mysql-header-path=PATH 		Location of mysql.h],
	   [
		mysql_header_path="$withval"
	   ])
AC_ARG_WITH(mysql_lib_path,
	   [  --with-mysql-lib-path=PATH 		Location of libmysqlclient.a],
	   [
		mysql_lib_path="$withval"
	   ])
AC_ARG_WITH(mysql_path,
	   [  --with-mysql-path=PATH 			Location of mysql],
	   [
		mysql_path="$withval"
	   ])

PKG_CHECK_MODULES(JANSSON, jansson >= 2.3)
AC_ARG_WITH(jansson_header_path,
       [  --with-jansson-header-path=PATH        Location of Jansson includes directory],
       [
        jansson_header_path="$withval"
       ],
     [
    jansson_header_path="$JANSSON_CFLAGS"
     ])
AC_ARG_WITH(jansson_lib_path,
       [  --with-jansson-lib-path=PATH       Location of jansson lib],
       [
        jansson_lib_path="$withval"
       ],
     [
    jansson_lib_path="$JANSSON_LIBS"
     ])
JANSON_CFLAGS="$jansson_header_path"
AC_SUBST(JANSON_CFLAGS)
JANSON_LIBS="$jansson_lib_path"
AC_SUBST(JANSON_LIBS)
JANSSON_LIBDIR=`pkg-config --variable=libdir jansson`
AC_SUBST(JANSSON_LIBDIR)

optimization="no"
AC_ARG_ENABLE(optimization,
        [  --enable-optimization          turn on optimization flags],
        [optimization="yes"
        AC_MSG_WARN("Optimization activated")],
        [optimization="no"]
        )

debug="no"
AC_ARG_ENABLE(debug,
  	   [  --enable-debug          turn on debug flags],
        [debug="yes"
        AC_MSG_WARN("Debug activated")],
        [debug="no"]
	   )
AM_CONDITIONAL(DEBUG_MODE, test "x$debug" = "xyes")

case "${host}" in
        *-*-solaris*)   PLATFORM=SUN_OS
                        SAMPLE_EXTRA_LIBS="-lxnet -lsocket -lnsl"
                        ;;
        *-*-sysv5*)     PLATFORM=UNIXWARE ;;
        *-*-UnixWare*)  PLATFORM=UNIXWARE ;;
        *-*-unixware*)  PLATFORM=UNIXWARE ;;
        *-*-sco3.2v5*)  PLATFORM=OPENSERVER
                        SAMPLE_EXTRA_LIBS="-lsocket"
                        ;;
        *-*-linux*)     PLATFORM=LINUX
		              if test "x$optimization" = "xyes" ; then
	                   CFLAGS="-O3 -g0 -finline-functions -funswitch-loops -fgcse-after-reload -ftree-loop-linear -ftree-loop-im -ftree-loop-ivcanon -fivopts -ftree-vectorize -ftracer -fgcse-sm -fgcse-las -fgcse-after-reload -funroll-loops -freorder-blocks-and-partition -fweb"
                  else 
                    if test "x$debug" = "xyes"; then
                        CFLAGS="-g3 -Wall -Wextra  -O0" 
                    fi
                  fi
			            ;;
                *-*-cygwin*)    PLATFORM=CYGWIN
                        ;;
        *-*-freebsd*)   PLATFORM=FREEBSD ;;
        *-*-irix*)      PLATFORM=IRIX
                        ;;
        *-*-aix*)       PLATFORM=AIX
                      case "${host}" in
                         *-*-aix4.1*)   osver=AIX41;;
                         *-*-aix4.2*)   osver=AIX42;;
                         *-*-aix4.3*)   osver=AIX43;;
                         *-*-aix4.*)    osver=AIX43;;
                      esac;;
        *-sequent-*)    PLATFORM=PTX ;;
        *-*-hp*)        PLATFORM=HP_UX ;
                        case "${host}" in
                            *-*-hpux11*)  osver=HPUX11;;
                            *-*-hpux10*)  osver=HPUX10;;
                            *)            osver=HPUX11;;
                        esac;;
        *-*-mvs*)       PLATFORM=OS390 ;;
        *-*-os400*)     PLATFORM=OS400 ;;
        *-*-OS400*)     PLATFORM=OS400 ;;
        *-*-osf*)       PLATFORM=TRU64
				        OPT="-g3"
				        REENTRANT="1"
                               ;;
        *-apple-*)      PLATFORM=__APPLE__
				        OPT="-g3"
				        ;;
        *)              PLATFORM=UNKNOWN ;;
esac

AC_OUTPUT([Makefile src/Makefile])
