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

#ifndef __COMMANDS_H_
#define __COMMANDS_H_

#include <stdarg.h>
#include <stdio.h>
#include "agent_common.h"
#include "agent_db.h"

/* Macros with experiment status/errors */
#define COMMAND_STATUS_UNKNOWN    "OPH_STATUS_UNKNOWN"
#define COMMAND_STATUS_PENDING    "OPH_STATUS_PENDING"
#define COMMAND_STATUS_RUNNING    "OPH_STATUS_RUNNING"
#define COMMAND_STATUS_COMPLETED  "OPH_STATUS_COMPLETED"
#define COMMAND_STATUS_ERROR      "OPH_STATUS_ERROR"

#define COMMAND_STATUS_CODE_UNKNOWN    100
#define COMMAND_STATUS_CODE_PENDING    200
#define COMMAND_STATUS_CODE_RUNNING    300
#define COMMAND_STATUS_CODE_COMPLETED  400
#define COMMAND_STATUS_CODE_ERROR      500
#define COMMAND_STATUS_CODE_ASSIGNED   600

/* Macros for clearing house system status/error */
#define COMMAND_CHSTATUS_CODE_UNSET     1000
#define COMMAND_CHSTATUS_CODE_SET       2000
#define COMMAND_CHSTATUS_CODE_COMPLETED 4000
#define COMMAND_CHSTATUS_CODE_ERROR     5000

/* Macros with cluster deploy status/errors */
#define COMMAND_DPSTATUS_EMPTY      "EMPTY"
#define COMMAND_DPSTATUS_RUNNING    "RUNNING"
#define COMMAND_DPSTATUS_FINISHED   "FINISHED"
#define COMMAND_DPSTATUS_ERROR      "ERROR"

#define COMMAND_DPSTATUS_CODE_EMPTY    100
#define COMMAND_DPSTATUS_CODE_RUNNING  200
#define COMMAND_DPSTATUS_CODE_FINISHED 300
#define COMMAND_DPSTATUS_CODE_ERROR    400

/* Macros with coordinate system */
#define COMMAND_COORDINATE_WGS84_360  "EPSG:4326 - CMIP5"
#define COMMAND_COORDINATE_WGS84      "EPSG:4326"

/* Macros with south america boundaries */
#define COMMAND_MAX_LAT 14.0
#define COMMAND_MIN_LAT -60.0
#define COMMAND_MAX_LON -26.0
#define COMMAND_MIN_LON -93.0

/* Macros for experiment commands */ 
#define COMMAND_CLIMATESEBAL     "%s -H %s -P %s -u %s -p %s -w %s/%s -a '%lld','%s','%s','%s/%s','%s/%s','%s','%s','lat|lon|time','%s','%s','%s','%s','out1','out2','%s','%s','%s','%s|%s' --json"
#define COMMAND_SEBALINTERANNUAL "%s -H %s -P %s -u %s -p %s -w %s/%s -a '%lld','%s','%s/%s','%s','%s','lat|lon|time','1:%d','%s','%s','%s','out1','out2','%s','%s','%s' --json"
#define COMMAND_MODELINTERCOMPARISON "%s -H %s -P %s -u %s -p %s -w %s/%s -a '%lld','%s','1:%d','%s','1:%d','%s','1:%d','%s','1:%d','%s/%s','%s','lat|lon|time','%s','%s','out1','out2','%s' --json"
#define COMMAND_RELHEIGHT       "%s -H %s -P %s -u %s -p %s -w %s/%s -a '%lld','%s/%s','%s','%s','%s','out1' --json"
#define COMMAND_LIDARINTERCOMPARISON "%s -H %s -P %s -u %s -p %s -w %s/%s -a '%lld','%s/%s','%s','%s','%s','out1',%s --json"
#define COMMAND_ENM             "%s -H %s -P %s -u %s -p %s -w %s/%s -a '%lld','%s','%s','%s','%s','%s','out' --json"

#define COMMAND_LENGTH              1024
#define COMMAND_VIEW_JOBID          "%s -H %s -P %s -u %s -p %s -e 'view %s' --json"
#define COMMAND_FILTER_LENGTH       256

#define COMMAND_DEPLOY_CLUSTER      "OPH_AUTH_HEADER='%s' OPH_INFRASTRUCTURE_URL='%s' %s -H %s -P %s -u %s -p %s -e 'deploy %s' --json"
#define COMMAND_STATUS_CLUSTER      "OPH_AUTH_HEADER='%s' OPH_INFRASTRUCTURE_URL='%s' %s -H %s -P %s -u %s -p %s -e 'deploy_status' --json"
#define COMMAND_ADDRESS_CLUSTER     "OPH_AUTH_HEADER='%s' OPH_INFRASTRUCTURE_URL='%s' %s -H %s -P %s -u %s -p %s -e 'get_server' --json"
#define COMMAND_UNDEPLOY_CLUSTER    "OPH_AUTH_HEADER='%s' OPH_INFRASTRUCTURE_URL='%s' %s -H %s -P %s -u %s -p %s -e 'undeploy' --json"

/**
 * \brief               Function to run a command with json output
 * \param json_string   Json string used to store command output
 * \param current_size  Current size of json string   
 * \param cmd           Command to be run   
 * \return 0 if successfull, another error code otherwise
 */
int run_cmd(char **json_string, size_t *current_size, char *cmd);

/**
 * \brief               Function to build an experiment command
 * \param conf          Configuration parameters 
 * \param con           Connection parameters 
 * \param serv_addr     PDAS server address
 * \param out_path      Path where experiment output should be stored
 * \param jobid         Job ID
 * \param command       String for output command
 * \return 0 if successfull, another error code otherwise
 */
int build_experiment_cmd(configuration_struct *conf, connection_struct *con, char * serv_addr, char *out_path, long long jobid, char **command);


#endif // __COMMANDS_H_
