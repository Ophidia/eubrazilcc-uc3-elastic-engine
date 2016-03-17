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

LIBRARY =

LIBRARY += libdebug.la
libdebug_la_SOURCES = debug.c 
libdebug_la_CFLAGS= -prefer-pic -I. ${lib_CFLAGS} $(OPT) 
libdebug_la_LDFLAGS = -static
libdebug_la_LIBADD = -lz -lm

LIBRARY += liboph_json.la
liboph_json_la_SOURCES = oph_json/oph_json_common.c oph_json/oph_json_graph.c oph_json/oph_json_grid.c oph_json/oph_json_multigrid.c oph_json/oph_json_text.c oph_json/oph_json_tree.c
liboph_json_la_CFLAGS= -prefer-pic -I. -Ioph_json $(JANSSON_CFLAGS) ${lib_CFLAGS} $(OPT) 
liboph_json_la_LDFLAGS = -static -Wl,-rpath -Wl,$(JANSSON_LIBDIR)
liboph_json_la_LIBADD = $(JANSSON_LIBS)

LIBRARY += libagent_common.la
libagent_common_la_SOURCES = agent_common.c
libagent_common_la_CFLAGS= -prefer-pic -I. ${lib_CFLAGS} $(OPT) -DAGENT_LOCATION=\"${prefix}\"
libagent_common_la_LDFLAGS = -static
libagent_common_la_LIBADD = -lz -lm -L. -ldebug

LIBRARY += libagent_db.la
libagent_db_la_SOURCES = agent_db.c
libagent_db_la_CFLAGS= -prefer-pic -I. ${lib_CFLAGS} $(MYSQL_CFLAGS) $(OPT)
libagent_db_la_LDFLAGS = -static 
libagent_db_la_LIBADD = -lz -lm $(MYSQL_LDFLAGS) -L. -ldebug

LIBRARY += libagent_command.la
libagent_command_la_SOURCES = agent_commands.c
libagent_command_la_CFLAGS= -prefer-pic -I. ${lib_CFLAGS} $(OPT) -DAGENT_LOCATION=\"${prefix}\" $(MYSQL_CFLAGS)  
libagent_command_la_LDFLAGS = -static 
libagent_command_la_LIBADD = -L. -ldebug -lagent_db 

LIBRARY += libagent_queue.la
libagent_queue_la_SOURCES = agent_queue.c
libagent_queue_la_CFLAGS= -prefer-pic -I. ${lib_CFLAGS}  $(OPT)
libagent_queue_la_LDFLAGS = -static 
libagent_queue_la_LIBADD = -lz -lm -L. -ldebug

LIBRARY += libagent_json.la
libagent_json_la_SOURCES = agent_json.c
libagent_json_la_CFLAGS= -prefer-pic -I. ${lib_CFLAGS} $(OPT)  $(MYSQL_CFLAGS) 
libagent_json_la_LDFLAGS = -static 
libagent_json_la_LIBADD = -lz -lm -L. -ldebug -loph_json

LIBRARY += libagent_folder.la
libagent_folder_la_SOURCES = agent_folder.c
libagent_folder_la_CFLAGS= -prefer-pic -I. ${lib_CFLAGS}  $(OPT) $(MYSQL_CFLAGS)
libagent_folder_la_LDFLAGS = -static 
libagent_folder_la_LIBADD = -lz -lm -L. -ldebug -lagent_db

LIBRARY += libagent_routines.la
libagent_routines_la_SOURCES = agent_routines.c
libagent_routines_la_CFLAGS= -prefer-pic -I. ${lib_CFLAGS}  $(MYSQL_CFLAGS)  $(OPT)
libagent_routines_la_LDFLAGS = -static 
libagent_routines_la_LIBADD = -lz -lm -L. -ldebug -lagent_db -lagent_common -lagent_json -lagent_command -lagent_folder $(LIBCURL_LIB)

LIBRARY += libagent_thread.la
libagent_thread_la_SOURCES = agent_thread.c
libagent_thread_la_CFLAGS= -prefer-pic -I. ${lib_CFLAGS} $(MYSQL_CFLAGS)  $(OPT)
libagent_thread_la_LDFLAGS = -static 
libagent_thread_la_LIBADD = -lz -lm -L. -ldebug -lagent_db -lagent_queue -lagent_routines -lagent_command -lagent_json

noinst_LTLIBRARIES = $(LIBRARY)

bin_PROGRAMS=engine
bindir=${prefix}/bin

engine_SOURCES= engine.c
if DEBUG_MODE
engine_CFLAGS= $(OPT) -I. ${lib_CFLAGS} $(MYSQL_CFLAGS) -DAGENT_LOCATION=\"${prefix}\" -DENABLE_DEBUG
else
engine_CFLAGS= $(OPT) -I. ${lib_CFLAGS} $(MYSQL_CFLAGS) -DAGENT_LOCATION=\"${prefix}\"
endif
engine_LDADD= -L. -lagent_db -ldebug -lagent_common -lagent_thread -lagent_routines
