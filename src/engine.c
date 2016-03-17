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
#include "agent_thread.h"
#include "agent_routines.h"
#include "agent_db.h"
#include <time.h>

#include <signal.h>
#include <pthread.h>

#ifdef ENABLE_DEBUG
int msglevel = LOG_DEBUG_T;
#else
int msglevel = LOG_WARNING_T;
#endif

/* Global structures to be freed with signal */
configuration_struct conf;
connection_struct con;

/* Global variables for thread handling */
pthread_rwlock_t       rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t       quit_lock = PTHREAD_RWLOCK_INITIALIZER;
pthread_t thread_cluster[MAX_NUM_CLUSTER];
pthread_t thread_service[2];
pthread_attr_t attr;

cluster_data data;

/* global flag to coordinate thread stop */
char agent_quit = 0; 

int catch_signal(int signo, void *func)
{
    /* Adapted from Stevens UNP Vol. 1, fig. 5.6 */

    struct sigaction	new_act, old_act;

    new_act.sa_handler = func;
    sigemptyset(&new_act.sa_mask);
    new_act.sa_flags = 0;

    if (signo == SIGALRM) {
    #ifdef	SA_INTERRUPT
        new_act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
    #endif
    } else {
    #ifdef	SA_RESTART
        new_act.sa_flags |= SA_RESTART;	 /* SVR4, 44BSD */
    #endif
    }

    int res;
    if( (res = sigaction(signo, &new_act, &old_act)) < 0){
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to setup signal\n");
    }
    return res;
}

int main ()
{
	int i;

  char log_prefix[CONFIGURATION_LINE];
  snprintf(log_prefix,CONFIGURATION_LINE, AGENT_LOCATION);
  set_log_prefix(log_prefix);

	logging(LOG_INFO, __FILE__, __LINE__, "***STARTING ENGINE***\n" );

  /* Initialize and set thread detached attribute */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  /* Setup configuration struct */
  init_configuration(&conf);
  init_db_lib();

  if( read_config_file(&conf) )
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to read config file\n" );
    free_configuration(&conf);
    end_db_lib();
    return -1;
  }

  /* Setup connection structure */
  if(init_connection(&con,&conf))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to setup connection structure\n" );
    free_configuration(&conf);
    end_db_lib();
    return -1;
  }

  /* Setup global thread data */
  if(init_cluster_data(&data))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to setup thread structure\n" );
    free_configuration(&conf);
    free_connection(&con);
    end_db_lib();
    return -1;
  }

	void release(int);
  unsigned int *status;

  struct timespec timer;
  timer.tv_sec = conf.check_exp_timer;
  timer.tv_nsec = 0;
  struct timespec remainder;

  long long *expids = NULL;
  int *expsizes = NULL;
  int n = 0;
  unsigned int cl_tid[MAX_NUM_CLUSTER] = {0};
  unsigned int sr_tid[2] = {0,1};

  int rc = 0;
  /* Spawn cluster threads */
  for(n=0; n<conf.tot_num_cluster; n++) 
  {
    logging(LOG_DEBUG,__FILE__,__LINE__,"Main: creating cluster-thread %d\n", n);
    cl_tid[n] = n;
    rc = pthread_create(&thread_cluster[n], &attr, cluster_manager, (void *)&(cl_tid[n])); 
    if (rc) {
		  logging(LOG_ERROR, __FILE__, __LINE__, "Error while spawning cluster-threads: %d\n", rc );
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
      goto __CLEANUP_PROC;
    }
  }

  /* Spawn service threads */
  logging(LOG_DEBUG,__FILE__,__LINE__,"Main: creating service-thread %d\n", sr_tid[0]);
  rc = pthread_create(&thread_service[0], &attr, clearing_house_manager, (void *)&(sr_tid[0])); 
  if (rc) {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Error while spawning service-threads: %d\n", rc );
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
    goto __CLEANUP_PROC;
  }
  logging(LOG_DEBUG,__FILE__,__LINE__,"Main: creating service-thread %d\n", sr_tid[1]);
  rc = pthread_create(&thread_service[1], &attr, delete_storage_manager, (void *)&(sr_tid[1])); 
  if (rc) {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Error while spawning service-threads: %d\n", rc );
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
    goto __CLEANUP_PROC;
  }

  /* To avoid signal catching by only the master thread, add signal function after spawning */
  if(catch_signal(SIGINT, release) || catch_signal(SIGABRT, release) || catch_signal(SIGQUIT, release))
 {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Error while setting signal handlers\n");
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
    goto __CLEANUP_PROC;
  }


  while(1)
  {
    /* Check for system termination */
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
	      logging(LOG_DEBUG,__FILE__,__LINE__,"CLOSING MASTER...%d\n", -1);
        break;
      }
      if(pthread_rwlock_unlock(&quit_lock) != 0)
      {
        logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
      }
    }

    /* Run execution routine */
    expids = NULL;
    expsizes = NULL;

    /* Read list of experiments pending */ 
    if(get_new_exp_list(&con, &expids, &expsizes))
    {
		  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to get new experiments\n" );
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
      logging(LOG_DEBUG,__FILE__,__LINE__,"CLOSING MASTER...%d\n", -1);
      break;
    }

    if(!expids || !expsizes)
    {
  	    logging(LOG_DEBUG,__FILE__,__LINE__,"No new experiment found! Retry\n");
    }
    else
    {
      /* Assign experiments to thread queues */
      if(assign_experiments(&con, expids, expsizes))
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to update experiments\n" );
        free(expids);
        free(expsizes);
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
	      logging(LOG_DEBUG,__FILE__,__LINE__,"CLOSING MASTER...%d\n", -1);
        break;
      }  
      free(expids);
      free(expsizes);
    }

    /* RUn status update routine */
    if(update_cluster_status(&con))
    {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to update cluster status\n" );
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
	      logging(LOG_DEBUG,__FILE__,__LINE__,"CLOSING MASTER...%d\n", -1);
        break;
    }

    nanosleep(&timer, &remainder);
  }

__CLEANUP_PROC:
  /* Free attribute and wait for the other threads */
  pthread_attr_destroy(&attr);
  for(i = 0; i < conf.tot_num_cluster; i++) {
    rc = pthread_join(thread_cluster[i], (void **)&status);
    if (rc) {
		  logging(LOG_ERROR, __FILE__, __LINE__, "Error while joining cluster-threads: %d\n", rc );
    }
    logging(LOG_DEBUG,__FILE__,__LINE__,"Main: completed join with cluster-thread %d\n",i);
  }
  rc = pthread_join(thread_service[0], (void **)&status);
  if (rc) {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Error while joining service-threads: %d\n", rc );
  }
  logging(LOG_DEBUG,__FILE__,__LINE__,"Main: completed join with service-thread %d\n",0);
  rc = pthread_join(thread_service[1], (void **)&status);
  if (rc) {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Error while joining service-threads: %d\n", rc );
  }
  logging(LOG_DEBUG,__FILE__,__LINE__,"Main: completed join with service-thread %d\n",1);
  pthread_rwlock_destroy(&rwlock);
  pthread_rwlock_destroy(&quit_lock);

  /* Cleanup procedures */
  delete_transient(&conf, &con);  
  free_connection(&con);
  end_db_lib();
  free_cluster_data(&data);  
  free_configuration(&conf); 
 
	logging(LOG_INFO, __FILE__, __LINE__, "***STOPPING ENGINE***\n" );
  pthread_exit(NULL);
}

//Garbage collecition function
void release(int signo)
{
  logging(LOG_DEBUG,__FILE__,__LINE__,"Release proc caused by SIGNAL number %d\n", signo);

  if(pthread_rwlock_wrlock(&quit_lock) != 0)
  {
    logging(LOG_ERROR,__FILE__,__LINE__,"Unable to lock mutex\n");
  }
  else
  {
    agent_quit = 1;
	logging(LOG_DEBUG,__FILE__,__LINE__,"Setting quit flag to: %d\n", agent_quit);
    if(pthread_rwlock_unlock(&quit_lock) != 0){
      logging(LOG_ERROR,__FILE__,__LINE__,"Unable to unlock mutex\n");
    }
  }

  unsigned int *status;
  int n = 0;
  int rc = 0;
  /* Free attribute and wait for the other threads */
  pthread_attr_destroy(&attr);
  for(n = 0; n < conf.tot_num_cluster; n++) {
    rc = pthread_join(thread_cluster[n], (void **)&status);
    if (rc) {
		  logging(LOG_ERROR, __FILE__, __LINE__, "Error while joining cluster-threads: %d\n", rc );
    }
    logging(LOG_DEBUG,__FILE__,__LINE__,"Main: completed join with cluster-thread %d\n",n);
  }
  rc = pthread_join(thread_service[0], (void **)&status);
  if (rc) {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Error while joining service-threads: %d\n", rc );
  }
  logging(LOG_DEBUG,__FILE__,__LINE__,"Main: completed join with service-thread %d\n",0);
  rc = pthread_join(thread_service[1], (void **)&status);
  if (rc) {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Error while joining service-threads: %d\n", rc );
  }
  logging(LOG_DEBUG,__FILE__,__LINE__,"Main: completed join with service-thread %d\n",1);
  pthread_rwlock_destroy(&rwlock);
  pthread_rwlock_destroy(&quit_lock);

  //Cleanup procedures
  delete_transient(&conf, &con);  
  free_connection(&con);
  end_db_lib();
  free_cluster_data(&data);  
  free_configuration(&conf);  
	logging(LOG_INFO, __FILE__, __LINE__, "***STOPPING ENGINE***\n" );
  pthread_exit(NULL);
}
