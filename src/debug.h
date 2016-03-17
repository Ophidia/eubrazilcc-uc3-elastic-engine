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

#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include <stdio.h>

#define LOGGING_MAX_STRING 100

#define LOG_OUTPUT_FORMAT1 "[%s][%s][%s][%ld]\t\t"
#define LOG_OUTPUT_FORMAT2 "[%s][%s][%ld]\t\t"

#define LOG_ERROR 	1
#define LOG_INFO 	2
#define LOG_WARNING 	3
#define LOG_DEBUG	4

#define LOG_ERROR_MESSAGE "ERROR"
#define LOG_INFO_MESSAGE "INFO"
#define LOG_WARNING_MESSAGE "WARNING"
#define LOG_DEBUG_MESSAGE "DEBUG"
#define LOG_UNKNOWN_MESSAGE "UNKNOWN"

#define LOG_ERROR_T 	11
#define LOG_INFO_T	12
#define LOG_WARNING_T 	13
#define LOG_DEBUG_T	14

#define LOGGING_PATH "log/engine.log"
#define LOGGING_PATH_WITH_PREFIX "%s/log/engine.log"

#if defined(NDEBUG) && defined(__GNUC__)
/* gcc's cpp has extensions; it allows for macros with a variable number of
   arguments. We use this extension here to preprocess pmesg away. */
#define pmesg(level, source, line_number, format, args...) ((void)0)
#else
void pmesg(int level, const char *source, long int line_number, const char *format, ...);
/* print a message, if it is considered significant enough.
      Adapted from [K&R2], p. 174 */
#endif

/**
 * \brief             Function to save logging message into file
 * \param level       Level of message (dbug, error, etc.)
 * \param source      Source file where the message occured
 * \param line_number Line number where the message occured
 * \param format      Error message with printf format.
 * \return 0 if successfull, -1 otherwise
 */
void logging(int level, const char *source, long int line_number, const char *format, ...);

/**
 * \brief       Function to initialize log file path
 * \param p     Log path
 * \return 0 if successfull, -1 otherwise
 */
void set_log_prefix(char* p);

#endif /* DEBUG_H */

