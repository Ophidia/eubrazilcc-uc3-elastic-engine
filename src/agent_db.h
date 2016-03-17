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

#ifndef __AGENT_DB_H
#define __AGENT_DB_H

#include <mysql.h>
#include "agent_common.h"

#define DB_NULL_FIELD "NULL"
#define DB_MAX_FIELD_LENGTH 1024

#define DB_EXP_CHSTATUS_NOTSTORED 0 
#define DB_EXP_CHSTATUS_ONGOING   1
#define DB_EXP_CHSTATUS_ERROR     2
#define DB_EXP_CHSTATUS_STORED    3

/**
 * \brief           Structure that contains database connection parameters
 * \param name      name of database
 * \param host      name of host
 * \param port      MySQL port
 * \param user      username to connect to MySQL
 * \param pass      password to connect to MySQL
 * \param wfolder   folder where the workflow are located
 * \param conn      pointer to a MYSQL connection
 */
typedef struct
{
	char *name;
	char *host;
	char *port;
	char *user;
	char *pass;
	MYSQL *conn;
} connection_struct;

/**
 * \brief                 Enumeration type defining the experiment strcuture type (0 is reserved for unknown type) 
 */
typedef enum 
{
   EXP_CLIMATESEBAL = 1,
   EXP_SEBALINTERANNUAL = 2,
   EXP_MODELINTERCOMPARISON = 3,
   EXP_RELATIVEHEIGHT = 4,
   EXP_LIDARINTERCOMPARISON = 5,
   EXP_ENM = 6
}exp_type;

/**
 * \brief                 Structure that contains SEBAL interannual experiment info
 * \param workflowcode    Name of workflow
 * \param workflowfile    Name of workflow file
 * \param bbox            Bounding box values
 * \param timeinterval    First and last time steps
 * \param folder          Input file folder
 * \param filename        Input file name
 * \param coordinate      Coordinate system used by dataset
 * \param timerange       Dataset time range
 * \param timefrequency   Dataset time frequency
 * \param basetime        Dataset base time
 * \param units           Dataset units
 * \param calendar        Dataset calendar
 * \param variable        Dataset variable
 * \param variablename    Dataset variable long name
 * \param pdasid          ID of data in PDAS
 */
typedef struct
{
	char *workflowcode;
	char *workflowfile;
  double bbox[4];
  char *timeinterval[2];
  char *filename;
  char *folder;
	char *coordinate;
	char *timerange;
	char *timefrequency;
	char *basetime;
	char *units;
  char *calendar;
	char *variable;
	char *variablename;
  char *pdasid;
} experiment_sebalinterannual;

/**
 * \brief                 Structure that contains Climate - SEBAL intercomparison experiment info
 * \param workflowcode    Name of workflow
 * \param workflowfile    Name of workflow file
 * \param bbox            Bounding box values
 * \param timeinterval    First and last time steps
 * \param climate_folder          Climate dataset input file folder
 * \param climate_filename        Climate dataset input file name
 * \param climate_coordinate      Climate dataset coordinate system
 * \param climate_timerange       Climate dataset time range
 * \param climate_timefrequency   Climate dataset time frequency
 * \param climate_basetime        Climate dataset base time
 * \param climate_units           Climate dataset units
 * \param climate_calendar        Climate dataset calendar
 * \param climate_variable        Climate dataset variable
 * \param climate_variablename    Climate dataset variable long name
 * \param satellite_folder          Satellite dataset input file folder
 * \param satellite_filename        Satellite dataset input file name
 * \param satellite_coordinate      Satellite dataset coordinate system
 * \param satellite_timerange       Satellite dataset time range
 * \param satellite_timefrequency   Satellite dataset time frequency
 * \param satellite_basetime        Satellite dataset base time
 * \param satellite_units           Satellite dataset units
 * \param satellite_calendar        Satellite dataset calendar
 * \param satellite_units           Satellite dataset variable
 * \param satellite_variable        Satellite dataset variable
 * \param satellite_variablename    Satellite dataset variable long name
 * \param satellite_pdasid          ID of data in PDAS
 */
typedef struct
{
	char *workflowcode;
	char *workflowfile;
  double bbox[4];
  char *timeinterval[2];
  char *climate_filename;
  char *climate_folder;
	char *climate_coordinate;
	char *climate_timerange;
	char *climate_timefrequency;
	char *climate_basetime;
	char *climate_units;
  char *climate_calendar;
	char *climate_variable;
	char *climate_variablename;
  char *satellite_filename;
  char *satellite_folder;
	char *satellite_coordinate;
	char *satellite_timerange;
	char *satellite_timefrequency;
	char *satellite_basetime;
	char *satellite_units;
  char *satellite_calendar;
	char *satellite_variable;
	char *satellite_variablename;
  char *satellite_pdasid;
} experiment_climatesebal;

/**
 * \brief                 Structure that contains Climate Model Intecomparison experiment info
 * \param workflowcode    Name of workflow
 * \param workflowfile    Name of workflow file
 * \param bbox            Bounding box values
 * \param timeinterval    First and last time steps
 * \param folder          Input file folder
 * \param coordinate      Coordinate system used by dataset
 * \param timerange       Dataset time range
 * \param timefrequency   Dataset time frequency
 * \param basetime        Dataset base time
 * \param units           Dataset units
 * \param calendar        Dataset calendar
 * \param models          Null-terminated array of model names
 * \param scenarios       Null-terminated array of scenario names
 * \param indicators      Array of indicators names
 */
typedef struct
{
	char *workflowcode;
	char *workflowfile;
  double bbox[4];
  char *timeinterval[2];
  char *folder;
	char *coordinate;
	char *timerange;
	char *timefrequency;
	char *basetime;
	char *units;
  char *calendar;
	char **models;
	char **scenarios;
	char *indicators[4];
} experiment_modelintercomparison;

/**
 * \brief                 Structure that contains LiDAR product Intecomparison experiment info
 * \param workflowcode    Name of workflow
 * \param workflowfile    Name of workflow file
 * \param folder          Input file folder
 * \param filename        Input file name
 * \param coordinate      Coordinate system used by dataset
 * \param products        Array of LiDAR products names
 */
typedef struct
{
	char *workflowcode;
	char *workflowfile;
  char *filename;
  char *folder;
	char *coordinate;
	char *products[9];
} experiment_lidarintercomparison;

/**
 * \brief                 Structure that contains Relative Height Analysis experiment info
 * \param workflowcode    Name of workflow
 * \param workflowfile    Name of workflow file
 * \param folder          Input file folder
 * \param filename        Input file name
 * \param coordinate      Coordinate system used by dataset
 */
typedef struct
{
	char *workflowcode;
	char *workflowfile;
  char *filename;
  char *folder;
	char *coordinate;
} experiment_relheight;

/**
 * \brief                 Structure that contains ENM experiment info
 * \param workflowcode    Name of workflow
 * \param workflowfile    Name of workflow file
 * \param species         Name of species
 * \param num_occurrences Number of occurrences
 * \param x               Array of x coordinate 
 * \param y               Array of y coordinate 
 */
typedef struct
{
	char *workflowcode;
	char *workflowfile;
  char *species;
  unsigned int num_occurrences;
  double *x;
  double *y;
} experiment_enm;

/**
 * \brief     Function to initilize MySQL library.
 * \return 0 if successfull, another error code otherwise
 */
int init_db_lib();

/**
 * \brief     Function to end MySQL library.
 * \return 0 if successfull, another error code otherwise
 */
int end_db_lib();


/**
 * \brief     Function to initilize connection structure.
 * \param con Pointer to an allocated connection structure
 * \return 0 if successfull, another error code otherwise
 */
int init_connection(connection_struct *con, configuration_struct *conf);

/**
 * \brief     Function to free memory allocated.
 * \param con Pointer to an allocated connection structure
 * \return 0 if successfull, another error code otherwise
 */
int free_connection(connection_struct *con);

/**
 * \brief     Function to connect to database. WARNING: Call this function before any other function of this library
 * \param con Pointer to structure containing database parameters
 * \param multi Flag to enable multi statement
 * \return 0 if successfull, another error code otherwise
 */
int db_connect(connection_struct *con, short int multi);

/**
 * \brief     Function to disconnect from the database
 * \param con Pointer to structure containig database connection
 * \return 0 if successfull, another error code otherwise
 */
int db_disconnect(connection_struct *con);

/**
 * \brief       Function to initilize an experiment structure.
 * \param exp   Void pointer to a non-allocated experiment structure
 * \param type  Type of experiment structure to be initialized
 * \return 0 if successfull, another error code otherwise
 */
int init_experiment(void **exp, exp_type type);

/**
 * \brief     Function to free experiment structure allocated.
 * \param exp   Void pointer to an allocated experiment structure
 * \param type  Type of experiment structure to be freed
 * \return 0 if successfull, another error code otherwise
 */
int free_experiment(void **exp, exp_type type);

/**
 * \brief           Function to get list of all new experiments submitted (those in pending status)
 * \param con       Pointer to structure containing database connection
 * \param expids    Pointer to null-terminated array of experiment id to be filled (may also be empty)
 * \param expsizes  Pointer to null-terminated array of experiment sizes to be filled
 * \return 0 if successfull, another error code otherwise
 */
int get_new_exp_list(connection_struct *con, long long **expids, int **expsizes);

/**
 * \brief                 Function to get list of all experiments to be stored in clearing house (those with clearing house flag set)
 * \param con             Pointer to structure containing database connection
 * \param expids          Pointer to null-terminated array of experiment id to be filled (may also be empty)
 * \param submissiondates Pointer to null-terminated array of experiment submission date to be filled
 * \return 0 if successfull, another error code otherwise
 */
int get_new_ch_exp_list(connection_struct *con, long long **expids, char ***submissiondates);

/**
 * \brief       Function to get list of all experiments to be deleted from file system and set them to unavailable (those older than a certain period)
 * \param con   Pointer to structure containing database connection
 * \param expids          Pointer to null-terminated array of experiment id to be filled (may also be empty)
 * \param expchstatus     Pointer to null-terminated array of experiment clearing house status values to be filled (may also be empty)
 * \param submissiondates Pointer to null-terminated array of experiment submission date to be filled
 * \return 0 if successfull, another error code otherwise
 */
int get_old_exp_list(connection_struct *con, long long **expids, short **expchstatus, char ***submissiondates);

/**
 * \brief           Function to get list of all files related to an experiment
 * \param con       Pointer to structure containing database connection
 * \param epxid     Id of experiment to look at
 * \param files      Pointer to null-terminated array of files to be filled (may also be empty)
 * \param extensions Pointer to null-terminated array of file extesions to be filled
 * \param file_num   Number of file related to experiment
 * \return 0 if successfull, another error code otherwise
 */
int get_exp_files(connection_struct *con, long long expid, char ***files, char ***extensions, int *file_num);

/**
 * \brief           Function to get list of all arguments related to an experiment
 * \param con       Pointer to structure containing database connection
 * \param expid     Id of experiment to be retrieved
 * \param type      Type of experiment structure to be filled 
 * \param exp       Void pointer for experiment sctructure
 * \return 0 if successfull, another error code otherwise
 */
int get_experiment_info(connection_struct *con, long long expid, exp_type *type, void **exp);

/**
 * \brief               Function to add output to an experiment
 * \param con           Pointer to structure containing database connection
 * \param expid         Id of experiment to be retrieved
 * \param type          Type of experiment structure to be filled 
 * \param output_names  Null-terminated array of output names
 * \return 0 if successfull, another error code otherwise
 */
int add_experiment_output(connection_struct *con,  long long expid, exp_type type, char **output_names);


/**
 * \brief             Function to update experiment status
 * \param con         Pointer to structure containing database connection
 * \param expid       Experiment id to be updated
 * \param exp_status  Status of experiment to be updated
 * \param wfstring    Workflow string
 * \param wid         Workflow job id
 * \return 0 if successfull, another error code otherwise
 */
int update_exp_status(connection_struct *con,  long long expid, short int exp_status, char *wfstring, char *wid);

/**
 * \brief             Function to update experiment status in clearing house 
 * \param con         Pointer to structure containing database connection
 * \param expid       Experiment id to be updated
 * \param ch_status   Clearing house status of experiment to be updated
 * \return 0 if successfull, another error code otherwise
 */
int update_exp_ch(connection_struct *con, long long expid, short int ch_status);

/**
 * \brief               Function to add new cluster status to history
 * \param con           Pointer to structure containing database connection
 * \param tid           Thread id of cluster
 * \param job_num       Number of job currently managed by the cluster
 * \param deploy_status Status of the cluster
 * \param server_add    Address of the cluster
 * \return 0 if successfull, another error code otherwise
 */
int add_cluster_hist_status(connection_struct *con, int tid, int job_num, short int deploy_status, char *server_add);

/**
 * \brief               Function to update current cluster status
 * \param con           Pointer to structure containing database connection
 * \param tid           Thread id of cluster
 * \param job_num       Number of job currently managed by the cluster
 * \param deploy_status Status of the cluster
 * \param server_add    Address of the cluster
 * \return 0 if successfull, another error code otherwise
 */
int add_cluster_curr_status(connection_struct *con, int tid, int job_num, short int deploy_status, char *server_add);

/**
 * \brief             Function to delete transient info (cluster statuses and unsaved experiments)
 * \param con         Pointer to structure containing database connection
 * \return 0 if successfull, another error code otherwise
 */
int clean_db_transient(connection_struct *con);

#endif  //__AGENT_DB_H
