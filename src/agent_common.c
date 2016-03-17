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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "debug.h"
#include "agent_error.h"
#include "agent_common.h"

extern int msglevel;

int init_configuration(configuration_struct *conf)
{
	if(!conf)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  conf->db_name = NULL;
  conf->db_host = NULL;
  conf->db_port = NULL;
  conf->db_user = NULL;
  conf->db_pass = NULL;
  conf->wf_path = NULL;
  conf->data_path = NULL;
  conf->tmp_path = NULL;
  conf->ch_path = NULL;
  conf->term_exec = NULL;
  conf->term_port = NULL;
  conf->term_pass = NULL;
  conf->term_user = NULL;
  conf->auth_header = NULL;
  conf->infrastr_url = NULL;
  conf->radl_file = NULL;
  conf->check_pdasjob_timer = 0;
  conf->check_exp_timer = 0;
  conf->check_delete_timer = 0;
  conf->check_cluster_timer = 0;
  conf->static_deploy = 0;
  conf->static_host = NULL; 
  conf->static_num_cores = 0;
  conf->tot_num_cluster = 0;
  conf->cloud_num_cores = 0;
  conf->job_threshold_factor = 0;
  conf->cloud_undeploy_cycles = 0;

	return AGENT_SUCCESS;
}

int free_configuration(configuration_struct *conf)
{
	if(!conf)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	if(conf->db_name)
  {
		free(conf->db_name);
		conf->db_name=NULL;
	}
	if(conf->db_host)
  {
		free(conf->db_host);
		conf->db_host = NULL;
	}
	if(conf->db_port)
  {
		free(conf->db_port);
		conf->db_port=NULL;
	}
	if(conf->db_user)
  {
		free(conf->db_user);
		conf->db_user=NULL;
	}
	if(conf->db_pass)
  {
		free(conf->db_pass);
		conf->db_pass=NULL;
	}
  if(conf->wf_path)
  {
		free(conf->wf_path);
		conf->wf_path=NULL;
	}
  if(conf->data_path)
  {
		free(conf->data_path);
		conf->data_path=NULL;
	}
  if(conf->tmp_path)
  {
		free(conf->tmp_path);
		conf->tmp_path=NULL;
	}
  if(conf->ch_path)
  {
		free(conf->ch_path);
		conf->ch_path=NULL;
	}
  if(conf->term_exec)
  {
		free(conf->term_exec);
		conf->term_exec=NULL;
	}
  if(conf->term_port)
  {
		free(conf->term_port);
		conf->term_port=NULL;
	}
  if(conf->term_pass)
  {
		free(conf->term_pass);
		conf->term_pass=NULL;
	}
  if(conf->term_user)
  {
		free(conf->term_user);
		conf->term_user=NULL;
	}
  if(conf->auth_header)
  {
		free(conf->auth_header);
		conf->auth_header=NULL;
	}
  if(conf->infrastr_url)
  {
		free(conf->infrastr_url);
		conf->infrastr_url=NULL;
	}
  if(conf->radl_file)
  {
		free(conf->radl_file);
		conf->radl_file=NULL;
	}
  if(conf->check_pdasjob_timer)
  {
		conf->check_pdasjob_timer=0;
	}
  if(conf->check_exp_timer)
  {
		conf->check_exp_timer=0;
	}
  if(conf->check_delete_timer)
  {
		conf->check_delete_timer=0;
	}
  if(conf->check_cluster_timer)
  {
		conf->check_cluster_timer=0;
	}
  if(conf->static_deploy)
  {
		conf->static_deploy=0;
	}
  if(conf->static_host)
  {
		free(conf->static_host);
		conf->static_host=NULL;
	}
  if(conf->static_num_cores)
  {
		conf->static_num_cores=0;
	}
  if(conf->tot_num_cluster)
  {
		conf->tot_num_cluster=0;
	}
  if(conf->cloud_num_cores)
  {
		conf->cloud_num_cores=0;
	}
  if(conf->job_threshold_factor)
  {
		conf->job_threshold_factor=0;
	}
  if(conf->cloud_undeploy_cycles)
  {
		conf->cloud_undeploy_cycles=0;
	}

	return AGENT_SUCCESS;
}

int read_config_file(configuration_struct *conf)
{
  FILE 					*fp = NULL;
  char 					line[CONFIGURATION_LINE] = {'\0'};
  char 					*result = NULL;
  char          *argument = NULL;
  char          *argument_value = NULL;
  int           argument_length = 0;
  unsigned int           i = 0;

  if (!conf)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
    return AGENT_ERROR_QUIT;
  }

  fp = fopen (CONFIGURATION_FILE, "r");
  if (!fp)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to open file %s\n", CONFIGURATION_FILE );
    return AGENT_ERROR_QUIT;
  }

  while (!feof (fp))
  {
    result = fgets (line, CONFIGURATION_LINE, fp);
    if (!result)
    {
      if (ferror (fp))
      {
        fclose (fp);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to read file line from %s\n", CONFIGURATION_FILE );
        return AGENT_ERROR_QUIT;
      }
      else
      {
        break;
      }
    }

    /* Remove trailing newline */
    if (line[strlen(line) - 1] == '\n')
      line[strlen(line) - 1] = '\0';

    /* Skip comment lines */
    if (line[0] == '#')
    {
  		logging(LOG_DEBUG, __FILE__, __LINE__, "Read comment line: %s\n", line );
      continue;
    }

    /* Check if line contains only spaces */
    for(i = 0; i < strlen(line) && i < CONFIGURATION_LINE; i++)
    {
      if(!isspace((unsigned char)line[i]))
        break;
    }
    if( i == strlen(line) || i == CONFIGURATION_LINE)
    {
  		logging(LOG_DEBUG, __FILE__, __LINE__, "Read empty or blank line\n" );
      continue;
    }
    
    /* Split argument and value on '=' character */
    for(i = 0; i < strlen(line) && i < CONFIGURATION_LINE; i++)
    {
      if(line[i] == '=')
        break;
    }
    if( i == strlen(line) || i == CONFIGURATION_LINE)
    {
  		logging(LOG_WARNING, __FILE__, __LINE__, "Read invalid line: %s\n", line );
      continue;
    }

    argument_length = strlen(line) - i - 1;

    argument = (char *)strndup(line, sizeof(char) * i);
    if (!argument)
    {
      fclose (fp);
  		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for configuration argument\n" );
      return AGENT_ERROR_QUIT;
    }

    argument_value = (char *)strndup(line + i + 1 , sizeof(char) * argument_length);
    if (!argument_value)
    {
      fclose (fp);
      free(argument);
  		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for configuration argument\n" );
      return AGENT_ERROR_QUIT;
    }

    if(!strncasecmp(argument, CONFIG_DB_NAME, MAX_LENGTH(argument, CONFIG_DB_NAME)))
    {
      conf->db_name = argument_value;
    }    
    else if(!strncasecmp(argument, CONFIG_DB_HOST, MAX_LENGTH(argument, CONFIG_DB_HOST)))
    {
      conf->db_host = argument_value;
    }    
    else if(!strncasecmp(argument, CONFIG_DB_PORT, MAX_LENGTH(argument, CONFIG_DB_PORT)))
    {
      conf->db_port = argument_value;
    }    
    else if(!strncasecmp(argument, CONFIG_DB_USER, MAX_LENGTH(argument, CONFIG_DB_USER)))
    {
      conf->db_user = argument_value;
    }    
    else if(!strncasecmp(argument, CONFIG_DB_PASS, MAX_LENGTH(argument, CONFIG_DB_PASS)))
    {
      conf->db_pass = argument_value;
    }    
    else if(!strncasecmp(argument, CONFIG_WF_PATH, MAX_LENGTH(argument, CONFIG_WF_PATH)))
    {
      conf->wf_path = argument_value;
    }    
    else if(!strncasecmp(argument, CONFIG_DATA_PATH, MAX_LENGTH(argument, CONFIG_DATA_PATH)))
    {
      conf->data_path = argument_value;
    }    
    else if(!strncasecmp(argument, CONFIG_TMP_PATH, MAX_LENGTH(argument, CONFIG_TMP_PATH)))
    {
      conf->tmp_path = argument_value;
    }  
    else if(!strncasecmp(argument, CONFIG_CH_PATH, MAX_LENGTH(argument, CONFIG_CH_PATH)))
    {
      conf->ch_path = argument_value;
    }  
    else if(!strncasecmp(argument, CONFIG_TERM_EXEC, MAX_LENGTH(argument, CONFIG_TERM_EXEC)))
    {
      conf->term_exec = argument_value;
    }    
    else if(!strncasecmp(argument, CONFIG_TERM_PORT, MAX_LENGTH(argument, CONFIG_TERM_PORT)))
    {
      conf->term_port = argument_value;
    }    
    else if(!strncasecmp(argument, CONFIG_TERM_USER, MAX_LENGTH(argument, CONFIG_TERM_USER)))
    {
      conf->term_user = argument_value;
    }    
    else if(!strncasecmp(argument, CONFIG_TERM_PASS, MAX_LENGTH(argument, CONFIG_TERM_PASS)))
    {
      conf->term_pass = argument_value;
    }    
    else if(!strncasecmp(argument, CONFIG_CH_PATH, MAX_LENGTH(argument, CONFIG_CH_PATH)))
    {
      conf->ch_path = argument_value;
    }  
    else if(!strncasecmp(argument, CONFIG_AUTH_HEADER, MAX_LENGTH(argument, CONFIG_AUTH_HEADER)))
    {
      conf->auth_header = argument_value;
    }  
    else if(!strncasecmp(argument, CONFIG_INFRASTR_URL, MAX_LENGTH(argument, CONFIG_INFRASTR_URL)))
    {
      conf->infrastr_url = argument_value;
    }  
    else if(!strncasecmp(argument, CONFIG_RADL_FILE, MAX_LENGTH(argument, CONFIG_RADL_FILE)))
    {
      conf->radl_file = argument_value;
    }  
    else if(!strncasecmp(argument, CONFIG_CHECK_PDASJOB_TIMER, MAX_LENGTH(argument, CONFIG_CHECK_PDASJOB_TIMER)))
    {
      conf->check_pdasjob_timer = (long long)strtoll(argument_value, NULL, 10);
      free(argument_value);
      if(conf->check_pdasjob_timer <= 0)
      {
        fclose (fp);
        free(argument);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Value of %s specified in configuration file is smaller than minimum value allowed 0\n", CONFIG_CHECK_PDASJOB_TIMER);
        return AGENT_ERROR_QUIT;
      }
    }  
    else if(!strncasecmp(argument, CONFIG_CHECK_EXP_TIMER, MAX_LENGTH(argument, CONFIG_CHECK_EXP_TIMER)))
    {
      conf->check_exp_timer = (long long)strtoll(argument_value, NULL, 10);
      free(argument_value);
      if(conf->check_exp_timer <= 0)
      {
        fclose (fp);
        free(argument);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Value of %s specified in configuration file is smaller than minimum value allowed 0\n", CONFIG_CHECK_EXP_TIMER);
        return AGENT_ERROR_QUIT;
      }
    }  
    else if(!strncasecmp(argument, CONFIG_CHECK_DELETE_TIMER, MAX_LENGTH(argument, CONFIG_CHECK_DELETE_TIMER)))
    {
      conf->check_delete_timer = (long long)strtoll(argument_value, NULL, 10);
      free(argument_value);
      if(conf->check_delete_timer <= 0)
      {
        fclose (fp);
        free(argument);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Value of %s specified in configuration file is smaller than minimum value allowed 0\n", CONFIG_CHECK_DELETE_TIMER);
        return AGENT_ERROR_QUIT;
      }
    }  
    else if(!strncasecmp(argument, CONFIG_CHECK_CLUSTER_TIMER, MAX_LENGTH(argument, CONFIG_CHECK_CLUSTER_TIMER)))
    {
      conf->check_cluster_timer = (long long)strtoll(argument_value, NULL, 10);
      free(argument_value);
      if(conf->check_cluster_timer <= 0)
      {
        fclose (fp);
        free(argument);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Value of %s specified in configuration file is smaller than minimum value allowed 0\n", CONFIG_CHECK_CLUSTER_TIMER);
        return AGENT_ERROR_QUIT;
      }
    } 
    else if(!strncasecmp(argument, CONFIG_STATIC_DEPLOYMENT, MAX_LENGTH(argument, CONFIG_STATIC_DEPLOYMENT)))
    {
      if(!strncasecmp(argument_value, "yes", MAX_LENGTH(argument_value, "yes")) || !strncasecmp(argument_value, "y", MAX_LENGTH(argument_value, "y")))
      {
        conf->static_deploy = 1;
        free(argument_value);
      }
    }  
    else if(!strncasecmp(argument, CONFIG_STATIC_HOST, MAX_LENGTH(argument, CONFIG_STATIC_HOST)))
    {
      conf->static_host = argument_value;
    }    
    else if(!strncasecmp(argument, CONFIG_STATIC_NUM_CORES, MAX_LENGTH(argument, CONFIG_STATIC_NUM_CORES)))
    {
      conf->static_num_cores = (long long)strtoll(argument_value, NULL, 10);
      free(argument_value);
      if(conf->static_num_cores > MAX_NUM_CORE)
      {
        fclose (fp);
        free(argument);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Value of %s specified in configuration file is bigger than max value allowed %lld\n", CONFIG_STATIC_NUM_CORES, MAX_NUM_CORE);
        return AGENT_ERROR_QUIT;
      }
      if(conf->static_num_cores <= 0)
      {
        fclose (fp);
        free(argument);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Value of %s specified in configuration file is smaller than minimum value allowed 0\n", CONFIG_STATIC_NUM_CORES);
        return AGENT_ERROR_QUIT;
      }
    }  
    else if(!strncasecmp(argument, CONFIG_TOT_NUM_CLUSTER, MAX_LENGTH(argument, CONFIG_TOT_NUM_CLUSTER)))
    {
      conf->tot_num_cluster = (long long)strtoll(argument_value, NULL, 10);
      free(argument_value);
      if(conf->tot_num_cluster > MAX_NUM_CLUSTER)
      {
        fclose (fp);
        free(argument);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Value of %s specified in configuration file is bigger than max value allowed %lld\n", CONFIG_TOT_NUM_CLUSTER, MAX_NUM_CLUSTER);
        return AGENT_ERROR_QUIT;
      }
      if(conf->tot_num_cluster <= 0)
      {
        fclose (fp);
        free(argument);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Value of %s specified in configuration file is smaller than minimum value allowed 0\n", CONFIG_TOT_NUM_CLUSTER);
        return AGENT_ERROR_QUIT;
      }
    }  
    else if(!strncasecmp(argument, CONFIG_CLOUD_NUM_CORES, MAX_LENGTH(argument, CONFIG_CLOUD_NUM_CORES)))
    {
      conf->cloud_num_cores = (long long)strtoll(argument_value, NULL, 10);
      free(argument_value);
      if(conf->cloud_num_cores > MAX_NUM_CORE)
      {
        fclose (fp);
        free(argument);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Value of %s specified in configuration file is bigger than max value allowed %lld\n", CONFIG_CLOUD_NUM_CORES, MAX_NUM_CORE);
        return AGENT_ERROR_QUIT;
      }
      if(conf->cloud_num_cores <= 0)
      {
        fclose (fp);
        free(argument);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Value of %s specified in configuration file is smaller than minimum value allowed 0\n", CONFIG_CLOUD_NUM_CORES);
        return AGENT_ERROR_QUIT;
      }
    }  
    else if(!strncasecmp(argument, CONFIG_JOB_THRESHOLD_FACTOR, MAX_LENGTH(argument, CONFIG_JOB_THRESHOLD_FACTOR)))
    {
      conf->job_threshold_factor = (long long)strtoll(argument_value, NULL, 10);
      free(argument_value);
      if(conf->job_threshold_factor <= 0)
      {
        fclose (fp);
        free(argument);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Value of %s specified in configuration file is smaller than minimum value allowed 0\n", CONFIG_JOB_THRESHOLD_FACTOR);
        return AGENT_ERROR_QUIT;
      }
    }  
    else if(!strncasecmp(argument, CONFIG_CLOUD_UNDEPLOY_CYCLES, MAX_LENGTH(argument, CONFIG_CLOUD_UNDEPLOY_CYCLES)))
    {
      conf->cloud_undeploy_cycles = (long long)strtoll(argument_value, NULL, 10);
      free(argument_value);
      if(conf->cloud_undeploy_cycles <= 0)
      {
        fclose (fp);
        free(argument);
    		logging(LOG_ERROR, __FILE__, __LINE__, "Value of %s specified in configuration file is smaller than minimum value allowed 0\n", CONFIG_CLOUD_UNDEPLOY_CYCLES);
        return AGENT_ERROR_QUIT;
      }
    }  
    else{
      free(argument_value);
    }
    free(argument);
    
  }

  fclose(fp);
  return AGENT_SUCCESS;
}
