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
#include <my_global.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "agent_db.h"
#include "debug.h"
#include "agent_query.h"
#include "agent_commands.h"
#include "agent_error.h"

extern int msglevel;

/**
 * \brief               Simple auxiliary function to convert LiDAR short product name to full text
 * \param product_name  Short LiDAR product name
 * \return the full text if successfull, NULL otherwise
 */
char *_lidar_expand_title(char *product_name)
{
  if (!product_name) {
    logging(LOG_ERROR, __FILE__, __LINE__,"Product name to be converted not given\n");
    return NULL;
  }

  if(!strncasecmp(product_name, "agb", MAX_LENGTH(product_name, "agb")))
  {
    return "Aboveground Biomass";
  } 
  if(!strncasecmp(product_name, "aspect", MAX_LENGTH(product_name, "aspect")))
  {
    return "Aspect";
  } 
  if(!strncasecmp(product_name, "cover", MAX_LENGTH(product_name, "cover")))
  {
    return "Forest Cover";
  } 
  if(!strncasecmp(product_name, "chm", MAX_LENGTH(product_name, "chm")))
  {
    return "Canopy Height Model";
  } 
  if(!strncasecmp(product_name, "dsm", MAX_LENGTH(product_name, "dsm")))
  {
    return "Digital Surface Model";
  } 
  if(!strncasecmp(product_name, "dtm", MAX_LENGTH(product_name, "dtm")))
  {
    return "Digital Terrain Model";
  } 
  if(!strncasecmp(product_name, "pd", MAX_LENGTH(product_name, "pd")))
  {
    return "Point Density";
  } 
  if(!strncasecmp(product_name, "rh50", MAX_LENGTH(product_name, "rh50")))
  {
    return "Relative Height 50%";
  } 
  if(!strncasecmp(product_name, "slope", MAX_LENGTH(product_name, "slope")))
  {
    return "Slope Angle";
  } 

  return NULL;  
}


int init_db_lib()
{
  if (mysql_library_init(0, NULL, NULL)) {
    logging(LOG_ERROR, __FILE__, __LINE__,"MySQL initialization error\n");
    return AGENT_ERROR_QUIT;
  }
  return AGENT_SUCCESS;
}

int end_db_lib()
{
	mysql_library_end();
	return AGENT_SUCCESS;

}

int init_connection(connection_struct *con, configuration_struct *conf)
{
	if(!con || !conf)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	con->name = (char*)strndup(conf->db_name, strlen(conf->db_name));
  if(!conf->db_name){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for connection struct\n" );
	  return AGENT_ERROR_QUIT;
  }
	con->host = (char*)strndup(conf->db_host, strlen(conf->db_host));
  if(!conf->db_host){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for connection struct\n" );
    free(conf->db_name);
	  return AGENT_ERROR_QUIT;
  }
	con->port = (char*)strndup(conf->db_port, strlen(conf->db_port));
  if(!conf->db_port){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for connection struct\n" );
    free(conf->db_name);
    free(conf->db_host);
	  return AGENT_ERROR_QUIT;
  }  
	con->user = (char*)strndup(conf->db_user, strlen(conf->db_user));
  if(!conf->db_user){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for connection struct\n" );
    free(conf->db_name);
    free(conf->db_port);
    free(conf->db_host);
	  return AGENT_ERROR_QUIT;
  }  
	con->pass = (char*)strndup(conf->db_pass, strlen(conf->db_pass));
  if(!conf->db_pass){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for connection struct\n" );
    free(conf->db_name);
    free(conf->db_port);
    free(conf->db_user);
    free(conf->db_host);
	  return AGENT_ERROR_QUIT;
  }  
	con->conn = NULL;
  
	return AGENT_SUCCESS;
}

int free_connection(connection_struct *con)
{
	if(!con)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	if(con->name)
  {
		free(con->name);
		con->name=NULL;
	}
	if(con->host)
  {
		free(con->host);
		con->host = NULL;
	}
	if(con->port)
  {
		free(con->port);
		con->port=NULL;
	}
	if(con->user)
  {
		free(con->user);
		con->user=NULL;
	}
	if(con->pass)
  {
		free(con->pass);
		con->pass=NULL;
	}
	if(con->conn)
  {
		con->conn=NULL;
	}
	return AGENT_SUCCESS;
}

int db_connect(connection_struct *con, short int multi)
{
	if(!con)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  con->conn = NULL;
	if( !(con->conn = mysql_init (NULL)) )
  {
		logging(LOG_ERROR, __FILE__, __LINE__,"MySQL initialization error %s\n", mysql_error(con->conn));
    return AGENT_ERROR_MYSQL;
  }

	/* Connect to database */
	if (!mysql_real_connect(con->conn, con->host, con->user, con->pass, con->name, (int)strtol(con->port, NULL, 10), NULL, (multi ? CLIENT_MULTI_STATEMENTS : 0)))
  {
		logging(LOG_ERROR, __FILE__, __LINE__,"MySQL connection error: %s\n", mysql_error(con->conn));
		db_disconnect(con);
    return AGENT_ERROR_MYSQL;
	}
	return AGENT_SUCCESS;
}

int db_disconnect(connection_struct *con)
{
	if(!con)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	if(con->conn)
  {
		mysql_close(con->conn);
		con->conn = NULL;
	}
  mysql_thread_end();
	return AGENT_SUCCESS;
}

int init_experiment(void **exp, exp_type type)
{
	if(!exp)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}
  *exp = NULL;

  switch(type)
  {
    case EXP_SEBALINTERANNUAL:
    {
      experiment_sebalinterannual *tmp_exp = (experiment_sebalinterannual *)malloc(sizeof(experiment_sebalinterannual));
      if(!tmp_exp)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment structure\n" );
        return AGENT_ERROR_QUIT;
      }
   
	    tmp_exp->workflowcode = NULL;
	    tmp_exp->workflowfile = NULL;
      memset(tmp_exp->bbox, 0, sizeof(tmp_exp->bbox));
	    memset(tmp_exp->timeinterval, 0, sizeof(tmp_exp->timeinterval));
	    tmp_exp->filename = NULL;
	    tmp_exp->folder = NULL;
	    tmp_exp->coordinate = NULL;
	    tmp_exp->timerange = NULL;
	    tmp_exp->timefrequency = NULL;
	    tmp_exp->basetime = NULL;
	    tmp_exp->units = NULL;
	    tmp_exp->calendar = NULL;
	    tmp_exp->variable = NULL;
	    tmp_exp->variablename = NULL;
      tmp_exp->pdasid = NULL;

      *exp = tmp_exp;
      break;
    }
    case EXP_CLIMATESEBAL:
    {
      experiment_climatesebal *tmp_exp = (experiment_climatesebal *)malloc(sizeof(experiment_climatesebal));
      if(!tmp_exp)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment structure\n" );
        return AGENT_ERROR_QUIT;
      }
   
	    tmp_exp->workflowcode = NULL;
	    tmp_exp->workflowfile = NULL;
      memset(tmp_exp->bbox, 0, sizeof(tmp_exp->bbox));
	    memset(tmp_exp->timeinterval, 0, sizeof(tmp_exp->timeinterval));
	    tmp_exp->climate_filename = NULL;
	    tmp_exp->climate_folder = NULL;
	    tmp_exp->climate_coordinate = NULL;
	    tmp_exp->climate_timerange = NULL;
	    tmp_exp->climate_timefrequency = NULL;
	    tmp_exp->climate_basetime = NULL;
	    tmp_exp->climate_units = NULL;
	    tmp_exp->climate_calendar = NULL;
	    tmp_exp->climate_variable = NULL;
	    tmp_exp->climate_variablename = NULL;
	    tmp_exp->satellite_filename = NULL;
	    tmp_exp->satellite_folder = NULL;
	    tmp_exp->satellite_coordinate = NULL;
	    tmp_exp->satellite_timerange = NULL;
	    tmp_exp->satellite_timefrequency = NULL;
	    tmp_exp->satellite_basetime = NULL;
	    tmp_exp->satellite_units = NULL;
	    tmp_exp->satellite_calendar = NULL;
	    tmp_exp->satellite_variable = NULL;
	    tmp_exp->satellite_variablename = NULL;
	    tmp_exp->satellite_pdasid = NULL;

      *exp = tmp_exp;
      break;
    }    
    case EXP_MODELINTERCOMPARISON:
    { 
      experiment_modelintercomparison *tmp_exp = (experiment_modelintercomparison *)malloc(sizeof(experiment_modelintercomparison));
      if(!tmp_exp)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment structure\n" );
        return AGENT_ERROR_QUIT;
      }
   
	    tmp_exp->workflowcode = NULL;
	    tmp_exp->workflowfile = NULL;
      memset(tmp_exp->bbox, 0, sizeof(tmp_exp->bbox));
	    memset(tmp_exp->timeinterval, 0, sizeof(tmp_exp->timeinterval));
	    tmp_exp->folder = NULL;
	    tmp_exp->coordinate = NULL;
	    tmp_exp->timerange = NULL;
	    tmp_exp->timefrequency = NULL;
	    tmp_exp->basetime = NULL;
	    tmp_exp->units = NULL;
	    tmp_exp->calendar = NULL;
	    tmp_exp->models = NULL;
	    tmp_exp->scenarios = NULL;
	    memset(tmp_exp->indicators, 0, sizeof(tmp_exp->indicators));

      *exp = tmp_exp;
      break;
    }
    case EXP_RELATIVEHEIGHT:
    {
      experiment_relheight *tmp_exp = (experiment_relheight *)malloc(sizeof(experiment_relheight));
      if(!tmp_exp)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment structure\n" );
        return AGENT_ERROR_QUIT;
      }

	    tmp_exp->workflowcode = NULL;
	    tmp_exp->workflowfile = NULL;
	    tmp_exp->folder = NULL;
	    tmp_exp->filename = NULL;
	    tmp_exp->coordinate = NULL;

      *exp = tmp_exp;
      break;
    }
    case EXP_LIDARINTERCOMPARISON:
    {
      experiment_lidarintercomparison *tmp_exp = (experiment_lidarintercomparison *)malloc(sizeof(experiment_lidarintercomparison));
      if(!tmp_exp)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment structure\n" );
        return AGENT_ERROR_QUIT;
      }

	    tmp_exp->workflowcode = NULL;
	    tmp_exp->workflowfile = NULL;
	    tmp_exp->folder = NULL;
	    tmp_exp->filename = NULL;
	    tmp_exp->coordinate = NULL;
	    memset(tmp_exp->products, 0, sizeof(tmp_exp->products));

      *exp = tmp_exp;
      break;
    }
    case EXP_ENM:
    {
      experiment_enm *tmp_exp = (experiment_enm *)malloc(sizeof(experiment_enm));
      if(!tmp_exp)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment structure\n" );
        return AGENT_ERROR_QUIT;
      }

	    tmp_exp->workflowcode = NULL;
	    tmp_exp->workflowfile = NULL;
	    tmp_exp->species = NULL;
	    tmp_exp->x = NULL;
      tmp_exp->y = NULL;
	    tmp_exp->num_occurrences = 0;

      *exp = tmp_exp;
      break;
    }
    default:
    {
	    logging(LOG_ERROR, __FILE__, __LINE__, "Experiment type not recognized\n" );
	    return AGENT_ERROR_QUIT;
    }
  }

	return AGENT_SUCCESS;
}

int free_experiment(void **exp, exp_type type)
{
	if(!exp)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  int i = 0;

  switch(type)
  {
    case EXP_SEBALINTERANNUAL:
    {
      experiment_sebalinterannual *tmp_exp = (experiment_sebalinterannual *)(*exp);
      if(tmp_exp != NULL)
      {

	      if(tmp_exp->workflowcode)
        {
          free(tmp_exp->workflowcode);
          tmp_exp->workflowcode = NULL;
        }
	      if(tmp_exp->workflowfile)
        {
          free(tmp_exp->workflowfile);
          tmp_exp->workflowfile = NULL;
        }
        memset(tmp_exp->bbox, 0, sizeof(tmp_exp->bbox));
        for(i = 0; i < 2; i++)
        {
          if(tmp_exp->timeinterval[i])
          {
            free(tmp_exp->timeinterval[i]);
            tmp_exp->timeinterval[i] = 0;
          }
        }
	      if(tmp_exp->filename)
        {
          free(tmp_exp->filename);
          tmp_exp->filename = NULL;
        }
	      if(tmp_exp->folder)
        {
          free(tmp_exp->folder);
          tmp_exp->folder = NULL;
        }
	      if(tmp_exp->coordinate)
        {
          free(tmp_exp->coordinate);
          tmp_exp->coordinate = NULL;
        }
	      if(tmp_exp->timerange)
        {
          free(tmp_exp->timerange);
          tmp_exp->timerange = NULL;
        }
	      if(tmp_exp->timefrequency)
        {
          free(tmp_exp->timefrequency);
          tmp_exp->timefrequency = NULL;
        }
	      if(tmp_exp->basetime)
        {
          free(tmp_exp->basetime);
          tmp_exp->basetime = NULL;
        }
	      if(tmp_exp->units)
        {
          free(tmp_exp->units);
          tmp_exp->units = NULL;
        }
	      if(tmp_exp->calendar)
        {
          free(tmp_exp->calendar);
          tmp_exp->calendar = NULL;
        }
	      if(tmp_exp->variable)
        {
          free(tmp_exp->variable);
          tmp_exp->variable = NULL;
        }
	      if(tmp_exp->variablename)
        {
          free(tmp_exp->variablename);
          tmp_exp->variablename = NULL;
        }
	      if(tmp_exp->pdasid)
        {
          free(tmp_exp->pdasid);
          tmp_exp->pdasid = NULL;
        }
        
        free(*exp);
        *exp = NULL;
      }
      break;
    }
    case EXP_CLIMATESEBAL:
    {
      experiment_climatesebal *tmp_exp = (experiment_climatesebal *)(*exp);
      if(tmp_exp != NULL)
      {
	      if(tmp_exp->workflowcode)
        {
          free(tmp_exp->workflowcode);
          tmp_exp->workflowcode = NULL;
        }
	      if(tmp_exp->workflowfile)
        {
          free(tmp_exp->workflowfile);
          tmp_exp->workflowfile = NULL;
        }
        memset(tmp_exp->bbox, 0, sizeof(tmp_exp->bbox));
        for(i = 0; i < 2; i++)
        {
          if(tmp_exp->timeinterval[i])
          {
            free(tmp_exp->timeinterval[i]);
            tmp_exp->timeinterval[i] = 0;
          }
        }
	      if(tmp_exp->climate_filename)
        {
          free(tmp_exp->climate_filename);
          tmp_exp->climate_filename = NULL;
        }
	      if(tmp_exp->climate_folder)
        {
          free(tmp_exp->climate_folder);
          tmp_exp->climate_folder = NULL;
        }
	      if(tmp_exp->climate_coordinate)
        {
          free(tmp_exp->climate_coordinate);
          tmp_exp->climate_coordinate = NULL;
        }
	      if(tmp_exp->climate_timerange)
        {
          free(tmp_exp->climate_timerange);
          tmp_exp->climate_timerange = NULL;
        }
	      if(tmp_exp->climate_timefrequency)
        {
          free(tmp_exp->climate_timefrequency);
          tmp_exp->climate_timefrequency = NULL;
        }
	      if(tmp_exp->climate_basetime)
        {
          free(tmp_exp->climate_basetime);
          tmp_exp->climate_basetime = NULL;
        }
	      if(tmp_exp->climate_units)
        {
          free(tmp_exp->climate_units);
          tmp_exp->climate_units = NULL;
        }
	      if(tmp_exp->climate_calendar)
        {
          free(tmp_exp->climate_calendar);
          tmp_exp->climate_calendar = NULL;
        }
	      if(tmp_exp->climate_variable)
        {
          free(tmp_exp->climate_variable);
          tmp_exp->climate_variable = NULL;
        }
	      if(tmp_exp->climate_variablename)
        {
          free(tmp_exp->climate_variablename);
          tmp_exp->climate_variablename = NULL;
        }
	      if(tmp_exp->satellite_filename)
        {
          free(tmp_exp->satellite_filename);
          tmp_exp->satellite_filename = NULL;
        }
	      if(tmp_exp->satellite_folder)
        {
          free(tmp_exp->satellite_folder);
          tmp_exp->satellite_folder = NULL;
        }
	      if(tmp_exp->satellite_coordinate)
        {
          free(tmp_exp->satellite_coordinate);
          tmp_exp->satellite_coordinate = NULL;
        }
	      if(tmp_exp->satellite_timerange)
        {
          free(tmp_exp->satellite_timerange);
          tmp_exp->satellite_timerange = NULL;
        }
	      if(tmp_exp->satellite_timefrequency)
        {
          free(tmp_exp->satellite_timefrequency);
          tmp_exp->satellite_timefrequency = NULL;
        }
	      if(tmp_exp->satellite_basetime)
        {
          free(tmp_exp->satellite_basetime);
          tmp_exp->satellite_basetime = NULL;
        }
	      if(tmp_exp->satellite_units)
        {
          free(tmp_exp->satellite_units);
          tmp_exp->satellite_units = NULL;
        }
	      if(tmp_exp->satellite_calendar)
        {
          free(tmp_exp->satellite_calendar);
          tmp_exp->satellite_calendar = NULL;
        }
	      if(tmp_exp->satellite_variable)
        {
          free(tmp_exp->satellite_variable);
          tmp_exp->satellite_variable = NULL;
        }
	      if(tmp_exp->satellite_variablename)
        {
          free(tmp_exp->satellite_variablename);
          tmp_exp->satellite_variablename = NULL;
        }
	      if(tmp_exp->satellite_pdasid)
        {
          free(tmp_exp->satellite_pdasid);
          tmp_exp->satellite_pdasid = NULL;
        }

        free(*exp);
        *exp = NULL;
      }
	    break;
    }  
    case EXP_MODELINTERCOMPARISON:
    { 
      experiment_modelintercomparison *tmp_exp = (experiment_modelintercomparison *)(*exp);
      char *tmp_var =  NULL;
      int tmp_index = 0;
      if(tmp_exp != NULL)
      {

	      if(tmp_exp->workflowcode)
        {
          free(tmp_exp->workflowcode);
          tmp_exp->workflowcode = NULL;
        }
	      if(tmp_exp->workflowfile)
        {
          free(tmp_exp->workflowfile);
          tmp_exp->workflowfile = NULL;
        }
        memset(tmp_exp->bbox, 0, sizeof(tmp_exp->bbox));
        for(i = 0; i < 2; i++)
        {
          if(tmp_exp->timeinterval[i])
          {
            free(tmp_exp->timeinterval[i]);
            tmp_exp->timeinterval[i] = 0;
          }
        }
	      if(tmp_exp->folder)
        {
          free(tmp_exp->folder);
          tmp_exp->folder = NULL;
        }
	      if(tmp_exp->coordinate)
        {
          free(tmp_exp->coordinate);
          tmp_exp->coordinate = NULL;
        }
	      if(tmp_exp->timerange)
        {
          free(tmp_exp->timerange);
          tmp_exp->timerange = NULL;
        }
	      if(tmp_exp->timefrequency)
        {
          free(tmp_exp->timefrequency);
          tmp_exp->timefrequency = NULL;
        }
	      if(tmp_exp->basetime)
        {
          free(tmp_exp->basetime);
          tmp_exp->basetime = NULL;
        }
	      if(tmp_exp->units)
        {
          free(tmp_exp->units);
          tmp_exp->units = NULL;
        }
	      if(tmp_exp->calendar)
        {
          free(tmp_exp->calendar);
          tmp_exp->calendar = NULL;
        }
	      if(tmp_exp->models)
        {
          tmp_index = 0;
          tmp_var = tmp_exp->models[tmp_index];
          while(tmp_var != NULL)
          {
            free(tmp_var);
            tmp_var = NULL;
            tmp_var = tmp_exp->models[++tmp_index];
          }
          free(tmp_exp->models);
          tmp_exp->models = NULL;
        }
	      if(tmp_exp->scenarios)
        {
          tmp_index = 0;
          tmp_var = tmp_exp->scenarios[tmp_index];
          while(tmp_var != NULL)
          {
            free(tmp_var);
            tmp_var = NULL;
            tmp_var = tmp_exp->scenarios[++tmp_index];
          }
          free(tmp_exp->scenarios);
          tmp_exp->scenarios = NULL;
        }
        for(i = 0; i < 4; i++)
        {
          if(tmp_exp->indicators[i])
          {
            free(tmp_exp->indicators[i]);
            tmp_exp->indicators[i] = 0;
          }
        }
        free(*exp);
        *exp = NULL;
      }
      break;
    }
    case EXP_RELATIVEHEIGHT:
    {
      experiment_relheight *tmp_exp = (experiment_relheight *)(*exp);
      if(tmp_exp != NULL)
      {

	      if(tmp_exp->workflowcode)
        {
          free(tmp_exp->workflowcode);
          tmp_exp->workflowcode = NULL;
        }
	      if(tmp_exp->workflowfile)
        {
          free(tmp_exp->workflowfile);
          tmp_exp->workflowfile = NULL;
        }
	      if(tmp_exp->filename)
        {
          free(tmp_exp->filename);
          tmp_exp->filename = NULL;
        }
	      if(tmp_exp->folder)
        {
          free(tmp_exp->folder);
          tmp_exp->folder = NULL;
        }
	      if(tmp_exp->coordinate)
        {
          free(tmp_exp->coordinate);
          tmp_exp->coordinate = NULL;
        }
        
        free(*exp);
        *exp = NULL;
      }
      break;
    }
    case EXP_LIDARINTERCOMPARISON:
    {
      experiment_lidarintercomparison *tmp_exp = (experiment_lidarintercomparison *)(*exp);
      if(tmp_exp != NULL)
      {

	      if(tmp_exp->workflowcode)
        {
          free(tmp_exp->workflowcode);
          tmp_exp->workflowcode = NULL;
        }
	      if(tmp_exp->workflowfile)
        {
          free(tmp_exp->workflowfile);
          tmp_exp->workflowfile = NULL;
        }
	      if(tmp_exp->folder)
        {
          free(tmp_exp->folder);
          tmp_exp->folder = NULL;
        }
	      if(tmp_exp->filename)
        {
          free(tmp_exp->filename);
          tmp_exp->filename = NULL;
        }
	      if(tmp_exp->coordinate)
        {
          free(tmp_exp->coordinate);
          tmp_exp->coordinate = NULL;
        }
        for(i = 0; i < 9; i++)
        {
          if(tmp_exp->products[i])
          {
            free(tmp_exp->products[i]);
            tmp_exp->products[i] = 0;
          }
        }
        
        free(*exp);
        *exp = NULL;
      }
      break;
    }
    case EXP_ENM:
    {
      experiment_enm *tmp_exp = (experiment_enm *)(*exp);
      if(tmp_exp != NULL)
      {

	      if(tmp_exp->workflowcode)
        {
          free(tmp_exp->workflowcode);
          tmp_exp->workflowcode = NULL;
        }
	      if(tmp_exp->workflowfile)
        {
          free(tmp_exp->workflowfile);
          tmp_exp->workflowfile = NULL;
        }
	      if(tmp_exp->species)
        {
          free(tmp_exp->species);
          tmp_exp->species = NULL;
        }
	      if(tmp_exp->x)
        {
          free(tmp_exp->x);
          tmp_exp->x = NULL;
        }
	      if(tmp_exp->y)
        {
          free(tmp_exp->y);
          tmp_exp->y = NULL;
        }
        tmp_exp->num_occurrences = 0;
        
        free(*exp);
        *exp = NULL;
      }
      break;
    }
    default:
    {
	    logging(LOG_ERROR, __FILE__, __LINE__, "Experiment type not recognized\n" );
	    return AGENT_ERROR_QUIT;
    }  
  }

	return AGENT_SUCCESS;
}

int get_new_exp_list(connection_struct *con, long long **expids, int **expsizes)
{
	if(!con || !expids || !expsizes)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	char      query[QUERYLEN] = {'\0'};
  int       i = 0;
	int       n = 0;
  int       num_rows = 0;
	MYSQL_RES *res;
	MYSQL_ROW row;

  /* Assure experiment pointer remains null until the end of execution*/
  *expids = NULL;

  if(db_connect(con, 0))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to connect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  /* Get number of new experiments*/
	n = snprintf(query, QUERYLEN, QUERY_METADB_GET_PENDING_EXPERIMENTS);
	if(n >= QUERYLEN)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

	if (mysql_query(con->conn, query))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

	res = mysql_store_result(con->conn);

  num_rows = mysql_num_rows(res);
	if (num_rows < 1)
	{
		logging(LOG_WARNING, __FILE__, __LINE__, "No new experiment found by query\n");
		mysql_free_result(res);
    db_disconnect(con);
		return AGENT_SUCCESS;
	}

	if (mysql_field_count(con->conn) != 2)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
		mysql_free_result(res);
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

  /* Read experiment ids */
  long long *tmp_exp = (long long *)calloc((num_rows + 1), sizeof(long long));
  if(!tmp_exp){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment id list\n" );
	  mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }
  int *tmp_sizes = (int *)calloc((num_rows + 1), sizeof(int));
  if(!tmp_exp){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment id list\n" );
       free(tmp_exp);
	  mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }

  i = 0;
	while ((row = mysql_fetch_row(res)) != NULL)
  {
    tmp_exp[i] = (long long)strtoll(row[0], NULL, 10);
    tmp_sizes[i] = (int)strtol(row[1], NULL, 10);
    i++;
  }

	mysql_free_result(res);

  if(db_disconnect(con))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Unable to disconnect with the given params\n" );
       free(tmp_exp);
       free(tmp_sizes);
		return AGENT_ERROR_MYSQL;
  }  

  *expids = tmp_exp;
  *expsizes = tmp_sizes;

  return AGENT_SUCCESS;
}

int get_new_ch_exp_list(connection_struct *con, long long **expids, char ***submissiondates)
{
	if(!con || !expids || !submissiondates)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	char      query[QUERYLEN] = {'\0'};
  int       i = 0;
	int       n = 0;
  int       num_rows = 0;
	MYSQL_RES *res;
	MYSQL_ROW row;

  /* Assure experiment pointer remains null until the end of execution*/
  *expids = NULL;
  *submissiondates = NULL;

  if(db_connect(con, 0))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to connect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  /* Get number of new experiments for CH */
	n = snprintf(query, QUERYLEN, QUERY_METADB_GET_PENDING_EXPERIMENTS_CLEARING);
	if(n >= QUERYLEN)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

	if (mysql_query(con->conn, query))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

	res = mysql_store_result(con->conn);

  num_rows = mysql_num_rows(res);
	if (num_rows < 1)
	{
		logging(LOG_WARNING, __FILE__, __LINE__, "No experiment to store found by query\n");
		mysql_free_result(res);
    db_disconnect(con);
		return AGENT_SUCCESS;
	}

	if (mysql_field_count(con->conn) != 2)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
		mysql_free_result(res);
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

  /* Read experiment ids and submission dates */
  long long *tmp_exp = (long long *)calloc((num_rows + 1), sizeof(long long));
  if(!tmp_exp){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment id list\n" );
	  mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }
  char **tmp_dates = (char **)calloc((num_rows + 1), sizeof(char *));
  if(!tmp_dates){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment id list\n" );
    free(tmp_exp);  
  	mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }

  i = 0;
	while ((row = mysql_fetch_row(res)) != NULL)
  {
    tmp_exp[i] = (long long)strtoll(row[0], NULL, 10);
    tmp_dates[i] = (char *)strndup(row[1], strlen(row[1]));
    i++;
  }

	mysql_free_result(res);

  if(db_disconnect(con))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Unable to disconnect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  *expids = tmp_exp;
  *submissiondates = tmp_dates;  

  return AGENT_SUCCESS;
}

int get_old_exp_list(connection_struct *con, long long **expids, short **expchstatus, char ***submissiondates)
{
	if(!con || !expids || !expchstatus || !submissiondates)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	char      query[QUERYLEN] = {'\0'};
  int       i = 0;
	int       n = 0;
  int       num_rows = 0;
	MYSQL_RES *res;
	MYSQL_ROW row;

  /* Assure experiment pointer remains null until the end of execution*/
  *expids = NULL;
  *expchstatus = NULL;
  *submissiondates = NULL;

  if(db_connect(con, 1))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to connect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  if(mysql_autocommit(con->conn, 0))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to stop MySQL autocommit mode\n" );
		return AGENT_ERROR_MYSQL;
  }  

  /* Get number of old experiments*/
	n = snprintf(query, QUERYLEN, QUERY_METADB_GET_OLD_EXPERIMENTS);
	if(n >= QUERYLEN)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

	if (mysql_query(con->conn, query))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

	res = mysql_store_result(con->conn);

  num_rows = mysql_num_rows(res);
	if (num_rows < 1)
	{
		logging(LOG_WARNING, __FILE__, __LINE__, "No experiment to make unavailable found by query\n");
		mysql_free_result(res);
    db_disconnect(con);
		return AGENT_SUCCESS;
	}

	if (mysql_field_count(con->conn) != 3)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
		mysql_free_result(res);
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

	n = snprintf(query, QUERYLEN, QUERY_METADB_UPDATE_OLD_EXPERIMENTS);
	if(n >= QUERYLEN)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

	if (mysql_query(con->conn, query))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}
  mysql_commit(con->conn);

  if(mysql_autocommit(con->conn, 1))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to restart MySQL autocommit mode\n" );
		return AGENT_ERROR_MYSQL;
  }  

  /* Read experiment ids */
  long long *tmp_exp = (long long *)calloc((num_rows + 1), sizeof(long long));
  if(!tmp_exp){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment id list\n" );
	  mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }
  short *tmp_expch = (short *)calloc((num_rows + 1), sizeof(short));
  if(!tmp_exp){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment ch status list\n" );
    free(tmp_exp);
	  mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }
  char **tmp_dates = (char **)calloc((num_rows + 1), sizeof(char *));
  if(!tmp_dates){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment submission date list\n" );
    free(tmp_exp);  
    free(tmp_expch);  
  	mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }

  i = 0;
	while ((row = mysql_fetch_row(res)) != NULL)
  {
    tmp_exp[i] = (long long)strtoll(row[0], NULL, 10);
    tmp_expch[i] = (short)strtol(row[1], NULL, 10);
    tmp_dates[i] = (char *)strndup(row[2], strlen(row[2]));
    i++;
  }

	mysql_free_result(res);

  if(db_disconnect(con))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Unable to disconnect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  *expids = tmp_exp;
  *expchstatus = tmp_expch;
  *submissiondates = tmp_dates;  

  return AGENT_SUCCESS;
}


int get_exp_files(connection_struct *con, long long expid, char ***files, char ***extensions, int *file_num)
{
	if(!expid || !con || !files || !extensions || !file_num)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	char      query[QUERYLEN] = {'\0'};
  int       i = 0;
	int       n = 0;
  int       num_rows = 0;
	MYSQL_RES *res;
	MYSQL_ROW row;

  /* Assure files pointer remains null until the end of execution*/
  *files = NULL;
  *extensions = NULL;
  *file_num = 0;

  if(db_connect(con, 0))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to connect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  /* Get number of experiment files */
  n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXPERIMENT_FILES, expid);
  if(n >= QUERYLEN)
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }

  if (mysql_query(con->conn, query))
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
	  return AGENT_ERROR_MYSQL;
  }

  res = mysql_store_result(con->conn);

  num_rows = mysql_num_rows(res);
  if (num_rows < 1)
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "No experiment files found by query!\n");
  	mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_SUCCESS;
  }

  if (mysql_field_count(con->conn) != 2)
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
  	mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }

  /* Read file name and extensions */
  char **tmp_files = (char **)calloc((num_rows + 1), sizeof(char *));
  if(!tmp_files){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment file list\n" );
	  mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }
  char **tmp_extensions = (char **)calloc((num_rows + 1), sizeof(char *));
  if(!tmp_extensions){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment file list\n" );
    free(tmp_files);
	  mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }

  i = 0;
	while ((row = mysql_fetch_row(res)) != NULL)
  {
    tmp_files[i] = (char * )strndup(row[0], strlen(row[0]));
    tmp_extensions[i] = (char * )strndup(row[1], strlen(row[1]));
    i++;
  }

	mysql_free_result(res);

  if(db_disconnect(con))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Unable to disconnect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  *files = tmp_files;
  *extensions = tmp_extensions;
  *file_num = num_rows;

  return AGENT_SUCCESS;
}

int  get_experiment_info(connection_struct *con, long long expid, exp_type *type, void **exp)
{
	if(!expid || !con || !exp || !type )
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	char      query[QUERYLEN] = {'\0'};
	int       n = 0;
  unsigned int i = 0;
  int       num_rows = 0;
	MYSQL_RES *res;
	MYSQL_ROW row;
  exp_type  tmp_type = 0;

  static const char *exp_types_map[] = { "climatesebal", "sebalinterannual", "modelintercomparison","relheight","lidarintercomparison","enm"};  

  if(db_connect(con, 0))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to connect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  /* Get experiment type */
  n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXPERIMENT_TYPE, expid);
  if(n >= QUERYLEN)
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }

  if (mysql_query(con->conn, query))
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
	  return AGENT_ERROR_MYSQL;
  }

  res = mysql_store_result(con->conn);

  num_rows = mysql_num_rows(res);
  if (num_rows != 1)
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "No info found for given experiment!\n");
  	mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }
  if (mysql_field_count(con->conn) != 1)
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
  	mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }

  row = mysql_fetch_row(res);
  if (row == NULL)
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found\n");
  	mysql_free_result(res);
    db_disconnect(con);
    return AGENT_ERROR_MYSQL;
  }

  for(i = 0; i < sizeof(exp_types_map); i++)
  {
    if(!strncasecmp(row[0], exp_types_map[i], MAX_LENGTH(row[0], exp_types_map[i])))
    {
        tmp_type = i + 1;
        break;    
    }
  }
  if(i >= sizeof(exp_types_map))
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Type %s not recognized\n", row[0]);
  	mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }
  
	mysql_free_result(res);

  /* Initialize experiment structure */
  void *tmp_exp = NULL;
  if(init_experiment(&tmp_exp, tmp_type))
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to create struct for experiment type %d\n", tmp_type);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }
 
  /* Select experiment info */
  switch(tmp_type)
  {
    case EXP_SEBALINTERANNUAL:
    {
      n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXP_SEBALINTERANNUAL_INFO, expid);
      if(n >= QUERYLEN)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_QUIT;
      }

      if (mysql_query(con->conn, query))
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_MYSQL;
      }

      res = mysql_store_result(con->conn);

      num_rows = mysql_num_rows(res);
      if (num_rows < 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      else if(num_rows > 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Too many experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      if (mysql_field_count(con->conn) != 19)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }

      /* Read experiment info */
      row = mysql_fetch_row(res);
      if (row == NULL)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_MYSQL;
      }

      ((experiment_sebalinterannual*)tmp_exp)->workflowcode = (char * )strndup(row[0], strlen(row[0]));
      ((experiment_sebalinterannual*)tmp_exp)->workflowfile = (char * )strndup(row[1], strlen(row[1]));
      ((experiment_sebalinterannual*)tmp_exp)->bbox[0] = (double)strtod(row[2], NULL);
      ((experiment_sebalinterannual*)tmp_exp)->bbox[1] = (double)strtod(row[3], NULL);
      ((experiment_sebalinterannual*)tmp_exp)->bbox[2] = (double)strtod(row[4], NULL);
      ((experiment_sebalinterannual*)tmp_exp)->bbox[3] =(double)strtod(row[5], NULL);
      ((experiment_sebalinterannual*)tmp_exp)->timeinterval[0] = (char * )strndup(row[6], strlen(row[6]));
      ((experiment_sebalinterannual*)tmp_exp)->timeinterval[1] = (char * )strndup(row[7], strlen(row[7]));
      ((experiment_sebalinterannual*)tmp_exp)->filename = (char * )strndup(row[8], strlen(row[8]));
      ((experiment_sebalinterannual*)tmp_exp)->folder = (char * )strndup(row[9], strlen(row[9]));
      ((experiment_sebalinterannual*)tmp_exp)->coordinate = (char * )strndup(row[10], strlen(row[10]));
      ((experiment_sebalinterannual*)tmp_exp)->timerange = (char * )(row[11] ? strndup(row[11], strlen(row[11])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_sebalinterannual*)tmp_exp)->timefrequency = (char * )(row[12] ? strndup(row[12], strlen(row[12])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_sebalinterannual*)tmp_exp)->basetime = (char * )(row[13] ? strndup(row[13], strlen(row[13])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_sebalinterannual*)tmp_exp)->units = (char * )(row[14] ? strndup(row[14], strlen(row[14])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_sebalinterannual*)tmp_exp)->calendar = (char * )(row[15] ? strndup(row[15], strlen(row[15])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_sebalinterannual*)tmp_exp)->variable = (char * )strndup(row[16], strlen(row[16]));
      ((experiment_sebalinterannual*)tmp_exp)->variablename = (char * )strndup(row[17], strlen(row[17]));
      ((experiment_sebalinterannual*)tmp_exp)->pdasid = (char * )strndup(row[18], strlen(row[18]));

    	mysql_free_result(res);
      break;
    }
    case EXP_CLIMATESEBAL:
    {
      n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXP_CLIMATESEBAL_INFO_COMMON, expid);
      if(n >= QUERYLEN)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_QUIT;
      }

      if (mysql_query(con->conn, query))
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_MYSQL;
      }

      res = mysql_store_result(con->conn);

      num_rows = mysql_num_rows(res);
      if (num_rows < 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      else if(num_rows > 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Too many experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      if (mysql_field_count(con->conn) != 8)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }

      /* Read experiment info */
      row = mysql_fetch_row(res);
      if (row == NULL)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_MYSQL;
      }

      ((experiment_climatesebal*)tmp_exp)->workflowcode = (char * )strndup(row[0], strlen(row[0]));
      ((experiment_climatesebal*)tmp_exp)->workflowfile = (char * )strndup(row[1], strlen(row[1]));
      ((experiment_climatesebal*)tmp_exp)->bbox[0] = (double)strtod(row[2], NULL);
      ((experiment_climatesebal*)tmp_exp)->bbox[1] = (double)strtod(row[3], NULL);
      ((experiment_climatesebal*)tmp_exp)->bbox[2] = (double)strtod(row[4], NULL);
      ((experiment_climatesebal*)tmp_exp)->bbox[3] =(double)strtod(row[5], NULL);
      ((experiment_climatesebal*)tmp_exp)->timeinterval[0] = (char * )strndup(row[6], strlen(row[6]));
      ((experiment_climatesebal*)tmp_exp)->timeinterval[1] = (char * )strndup(row[7], strlen(row[7]));

    	mysql_free_result(res);

      n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXP_CLIMATESEBAL_INFO_SATELLITE, expid);
      if(n >= QUERYLEN)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_QUIT;
      }

      if (mysql_query(con->conn, query))
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_MYSQL;
      }

      res = mysql_store_result(con->conn);

      num_rows = mysql_num_rows(res);
      if (num_rows < 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      else if(num_rows > 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Too many experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      if (mysql_field_count(con->conn) != 11)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }

      /* Read job info */
      row = mysql_fetch_row(res);
      if (row == NULL)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_MYSQL;
      }

      ((experiment_climatesebal*)tmp_exp)->satellite_filename = (char * )strndup(row[0], strlen(row[0]));
      ((experiment_climatesebal*)tmp_exp)->satellite_folder = (char * )strndup(row[1], strlen(row[1]));
      ((experiment_climatesebal*)tmp_exp)->satellite_coordinate = (char * )strndup(row[2], strlen(row[2]));
      ((experiment_climatesebal*)tmp_exp)->satellite_timerange = (char * )(row[3] ? strndup(row[3], strlen(row[3])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_climatesebal*)tmp_exp)->satellite_timefrequency = (char * )(row[4] ? strndup(row[4], strlen(row[4])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_climatesebal*)tmp_exp)->satellite_basetime = (char * )(row[5] ? strndup(row[5], strlen(row[5])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_climatesebal*)tmp_exp)->satellite_units = (char * )(row[6] ? strndup(row[6], strlen(row[6])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_climatesebal*)tmp_exp)->satellite_calendar = (char * )(row[7] ? strndup(row[7], strlen(row[7])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_climatesebal*)tmp_exp)->satellite_variable = (char * )strndup(row[8], strlen(row[8]));
      ((experiment_climatesebal*)tmp_exp)->satellite_variablename = (char * )strndup(row[9], strlen(row[9]));
      ((experiment_climatesebal*)tmp_exp)->satellite_pdasid = (char * )strndup(row[10], strlen(row[10]));

    	mysql_free_result(res);

      n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXP_CLIMATESEBAL_INFO_CLIMATE, expid);
      if(n >= QUERYLEN)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_QUIT;
      }

      if (mysql_query(con->conn, query))
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_MYSQL;
      }

      res = mysql_store_result(con->conn);

      num_rows = mysql_num_rows(res);
      if (num_rows < 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      else if(num_rows > 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Too many experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      if (mysql_field_count(con->conn) != 10)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }

      /* Read experiment info */
      row = mysql_fetch_row(res);
      if (row == NULL)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_MYSQL;
      }

      ((experiment_climatesebal*)tmp_exp)->climate_filename = (char * )strndup(row[0], strlen(row[0]));
      ((experiment_climatesebal*)tmp_exp)->climate_folder = (char * )strndup(row[1], strlen(row[1]));
      ((experiment_climatesebal*)tmp_exp)->climate_coordinate = (char * )strndup(row[2], strlen(row[2]));
      ((experiment_climatesebal*)tmp_exp)->climate_timerange = (char * )(row[3] ? strndup(row[3], strlen(row[3])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_climatesebal*)tmp_exp)->climate_timefrequency = (char * )(row[4] ? strndup(row[4], strlen(row[4])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_climatesebal*)tmp_exp)->climate_basetime = (char * )(row[5] ? strndup(row[5], strlen(row[5])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_climatesebal*)tmp_exp)->climate_units = (char * )(row[6] ? strndup(row[6], strlen(row[6])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_climatesebal*)tmp_exp)->climate_calendar = (char * )(row[7] ? strndup(row[7], strlen(row[7])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_climatesebal*)tmp_exp)->climate_variable = (char * )strndup(row[8], strlen(row[8]));
      ((experiment_climatesebal*)tmp_exp)->climate_variablename = (char * )strndup(row[9], strlen(row[9]));

    	mysql_free_result(res);

      break;
    }   
    case EXP_MODELINTERCOMPARISON:
    {
      n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXP_MODELINTERCOMPARISON_INFO_COMMON, expid);
      if(n >= QUERYLEN)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_QUIT;
      }

      if (mysql_query(con->conn, query))
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_MYSQL;
      }

      res = mysql_store_result(con->conn);

      num_rows = mysql_num_rows(res);
      if (num_rows < 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      else if(num_rows > 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Too many experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      if (mysql_field_count(con->conn) != 12)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }

      /* Read experiment info */
      row = mysql_fetch_row(res);
      if (row == NULL)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_MYSQL;
      }

      ((experiment_modelintercomparison*)tmp_exp)->workflowcode = (char * )strndup(row[0], strlen(row[0]));
      ((experiment_modelintercomparison*)tmp_exp)->workflowfile = (char * )strndup(row[1], strlen(row[1]));
      ((experiment_modelintercomparison*)tmp_exp)->bbox[0] = (double)strtod(row[2], NULL);
      ((experiment_modelintercomparison*)tmp_exp)->bbox[1] = (double)strtod(row[3], NULL);
      ((experiment_modelintercomparison*)tmp_exp)->bbox[2] = (double)strtod(row[4], NULL);
      ((experiment_modelintercomparison*)tmp_exp)->bbox[3] =(double)strtod(row[5], NULL);
      ((experiment_modelintercomparison*)tmp_exp)->timeinterval[0] = (char * )strndup(row[6], strlen(row[6]));
      ((experiment_modelintercomparison*)tmp_exp)->timeinterval[1] = (char * )strndup(row[7], strlen(row[7]));
      ((experiment_modelintercomparison*)tmp_exp)->indicators[0] = ((int)strtol(row[8], NULL, 10) == 1 ? (char * )strndup("TNn", strlen("TNn")) : NULL);
      ((experiment_modelintercomparison*)tmp_exp)->indicators[1] = ((int)strtol(row[9], NULL, 10) == 1 ? (char * )strndup("TNx", strlen("TNx")) : NULL);
      ((experiment_modelintercomparison*)tmp_exp)->indicators[2] = ((int)strtol(row[10], NULL, 10) == 1 ? (char * )strndup("TXn", strlen("TXn")) : NULL);
      ((experiment_modelintercomparison*)tmp_exp)->indicators[3] = ((int)strtol(row[11], NULL, 10) == 1 ? (char * )strndup("TXx", strlen("TXx")) : NULL);

    	mysql_free_result(res);

      n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXP_MODELINTERCOMPARISON_INFO_DATA);
      if(n >= QUERYLEN)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_QUIT;
      }

      if (mysql_query(con->conn, query))
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_MYSQL;
      }

      res = mysql_store_result(con->conn);

      num_rows = mysql_num_rows(res);
      if (num_rows < 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      else if(num_rows > 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Too many experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      if (mysql_field_count(con->conn) != 7)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }

      /* Read experiment info */
      row = mysql_fetch_row(res);
      if (row == NULL)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_MYSQL;
      }

      ((experiment_modelintercomparison*)tmp_exp)->folder = (char * )strndup(row[0], strlen(row[0]));
      ((experiment_modelintercomparison*)tmp_exp)->coordinate = (char * )strndup(row[1], strlen(row[1]));
      ((experiment_modelintercomparison*)tmp_exp)->timerange = (char * )(row[2] ? strndup(row[2], strlen(row[2])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_modelintercomparison*)tmp_exp)->timefrequency = (char * )(row[3] ? strndup(row[3], strlen(row[3])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_modelintercomparison*)tmp_exp)->basetime = (char * )(row[4] ? strndup(row[4], strlen(row[4])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_modelintercomparison*)tmp_exp)->units = (char * )(row[5] ? strndup(row[5], strlen(row[5])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));
      ((experiment_modelintercomparison*)tmp_exp)->calendar = (char * )(row[6] ? strndup(row[6], strlen(row[6])) : strndup(DB_NULL_FIELD, strlen(DB_NULL_FIELD)));

    	mysql_free_result(res);

      n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXP_MODELINTERCOMPARISON_INFO_MODEL);
      if(n >= QUERYLEN)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_QUIT;
      }

      if (mysql_query(con->conn, query))
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_MYSQL;
      }

      res = mysql_store_result(con->conn);

      num_rows = mysql_num_rows(res);
      if (num_rows < 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      if (mysql_field_count(con->conn) != 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }

      /* Read experiment info */
      ((experiment_modelintercomparison*)tmp_exp)->models = (char **)calloc(num_rows+1,sizeof(char*));
      if(!((experiment_modelintercomparison*)tmp_exp)->models)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment structure\n" );
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
        return AGENT_ERROR_QUIT;
      }

      i = 0;
	    while ((row = mysql_fetch_row(res)) != NULL)
      {
        ((experiment_modelintercomparison*)tmp_exp)->models[i] = (char * )strndup(row[0], strlen(row[0]));
        i++;
      }

    	mysql_free_result(res);

      n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXP_MODELINTERCOMPARISON_INFO_SCENARIO);
      if(n >= QUERYLEN)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_QUIT;
      }

      if (mysql_query(con->conn, query))
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_MYSQL;
      }

      res = mysql_store_result(con->conn);

      num_rows = mysql_num_rows(res);
      if (num_rows < 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      if (mysql_field_count(con->conn) != 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }

      /* Read experiment info */
      ((experiment_modelintercomparison*)tmp_exp)->scenarios = (char **)calloc(num_rows+1,sizeof(char*));
      if(!((experiment_modelintercomparison*)tmp_exp)->scenarios)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment structure\n" );
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
        return AGENT_ERROR_QUIT;
      }
      i = 0;
	    while ((row = mysql_fetch_row(res)) != NULL)
      {
        ((experiment_modelintercomparison*)tmp_exp)->scenarios[i] = (char * )strndup(row[0], strlen(row[0]));
        i++;
      }

    	mysql_free_result(res);
      break;
    }
    case EXP_RELATIVEHEIGHT:
    {
      n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXP_RELHEIGHT_INFO, expid);
      if(n >= QUERYLEN)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_QUIT;
      }

      if (mysql_query(con->conn, query))
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_MYSQL;
      }

      res = mysql_store_result(con->conn);

      num_rows = mysql_num_rows(res);
      if (num_rows < 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      else if(num_rows > 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Too many experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      if (mysql_field_count(con->conn) != 5)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }

      /* Read experiment info */
      row = mysql_fetch_row(res);
      if (row == NULL)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_MYSQL;
      }

      ((experiment_relheight*)tmp_exp)->workflowcode = (char * )strndup(row[0], strlen(row[0]));
      ((experiment_relheight*)tmp_exp)->workflowfile = (char * )strndup(row[1], strlen(row[1]));
      ((experiment_relheight*)tmp_exp)->filename = (char * )strndup(row[2], strlen(row[2]));
      ((experiment_relheight*)tmp_exp)->folder = (char * )strndup(row[3], strlen(row[3]));
      ((experiment_relheight*)tmp_exp)->coordinate = (char * )strndup(row[4], strlen(row[4]));

    	mysql_free_result(res);
      break;
    }
    case EXP_LIDARINTERCOMPARISON:
    {
      n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXP_LIDARINTERCOMPARISON_INFO, expid);
      if(n >= QUERYLEN)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_QUIT;
      }

      if (mysql_query(con->conn, query))
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_MYSQL;
      }

      res = mysql_store_result(con->conn);

      num_rows = mysql_num_rows(res);
      if (num_rows < 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      else if(num_rows > 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Too many experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      if (mysql_field_count(con->conn) != 14)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }

      /* Read experiment info */
      row = mysql_fetch_row(res);
      if (row == NULL)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_MYSQL;
      }

      ((experiment_lidarintercomparison*)tmp_exp)->workflowcode = (char * )strndup(row[0], strlen(row[0]));
      ((experiment_lidarintercomparison*)tmp_exp)->workflowfile = (char * )strndup(row[1], strlen(row[1]));
      ((experiment_lidarintercomparison*)tmp_exp)->filename = (char * )strndup(row[2], strlen(row[2]));
      ((experiment_lidarintercomparison*)tmp_exp)->folder = (char * )strndup(row[3], strlen(row[3]));
      ((experiment_lidarintercomparison*)tmp_exp)->coordinate = (char * )strndup(row[4], strlen(row[4]));
      ((experiment_lidarintercomparison*)tmp_exp)->products[0] = ((int)strtol(row[5], NULL, 10) == 1 ? (char * )strndup("dtm", strlen("dtm")) : NULL);
      ((experiment_lidarintercomparison*)tmp_exp)->products[1] = ((int)strtol(row[6], NULL, 10) == 1 ? (char * )strndup("dsm", strlen("dsm")) : NULL);
      ((experiment_lidarintercomparison*)tmp_exp)->products[2] = ((int)strtol(row[7], NULL, 10) == 1 ? (char * )strndup("chm", strlen("chm")) : NULL);
      ((experiment_lidarintercomparison*)tmp_exp)->products[3] = ((int)strtol(row[8], NULL, 10) == 1 ? (char * )strndup("rh50", strlen("rh50")) : NULL);
      ((experiment_lidarintercomparison*)tmp_exp)->products[4] = ((int)strtol(row[9], NULL, 10) == 1 ? (char * )strndup("agb", strlen("agb")) : NULL);
      ((experiment_lidarintercomparison*)tmp_exp)->products[5] = ((int)strtol(row[10], NULL, 10) == 1 ? (char * )strndup("cover", strlen("cover")) : NULL);
      ((experiment_lidarintercomparison*)tmp_exp)->products[6] = ((int)strtol(row[11], NULL, 10) == 1 ? (char * )strndup("aspect", strlen("aspect")) : NULL);
      ((experiment_lidarintercomparison*)tmp_exp)->products[7] = ((int)strtol(row[12], NULL, 10) == 1 ? (char * )strndup("slope", strlen("slope")) : NULL);
      ((experiment_lidarintercomparison*)tmp_exp)->products[8] = ((int)strtol(row[13], NULL, 10) == 1 ? (char * )strndup("pd", strlen("pd")) : NULL);

    	mysql_free_result(res);
      break;
    }
    case EXP_ENM:
    {
      n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXP_ENM_COMMON_INFO, expid);
      if(n >= QUERYLEN)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_QUIT;
      }

      if (mysql_query(con->conn, query))
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_MYSQL;
      }

      res = mysql_store_result(con->conn);

      num_rows = mysql_num_rows(res);
      if (num_rows < 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      else if(num_rows > 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Too many experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      if (mysql_field_count(con->conn) != 3)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }

      /* Read experiment info */
      row = mysql_fetch_row(res);
      if (row == NULL)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_MYSQL;
      }

      ((experiment_enm*)tmp_exp)->workflowcode = (char * )strndup(row[0], strlen(row[0]));
      ((experiment_enm*)tmp_exp)->workflowfile = (char * )strndup(row[1], strlen(row[1]));
      ((experiment_enm*)tmp_exp)->species = (char * )strndup(row[2], strlen(row[2]));

    	mysql_free_result(res);

      n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXP_ENM_POINTS_INFO, expid);
      if(n >= QUERYLEN)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_QUIT;
      }

      if (mysql_query(con->conn, query))
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
        free_experiment((void *)&tmp_exp, tmp_type);
        db_disconnect(con);
	      return AGENT_ERROR_MYSQL;
      }

      res = mysql_store_result(con->conn);

      num_rows = mysql_num_rows(res);
      if (num_rows < 1)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "No experiment info found by query!\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }
      if (mysql_field_count(con->conn) != 2)
      {
	      logging(LOG_ERROR, __FILE__, __LINE__, "Fields found by query is not as expected\n");
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
	      return AGENT_ERROR_QUIT;
      }

      /* Read experiment info */
      ((experiment_enm*)tmp_exp)->x = (double *)calloc(num_rows,sizeof(double*));
      ((experiment_enm*)tmp_exp)->y = (double *)calloc(num_rows,sizeof(double*));
      if(!((experiment_enm*)tmp_exp)->x || !((experiment_enm*)tmp_exp)->y)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment structure\n" );
      	mysql_free_result(res);
        db_disconnect(con);
        free_experiment((void *)&tmp_exp, tmp_type);
        return AGENT_ERROR_QUIT;
      }

      ((experiment_enm*)tmp_exp)->num_occurrences = (unsigned int)num_rows;


      i = 0;
	    while ((row = mysql_fetch_row(res)) != NULL)
      {
        ((experiment_enm*)tmp_exp)->x[i] = (double)strtod(row[0], NULL);
        ((experiment_enm*)tmp_exp)->y[i] = (double)strtod(row[1], NULL);

        i++;
      }

    	mysql_free_result(res);
      break;
    }
    default:
    {
      db_disconnect(con);
	    logging(LOG_ERROR, __FILE__, __LINE__, "Experiment type not recognized\n" );
	    return AGENT_ERROR_MYSQL;
    } 
  }

  if(db_disconnect(con))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Unable to disconnect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  *type = tmp_type;
  *exp = tmp_exp;

  return AGENT_SUCCESS;
}

int add_experiment_output(connection_struct *con,  long long expid, exp_type type, char **output_names)
{
	if(!con || !expid || !type || ((type == EXP_MODELINTERCOMPARISON || type == EXP_LIDARINTERCOMPARISON) && !output_names))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	char      query[QUERYLEN] = {'\0'};
	int       n = 0;
	MYSQL_RES *res;
  int       num_rows = 0;
  unsigned int i = 0;

  if(db_connect(con, 0))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to connect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  /* Check number of experiment files */
  n = snprintf(query, QUERYLEN, QUERY_METADB_GET_EXPERIMENT_FILES, expid);
  if(n >= QUERYLEN)
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }

  if (mysql_query(con->conn, query))
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
	  return AGENT_ERROR_MYSQL;
  }

  res = mysql_store_result(con->conn);

  num_rows = mysql_num_rows(res);
  if (num_rows > 0)
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Experiment files already inserted!\n");
  	mysql_free_result(res);
    db_disconnect(con);
	  return AGENT_ERROR_QUIT;
  }

	mysql_free_result(res);

  /* Execute static part */
  switch(type)
  {
    case EXP_SEBALINTERANNUAL:
    {
      n = snprintf(query, QUERYLEN, QUERY_METADB_ADD_EXPERIMENT_SEBALINTERANNUAL, expid, expid, expid, expid, expid, expid);
      break;
    }
    case EXP_CLIMATESEBAL:
    {
      n = snprintf(query, QUERYLEN, QUERY_METADB_ADD_EXPERIMENT_CLIMATESEBAL, expid, expid, expid, expid, expid, expid);
      break;
    }
    case EXP_MODELINTERCOMPARISON:
    { 
      n = snprintf(query, QUERYLEN, QUERY_METADB_ADD_EXPERIMENT_MODELINTERCOMPARISON_FIXED, expid, expid, expid, expid);
      break;
    }
    case EXP_RELATIVEHEIGHT:
    {
      n = snprintf(query, QUERYLEN, QUERY_METADB_ADD_EXPERIMENT_RELHEIGHT, expid, expid, expid, expid, expid, expid, expid, expid);
      break;
    }
    case EXP_LIDARINTERCOMPARISON:
    {
      n = snprintf(query, QUERYLEN, QUERY_METADB_ADD_EXPERIMENT_LIDARINTERCOMPARISON_FIXED, expid, expid, expid);
      break;
    }
    case EXP_ENM:
    {
      n = snprintf(query, QUERYLEN, QUERY_METADB_ADD_EXPERIMENT_ENM, expid, expid, expid,expid, expid, expid);
      break;
    }
    default:
    {
	    logging(LOG_ERROR, __FILE__, __LINE__, "Experiment type not recognized\n" );
	    return AGENT_ERROR_QUIT;
    }
  }

	if(n >= QUERYLEN)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

	if (mysql_query(con->conn, query))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}


  /* Execute dynamic part */
  switch(type)
  {
    case EXP_MODELINTERCOMPARISON:
    { 
      i = 0;
      while(output_names[i] != NULL)
      {
        n = snprintf(query, QUERYLEN, QUERY_METADB_ADD_EXPERIMENT_MODELINTERCOMPARISON_DYNAMIC, expid, i+1);

	      if(n >= QUERYLEN)
        {
		      logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
          db_disconnect(con);
		      return AGENT_ERROR_QUIT;
	      }

	      if (mysql_query(con->conn, query))
        {
		      logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
          db_disconnect(con);
		      return AGENT_ERROR_MYSQL;
	      }

        i++;
      }
      break;
    }
    case EXP_LIDARINTERCOMPARISON:
    {
      i = 0;
      while(output_names[i] != NULL)
      {
        n = snprintf(query, QUERYLEN, QUERY_METADB_ADD_EXPERIMENT_LIDARINTERCOMPARISON_DYNAMIC, expid, output_names[i], (_lidar_expand_title(output_names[i]) ? _lidar_expand_title(output_names[i]) : " "));
        if(n >= QUERYLEN)
        {
	        logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
          db_disconnect(con);
	        return AGENT_ERROR_QUIT;
        }

        if (mysql_query(con->conn, query))
        {
	        logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
          db_disconnect(con);
	        return AGENT_ERROR_MYSQL;
        }
        i++;
      }
      break;
    }
    default:
    {
      break;
    }
  }


  if(db_disconnect(con))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Unable to disconnect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  return AGENT_SUCCESS;
}

int update_exp_status(connection_struct *con,  long long expid, short int exp_status, char *wfstring, char *wid)
{
	if(!con || !expid || !exp_status || (exp_status == COMMAND_STATUS_CODE_COMPLETED && !wid) || (exp_status == COMMAND_STATUS_CODE_RUNNING && !wfstring))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}


	char      query[QUERYLEN] = {'\0'};
	int       n = 0;

  if(db_connect(con, 0))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to connect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  /* Update experiment*/
  switch(exp_status)
  {
    case COMMAND_STATUS_CODE_ASSIGNED:
    {
    	n = snprintf(query, QUERYLEN, QUERY_METADB_SET_ASSIGNED_EXPERIMENT,  expid);
      break;
    }
    case COMMAND_STATUS_CODE_COMPLETED:
    {
    	n = snprintf(query, QUERYLEN, QUERY_METADB_SET_COMPLETED_EXPERIMENT,  wid, expid);
      break;
    }
    case COMMAND_STATUS_CODE_RUNNING:
    {
    	n = snprintf(query, QUERYLEN, QUERY_METADB_SET_RUNNING_EXPERIMENT,  wfstring, expid);
      break;
    }
    case COMMAND_STATUS_CODE_ERROR:
    default:
    {
  	  n = snprintf(query, QUERYLEN, QUERY_METADB_SET_FAILED_EXPERIMENT, expid);
    }
  }
	if(n >= QUERYLEN)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

	if (mysql_query(con->conn, query))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

  if(db_disconnect(con))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Unable to disconnect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  return AGENT_SUCCESS;
}

int update_exp_ch(connection_struct *con, long long expid, short int ch_status)
{
	if(!con || !expid || !ch_status)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	char      query[QUERYLEN] = {'\0'};
	int       n = 0;

  if(db_connect(con, 0))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to connect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  /* Update experiment*/
  switch(ch_status)
  {
    case COMMAND_CHSTATUS_CODE_COMPLETED:
    {
    	n = snprintf(query, QUERYLEN, QUERY_METADB_SET_COMPLETED_EXPERIMENT_CLEARING,  expid);
      break;
    }
    case COMMAND_CHSTATUS_CODE_ERROR:
    default:
    {
  	  n = snprintf(query, QUERYLEN, QUERY_METADB_SET_FAILED_EXPERIMENT_CLEARING, expid);
    }
  }
	if(n >= QUERYLEN)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

	if (mysql_query(con->conn, query))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

  if(db_disconnect(con))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Unable to disconnect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  return AGENT_SUCCESS;
}

int add_cluster_hist_status(connection_struct *con, int tid, int job_num, short int deploy_status, char *server_add)
{
	if(!con || !server_add)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	char      query[QUERYLEN] = {'\0'};
	int       n = 0;

  if(db_connect(con, 0))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to connect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  /* Update cluster */
  n = snprintf(query, QUERYLEN, QUERY_METADB_ADD_CLUSTER_HISTORY, tid, job_num, deploy_status, server_add);
	if(n >= QUERYLEN)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

	if (mysql_query(con->conn, query))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

	if (mysql_query(con->conn, QUERY_METADB_DELETE_OLD_STATUS))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

  if(db_disconnect(con))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Unable to disconnect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  return AGENT_SUCCESS;
}

int add_cluster_curr_status(connection_struct *con, int tid, int job_num, short int deploy_status, char *server_add)
{
	if(!con || !server_add)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	char      query[QUERYLEN] = {'\0'};
	int       n = 0;

  if(db_connect(con, 0))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to connect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  if(mysql_autocommit(con->conn, 0))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to stop MySQL autocommit mode\n" );
		return AGENT_ERROR_MYSQL;
  }  

  /* Update job*/
  n = snprintf(query, QUERYLEN, QUERY_METADB_REMOVE_CLUSTER_CURRENT, tid);
	if(n >= QUERYLEN)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

	if (mysql_query(con->conn, query))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

  n = snprintf(query, QUERYLEN, QUERY_METADB_ADD_CLUSTER_CURRENT, tid, job_num, deploy_status, server_add);
	if(n >= QUERYLEN)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Size of query exceed query limit.\n");
    db_disconnect(con);
		return AGENT_ERROR_QUIT;
	}

	if (mysql_query(con->conn, query))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

  mysql_commit(con->conn);

  if(mysql_autocommit(con->conn, 1))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to restart MySQL autocommit mode\n" );
		return AGENT_ERROR_MYSQL;
  }  

  if(db_disconnect(con))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Unable to disconnect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  return AGENT_SUCCESS;
}

int clean_db_transient(connection_struct *con)
{
	if(!con)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  if(db_connect(con, 0))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to connect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

	if (mysql_query(con->conn, QUERY_METADB_DELETE_CLUSTERHIST))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

	if (mysql_query(con->conn, QUERY_METADB_DELETE_CLUSTERCURR))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

	if (mysql_query(con->conn, QUERY_METADB_UPDATE_TRANSIENT_EXEC_EXPERIMENT))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

	if (mysql_query(con->conn, QUERY_METADB_UPDATE_TRANSIENT_CH_EXPERIMENT))
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "MySQL query error: %s\n", mysql_error(con->conn));
    db_disconnect(con);
		return AGENT_ERROR_MYSQL;
	}

  if(db_disconnect(con))
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Unable to disconnect with the given params\n" );
		return AGENT_ERROR_MYSQL;
  }  

  return AGENT_SUCCESS;
}
