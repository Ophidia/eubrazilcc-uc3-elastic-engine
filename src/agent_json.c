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

#include "oph_json/oph_json_library.h"

#include "agent_error.h"
#include "agent_commands.h"
#include "agent_json.h"
#include "agent_common.h"


int get_infrastruct_url(char *json_string, char **infr_url)
{
	if (!json_string || !infr_url) 
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  char *start = NULL;
  char *end = NULL;

  start = strstr(json_string, JSON_INFRASTRUCT_URL);
  if(!start)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Unable to find job ID\n" );
	  return AGENT_ERROR_JSON;
  }

  end = strstr(start, JSON_INFRASTRUCT_URL_END);
  if(!end)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Error while parsing job ID\n" );
	  return AGENT_ERROR_JSON;
  }
  
  char *url_string = (char *)malloc((strlen(start) - strlen(end) - strlen(JSON_INFRASTRUCT_URL) + 1) *sizeof(char));
  if(!url_string)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Memory alloc error\n" );
    return AGENT_ERROR_QUIT;
  }

  strncpy (url_string, start + strlen(JSON_INFRASTRUCT_URL), (strlen(start) - strlen(end) - strlen(JSON_INFRASTRUCT_URL)));
  url_string[(strlen(start) - strlen(end) - strlen(JSON_INFRASTRUCT_URL))] = '\0';

  *infr_url = url_string;

  logging(LOG_DEBUG,__FILE__,__LINE__,"Infrastructure url: %s\n",*infr_url);

  return AGENT_SUCCESS;
}

int get_deploy_status(char *json_string, short int *deploy_status)
{
	if (!json_string || !deploy_status) 
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  char *start = NULL;
  char *end = NULL;

  start = strstr(json_string, JSON_STATUS);
  if(!start)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Unable to find job ID\n" );
	  return AGENT_ERROR_JSON;
  }

  end = strstr(start, JSON_STATUS_END);
  if(!end)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Error while parsing job ID\n" );
	  return AGENT_ERROR_JSON;
  }
  
  char *status_string = (char *)malloc((strlen(start) - strlen(end) - strlen(JSON_STATUS) + 1) *sizeof(char));
  if(!status_string)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Memory alloc error\n" );
    return AGENT_ERROR_QUIT;
  }

  strncpy (status_string, start + strlen(JSON_STATUS), (strlen(start) - strlen(end) - strlen(JSON_STATUS)));
  status_string[(strlen(start) - strlen(end) - strlen(JSON_STATUS))] = '\0';

	if (!strncmp(status_string, COMMAND_DPSTATUS_EMPTY, MAX_LENGTH(status_string, COMMAND_DPSTATUS_EMPTY))) 
  {
    *deploy_status = COMMAND_DPSTATUS_CODE_EMPTY;				
    free(status_string);
		return AGENT_SUCCESS;
	} 
	else if (!strncmp(status_string, COMMAND_DPSTATUS_RUNNING, MAX_LENGTH(status_string, COMMAND_DPSTATUS_RUNNING))) 
  {
    *deploy_status = COMMAND_DPSTATUS_CODE_RUNNING;				
    free(status_string);
		return AGENT_SUCCESS;
	} 
	else if (!strncmp(status_string, COMMAND_DPSTATUS_ERROR, MAX_LENGTH(status_string, COMMAND_DPSTATUS_ERROR))) 
  {
    *deploy_status = COMMAND_DPSTATUS_CODE_ERROR;				
    free(status_string);
		return AGENT_SUCCESS;
	} 
	else if (!strncmp(status_string, COMMAND_DPSTATUS_FINISHED, MAX_LENGTH(status_string, COMMAND_DPSTATUS_FINISHED))) 
  {
    *deploy_status = COMMAND_DPSTATUS_CODE_FINISHED;				
    free(status_string);
		return AGENT_SUCCESS;
	} 
  else 
  {
    /* If return status is unknown */
  	logging(LOG_ERROR, __FILE__, __LINE__, "Unknown deploy status!\n" );
    *deploy_status = COMMAND_DPSTATUS_CODE_ERROR;				
    free(status_string);
		return AGENT_ERROR_JSON;
	}

  logging(LOG_DEBUG,__FILE__,__LINE__,"Deploy status: %d\n",*deploy_status);
  free(status_string);

  return AGENT_SUCCESS;
}

int get_undeploy_status(char *json_string, short int *deploy_status)
{
	if (!json_string || !deploy_status) 
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  char *start = NULL;

  start = strstr(json_string, JSON_DELETE_ERROR);
  if(start)
  {
  	logging(LOG_WARNING, __FILE__, __LINE__, "Deploy error found\n" );
    *deploy_status = COMMAND_DPSTATUS_CODE_ERROR;
  	return AGENT_SUCCESS;
  }
  
  *deploy_status = COMMAND_DPSTATUS_CODE_FINISHED;

  logging(LOG_DEBUG,__FILE__,__LINE__,"Deploy status: %d\n",*deploy_status);
  return AGENT_SUCCESS;
}

int get_server_address(char *json_string, char **server_addr)
{
	if (!json_string || !server_addr) 
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  char *start = NULL;
  char *end = NULL;

  start = strstr(json_string, JSON_SERVER_ADDRESS);
  if(!start)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Unable to find server address\n" );
	  return AGENT_ERROR_JSON;
  }

  end = strstr(start, JSON_SERVER_ADDRESS_END);
  if(!end)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Error while parsing server address\n" );
	  return AGENT_ERROR_JSON;
  }
  
  char *address_string = (char *)malloc((strlen(start) - strlen(end) - strlen(JSON_SERVER_ADDRESS) + 1) *sizeof(char));
  if(!address_string)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Memory alloc error\n" );
    return AGENT_ERROR_QUIT;
  }

  strncpy (address_string, start + strlen(JSON_SERVER_ADDRESS), (strlen(start) - strlen(end) - strlen(JSON_SERVER_ADDRESS)));
  address_string[(strlen(start) - strlen(end) - strlen(JSON_SERVER_ADDRESS))] = '\0';

  *server_addr = address_string;

  logging(LOG_DEBUG,__FILE__,__LINE__,"PDAS server address: %s\n",*server_addr);

  return AGENT_SUCCESS;
}


int get_workflowid(char *json_string, char **wfid)
{
	if (!json_string || !wfid) 
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  char *start = NULL;
  char *end = NULL;

  start = strstr(json_string, JSON_JOBID_STRING);
  if(!start)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Unable to find job ID\n" );
		return AGENT_ERROR_JSON;
  }

  end = strstr(start, JSON_JOBID_SEPARATOR);
  if(!end)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Error while parsing job ID\n" );
		return AGENT_ERROR_JSON;
  }
  
  char *jobid_string = (char *)malloc((strlen(start) - strlen(end) - strlen(JSON_JOBID_STRING) + 1) *sizeof(char));
  if(!jobid_string)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Memory alloc error\n" );
		return AGENT_ERROR_QUIT;
  }

  strncpy (jobid_string, start + strlen(JSON_JOBID_STRING), (strlen(start) - strlen(end) - strlen(JSON_JOBID_STRING)));
  jobid_string[(strlen(start) - strlen(end) - strlen(JSON_JOBID_STRING))] = '\0';

  start = strstr(jobid_string, JSON_EXPERIMENT_STRING);
  if(!start)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Unable to find job ID\n" );
    free(jobid_string);
		return AGENT_ERROR_JSON;
  }

  *wfid = (char *)strndup(start + strlen(JSON_EXPERIMENT_STRING), strlen(start) -  strlen(JSON_EXPERIMENT_STRING));
  if(!*wfid)
  {
  	logging(LOG_ERROR, __FILE__, __LINE__, "Memory alloc error\n" );
    free(jobid_string);
		return AGENT_ERROR_QUIT;
  }

  logging(LOG_DEBUG,__FILE__,__LINE__,"Workflow ID: %s\n",*wfid);

  free(jobid_string);
  return AGENT_SUCCESS;
}

int get_job_status(char *json_string, short int *job_status) {
	if (!json_string || !job_status ) 
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	/* Conversion from JSON string to OPH_JSON structure */
	char *tmp_json_string = strdup(json_string);
	if (!tmp_json_string){
  	logging(LOG_ERROR, __FILE__, __LINE__, "Error while duplicating json string\n" );
		return AGENT_ERROR_QUIT;
	}

	oph_json *json = NULL;
	if (oph_json_from_json_string(&json,&tmp_json_string))
  {
		if (json) oph_json_free(json);
  	logging(LOG_ERROR, __FILE__, __LINE__, "Unable to convert string to Json\n" );
		return AGENT_ERROR_JSON;
	}

	/* Check for Json status. In case of ERROR exit */
	size_t n,valid=0;
	for (n = 0; n < json->responseKeyset_num; n++) 
  {
		if (!strcmp(json->responseKeyset[n],"status")) 
    {
			valid = 1;
			break;
		}
	}
	if (!valid) 
  {
		if (json) oph_json_free(json);
  	logging(LOG_ERROR, __FILE__, __LINE__, "Json string not valid!\n" );
		return AGENT_ERROR_JSON;
	}
	valid=0;
	for (n = 0; n < json->response_num; n++) 
  {
		if (!strcmp(json->response[n].objkey,"status")) 
    {
			if (!strcmp(((oph_json_obj_text *)(json->response[n].objcontent))[0].title, "SUCCESS")) 
      {
				valid = 1;
				break;
			} 
      else 
      {
        /* If an error occured return status error */
      	logging(LOG_WARNING, __FILE__, __LINE__, "Workflow execution failed!\n" );
        *job_status = COMMAND_STATUS_CODE_ERROR;				
        if (json) oph_json_free(json);
				return AGENT_SUCCESS;
			}
		}
	}
	if (!valid) {
		if (json) oph_json_free(json);
  	logging(LOG_ERROR, __FILE__, __LINE__, "Json string not valid!\n" );
		return AGENT_ERROR_JSON;
	}


	/* Check for status. In case of ERROR exit */
  valid=0;
	for (n = 0; n < json->responseKeyset_num; n++) 
  {
		if (!strcmp(json->responseKeyset[n],"workflow_status")) 
    {
			valid = 1;
			break;
		}
	}
	if (!valid) 
  {
		if (json) oph_json_free(json);
  	logging(LOG_ERROR, __FILE__, __LINE__, "Json string not valid!\n" );
		return AGENT_ERROR_JSON;
	}

	for (n = 0; n < json->response_num; n++) 
  {
		if (!strcmp(json->response[n].objkey,"workflow_status")) 
    {
			if (!strncmp(((oph_json_obj_text *)(json->response[n].objcontent))[0].message, COMMAND_STATUS_COMPLETED, MAX_LENGTH(((oph_json_obj_text *)(json->response[n].objcontent))[0].message, COMMAND_STATUS_COMPLETED))) 
      {
        /* If workflow is completed correctly */
        *job_status = COMMAND_STATUS_CODE_COMPLETED;				
        if (json) oph_json_free(json);
				return AGENT_SUCCESS;
			} 
			else if (!strncmp(((oph_json_obj_text *)(json->response[n].objcontent))[0].message, COMMAND_STATUS_PENDING, MAX_LENGTH(((oph_json_obj_text *)(json->response[n].objcontent))[0].message, COMMAND_STATUS_PENDING)) || !strncmp(((oph_json_obj_text *)(json->response[n].objcontent))[0].message, COMMAND_STATUS_UNKNOWN, MAX_LENGTH(((oph_json_obj_text *)(json->response[n].objcontent))[0].message, COMMAND_STATUS_UNKNOWN))) 
      {
        /* If workflow is waiting */
        *job_status = COMMAND_STATUS_CODE_PENDING;				
        if (json) oph_json_free(json);
				return AGENT_SUCCESS;
			}
      else if (!strncmp(((oph_json_obj_text *)(json->response[n].objcontent))[0].message, COMMAND_STATUS_RUNNING, MAX_LENGTH(((oph_json_obj_text *)(json->response[n].objcontent))[0].message, COMMAND_STATUS_RUNNING))) 
      {
        /* If workflow is running */
        *job_status = COMMAND_STATUS_CODE_RUNNING;				
        if (json) oph_json_free(json);
				return AGENT_SUCCESS;
			}
      else if (!strncmp(((oph_json_obj_text *)(json->response[n].objcontent))[0].message, COMMAND_STATUS_ERROR, MAX_LENGTH(((oph_json_obj_text *)(json->response[n].objcontent))[0].message, COMMAND_STATUS_ERROR))) 
      {
        /* If workflow stopped due to an error */
      	logging(LOG_WARNING, __FILE__, __LINE__, "Workflow execution failed!\n" );
        *job_status = COMMAND_STATUS_CODE_ERROR;				
        if (json) oph_json_free(json);
				return AGENT_SUCCESS;
			}
      else 
      {
        /* If return status is unknown */
      	logging(LOG_ERROR, __FILE__, __LINE__, "Unknown status!\n" );
        *job_status = COMMAND_STATUS_CODE_ERROR;				
        if (json) oph_json_free(json);
				return AGENT_ERROR_JSON;
			}
		}
	}

	if (json) oph_json_free(json);
	logging(LOG_ERROR, __FILE__, __LINE__, "Json string not valid!\n" );
	return AGENT_ERROR_JSON;
}

int get_jobid(char *json_string, char **jid) 
{
	if (!json_string || !jid ) 
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	/* Conversion from JSON string to OPH_JSON structure */
	char *tmp_json_string = strdup(json_string);
	if (!tmp_json_string){
  	logging(LOG_ERROR, __FILE__, __LINE__, "Error while duplicating json string\n" );
		return AGENT_ERROR_QUIT;
	}

	oph_json *json = NULL;
	if (oph_json_from_json_string(&json,&tmp_json_string))
  {
		if (json) oph_json_free(json);
  	logging(LOG_ERROR, __FILE__, __LINE__, "Unable to convert string to Json\n" );
		return AGENT_ERROR_JSON;
	}

	/* Check for Json status. In case of ERROR exit */
	size_t n,valid=0;
	for (n = 0; n < json->responseKeyset_num; n++) 
  {
		if (!strcmp(json->responseKeyset[n],"status")) 
    {
			valid = 1;
			break;
		}
	}
	if (!valid) 
  {
		if (json) oph_json_free(json);
  	logging(LOG_ERROR, __FILE__, __LINE__, "Json string not valid!\n" );
		return AGENT_ERROR_JSON;
	}
	valid=0;
	for (n = 0; n < json->response_num; n++) 
  {
		if (!strcmp(json->response[n].objkey,"status")) 
    {
			if (!strcmp(((oph_json_obj_text *)(json->response[n].objcontent))[0].title, "SUCCESS")) 
      {
				valid = 1;
				break;
			} 
      else 
      {
        /* If an error occured return status error */
      	logging(LOG_ERROR, __FILE__, __LINE__, "Job id retrieve execution failed!\n" );
        if (json) oph_json_free(json);
				return AGENT_SUCCESS;
			}
		}
	}
	if (!valid) {
		if (json) oph_json_free(json);
  	logging(LOG_ERROR, __FILE__, __LINE__, "Json string not valid!\n" );
		return AGENT_ERROR_JSON;
	}


	/* Get last job id. In case of ERROR exit */
  valid=0;
	for (n = 0; n < json->responseKeyset_num; n++) 
  {
		if (!strcmp(json->responseKeyset[n],"workflow_list")) 
    {
			valid = 1;
			break;
		}
	}
	if (!valid) 
  {
		if (json) oph_json_free(json);
  	logging(LOG_ERROR, __FILE__, __LINE__, "Json string not valid!\n" );
		return AGENT_ERROR_JSON;
	}

  char *wid = NULL;
  char *mid = NULL;

  valid = 0;
	for (n = 0; n < json->response_num; n++) 
  {
		if (!strcmp(json->response[n].objkey,"workflow_list")) 
    {
      /* Check if rows and columns are correct*/
      if((((oph_json_obj_grid *) json->response[n].objcontent)[0]).values_num1 < 1)
      {
      	logging(LOG_ERROR, __FILE__, __LINE__, "Json string not valid\n" );
    		if (json) oph_json_free(json);
		    return AGENT_ERROR_JSON;
	    }
      if((((oph_json_obj_grid *) json->response[n].objcontent)[0]).values_num2 < 5)
      {
      	logging(LOG_ERROR, __FILE__, __LINE__, "Json string not valid\n" );
    		if (json) oph_json_free(json);
		    return AGENT_ERROR_JSON;
	    }

      wid = (char *) strdup((((oph_json_obj_grid *) json->response[n].objcontent)[0]).values[(((oph_json_obj_grid *) json->response[n].objcontent)[0]).values_num1 - 1][2]);
	    if (!wid)
      {
      	logging(LOG_ERROR, __FILE__, __LINE__, "Error while duplicating json string\n" );
        if (json) oph_json_free(json);
		    return AGENT_ERROR_QUIT;
	    }

      mid = (char *) strdup((((oph_json_obj_grid *) json->response[n].objcontent)[0]).values[(((oph_json_obj_grid *) json->response[n].objcontent)[0]).values_num1 - 1][3]);
	    if (!mid)
      {
      	logging(LOG_ERROR, __FILE__, __LINE__, "Error while duplicating json string\n" );
    		if (wid) free(wid);
    		if (json) oph_json_free(json);
		    return AGENT_ERROR_QUIT;
	    }

      valid = 1;
      break;    
		}
	}

  if(!valid)
  {
	  if (json) oph_json_free(json);
    if (mid) free(mid);
    if (wid) free(wid);
	  logging(LOG_ERROR, __FILE__, __LINE__, "Json string not valid!\n" ); 
	  return AGENT_ERROR_JSON;
  }

  size_t length = snprintf(NULL, 0, JSON_JOBID_FORMAT, wid, mid);
  *jid = (char *)malloc(sizeof(char) * (length +1));
  if(!*jid)
  {
	  if (json) oph_json_free(json);
    free(mid);
    free(wid);
	  logging(LOG_ERROR, __FILE__, __LINE__, "Memory alloc error\n" ); 
	  return AGENT_ERROR_QUIT;
  }
    
	if (json) oph_json_free(json);
  snprintf(*jid, length + 1, JSON_JOBID_FORMAT, wid, mid);
  free(mid);
  free(wid);

  return AGENT_SUCCESS;
}
