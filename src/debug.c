/*
    EUBrazilCC UC3 Elastic Engine
    Copyright 2014-2015 EUBrazilCC (EU‐Brazil Cloud Connect)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "debug.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern int msglevel; /* the higher, the more messages... */
char* prefix=NULL;
char** backtrace=NULL;

#define MAX_LOG_LINE 1024

#if defined(NDEBUG) && defined(__GNUC__)
/* Nothing. pmesg has been "defined away" in debug.h already. */
#else
void pmesg(int level, const char* source, long int line_number, const char* format, ...) {
#ifdef NDEBUG
	/* Empty body, so a good compiler will optimise calls
	   to pmesg away */
#else
  va_list args;
	char log_type[10];

  int new_msglevel=msglevel % 10;
  if (level>new_msglevel)
          return;
		
	switch ( level ) {
		case LOG_ERROR:
		  sprintf(log_type,LOG_ERROR_MESSAGE);
		  break;
		case LOG_INFO:
		  sprintf(log_type,LOG_INFO_MESSAGE);
		  break;
		case LOG_WARNING:
		  sprintf(log_type,LOG_WARNING_MESSAGE);
		  break;
		case LOG_DEBUG:
		  sprintf(log_type,LOG_DEBUG_MESSAGE);
		  break;
		default:
		  sprintf(log_type,LOG_UNKNOWN_MESSAGE);
		  break;
		}

	char log_line[MAX_LOG_LINE];
	if (msglevel>10) {
		time_t t1=time(NULL);
		char *s=ctime(&t1);
   	s[strlen(s)-1]=0;        // remove \n
		snprintf(log_line,MAX_LOG_LINE,LOG_OUTPUT_FORMAT1,s,log_type, source,line_number);
	} else {
		snprintf(log_line,MAX_LOG_LINE,LOG_OUTPUT_FORMAT2,log_type, source,line_number);
	}

	fprintf(stderr,"%s",log_line);
	if (backtrace)
	{
		int length = 2 + (*backtrace ? strlen(*backtrace) : 0) + strlen(log_line);
		char tmp[length];
		snprintf(tmp, length, "%s%s", *backtrace ? *backtrace : "", log_line);
		if (*backtrace) free(*backtrace);
		*backtrace = strdup(tmp);
	}

	va_start(args, format);
	vsnprintf(log_line, MAX_LOG_LINE, format, args);
	va_end(args);

	fprintf(stderr,"%s",log_line);
	if (backtrace)
	{
		int length = 2 + (*backtrace ? strlen(*backtrace) : 0) + strlen(log_line);
		char tmp[length];
		snprintf(tmp, length, "%s%s", *backtrace ? *backtrace : "", log_line);
		if (*backtrace) free(*backtrace);
		*backtrace = strdup(tmp);
	}

#endif /* NDEBUG */
}
#endif /* NDEBUG && __GNUC__ */

void logging(int level, const char* source, long int line_number, const char* format, ...)
{
	int new_msglevel=msglevel % 10;
	if (level>new_msglevel) return;

	char namefile[LOGGING_MAX_STRING];
	if (prefix) snprintf(namefile,LOGGING_MAX_STRING, LOGGING_PATH_WITH_PREFIX ,prefix);
	else snprintf(namefile,LOGGING_MAX_STRING, LOGGING_PATH);
	FILE* log_file;
	if ((log_file = fopen(namefile,"a")))
	{
		va_list args;
		char log_type[10];

		switch ( level ) {
			case LOG_ERROR:
			  sprintf(log_type,LOG_ERROR_MESSAGE);
			  break;
			case LOG_INFO:
			  sprintf(log_type,LOG_INFO_MESSAGE);
			  break;
			case LOG_WARNING:
			  sprintf(log_type,LOG_WARNING_MESSAGE);
			  break;
			case LOG_DEBUG:
			  sprintf(log_type,LOG_DEBUG_MESSAGE);
			  break;
			default:
			  sprintf(log_type,LOG_UNKNOWN_MESSAGE);
			  break;
			}

		char log_line[MAX_LOG_LINE];

		if (msglevel>10) {
			time_t t1=time(NULL);
			char *s=ctime(&t1);
     	s[strlen(s)-1]=0;        // remove \n
			snprintf(log_line, MAX_LOG_LINE,LOG_OUTPUT_FORMAT1,s,log_type, source,line_number);
		} else {
			snprintf(log_line, MAX_LOG_LINE,LOG_OUTPUT_FORMAT2,log_type, source,line_number);
		}

		fprintf(log_file,"%s",log_line);
		if (backtrace)
		{
			int length = 2 + (*backtrace ? strlen(*backtrace) : 0) + strlen(log_line);
			char tmp[length];
			snprintf(tmp, length, "%s%s", *backtrace ? *backtrace : "", log_line);
			if (*backtrace) free(*backtrace);
			*backtrace = strdup(tmp);
		}

		va_start(args, format);
		vsnprintf(log_line, MAX_LOG_LINE, format, args);
		va_end(args);

		fprintf(log_file,"%s",log_line);
		if (backtrace)
		{
			int length = 2 + (*backtrace ? strlen(*backtrace) : 0) + strlen(log_line);
			char tmp[length];
			snprintf(tmp, length, "%s%s", *backtrace ? *backtrace : "", log_line);
			if (*backtrace) free(*backtrace);
			*backtrace = strdup(tmp);
		}

		fclose(log_file);
	}
	else pmesg(LOG_ERROR, source, line_number, "Error in opening log file '%s'\n",namefile);
}

void set_log_prefix(char* p) { prefix = p; }
