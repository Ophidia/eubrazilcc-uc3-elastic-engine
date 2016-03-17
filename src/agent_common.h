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

#ifndef __AGENT_COMMON_H
#define __AGENT_COMMON_H

/* Utility macros */
#define MAX_LENGTH(a,b) (strlen(a) >= strlen(b) ? strlen(a) : strlen(b))
#define CONFIG_BILLION   1000000000

/* Define for maximum number of cluster*/
#define MAX_NUM_CLUSTER 16
#define MAX_NUM_CORE    128

/*Macros for configuration argument */
#define CONFIG_DB_NAME          "METADB_DBNAME"
#define CONFIG_DB_HOST          "METADB_HOST"
#define CONFIG_DB_PORT          "METADB_PORT"
#define CONFIG_DB_USER          "METADB_USER"
#define CONFIG_DB_PASS          "METADB_PASS"
#define CONFIG_WF_PATH          "WORKFLOW_PATH"
#define CONFIG_DATA_PATH        "DATA_PATH"
#define CONFIG_TMP_PATH         "TEMP_PATH"
#define CONFIG_CH_PATH          "CLEARINGHOUSE_PATH"
#define CONFIG_TERM_EXEC        "OPH_TERM_EXEC" 
#define CONFIG_TERM_PORT        "OPH_TERM_PORT"
#define CONFIG_TERM_USER        "OPH_TERM_USER"
#define CONFIG_TERM_PASS        "OPH_TERM_PASS"
#define CONFIG_CHECK_PDASJOB_TIMER  "CHECK_PDASJOB_TIMER"
#define CONFIG_CHECK_DELETE_TIMER  "CHECK_DELETE_TIMER"
#define CONFIG_CHECK_CLUSTER_TIMER "CHECK_CLUSTER_TIMER"           
#define CONFIG_CHECK_EXP_TIMER   "CHECK_EXP_TIMER"     
#define CONFIG_AUTH_HEADER      "OPH_AUTH_HEADER"
#define CONFIG_INFRASTR_URL     "OPH_INFRASTRUCTURE_URL"
#define CONFIG_RADL_FILE        "RADL_FILE"
#define CONFIG_STATIC_DEPLOYMENT  "STATIC_DEPLOYMENT"
#define CONFIG_STATIC_HOST        "STATIC_HOST"
#define CONFIG_STATIC_NUM_CORES   "STATIC_NUM_CORES"
#define CONFIG_TOT_NUM_CLUSTER	"TOT_NUM_CLUSTER"
#define CONFIG_CLOUD_NUM_CORES	"CLOUD_NUM_CORES"
#define CONFIG_JOB_THRESHOLD_FACTOR "JOB_THRESHOLD_FACTOR"
#define CONFIG_CLOUD_UNDEPLOY_CYCLES "CLOUD_UNDEPLOY_CYCLES"

#define CONFIGURATION_FILE  AGENT_LOCATION"/etc/engine.cfg"
#define CONFIGURATION_LINE  512
#define OUTPUT_LINE         1024
#define JSON_BUFFER_SIZE    10 * 1000

/* Macros for file system */
#define FS_MAX_PATH_LEN             1024
#define FS_TMP_FOLDER_PATTERN       "%s/exp%lld"
#define FS_CH_FOLDER_PATTERN        "%s/%s/%s/%s/exp%lld"
#define FS_ARCHIVE_FOLDER_PATTERN   "%s/%s"
#define FS_FILENAME_PATTERN         "%s.%s"

/**
 * \brief               Structure that contains agent configuration parameters
 * \param db_name       name of database
 * \param db_host       name of host
 * \param db_port       MySQL port
 * \param db_user       username to connect to MySQL
 * \param db_pass       password to connect to MySQL
 * \param wf_path       folder where the workflows are located
 * \param data_path     folder where the input data is located
 * \param tmp_path      Path where temporary results are stored
 * \param ch_path       Clearing house system path
 * \param term_exec     Oph_term executable
 * \param term_port     Port used by Oph_term
 * \param term_pass     Pass used by Oph_term
 * \param term_user     User used by Oph_term
 * \param auth_header   ONE authentication header
 * \param infrastr_url  IM URL
 * \param radl_file     RADL file path
 * \param check_pdasjob_timer     Time between PDAS job status checks (nanoseconds)
 * \param check_exp_timer     Time between experiment status checks (seconds)
 * \param check_delete_timer  Time between experiment delete checks (seconds)
 * \param check_cluster_timer Time between cluster status checks (seconds)
 * \param static_deploy     Flag used to enable first static instance
 * \param static_host     Address of PDAS server on static instance
 * \param static_num_cores    Number of threads on first static instance
 * \param tot_num_cluster     Max number of clusters to be used (including static, if enabled)
 * \param cloud_num_cores     Max number of thread per cluster (only for cloud instance)
 * \param job_threshold_factor     Job queue threshold factor (to be multiplied by cores number)
 * \param cloud_undeploy_cycles     Number of idle cycles before undeploy cloud instance
 */
typedef struct
{
	char *db_name;
	char *db_host;
	char *db_port;
	char *db_user;
	char *db_pass;
	char *wf_path;
  char *data_path;
  char *tmp_path;
  char *ch_path;
  char *term_exec;
  char *term_port;
  char *term_pass;
  char *term_user;
  char *auth_header;
  char *infrastr_url;
  char *radl_file;
  long long check_pdasjob_timer;
  long long check_exp_timer;
  long long check_delete_timer;
  long long check_cluster_timer;
  char static_deploy;
  char *static_host;
  long long static_num_cores;
  long long tot_num_cluster;  
  long long cloud_num_cores;  
  long long job_threshold_factor;
  long long cloud_undeploy_cycles;
} configuration_struct;

/**
 * \brief     Function to initilize configuration structure.
 * \param con Pointer to an allocated configuration structure
 * \return 0 if successfull, another error code otherwise
 */
int init_configuration(configuration_struct *conf);

/**
 * \brief     Function to free memory allocated.
 * \param con Pointer to an allocated configuration structure
 * \return 0 if successfull, another error code otherwise
 */
int free_configuration(configuration_struct *conf);

/**
 * \brief     Function to read configuration params from configuration file
 * \param con Pointer to an allocated configuration structure
 * \return 0  if successfull, another error code otherwise
 */
int read_config_file(configuration_struct *conf);


#endif  //__AGENT_COMMON_H
