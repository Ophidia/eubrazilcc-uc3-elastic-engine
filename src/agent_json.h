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

#ifndef __AGENT_JSON_H_
#define __AGENT_JSON_H_

#include <stdarg.h>
#include <stdio.h>

/* Macros to define relevant strings inside Json output */
#define JSON_JOBID_STRING      "\"jobid\":\""
#define JSON_JOBID_SEPARATOR   "\","
#define JSON_EXPERIMENT_STRING "experiment?"
#define JSON_JOBID_FORMAT      "%s#%s" 
#define JSON_TITLE_FOR_CUBE 	"Output Cube"
#define JSON_INFRASTRUCT_URL    "The new infrastructure URL is now \\\""
#define JSON_INFRASTRUCT_URL_END "\\\"."
#define JSON_STATUS             "Installation and configuration "
#define JSON_STATUS_END         ".\\n"
#define JSON_SERVER_ADDRESS     "The new server address is now \\\""
#define JSON_SERVER_ADDRESS_END "\\\"."    
#define JSON_DELETE_ERROR       "Unable to do the DELETE request"    

/**
 * \brief                Function to get cluster infrastructure URL.
 * \param json_string   String with the json content
 * \param infr_url      URL of the infrastructure
 * \return 0 if successfull, another error code otherwise
 */
int get_infrastruct_url(char *json_string, char **infr_url);

/**
 * \brief                Function to get cluster deploy status.
 * \param json_string   String with the json content
 * \param deploy_status Status of the deployment of infrastructure
 * \return 0 if successfull, another error code otherwise
 */
int get_deploy_status(char *json_string, short int *deploy_status);

/**
 * \brief                Function to get cluster undeploy status.
 * \param json_string   String with the json content
 * \param deploy_status Status of the deployment of infrastructure
 * \return 0 if successfull, another error code otherwise
 */
int get_undeploy_status(char *json_string, short int *deploy_status);

/**
 * \brief                Function to get cluster server IP address.
 * \param json_string   String with the json content
 * \param server_addr   IP of the server of the infrastructure
 * \return 0 if successfull, another error code otherwise
 */
int get_server_address(char *json_string, char **server_addr);

/**
 * \brief                Function to get workflow id from Json.
 * \param json_string   String with the json content
 * \param wfid         ID of the workflow
 * \return 0 if successfull, another error code otherwise
 */
int get_workflowid(char *json_string, char **wfid);

/**
 * \brief                Function to get job status from Json.
 * \param json_string   String with the json content
 * \param job_status    Status of job
 * \return 0 if successfull, another error code otherwise
 */
int get_job_status(char *json_string, short int *job_status);

/**
 * \brief                Function to get job id from Json.
 * \param json_string   String with the json content
 * \param jid           Job ID
 * \return 0 if successfull, another error code otherwise
 */
int get_jobid(char *json_string, char **jid);

#endif // __AGENT_JSON_H_
