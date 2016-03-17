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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "debug.h"
#include <errno.h>

#include "agent_common.h"
#include "agent_error.h"
#include "agent_commands.h"
#include "agent_routines.h"
#include "agent_thread.h"
#include "agent_json.h"
#include <time.h>

#include <pthread.h>

/* Global variables for thread handling */
extern pthread_rwlock_t rwlock;
extern pthread_rwlock_t quit_lock;
extern int msglevel;
extern configuration_struct conf;
extern cluster_data data;
extern char agent_quit; 

/* Internal struct for subthread id handling */
typedef struct
{
  unsigned int  cl_tid;
  unsigned int  exp_tid;
} _sub_tid;

int init_cluster_data(cluster_data *data)
{
  if(!data)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  data->static_flag = (short int *)calloc(conf.tot_num_cluster, sizeof(short int));
  if(data->static_flag == NULL)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for cluster data\n" );
		return AGENT_ERROR_QUIT;
	}
  data->job_num = (int *)calloc(conf.tot_num_cluster, sizeof(int));
  if(data->job_num == NULL)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for cluster data\n" );
		return AGENT_ERROR_QUIT;
	}
  data->active = (short int *)calloc(conf.tot_num_cluster, sizeof(short int));
  if(data->active == NULL)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for cluster data\n" );
		return AGENT_ERROR_QUIT;
	}
  data->infrastruct_url = (char **)calloc(conf.tot_num_cluster, sizeof(char *));
  if(data->infrastruct_url == NULL)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for cluster data\n" );
		return AGENT_ERROR_QUIT;
	}
  data->server_address = (char **)calloc(conf.tot_num_cluster, sizeof(char *));
  if(data->server_address == NULL)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for cluster data\n" );
		return AGENT_ERROR_QUIT;
	}
  data->queue = (exp_queue **)calloc(conf.tot_num_cluster, sizeof(exp_queue *));
  if(data->queue == NULL)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for cluster data\n" );
		return AGENT_ERROR_QUIT;
	}
  data->exec_start = (short int *)calloc(conf.tot_num_cluster, sizeof(short int));
  if(data->exec_start == NULL)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for cluster data\n" );
		return AGENT_ERROR_QUIT;
	}

  /*Create array with stop flag for total number of threads*/
  long long tot_number_threads = ( conf.static_deploy == 1 ? ((conf.tot_num_cluster - 1)*conf.cloud_num_cores + conf.static_num_cores ) : conf.tot_num_cluster*conf.cloud_num_cores ); 
  data->exec_stop = (short int *)calloc(tot_number_threads, sizeof(short int));
  if(data->exec_stop == NULL)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for cluster data\n" );
		return AGENT_ERROR_QUIT;
	}

  /*Create queues*/
  int n = 0;
  for( n = 0; n < conf.tot_num_cluster; n++)
  {
    data->queue[n] = (exp_queue *)malloc(1*sizeof(exp_queue));
    if(data->queue[n] == NULL)
    {
	    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for new queue\n" );
      free_cluster_data(data);
	    return AGENT_ERROR_QUIT;  
    }
    init_queue(data->queue[n]);
  }

  return AGENT_SUCCESS;
}

int free_cluster_data(cluster_data *data)
{
  if(!data)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  int i = 0;

  free(data->static_flag);
  free(data->job_num);
  free(data->active);
  free(data->exec_start);
  free(data->exec_stop);
 
	for(i = 0; i < conf.tot_num_cluster; i++)
  {
    if(data->infrastruct_url[i])
    {
      free(data->infrastruct_url[i]);
      data->infrastruct_url[i] = NULL;
    }
    if(data->server_address[i])
    {
      free(data->server_address[i]);
      data->server_address[i] = NULL;
    }
    if(data->queue[i])
    {
      free_queue(data->queue[i]);
      free(data->queue[i]);
      data->queue[i] = NULL;
    }

  }
  free(data->infrastruct_url);
  free(data->server_address);
  free(data->queue);

  return AGENT_SUCCESS;
}

int assign_experiments(connection_struct *con, long long *expids, int *expsizes)
{
  if(!con || !expids || !expsizes)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  int n = 0;
  int i = 0;
  short int assigned = 0;
  int empty_queue = 0;
  int res = 0;
  long long curr_queue_capacity = 0;
  long long empty_queue_capacity = 0;

  if(pthread_rwlock_wrlock(&rwlock) != 0)
  {
    logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
  }
  else
  {
    /* Compute sizes for distribution*/  
    while(expids[n])
    {
      /* Check for thread termination */
      if(pthread_rwlock_rdlock(&quit_lock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
      }
      else
      {
        if(agent_quit == 1)
        {
          if(pthread_rwlock_unlock(&quit_lock) != 0)
          {
            logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
          }
          logging(LOG_DEBUG, __FILE__, __LINE__, "Caught program termination signal!\n" );
          if(pthread_rwlock_unlock(&rwlock) != 0){
            logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
          }
          return AGENT_ERROR_QUIT;
        }
        if(pthread_rwlock_unlock(&quit_lock) != 0)
        {
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
        }
      }

      /* Reset flag */
      assigned = 0;
      empty_queue = -1;

      for(i = 0; i < conf.tot_num_cluster; i++)
      {
        /* Privilege always active clusters with more free queue capacity */
        curr_queue_capacity = (data.static_flag[i] == 1 ? conf.static_num_cores*conf.job_threshold_factor : conf.cloud_num_cores*conf.job_threshold_factor ) - data.job_num[i];
        if( curr_queue_capacity > 0  && data.active[i] == 1 )
        {
          if(empty_queue < 0)
          {
              empty_queue = i;
          }
          else
          {
              empty_queue_capacity = (data.static_flag[empty_queue] == 1 ? conf.static_num_cores*conf.job_threshold_factor : conf.cloud_num_cores*conf.job_threshold_factor ) - data.job_num[empty_queue];
              if( curr_queue_capacity > empty_queue_capacity )
              {
                  empty_queue = i;
              }
          }
        }
      }

      if(empty_queue >= 0)
      {
          data.job_num[empty_queue] += expsizes[n];
          if(enqueue((data.queue[empty_queue]), expids[n]))
          {
            logging(LOG_ERROR, __FILE__, __LINE__, "Unable to add experiment to queue\n" );
            if(pthread_rwlock_unlock(&rwlock) != 0){
              logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
            }
            return AGENT_ERROR_QUIT;
          }
          assigned = 1;
      }
      
      /* If no active clusters are found, recheck inactive clusters*/
      if(assigned == 0)
      {
        for(i = 0; i < conf.tot_num_cluster; i++)
        {
          /* Start from lower id*/
          curr_queue_capacity = (data.static_flag[i] == 1 ? conf.static_num_cores*conf.job_threshold_factor : conf.cloud_num_cores*conf.job_threshold_factor ) - data.job_num[i];
          if( curr_queue_capacity > 0 && data.active[i] == 0 )
          {
            data.job_num[i] += expsizes[n];
            if(enqueue((data.queue[i]), expids[n]))
            {
	            logging(LOG_ERROR, __FILE__, __LINE__, "Unable to add experiment to queue\n" );
              if(pthread_rwlock_unlock(&rwlock) != 0){
                logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
              }
              return AGENT_ERROR_QUIT;
            }
            assigned = 1;
            break;
          }
        }
      }

      /* If no active, empty clusters are found, postpone jobs to next run*/
      if(assigned == 0)
      {
        break;
      }
      else
      {
	      logging(LOG_DEBUG,__FILE__,__LINE__,"Experiment %lld with %d jobs assigned\n", expids[n], expsizes[n]);

        if((res = update_exp_status(con, expids[n], COMMAND_STATUS_CODE_ASSIGNED, NULL, NULL)))
        {
	        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to update experiments\n" );
          if(pthread_rwlock_unlock(&rwlock) != 0){
            logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
          }
          return res;
        }  
      }
      n++;
    }

    /* Remove lock */
    if(pthread_rwlock_unlock(&rwlock) != 0){
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
    }
  }

  return AGENT_SUCCESS;
}

int update_cluster_status(connection_struct *con)
{
  if(!con)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  int i = 0;
  char no_address[5] = NO_ADDRESS;

  /* Other threads should not access the queues during status update */
  if(pthread_rwlock_rdlock(&rwlock) != 0)
  {
    logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
  }
  else
  {
    /* Add rows to history and current deployment */
    for(i = 0; i < conf.tot_num_cluster; i++)
    {
      if(add_cluster_hist_status(con, i, data.job_num[i], data.active[i], (data.server_address[i] ? data.server_address[i]: no_address)))
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to update historical status\n" );
        break;
      }  
    
      if(add_cluster_curr_status(con, i, data.job_num[i], data.active[i], (data.server_address[i] ? data.server_address[i]: no_address)))
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to update current status\n" );
        break;
      }  
    }

    /* Remove lock */
    if(pthread_rwlock_unlock(&rwlock) != 0){
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
    }
  }

  return AGENT_SUCCESS;
}

int _undeploy_cluster(configuration_struct *conf, char *infrastruct_url, short int *cluster_status )
{
	if(!conf || !infrastruct_url || !cluster_status )
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  short int deploy_status = 0;
  *cluster_status = COMMAND_DPSTATUS_CODE_ERROR;

  char command[COMMAND_LENGTH] = {'\0'};
  size_t length;

  size_t curr_size = JSON_BUFFER_SIZE;
  char *json_string = NULL;
  int res = 0;

  json_string = (char *)malloc(sizeof(char) * curr_size);
  if(json_string == NULL)
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Error allocating memory\n");
    return AGENT_ERROR_QUIT;
  }
  curr_size = JSON_BUFFER_SIZE;

  length = snprintf(command, COMMAND_LENGTH, COMMAND_UNDEPLOY_CLUSTER, conf->auth_header, infrastruct_url, conf->term_exec, conf->static_host, conf->term_port, conf->term_user, conf->term_pass);

  /* Run undeploy command and read output */
  if((res = run_cmd(&json_string, &curr_size, command)))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Failed to run undeploy command: %s\n", command);
    *cluster_status = COMMAND_DPSTATUS_CODE_ERROR;
    free(json_string);
    return res;
  }

  /* Get undeploy status */
  deploy_status = 0;

  if((res = get_undeploy_status(json_string, &deploy_status)))
  {
    logging(LOG_WARNING, __FILE__, __LINE__, "Error while checking undeploy status\n");
    *cluster_status = COMMAND_DPSTATUS_CODE_ERROR;
    free(json_string);
    return res;
  }

  if(json_string) free (json_string);
  json_string = NULL;

  *cluster_status = deploy_status;

  return AGENT_SUCCESS;
}

int _deploy_cluster(configuration_struct *conf, char **infrastruct_url, char **server_addr, short int *cluster_status )
{
	if(!conf || !infrastruct_url || !server_addr || !cluster_status )
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  char command[COMMAND_LENGTH] = {'\0'};
  size_t length;
  int res = 0;

  size_t curr_size = JSON_BUFFER_SIZE;
  char *json_string = NULL;

  struct timespec timer;
  timer.tv_sec = conf->check_cluster_timer;
  timer.tv_nsec = 0;
  struct timespec remainder;
  short int deploy_status = 0;

  *cluster_status = COMMAND_DPSTATUS_CODE_ERROR;

  json_string = (char *)malloc(sizeof(char) * curr_size);
  if(json_string == NULL)
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Error allocating memory\n");
    return AGENT_ERROR_QUIT;
  }
  curr_size = JSON_BUFFER_SIZE;

  length = snprintf(command, COMMAND_LENGTH, COMMAND_DEPLOY_CLUSTER, conf->auth_header, conf->infrastr_url, conf->term_exec, conf->static_host, conf->term_port, conf->term_user, conf->term_pass, conf->radl_file);

  /* Run deploy command and read output */
  if((res = run_cmd(&json_string, &curr_size, command)))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Failed to run deploy command: %s\n", command);
    *cluster_status = COMMAND_DPSTATUS_CODE_ERROR;
    free(json_string);
    return res;
  }

  /* Get infrastructure URL */
  char *tmp_infr_url = NULL;
  if ((res = get_infrastruct_url(json_string, &tmp_infr_url)))
  {
    logging(LOG_WARNING, __FILE__, __LINE__, "Failed to retrieve infrastructure URL\n");
    *cluster_status = COMMAND_DPSTATUS_CODE_ERROR;
    free(json_string);
    return res;
  }

  /* Check deploy status */
  length = snprintf(command, COMMAND_LENGTH, COMMAND_STATUS_CLUSTER, conf->auth_header, tmp_infr_url, conf->term_exec, conf->static_host, conf->term_port, conf->term_user, conf->term_pass);

  logging(LOG_DEBUG,__FILE__,__LINE__,"%s\n",command);

  /* Poll on deploy until it is completed */
  while(1)
  {
    /* Run check deploy cycle */
    if((res = run_cmd(&json_string, &curr_size, command)))
    {
  		logging(LOG_WARNING, __FILE__, __LINE__, "Failed to run check command: %s\n", command);
      free(tmp_infr_url);
      *cluster_status = COMMAND_DPSTATUS_CODE_ERROR;
      free(json_string);
      return res;
    }
 
    deploy_status = 0;

    if((res = get_deploy_status(json_string, &deploy_status)))
    {
      logging(LOG_WARNING, __FILE__, __LINE__, "Error while checking deploy status\n");
      free(tmp_infr_url);
      *cluster_status = COMMAND_DPSTATUS_CODE_ERROR;
      free(json_string);
      return res;
    }
    if(deploy_status == COMMAND_DPSTATUS_CODE_FINISHED || deploy_status == COMMAND_DPSTATUS_CODE_ERROR || deploy_status == COMMAND_DPSTATUS_CODE_EMPTY )
      break;
    else
      nanosleep(&timer, &remainder);

  }
  *cluster_status = deploy_status;

  char *server_ip = NULL;

  if(deploy_status == COMMAND_DPSTATUS_CODE_FINISHED)
  {
    /* Command executed correctly */

    /* Get deploy address */
    length = snprintf(command, COMMAND_LENGTH, COMMAND_ADDRESS_CLUSTER, conf->auth_header, tmp_infr_url, conf->term_exec, conf->static_host, conf->term_port, conf->term_user, conf->term_pass);
	logging(LOG_DEBUG,__FILE__,__LINE__,"%s\n",command);


    /* Read server address */
   if((res = run_cmd(&json_string, &curr_size, command)))
    {
  		logging(LOG_WARNING, __FILE__, __LINE__, "Failed to run output command: %s\n", command);
      free(tmp_infr_url);
      free(json_string);
      _undeploy_cluster(conf, tmp_infr_url, cluster_status);
      return res;
    }

    if((res = get_server_address(json_string, &server_ip)))
    {
      logging(LOG_WARNING, __FILE__, __LINE__, "Error while retrieving server IP address\n");
      free(server_ip);
      free(tmp_infr_url);
      _undeploy_cluster(conf, tmp_infr_url, cluster_status);
      return res;
    }

	  logging(LOG_DEBUG,__FILE__,__LINE__,"%s\n",server_ip);
  }

  if(json_string) free (json_string);
  json_string = NULL;

  *server_addr = server_ip;
  *infrastruct_url = tmp_infr_url;

  return AGENT_SUCCESS;
}

int _safe_service_thread_end(connection_struct *con, const char *thread_name, unsigned int tid)
{
  logging(LOG_DEBUG,__FILE__,__LINE__,"CLOSING %s-SERVICE-THREAD...%d\n", thread_name, tid);

  if(pthread_rwlock_wrlock(&quit_lock) != 0)
  {
    logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
  }
  else
  {
    agent_quit = 1;
  	logging(LOG_DEBUG,__FILE__,__LINE__,"Setting quit flag to: %d on SERVICE-THREAD %d\n", agent_quit, tid);
    if(pthread_rwlock_unlock(&quit_lock) != 0){
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
    }
  }
  if(con) free_connection(con);        

  pthread_exit((void*)&tid);
}

void *delete_storage_manager(void *arg)
{
  unsigned int tid = *((unsigned int *) arg);

  logging(LOG_DEBUG,__FILE__,__LINE__,"LAUNCHING DELETE-STORAGE THREAD...%d\n", tid);
 
  struct timespec timer;
  timer.tv_sec = conf.check_exp_timer;
  timer.tv_nsec = 0;
  struct timespec remainder;

  connection_struct thread_con;

  //Setup connection structure
  if(init_connection(&thread_con,&conf))
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to setup connection structure\n" );
    _safe_service_thread_end(&thread_con, "DELETE", tid);
  }

  long long *expids = NULL;
  short *expchstatus = NULL;
  char **submissiondate = NULL;
  int n = 0;
  long long wait = conf.check_delete_timer;

  while(1)
  {
    /* Reset state */
    expids = NULL;
    expchstatus = NULL;
    submissiondate = NULL;
    n = 0;
    wait = conf.check_delete_timer;

    /* Check for thread termination */
    if(pthread_rwlock_rdlock(&quit_lock) != 0)
    {
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
    }
    else
    {
      if(agent_quit == 1)
      {
        if(pthread_rwlock_unlock(&quit_lock) != 0)
        {
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
        }
        _safe_service_thread_end(&thread_con, "DELETE",  tid);
      }
      if(pthread_rwlock_unlock(&quit_lock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
      }
    }

    if(get_old_exp_list(&thread_con, &expids, &expchstatus, &submissiondate))
    {
      logging(LOG_ERROR, __FILE__, __LINE__, "Unable to get old experiments to purge\n" );
      _safe_service_thread_end(&thread_con, "DELETE",  tid);
    }

    if((!expids || !expchstatus || !submissiondate))
    {
      logging(LOG_DEBUG,__FILE__,__LINE__,"No old experiment to be deleted found! Retry\n");
    }
    else
    {
      /* Loop on experiments*/
      while(expids[n])
      {
        if(delete_experiment(&conf, &thread_con, expids[n], expchstatus[n], submissiondate[n]))
        {
		      logging(LOG_WARNING, __FILE__, __LINE__, "Error while deleting experiment files\n" );
          free(expids);
          free(expchstatus);
          n = 0;
          while(submissiondate[n]) free(submissiondate[n++]);
          free(submissiondate);
          _safe_service_thread_end(&thread_con, "DELETE",  tid);  
        }
        n++;
      }
      free(expids);
      free(expchstatus);
      n = 0;
      while(submissiondate[n]) free(submissiondate[n++]);
      free(submissiondate);
    }

    /* Wait for next loop */
    while(wait > 0){
      nanosleep(&timer, &remainder);
      wait -= conf.check_exp_timer;
      /* Check for thread termination */
      if(pthread_rwlock_rdlock(&quit_lock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
      }
      else
      {
        if(agent_quit == 1)
        {
          if(pthread_rwlock_unlock(&quit_lock) != 0)
          {
            logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
          }
          _safe_service_thread_end(&thread_con,  "DELETE", tid);
        }
        if(pthread_rwlock_unlock(&quit_lock) != 0)
        {
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
        }
      }
    }
  }

  _safe_service_thread_end(&thread_con,  "DELETE", tid);
}

void *clearing_house_manager(void *arg)
{
  unsigned int tid = *((unsigned int *) arg);

  logging(LOG_DEBUG,__FILE__,__LINE__,"LAUNCHING CLEARING-HOUSE THREAD...%d\n", tid);
 
  struct timespec timer;
  timer.tv_sec = conf.check_exp_timer;
  timer.tv_nsec = 0;
  struct timespec remainder;

  connection_struct thread_con;

  /* Setup connection structure */
  if(init_connection(&thread_con,&conf))
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to setup connection structure\n" );
    _safe_service_thread_end(&thread_con, "CLEARING-HOUSE", tid);
  }

  long long *expids = NULL;
  int *expsizes = NULL;
  int n = 0;
  short int exp_status = 0;
  char **submissiondate = NULL;
    
  while(1)
  {
    /* Reset state */
    expids = NULL;
    expsizes = NULL;
    n = 0;
    exp_status = 0;
    submissiondate = NULL;

    /* Check for thread termination */
    if(pthread_rwlock_rdlock(&quit_lock) != 0)
    {
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
    }
    else
    {
      if(agent_quit == 1)
      {
        if(pthread_rwlock_unlock(&quit_lock) != 0)
        {
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
        }
        _safe_service_thread_end(&thread_con, "CLEARING-HOUSE", tid);
      }
      if(pthread_rwlock_unlock(&quit_lock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
      }
    }

    /* Read list of experiments to be stored in CH */ 
    if(get_new_ch_exp_list(&thread_con, &expids, &submissiondate))
    {
      logging(LOG_ERROR, __FILE__, __LINE__, "Unable to get experiments to store into the clearing house system\n" );
      _safe_service_thread_end(&thread_con, "CLEARING-HOUSE", tid);
    }

    if(!expids || !submissiondate )
    {
      logging(LOG_DEBUG,__FILE__,__LINE__,"No experiment to store in the Clearing House found! Retry\n");
    }
    else
    {
      /* Loop on experiments*/
      while(expids[n])
      {
        /* Check for thread termination */
        if(pthread_rwlock_rdlock(&quit_lock) != 0)
        {
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
        }
        else
        {
          if(agent_quit == 1)
          {
            if(pthread_rwlock_unlock(&quit_lock) != 0)
            {
              logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
            }
            free(expids);
            n = 0;
            while(submissiondate[n]) free(submissiondate[n++]);
            free(submissiondate);
            _safe_service_thread_end(&thread_con, "CLEARING-HOUSE", tid);
          }
          if(pthread_rwlock_unlock(&quit_lock) != 0)
          {
            logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
          }
        }

        if(store_experiment(&conf, &thread_con, submissiondate[n], expids[n], &exp_status))
        {
		      logging(LOG_WARNING, __FILE__, __LINE__, "Error while storing experiment\n" );
          if(expids) free(expids);
          if(submissiondate) free(submissiondate);
          free(expids);
          n = 0;
          while(submissiondate[n]) free(submissiondate[n++]);
          free(submissiondate);
          _safe_service_thread_end(&thread_con, "CLEARING-HOUSE", tid);
        }
        else
        {
          if(exp_status == COMMAND_CHSTATUS_CODE_COMPLETED) 
        	    logging(LOG_DEBUG,__FILE__,__LINE__,"Experiment stored correctly\n");
          else
        	    logging(LOG_DEBUG,__FILE__,__LINE__,"Failure while storing experiment\n");
        } 
        n++;
      }
      free(expids);
      n = 0;
      while(submissiondate[n]) free(submissiondate[n++]);
      free(submissiondate);
    }

    /* Wait for next loop */
    nanosleep(&timer, &remainder);
  }

  _safe_service_thread_end(&thread_con, "CLEARING-HOUSE", tid);
}

int _safe_cluster_thread_end(connection_struct *con, unsigned int tid, pthread_t *thread_exp, pthread_attr_t *attr)
{
  logging(LOG_DEBUG,__FILE__,__LINE__,"CLOSING CLUSTER-THREAD...%d\n", tid);

  char *current_infr_url = NULL;
  char *current_serv_addr = NULL;
  short int is_static = 0;
  short int cluster_status = 0;
  long long i = 0;
  long long num_threads = 0;

  if(pthread_rwlock_wrlock(&quit_lock) != 0)
  {
    logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
  }
  else
  {
    agent_quit = 1;
	logging(LOG_DEBUG,__FILE__,__LINE__,"Setting quit flag to: %d on CLUSTER-THREAD %d\n", agent_quit, tid);
    if(pthread_rwlock_unlock(&quit_lock) != 0){
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
    }
  }

  /* Read cluster info */
  if(pthread_rwlock_rdlock(&rwlock) != 0)
  {
    logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
  }
  else
  {
    is_static = data.static_flag[tid];

    if(data.active[tid] == 1)
    {
      if(data.infrastruct_url[tid]){
        current_infr_url = (char *)strdup(data.infrastruct_url[tid]);
        if(current_infr_url == NULL)
        {
          logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for new queue\n" );
        }
      }
      if(data.server_address[tid]){
        current_serv_addr = (char *)strdup(data.server_address[tid]);
        if(current_serv_addr == NULL)
        {
          logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for new queue\n" );
        }
      }
    }

    if(pthread_rwlock_unlock(&rwlock) != 0){
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
    }
  }

  /* Wait for internal threads end */
  unsigned int *status;
  int rc = 0;

  if(thread_exp)
  {
    pthread_attr_destroy(attr);
    num_threads = ( is_static == 1 ? conf.static_num_cores : conf.cloud_num_cores );
    for(i = 0; i < num_threads; i++) {
      rc = pthread_join(thread_exp[i], (void **)&status);
      if (rc) {
		    logging(LOG_ERROR, __FILE__, __LINE__, "Error while joining experiment-threads: %d\n", rc );
      }
      logging(LOG_DEBUG,__FILE__,__LINE__,"Cluster Thread %d: completed join with experiment-thread %d\n",tid, i);
    }
  }

  if(current_infr_url && current_serv_addr)
  {
    /* Static node doesn't need undeployment */
    if(is_static == 1)
    {
      logging(LOG_DEBUG,__FILE__,__LINE__,"Deactivating static cluster: %d\n", tid);
    }
    else
    {
      logging(LOG_DEBUG,__FILE__,__LINE__,"Started undeploy of cluster through IM: %d\n", tid);
      if(_undeploy_cluster(&conf, current_infr_url,&cluster_status ) || cluster_status != COMMAND_DPSTATUS_CODE_FINISHED)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to undeploy cluster\n" );
      }
    }

    if(pthread_rwlock_wrlock(&rwlock) != 0)
    {
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
    }
    else
    {
      data.active[tid] = 0;
      free(data.infrastruct_url[tid]);
      data.infrastruct_url[tid] = NULL;
      free(data.server_address[tid]);
      data.server_address[tid] = NULL;

      if(pthread_rwlock_unlock(&rwlock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
      }
    }
  }

  if(current_infr_url) free(current_infr_url);
  if(current_serv_addr) free(current_serv_addr);
  if(con) free_connection(con);        

  pthread_exit((void*)&tid);
}

int _update_deployment(connection_struct *con, unsigned int tid, short int is_static, short int *undeploy_counter, short int *first_deploy, short int *run, char **current_serv_addr)
{
	if(!con || !undeploy_counter || !first_deploy  || !run || !current_serv_addr )
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  short int start_deploy = 0;
  short int start_undeploy = 0;
  short int cluster_status = 0;
  char *tmp_infr_url = NULL;
  char *tmp_serv_addr = NULL;

  if(pthread_rwlock_rdlock(&quit_lock) != 0)
  {
    logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
  }
  else
  {
    /* Check for thread termination */
    if(agent_quit == 1)
    {
      if(pthread_rwlock_unlock(&quit_lock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
      }
      logging(LOG_DEBUG, __FILE__, __LINE__, "Caught program termination signal!\n" );
      return AGENT_ERROR_THREAD;
    }

    if(pthread_rwlock_unlock(&quit_lock) != 0)
    {
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
    }
  }

  if(pthread_rwlock_rdlock(&rwlock) != 0)
  {
    logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
  }
  else
  {
    /* Check for jobs and deploy/undeploy event */
    if(data.job_num[tid] > 0)
    {
      *undeploy_counter = 0;
      if( data.active[tid] == 0 )
      {
        /* Activate deploy  */
        start_deploy = 1;
      }
      else
      {
        /* Read current IP address */
        *current_serv_addr = (char *)strdup(data.server_address[tid]);
        if(*current_serv_addr == NULL)
        {
          logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for new queue\n" );
          if(pthread_rwlock_unlock(&rwlock) != 0)
          {
            logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
          }
          return AGENT_ERROR_QUIT;
        }

      }
      *run = 1;
    }
    else
    {
      if( data.active[tid] == 1 )
      {
        (*undeploy_counter)++;
        if(*undeploy_counter >= conf.cloud_undeploy_cycles )
        {
          /* Activate Undeploy */
          start_undeploy = 1;
          /* Read current IP address */
          *current_serv_addr = (char *)strdup(data.server_address[tid]);
          tmp_infr_url = (char *)strdup(data.infrastruct_url[tid]);
          if(*current_serv_addr == NULL || tmp_infr_url == NULL)
          {
            logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for new queue\n" );
            if(pthread_rwlock_unlock(&rwlock) != 0)
            {
              logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
            }
            if(tmp_infr_url) free(tmp_infr_url);
            return AGENT_ERROR_QUIT;
          }
        }
      }        
      *run = 0;
    }

    /* Check for queue and job_num consistency */
    if( !((data.job_num[tid] > 0 && data.queue[tid]->num_elements > 0) || (data.job_num[tid] <=0 && data.queue[tid]->num_elements <= 0)))
    {
      if(pthread_rwlock_unlock(&rwlock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
      }
      if(tmp_infr_url) free(tmp_infr_url);
      logging(LOG_ERROR, __FILE__, __LINE__, "Queue of process %d is corrupted\n", tid);
      return AGENT_ERROR_THREAD;
    }
	  logging(LOG_DEBUG,__FILE__,__LINE__,"Jobs assigned to %d: %d\n", tid, data.job_num[tid]);

    if(pthread_rwlock_unlock(&rwlock) != 0)
    {
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
    }
  }

  if( is_static == 0 )
  {
    if(tid != 0)
    {
      /* If deploy/undpeloy of cluster */
      if(start_deploy || start_undeploy)
      {
        if(start_deploy)
        {
  	      logging(LOG_DEBUG,__FILE__,__LINE__,"Started deploy of cluster through IM: %d\n", tid);
          if(_deploy_cluster(&conf, &tmp_infr_url, &tmp_serv_addr, &cluster_status ) || cluster_status != COMMAND_DPSTATUS_CODE_FINISHED)
          {
            logging(LOG_ERROR, __FILE__, __LINE__, "Unable to deploy cluster\n" );
            return AGENT_ERROR_QUIT;
          }

          if(pthread_rwlock_wrlock(&rwlock) != 0)
          {
            logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
          }
          else
          {
            data.active[tid] = 1;
            data.infrastruct_url[tid] = tmp_infr_url;
            data.server_address[tid] = tmp_serv_addr;
            *current_serv_addr = (char *)strdup(data.server_address[tid]);
            if(pthread_rwlock_unlock(&rwlock) != 0)
            {
              logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
            }
          }
        }
        else
        {
	        logging(LOG_DEBUG,__FILE__,__LINE__,"Started undeploy of cluster  through IM: %d\n", tid);
          if(_undeploy_cluster(&conf, tmp_infr_url, &cluster_status ) || cluster_status != COMMAND_DPSTATUS_CODE_FINISHED)
          {
            logging(LOG_ERROR, __FILE__, __LINE__, "Unable to undeploy cluster\n" );
          }
          if(tmp_infr_url) free(tmp_infr_url);
          tmp_infr_url = NULL;

          if(pthread_rwlock_wrlock(&rwlock) != 0)
          {
            logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
          }
          else
          {
            data.active[tid] = 0;
            free(data.infrastruct_url[tid]);
            data.infrastruct_url[tid] = NULL;
            free(data.server_address[tid]);
            data.server_address[tid] = NULL;
            if(pthread_rwlock_unlock(&rwlock) != 0)
            {
              logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
            }
          }
        }
      }
    }
    else
    {
      if(tmp_infr_url) free(tmp_infr_url);
      tmp_infr_url = NULL;

      /* First system uses a less dynamic setup */
      if(!*first_deploy)
      {
        logging(LOG_DEBUG,__FILE__,__LINE__,"Started deploy of cluster through IM: %d\n", tid);
        if(_deploy_cluster(&conf, &tmp_infr_url, &tmp_serv_addr, &cluster_status ) || cluster_status != COMMAND_DPSTATUS_CODE_FINISHED)
        {
          logging(LOG_ERROR, __FILE__, __LINE__, "Unable to deploy cluster\n" );
          return AGENT_ERROR_QUIT;
        }

        *first_deploy = 1;

        if(pthread_rwlock_wrlock(&rwlock) != 0)
        {
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
        }
        else
        {
          data.active[tid] = 1;
          data.infrastruct_url[tid] = tmp_infr_url;
          data.server_address[tid] = tmp_serv_addr;
          *current_serv_addr = (char *)strdup(data.server_address[tid]);
          if(pthread_rwlock_unlock(&rwlock) != 0)
          {
            logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
          }
        }
      }
    }
  }
  else
  {
    if(tmp_infr_url) free(tmp_infr_url);
    tmp_infr_url = NULL;

    /* Static cluster */
    if(!*first_deploy)
    {
      logging(LOG_DEBUG,__FILE__,__LINE__,"Enabled static cluster: %d\n", tid);
      cluster_status = COMMAND_DPSTATUS_CODE_FINISHED;
      tmp_infr_url = strdup(NO_ADDRESS);
      tmp_serv_addr = strdup(conf.static_host);
      *first_deploy = 1;

      if(pthread_rwlock_wrlock(&rwlock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
      }
      else
      {
        data.active[tid] = 1;
        data.infrastruct_url[tid] = tmp_infr_url;
        data.server_address[tid] = tmp_serv_addr;
        *current_serv_addr = (char *)strdup(data.server_address[tid]);
        if(pthread_rwlock_unlock(&rwlock) != 0)
        {
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
        }
      }
    }
  }

  return AGENT_SUCCESS;
}

int _safe_exp_thread_end(connection_struct *con, _sub_tid *stid)
{
  logging(LOG_DEBUG,__FILE__,__LINE__,"CLOSING EXPERIMENT-THREAD...%d UNDER CLUSTER-THREAD... %d\n", stid->exp_tid, stid->cl_tid);

  if(pthread_rwlock_wrlock(&quit_lock) != 0)
  {
    logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
  }
  else
  {
    agent_quit = 1;
  	logging(LOG_DEBUG,__FILE__,__LINE__,"Setting quit flag to: %d on EXPERIMENT-THREAD...%d UNDER CLUSTER-THREAD... %d\n", agent_quit, stid->exp_tid, stid->cl_tid);
    if(pthread_rwlock_unlock(&quit_lock) != 0){
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
    }
  }
  if(con) free_connection(con);        

  pthread_exit((void*)&(stid->exp_tid));
}

void *experiment_manager(void *arg)
{
  _sub_tid *stid = ((_sub_tid *) arg); 

  logging(LOG_DEBUG,__FILE__,__LINE__,"LAUNCHING EXPERIMENT-THREAD...%d UNDER CLUSTER-THREAD... %d\n", stid->exp_tid, stid->cl_tid);
 
  struct timespec timer;
  timer.tv_sec = conf.check_exp_timer;
  timer.tv_nsec = 0;
  struct timespec remainder;

  short int exp_status = 0;
  long long curr_expid = 0;
  int job_number = 0;
  int n = 0;

  char *current_serv_addr = NULL;

  long long stop_index = 0;
  long long i = 0;

  short int is_static = 0;

  connection_struct thread_con;

  /* Setup connection structure */
  if(init_connection(&thread_con,&conf))
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to setup connection structure\n" );
    _safe_exp_thread_end(&thread_con, stid);
  }

  /* Compute index related to stop flag and check if cluster is static */
  if(pthread_rwlock_rdlock(&rwlock) != 0)
  {
    logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
  }
  else
  {
    for(i = 0; i < conf.tot_num_cluster; i++)
    {
      /* Check until current cluster */
      if(i >= stid->cl_tid) break;
      stop_index += ( data.static_flag[i] == 1 ? conf.static_num_cores : conf.cloud_num_cores);
    }    
    stop_index += stid->exp_tid; 

    is_static = data.static_flag[stid->cl_tid];

    /* Remove lock */
    if(pthread_rwlock_unlock(&rwlock) != 0){
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
    }
  }


  /* Loop on experiment queue */
  while(1)
  {
    /* Check for thread termination */
    if(pthread_rwlock_rdlock(&quit_lock) != 0)
    {
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
    }
    else
    {
      if(agent_quit == 1)
      {
        if(pthread_rwlock_unlock(&quit_lock) != 0)
        {
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
        }
        logging(LOG_DEBUG, __FILE__, __LINE__, "Caught program termination signal!\n" );
        _safe_exp_thread_end(&thread_con, stid);
      }

      /* Remove lock */
      if(pthread_rwlock_unlock(&quit_lock) != 0){
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
      }
    }

    /* Get new experiment id from thread queue */
    if(pthread_rwlock_wrlock(&rwlock) != 0)
    {
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
    }
    else
    {
      /* Check for thread experiment start */
      if(data.exec_start[stid->cl_tid] == 1)
      {
        dequeue((data.queue[stid->cl_tid]), &curr_expid);
        /* If experiment is available gather ip address otherwise update stop counter*/        
        if(curr_expid)
        {
          current_serv_addr = (char *)strdup(data.server_address[stid->cl_tid]);
          data.exec_stop[stop_index] = 0;
          logging(LOG_DEBUG,__FILE__,__LINE__,"Running experiment %lld on thread %d-%d\n", curr_expid, stid->cl_tid, stid->exp_tid);
        }
        else
        {
          data.exec_stop[stop_index] = 1;
          data.exec_start[stid->cl_tid] = 0;
          for(n = 0; n < ( is_static == 1 ? conf.static_num_cores : conf.cloud_num_cores ); n++)
          {
            if(data.exec_stop[(stop_index - stid->exp_tid) + n] == 0)
            {
              data.exec_start[stid->cl_tid] = 1;
              break;          
            }
          }
          logging(LOG_DEBUG,__FILE__,__LINE__,"Nothing to run on thread %d-%d\n", stid->cl_tid, stid->exp_tid);
        }
      }

      /* Remove lock */
      if(pthread_rwlock_unlock(&rwlock) != 0){
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
      }
    }

    /* If new experiment is found */
    if(curr_expid)
    {
      if(exec_experiment(&conf, &thread_con, current_serv_addr, curr_expid, &exp_status, &job_number))
      {
        logging(LOG_WARNING, __FILE__, __LINE__, "Error while executing experiment\n" );
        free(current_serv_addr);
        _safe_exp_thread_end(&thread_con, stid);
      }
      else
      {
        if(exp_status == COMMAND_STATUS_CODE_COMPLETED) 
              logging(LOG_DEBUG,__FILE__,__LINE__,"Experiment completed correctly\n");
        else
              logging(LOG_DEBUG,__FILE__,__LINE__,"Failure while executing experiment\n");
      }    

      /* Update job count in global struct */
      if(pthread_rwlock_wrlock(&rwlock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
      }
      else
      {
        data.job_num[stid->cl_tid] -= job_number;
        logging(LOG_DEBUG,__FILE__,__LINE__,"Removing %d jobs on thread %d\n", job_number, stid->cl_tid);
        /* Remove lock */
        if(pthread_rwlock_unlock(&rwlock) != 0){
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
        }
      }
    }

    exp_status = 0;
    job_number = 0;
    curr_expid = 0;
    if(current_serv_addr) free(current_serv_addr);
    current_serv_addr = NULL;

    /* Wait for next loop */
    nanosleep(&timer, &remainder);
  }

  _safe_exp_thread_end(&thread_con, stid);
}

void *cluster_manager(void *arg)
{
  unsigned int tid = *((unsigned int *) arg);

  logging(LOG_DEBUG,__FILE__,__LINE__,"LAUNCHING CLUSTER-THREAD...%d\n", tid);
 
  struct timespec timer;
  timer.tv_sec = conf.check_exp_timer;
  timer.tv_nsec = 0;
  struct timespec remainder;
  struct timespec internal_timer;
  internal_timer.tv_sec = conf.check_exp_timer;
  internal_timer.tv_nsec = 0;
  struct timespec internal_remainder;

  short int run = 1;
  short int undeploy_counter = 0;

  int n = 0;

  char *current_serv_addr = NULL;

  connection_struct thread_con;

  short int first_deploy = 0;

  /* Setup connection structure */
  if(init_connection(&thread_con,&conf))
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to setup connection structure\n" );
    _safe_cluster_thread_end(&thread_con, tid, NULL, NULL);
  }

  /* Set first cluster as static, if enabled */
  short int is_static = 0;
  if(conf.static_deploy == 1 && tid == 0)
  {
    is_static = 1;
  }

  if(pthread_rwlock_wrlock(&rwlock) != 0)
  {
    logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
  }
  else
  {
    data.static_flag[tid] = is_static;

    /* Remove lock */
    if(pthread_rwlock_unlock(&rwlock) != 0){
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
    }
  }

  int rc = 0;
  /* Initialize and set thread detached attribute */
  pthread_t thread_exp[MAX_NUM_CORE];
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);

  _sub_tid stid[MAX_NUM_CORE]; 

  /* Spawn experiment threads */
  for(n=0; n < (is_static == 1 ? conf.static_num_cores : conf.cloud_num_cores); n++) 
  {
    logging(LOG_DEBUG,__FILE__,__LINE__,"CLUSTER-THREAD %d: creating experiment-thread %d\n", tid, n);
    /* Manage sub-thread id */
    stid[n].cl_tid = tid;
    stid[n].exp_tid = n;
    rc = pthread_create(&thread_exp[n], &thread_attr, experiment_manager, (void *)&(stid[n])); 
    if (rc) {
		  logging(LOG_ERROR, __FILE__, __LINE__, "Error while spawning experiment-threads: %d\n", rc );
      if(pthread_rwlock_wrlock(&quit_lock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
      }
      else
      {
        agent_quit = 1;
        if(pthread_rwlock_unlock(&quit_lock) != 0){
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
        }
      }
      _safe_cluster_thread_end(&thread_con, tid, thread_exp, &thread_attr);
    }
  }

  
  while(1)
  {
    /* Reset state */
    run = 0;

    /* Check and update deployment/undeployment */
    if(_update_deployment(&thread_con, tid, is_static, &undeploy_counter, &first_deploy, &run, &current_serv_addr))
    {
      logging(LOG_ERROR, __FILE__, __LINE__, "Unable to deploy/undeploy cluster\n" );
      if(current_serv_addr) free(current_serv_addr);
      _safe_cluster_thread_end(&thread_con, tid, thread_exp, &thread_attr);
    }  

    /* Enable experiment threads execution */
    if(run)
    {
      /* Check for thread termination */
      if(pthread_rwlock_rdlock(&quit_lock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
      }
      else
      {
        if(agent_quit == 1)
        {
          if(pthread_rwlock_unlock(&quit_lock) != 0)
          {
            logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
          }
          logging(LOG_DEBUG, __FILE__, __LINE__, "Caught program termination signal!\n" );
          free(current_serv_addr);
          _safe_cluster_thread_end(&thread_con, tid, thread_exp, &thread_attr);
        }
        if(pthread_rwlock_unlock(&quit_lock) != 0){
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
        }
      }

      /* Update experiment execution flag */
      if(pthread_rwlock_wrlock(&rwlock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
      }
      else
      {
        data.exec_start[tid] = 1;        
        logging(LOG_DEBUG,__FILE__,__LINE__,"Enabled execution on cluster thread %d\n", tid);
        /* Remove lock */
        if(pthread_rwlock_unlock(&rwlock) != 0){
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
        }
      }

      /* Wait for experiment stop */
      while(1)
      {
        /* Check experiment execution flag */
        if(pthread_rwlock_rdlock(&quit_lock) != 0)
        {
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
        }
        else
        {
          /* Check for thread termination */
          if(agent_quit == 1)
          {
            if(pthread_rwlock_unlock(&quit_lock) != 0)
            {
              logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
            }
            logging(LOG_DEBUG, __FILE__, __LINE__, "Caught program termination signal!\n" );
            free(current_serv_addr);
            _safe_cluster_thread_end(&thread_con, tid, thread_exp, &thread_attr);
          }

          if(pthread_rwlock_unlock(&quit_lock) != 0){
            logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
          }
        }

        /* Update experiment execution flag */
        if(pthread_rwlock_rdlock(&rwlock) != 0)
        {
          logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
        }
        else
        {
          /* Check for thread experiment stop */
          if(data.exec_start[tid] == 0)
          {
            if(pthread_rwlock_unlock(&rwlock) != 0)
            {
              logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
            }
            logging(LOG_DEBUG, __FILE__, __LINE__, "Experiment thread execution on cluster thread %d has stopped\n", tid);
            break;
          }
          logging(LOG_DEBUG,__FILE__,__LINE__,"Experiment thread execution on cluster thread %d goes on\n", tid);
          /* Remove lock */
          if(pthread_rwlock_unlock(&rwlock) != 0){
            logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
          }
        }
        /* Wait for next loop */
        nanosleep(&internal_timer, &internal_remainder);
      }
    }

    if(current_serv_addr) free(current_serv_addr);
    current_serv_addr = NULL;

    /* Wait for next loop */
    nanosleep(&timer, &remainder);
  }

  _safe_cluster_thread_end(&thread_con, tid, thread_exp, &thread_attr);
}
