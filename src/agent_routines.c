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

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "debug.h"
#include <errno.h>
#include <time.h>

#include "agent_error.h"
#include "agent_commands.h"
#include "agent_json.h"
#include "agent_routines.h"
#include "agent_folder.h"
#include "agent_db.h"

#include <ftw.h>
#include <unistd.h>

extern int msglevel;

/* Launch an experiment workflow */
int _launch_exp_cmd(char **json_string, size_t *curr_size, char *command, char **wid)
{
	if(!command || !wid || !json_string || !curr_size)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  int res = 0;

  *wid = NULL;
  res = run_cmd(json_string, curr_size, command); 
  if( res )
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Failed to run experiment command: %s\n", command);
		return res;
  }
  /* Get experiment workflow id */
  res = get_workflowid(*json_string, wid);
  if ( res )
  {
    logging(LOG_WARNING, __FILE__, __LINE__, "Failed to retrieve workflow id\n");
		return res;
  }

  return AGENT_SUCCESS;
}

/* Check experiment command status from json */
int _check_exp_cmd_status(configuration_struct *conf, char *wid, char **json_string, size_t *curr_size, char *serv_addr, short int *pdasjob_status)
{
	if(!conf || !wid || !json_string || !curr_size || !serv_addr || !pdasjob_status)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  long long nsec = 0;
  long long sec = 0;
  /*Compute nanoseconds for timespec struct */
  if(conf->check_pdasjob_timer >= CONFIG_BILLION)
  {
    sec = (long long)(conf->check_pdasjob_timer/CONFIG_BILLION);
    nsec = conf->check_pdasjob_timer - sec*CONFIG_BILLION;
  }
  else
  {
    nsec = conf->check_pdasjob_timer;
  }

  struct timespec timer;
  timer.tv_sec = sec;
  timer.tv_nsec = nsec;
  struct timespec remainder;

  char command[COMMAND_LENGTH] = {'\0'};
  size_t length;
  int res = AGENT_SUCCESS;

  /* Check job status */
  length = snprintf(command, COMMAND_LENGTH, COMMAND_VIEW_JOBID, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, wid);

  /* Loop on job status */
  while(1)
  {
    /* Run check experiment cycle */
    if((res = run_cmd(json_string, curr_size, command)))
    {
  		logging(LOG_WARNING, __FILE__, __LINE__, "Failed to run check command: %s\n", command);
      *pdasjob_status = COMMAND_STATUS_CODE_ERROR;
      break;
    }
  
    *pdasjob_status = 0;

    if((res = get_job_status(*json_string, pdasjob_status)))
    {
      logging(LOG_WARNING, __FILE__, __LINE__, "Error while checking experiment status\n");
      *pdasjob_status = COMMAND_STATUS_CODE_ERROR;
      break;
    }

    if(*pdasjob_status == COMMAND_STATUS_CODE_COMPLETED || *pdasjob_status == COMMAND_STATUS_CODE_ERROR)
      break;
    else
      nanosleep(&timer, &remainder);
  }

  return res;
}

/* Internal function to execute and update a single experiment */
int _exec_experiment(configuration_struct *conf, connection_struct *con, char * serv_addr, long long expid, short int *exp_status, char **wid)
{
	if( !conf || !con || !expid || !serv_addr || !exp_status || !wid )
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  *exp_status = COMMAND_STATUS_CODE_ERROR;
  *wid = NULL;
  char *command = NULL;
  char path[FS_MAX_PATH_LEN];
  snprintf(path, sizeof(path),FS_TMP_FOLDER_PATTERN, conf->tmp_path, expid);
  int res = 0;

  size_t curr_size = JSON_BUFFER_SIZE;
  char *json_string = (char *)malloc(sizeof(char) * curr_size);
  if(!json_string)
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Error re-allocating memory\n");
    return AGENT_ERROR_QUIT;
  }
  curr_size = JSON_BUFFER_SIZE;

  /* Create directories if not exist */
  if(folder_rec_mkdir(path)){
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to create dir %s\n", path);
    free (json_string);
    return AGENT_ERROR_QUIT;
  }

  /* Create experiment command */
  res = build_experiment_cmd(conf, con, serv_addr, path, expid, &command);  
  if( res )
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Failed to build experiment command: %s\n", (command ? command: "ERROR"));
    free (json_string);
    return res;
  }

  /* Set experiment to running status */
  if(update_exp_status(con, expid, COMMAND_STATUS_CODE_RUNNING, command, NULL))
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to update experiments\n" );
    free(command);
    free (json_string);
		return AGENT_ERROR_QUIT;
  }  

  res = _launch_exp_cmd(&json_string, &curr_size, command, wid); 
  if( res )
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Failed to run experiment command: %s\n", command);
    free(command);
    free(json_string);
    return res;
  }
  free(command);

  /* Poll on WID until it is completed */
  res = _check_exp_cmd_status(conf, *wid, &json_string, &curr_size, serv_addr, exp_status); 
  if( res )
  {
    logging(LOG_WARNING, __FILE__, __LINE__, "Error while retrieving the last experiment id\n");
    free(json_string);
    return res;
  }
  free(json_string);

  if(*exp_status == COMMAND_STATUS_CODE_COMPLETED)
  {
    /* Check file existance */
    res = folder_check_file_existance(con, expid, path);
    if(res)
    {
      logging(LOG_ERROR, __FILE__, __LINE__, "Unable to check output files existance\n" );
      return res;
    }
  }

  logging(LOG_DEBUG,__FILE__,__LINE__,"Status %d\n", *exp_status);

  return AGENT_SUCCESS;
}

/* Function to run an experiment */
int exec_experiment(configuration_struct *conf, connection_struct *con, char * serv_addr, long long expid, short int *exp_status, int *exp_weight)
{
	if(!con || !conf || !serv_addr || !expid || !exp_status || !exp_weight)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  *exp_status = COMMAND_STATUS_CODE_ERROR;
  *exp_weight = 1;

  char *wid = NULL;

  /* Exec Experiment */
  if(_exec_experiment(conf, con, serv_addr, expid, exp_status, &wid) || *exp_status != COMMAND_STATUS_CODE_COMPLETED)
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Failed to run experiment with id: %lld\n", expid);
    *exp_status = COMMAND_STATUS_CODE_ERROR;
  }
  else
  {
    logging(LOG_DEBUG,__FILE__,__LINE__,"Experiment executed\n");
    *exp_status = COMMAND_STATUS_CODE_COMPLETED;
  }

  if(update_exp_status(con, expid, *exp_status, NULL, wid))
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to update experiments\n" );
    if(wid) free(wid);
    return AGENT_ERROR_QUIT;
  }  

  if(wid) free(wid);

  return AGENT_SUCCESS;
}

/* Internal function to store in CH the output of one experiment */
int _store_experiment(configuration_struct *conf, connection_struct *con, char *submissiondate, long long expid)
{
	if( !conf || !con || !expid || !submissiondate )
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  char tmp_path[FS_MAX_PATH_LEN];
  char ch_path[FS_MAX_PATH_LEN];
  int res = 0;

  /* Split submission date */
  char year[5] = {0};
  char month[3] = {0};
  char day[3] = {0};

  strncpy ( year, submissiondate, 4 );
  strncpy ( month, submissiondate + 5, 2 );
  strncpy ( day, submissiondate + 8, 2 );
 
  /* Create directories if not exist */
  snprintf(ch_path, sizeof(ch_path),FS_CH_FOLDER_PATTERN, conf->ch_path, year, month, day, expid);
  if(folder_rec_mkdir(ch_path)){
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to create dir %s\n", ch_path);
    return AGENT_ERROR_QUIT;
  }

  /* Check file existance and move from tmp path to ch path */
  snprintf(tmp_path, sizeof(tmp_path),FS_TMP_FOLDER_PATTERN, conf->tmp_path, expid);
  res = folder_move_files(con, expid, tmp_path, ch_path);
  if( res )
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to move files from %s to %s\n", tmp_path, ch_path );
    return res;
  }

  logging(LOG_DEBUG,__FILE__,__LINE__,"Files stored in the clearing house system!\n");

  return AGENT_SUCCESS;
}

/* Function to store all files in experiment list */
int store_experiment(configuration_struct *conf, connection_struct *con, char *submissiondate, long long expid, short int *exp_status)
{
	if(!conf || !con || !submissiondate || !expid || !exp_status)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  *exp_status = COMMAND_CHSTATUS_CODE_ERROR;
  char tmp_path[FS_MAX_PATH_LEN];
  int fd_limit = 4;
  int res = 0;

  /* Store experiment */
  if(_store_experiment(conf, con, submissiondate, expid))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Failed to store experiment with id: %lld\n", expid);
    *exp_status = COMMAND_CHSTATUS_CODE_ERROR;
  }
  else
  {
    logging(LOG_DEBUG,__FILE__,__LINE__,"Experiment files stored\n");
    *exp_status = COMMAND_CHSTATUS_CODE_COMPLETED;
  }

  /* Update experiment status in ch */
  if(update_exp_ch(con, expid, *exp_status))
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to update experiments\n" );
    return AGENT_ERROR_QUIT;
  }  

  /* Delete exp folder */
  snprintf(tmp_path, sizeof(tmp_path),FS_TMP_FOLDER_PATTERN, conf->tmp_path, expid);
  
  res = nftw(tmp_path, folder_delete_files, fd_limit, FTW_DEPTH | FTW_PHYS);
  if(res != 0)
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Failed to delete folder of experiment id: %lld\n", expid);
  }

  return AGENT_SUCCESS;
}

int delete_experiment(configuration_struct *conf, connection_struct *con, long long expid, short expchstatus, char *submissiondate)
{
	if(!conf || !con || !expid || !submissiondate)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  int res = 0;
  char folder_path[FS_MAX_PATH_LEN];
  int fd_limit = 4;

  /* Split submission date */
  char year[5] = {0};
  char month[3] = {0};
  char day[3] = {0};

  strncpy ( year, submissiondate, 4 );
  strncpy ( month, submissiondate + 5, 2 );
  strncpy ( day, submissiondate + 8, 2 );
 
  /* Select folder of experiment to be deleted */
  if(expchstatus == DB_EXP_CHSTATUS_NOTSTORED)
  {
    snprintf(folder_path, sizeof(folder_path),FS_TMP_FOLDER_PATTERN, conf->tmp_path, expid);
  }  
  else if(expchstatus == DB_EXP_CHSTATUS_STORED || expchstatus == DB_EXP_CHSTATUS_ERROR)
  {
    snprintf(folder_path, sizeof(folder_path),FS_CH_FOLDER_PATTERN, conf->ch_path, year, month, day, expid);
  }  

  /* Delete exp folder */
  res = nftw(folder_path, folder_delete_files, fd_limit, FTW_DEPTH | FTW_PHYS);
  if(res != 0)
  {
  		logging(LOG_WARNING, __FILE__, __LINE__, "Failed to delete folder of experiment id: %lld\n", expid);
  }

  return AGENT_SUCCESS;
}

int delete_transient(configuration_struct *conf, connection_struct *con)
{
	if(!conf || !con)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  clean_db_transient(con);

  return AGENT_SUCCESS;
}
