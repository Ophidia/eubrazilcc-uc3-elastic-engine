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

#ifndef __ERROR_H
#define __ERROR_H

#define AGENT_SUCCESS         0
/* Critical error: allocs, buffer overflows, missing parameters, file not accessible, configuration error, pipe opening, wrong paths or permissions,  */
#define AGENT_ERROR_QUIT      -100
/* Error interaction with MySQL: connect, execute query, disconnect, option setup */
#define AGENT_ERROR_MYSQL     -101
/* Error related to PDAS json output: parsing, schema interpretation  */
#define AGENT_ERROR_JSON      -102
/* Error related to command setup: invalid time range, invalid bounding box, missing output files, invalid command  */
#define AGENT_ERROR_COMMAND   -103
/* Erro related to execution: missing output files  */
#define AGENT_ERROR_EXECUTION -104
/* Erro related to thread errors: inconsistent data, thread termination  */
#define AGENT_ERROR_THREAD    -105

#endif  //__OPH_ERROR_H
