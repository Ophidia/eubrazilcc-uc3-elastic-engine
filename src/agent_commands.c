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

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <time.h>
#include <unistd.h>
#include "debug.h"
#include <errno.h>

#include "agent_error.h"
#include "agent_commands.h"

#define COORDINATE_DELIMITER '|'
#define COORDINATE_SEPARATOR ':'
#define TIME_SEPARATOR       '_'

/* Internal functions for bounding box - coordinate management */
int _set_boundingbox(double (*bbox)[4], char *coordinate, char **out_bbox)
{
  if(!bbox || !coordinate || !out_bbox)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}  

  /* Keep output null until end */
  *out_bbox = NULL; 
  char *tmp_bounding_box = NULL;
    
  double xmin = (*bbox)[0], xmax = (*bbox)[1], ymin = (*bbox)[2], ymax = (*bbox)[3];

	logging(LOG_DEBUG, __FILE__, __LINE__, "Bounding box before area restriction: %f:%f|%f:%f\n", ymin, ymax, xmin, xmax);
  /* First force values inside boundaries */
  if(xmin < COMMAND_MIN_LON)
  {
    xmin = COMMAND_MIN_LON;
  }
  else if(xmin > COMMAND_MAX_LON)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Bounding box exceeds area of interest\n" );
		return AGENT_ERROR_COMMAND;
	}  
  if(xmax > COMMAND_MAX_LON)
  {
    xmax = COMMAND_MAX_LON;
  }
  else if(xmax < COMMAND_MIN_LON)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Bounding box exceeds area of interest\n" );
		return AGENT_ERROR_COMMAND;
	}  
  if(ymin < COMMAND_MIN_LAT)
  {
    ymin = COMMAND_MIN_LAT;
  }
  else if(ymin > COMMAND_MAX_LAT)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Bounding box exceeds area of interest\n" );
		return AGENT_ERROR_COMMAND;
	}  
  if(ymax > COMMAND_MAX_LAT)
  {
    ymax = COMMAND_MAX_LAT;
  }
  else if(ymax < COMMAND_MIN_LAT)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Bounding box exceeds area of interest\n" );
		return AGENT_ERROR_COMMAND;
	}  
	logging(LOG_DEBUG, __FILE__, __LINE__, "Bounding after area restriction: %f:%f|%f:%f\n", ymin, ymax, xmin, xmax);

  /* Convert systems*/
  if(!strncasecmp(coordinate, COMMAND_COORDINATE_WGS84_360, MAX_LENGTH(coordinate, COMMAND_COORDINATE_WGS84_360)))
  {
      if(xmin < 0)
          xmin += 360;
      if(xmax < 0)
          xmax += 360;
  }

  size_t n = snprintf(NULL, 0, "%.10f:%.10f|%.10f:%.10f", ymin, ymax, xmin, xmax);
  tmp_bounding_box = (char *)malloc( sizeof(char) * (n+1));
  if(!tmp_bounding_box)
  {
      logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for bounding box conversion\n" );
      return AGENT_ERROR_QUIT;
  }
  snprintf(tmp_bounding_box, n+1, "%.10f:%.10f|%.10f:%.10f", ymin, ymax, xmin, xmax);

  logging(LOG_DEBUG, __FILE__, __LINE__, "New bounding box is: %s\n", tmp_bounding_box);

  *out_bbox = tmp_bounding_box;

  return AGENT_SUCCESS;
}

/* Internal functions for time management */
int _convert_date_to_day(int y, int m, int d, long long* g)
{
	if (!g )
	{
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

	int offset=0;
	if ( (y > 1582) || ( (y == 1582) && ( (m > 10) || ( (m == 10) && (d >= 15) ) ) ) ) offset = 1;
	else if ( (y < 1582) || ( (y == 1582) && ( (m < 10) || ( (m == 10) && (d <= 4) ) ) ) ) offset = -1;
	else
	{
		logging(LOG_ERROR, __FILE__, __LINE__, "Wrong date format\n" );
		return AGENT_ERROR_COMMAND;
	}

	m = (m + 9) % 12;
	y = y - m/10;
	*g = 365L*y + y/4 + (offset>0 ? - y/100 + y/400 : 0) + (m*306L + 5)/10 + ( d + offset - 2 );

	return AGENT_SUCCESS;
}

int _convert_time_filter(char *timeinterval[2], char *units, char *basetime, char** output_filter)
{
	if (!timeinterval || !units || !basetime || !output_filter)
	{
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  /* Keep output null until end */
	*output_filter = NULL;

  /* Pre-parse YYYY:YYYY values */
  char *temp_gateway_filter = (char*)malloc((strlen("YYYY-mm-dd HH:MM:SS_YYYY-mm-dd HH:MM:SS") + 1)*sizeof(char));
  if(!temp_gateway_filter)
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for time filter conversion\n" );
    return AGENT_ERROR_QUIT;
  }
  snprintf(temp_gateway_filter,strlen("YYYY-mm-dd HH:MM:SS_YYYY-mm-dd HH:MM:SS") + 1,"%s-01-01 00:00:00_%s-12-31 23:59:59", timeinterval[0], timeinterval[1]);

  /* Get base time in seconds since 0000-00-00 00:00:00 */ 
	struct tm tm_value;
	memset(&tm_value, 0, sizeof(struct tm));
	long long base_value_time = 0, value_time;
	if (basetime && strlen(basetime))
	{
		strptime(basetime, "%Y-%m-%d %H:%M:%S", &tm_value);
		tm_value.tm_year+=1900;
		tm_value.tm_mon++;
		if(_convert_date_to_day(tm_value.tm_year, tm_value.tm_mon, tm_value.tm_mday, &base_value_time))
    {
      logging(LOG_ERROR, __FILE__, __LINE__, "Unable to convert basetime\n" );
      free(temp_gateway_filter);
      return AGENT_ERROR_COMMAND;
    }
		base_value_time = tm_value.tm_sec + 60*(tm_value.tm_min + 60*(tm_value.tm_hour + 24*base_value_time));
		memset(&tm_value, 0, sizeof(struct tm));
	}

  /* Compute scaling factor based on time unit */
	double scaling_factor = 1.0, _value;
	switch (units[0])
	{
		case 'd': scaling_factor *= 4.0;
		case '6': scaling_factor *= 2.0;
		case '3': scaling_factor *= 3.0;
		case 'h': scaling_factor *= 60.0;
		case 'm': scaling_factor *= 60.0;
		case 's': break;
		default: logging(LOG_WARNING, __FILE__, __LINE__, "Unrecognized or unsupported units\n");
	}

	int n,nn,nnn;
  char *pch = NULL, *save_pointer = NULL;
  char *temp = (char*)strndup(temp_gateway_filter, strlen(temp_gateway_filter) + 1);
  if(!temp)
  {
      logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for time filter conversion\n" );
      free(temp_gateway_filter);
      return AGENT_ERROR_QUIT;
  }

  /* Validate interval bounds */
	nnn=strlen(temp);
	char separator[1+nnn];
	for (n=nn=0;n<nnn;++n)
	{
		if (temp[n]==',') separator[nn++] = ',';
		else if (temp[n]==TIME_SEPARATOR)
		{
			if (nn && (separator[nn-1]==':'))
			{
				logging(LOG_ERROR, __FILE__, __LINE__, "Interval bounds are not correctly set in '%s'.", temp);
        free(temp);
        free(temp_gateway_filter);
				return AGENT_ERROR_COMMAND;
			}
			separator[nn++] = ':';
		}
	}
	separator[nn]=0;
	n=nn=0;

  char *tmp_output_string = (char *)malloc(COMMAND_FILTER_LENGTH*sizeof(char));
  if(!tmp_output_string)
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for time filter conversion\n" );
    free(temp);
    free(temp_gateway_filter);
    return AGENT_ERROR_QUIT;
  }

	while ((pch = strtok_r(pch ? NULL : temp, ",_", &save_pointer)))
	{
		value_time = 0;
		tm_value.tm_year = -1;

		strptime(pch, "%Y-%m-%d %H:%M:%S", &tm_value);
		if (tm_value.tm_year < 0)
    {
	    logging(LOG_ERROR, __FILE__, __LINE__, "Wrong date format\n" );
      free(temp);
      free(tmp_output_string);
      free(temp_gateway_filter);
	    return AGENT_ERROR_COMMAND;
    }
		tm_value.tm_year+=1900;
		tm_value.tm_mon++;

		if(_convert_date_to_day(tm_value.tm_year, tm_value.tm_mon, tm_value.tm_mday, &value_time))
    {
      logging(LOG_ERROR, __FILE__, __LINE__, "Unable to convert time\n" );
      free(temp);
      free(tmp_output_string);
      free(temp_gateway_filter);
      return AGENT_ERROR_COMMAND;
    }
		value_time = tm_value.tm_sec + 60*(tm_value.tm_min + 60*(tm_value.tm_hour + 24*value_time)) - base_value_time;

		_value = ((double)value_time) / scaling_factor;
    n += snprintf(tmp_output_string+n, COMMAND_FILTER_LENGTH - n,"%f%c",_value,separator[nn++]);
	}

  free(temp_gateway_filter);
  free(temp);

  *output_filter = tmp_output_string;

	return AGENT_SUCCESS;
}

/* Build subset string based on bounding box, coordinate system and time info */
int _build_subset_string(double (*bbox)[4], char *coordinate, char *timeinterval[2], char *basetime, char *units, char **subset_string)
{
	if(!bbox || !coordinate || !timeinterval || !basetime || !units || !subset_string)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}  

  /* Keep output null until end */
  *subset_string = NULL; 

  char *tmp_subset = NULL;  
  char *tmp_spatial_filter = NULL;
  char *tmp_time_filter = NULL;

  int res = 0;

  res = _set_boundingbox(bbox, coordinate, &tmp_spatial_filter); 
  if( res )
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to convert bounding-box\n" );
    return res;
  }  

  res = _convert_time_filter(timeinterval, units, basetime, &tmp_time_filter);
  if( res )
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to convert time filter\n" );
    if(tmp_spatial_filter) free(tmp_spatial_filter);
    return res;
  } 

  size_t n = snprintf(NULL, 0, "%s|%s", tmp_spatial_filter, tmp_time_filter);
  tmp_subset = (char *)malloc( sizeof(char) * (n+1));
  if(!tmp_subset)
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for subset filter\n" );
    if(tmp_spatial_filter) free(tmp_spatial_filter);
    if(tmp_time_filter) free(tmp_time_filter);
    return AGENT_ERROR_QUIT;
  }
  snprintf(tmp_subset, n+1, "%s|%s", tmp_spatial_filter, tmp_time_filter);
  if(tmp_spatial_filter) free(tmp_spatial_filter);
  if(tmp_time_filter) free(tmp_time_filter);

  logging(LOG_DEBUG, __FILE__, __LINE__, "New bounding box is: %s\n", tmp_subset);

  *subset_string = tmp_subset;

  return AGENT_SUCCESS;
}

int run_cmd(char **json_string, size_t *current_size, char *cmd)
{
	if(!cmd || !json_string || !current_size)
    {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}  

  FILE *fp;
  char buffer[OUTPUT_LINE] = {'\0'};
  size_t length = 0;
  size_t used = 0;
  char *tmp_json_string = NULL;  

  /* Open the command for reading. */
  logging(LOG_DEBUG, __FILE__, __LINE__, "Command run is: %s\n", cmd);
  fp = popen(cmd, "r");
  if (fp == NULL) 
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Failed to run command: %d\n", errno );
    return AGENT_ERROR_COMMAND;
  }

  /* Read the output a line at a time */
  while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) 
  {
    /* Check if the output will fit the string buffer */
    length = strlen(buffer);
    if (used + length >= *current_size)
    {
      /* Double current_size each time */
      tmp_json_string = (char*) realloc (*json_string, (*current_size * 2 )* sizeof(char));
      if (tmp_json_string) 
      {
          *json_string = tmp_json_string;
          tmp_json_string = NULL;
          *current_size *= 2;
      }
      else 
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Error re-allocating memory\n");
        pclose(fp);
        return AGENT_ERROR_QUIT;
      }
    }
    /* Add it to the buffer */
    memcpy(*json_string + used, buffer, length);
    used += length;
    (*json_string)[used] = '\0';
  }

  pclose(fp);
  return AGENT_SUCCESS;
}

int build_experiment_cmd(configuration_struct *conf, connection_struct *con, char * serv_addr, char *out_path, long long expid, char **command)
{
	if(!conf || !serv_addr || !out_path || !expid || !command)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}  

  /* Get experiment info */
  int res = 0;
  unsigned int i = 0;

  void *experiment = NULL;
  exp_type type = 0;
  res = get_experiment_info(con, expid, &type, &experiment);
  if(res)
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to get experiment info\n" );
    return res;
  }

  char **output_names = NULL;
 
  /* Select experiment type to compute */
  char *command_pattern = NULL;
  char *tmp_cmd = NULL;
  int commandlen = 0;

  switch(type)
  {
    case EXP_CLIMATESEBAL:
    {
      experiment_climatesebal *tmp_exp = (experiment_climatesebal *)experiment;

      char *subset_string1 = NULL;
      char *subset_string2 = NULL;

      if( !strncasecmp(tmp_exp->satellite_basetime, DB_NULL_FIELD, MAX_LENGTH(tmp_exp->satellite_basetime, DB_NULL_FIELD)) || !strncasecmp(tmp_exp->climate_basetime, DB_NULL_FIELD, MAX_LENGTH(tmp_exp->climate_basetime, DB_NULL_FIELD)) || !strncasecmp(tmp_exp->satellite_units, DB_NULL_FIELD, MAX_LENGTH(tmp_exp->satellite_units, DB_NULL_FIELD)) || !strncasecmp(tmp_exp->climate_units, DB_NULL_FIELD, MAX_LENGTH(tmp_exp->climate_units, DB_NULL_FIELD)))
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to build subset string since some arguments are null\n" );
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }

      res = _build_subset_string(&(tmp_exp->bbox), tmp_exp->satellite_coordinate, tmp_exp->timeinterval, tmp_exp->satellite_basetime, tmp_exp->satellite_units, &subset_string1); 
      if( res )
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to build subset string 1\n" );
        free_experiment(&experiment, type);
        return res;
      }

      res = _build_subset_string(&(tmp_exp->bbox), tmp_exp->climate_coordinate, tmp_exp->timeinterval, tmp_exp->climate_basetime, tmp_exp->climate_units, &subset_string2); 
      if( res )
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to build subset string 2\n" );
        free(subset_string1);
        free_experiment(&experiment, type);
        return res;
      }

      command_pattern = COMMAND_CLIMATESEBAL;            
      commandlen = snprintf(NULL, 0, command_pattern, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, conf->wf_path, tmp_exp->workflowfile, expid, tmp_exp->satellite_variable, tmp_exp->climate_variable, conf->data_path, tmp_exp->satellite_folder, conf->data_path, tmp_exp->climate_folder, tmp_exp->satellite_filename, tmp_exp->climate_filename, subset_string1, subset_string2, out_path, conf->wf_path, tmp_exp->satellite_variablename, tmp_exp->climate_variablename, tmp_exp->satellite_pdasid,tmp_exp->timeinterval[0],tmp_exp->timeinterval[1]);

      tmp_cmd = (char *)malloc(sizeof(char) * (commandlen + 1) );
      if(!tmp_cmd)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment command\n" );
        free_experiment(&experiment, type);
        free(subset_string1);
        free(subset_string2);
        return AGENT_ERROR_QUIT;
      }
      snprintf(tmp_cmd, commandlen + 1, command_pattern, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, conf->wf_path, tmp_exp->workflowfile, expid, tmp_exp->satellite_variable, tmp_exp->climate_variable, conf->data_path, tmp_exp->satellite_folder, conf->data_path, tmp_exp->climate_folder, tmp_exp->satellite_filename, tmp_exp->climate_filename, subset_string1, subset_string2, out_path, conf->wf_path, tmp_exp->satellite_variablename, tmp_exp->climate_variablename, tmp_exp->satellite_pdasid,tmp_exp->timeinterval[0],tmp_exp->timeinterval[1]);


      free(subset_string1);
      free(subset_string2);
      break;
    }
    case EXP_SEBALINTERANNUAL:
    {
      experiment_sebalinterannual *tmp_exp = (experiment_sebalinterannual *)experiment;

      char *subset_string = NULL;

      if( !strncasecmp(tmp_exp->basetime, DB_NULL_FIELD, MAX_LENGTH(tmp_exp->basetime, DB_NULL_FIELD)) || !strncasecmp(tmp_exp->units, DB_NULL_FIELD, MAX_LENGTH(tmp_exp->units, DB_NULL_FIELD)))
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to build subset string since some arguments are null\n" );
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }

      res = _build_subset_string(&(tmp_exp->bbox), tmp_exp->coordinate, tmp_exp->timeinterval, tmp_exp->basetime, tmp_exp->units, &subset_string); 
      if( res )
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to build subset string 1\n" );
        free_experiment(&experiment, type);
        return res;
      }

      //Build year strings
      int year_counter = (int)strtol(tmp_exp->timeinterval[1],NULL,10) - (int)strtol(tmp_exp->timeinterval[0],NULL,10) + 1;
      int k = 0;
      long long curr_len1 = 0, curr_len2 = 0;
      int start_year = (int)strtol(tmp_exp->timeinterval[0],NULL,10);
      char year_string[5] = {'\0'};

      int actual_years = 0;
      for(k = 0; k < year_counter; k++)
      {
        snprintf(year_string, 5, "%d", start_year+k);
        if(strstr(tmp_exp->timerange, year_string))
        {
          actual_years++;          
        }
      }

      char *year_intervals = (char *)malloc(sizeof(char)*((strlen("YYYY")*2 + 1)*actual_years + (actual_years-1) + 1) );
      if(!year_intervals)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for year interval\n" );
        free(subset_string);
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }
      char *year_values = (char *)malloc(sizeof(char)*((strlen("YYYY"))*actual_years + (actual_years-1) + 1) );
      if(!year_values)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for year values\n" );
        free(subset_string);
        free(year_intervals);
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }

      int jj = 0;
      for(k = 0; k < year_counter; k++)
      {
        snprintf(year_string, 5, "%d", start_year+k);
        if(strstr(tmp_exp->timerange, year_string)){
          snprintf(year_intervals + curr_len1, (strlen("YYYY")*2 + 1) + 1,  "%d_%d",  start_year+k, start_year+k+1);    
          curr_len1 += (strlen("YYYY")*2 + 1);
          snprintf(year_values + curr_len2, strlen("YYYY") + 1,  "%d",  start_year+k);    
          curr_len2 += strlen("YYYY");
          if( jj != (actual_years - 1))
          {
            year_intervals[curr_len1] = COORDINATE_DELIMITER;    
            curr_len1++;
            year_values[curr_len2] = COORDINATE_DELIMITER;    
            curr_len2++;
          }
          jj++;
        }
      }
        logging(LOG_ERROR, __FILE__, __LINE__, "%s\n", year_values );

      command_pattern = COMMAND_SEBALINTERANNUAL;            
      commandlen = snprintf(NULL, 0, command_pattern, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, conf->wf_path, tmp_exp->workflowfile, expid, tmp_exp->variable, conf->data_path, tmp_exp->folder, tmp_exp->filename, subset_string, actual_years, year_intervals, out_path, conf->wf_path, year_values, tmp_exp->variablename, tmp_exp->pdasid);
      tmp_cmd = (char *)malloc(sizeof(char) * (commandlen + 1) );
      if(!tmp_cmd)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment command\n" );
        free(subset_string);
        free(year_intervals);
        free(year_values);
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }
      snprintf(tmp_cmd, commandlen + 1, command_pattern, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, conf->wf_path, tmp_exp->workflowfile, expid, tmp_exp->variable, conf->data_path, tmp_exp->folder, tmp_exp->filename, subset_string, actual_years, year_intervals, out_path, conf->wf_path, year_values, tmp_exp->variablename,  tmp_exp->pdasid);

      free(year_values);
      free(year_intervals);
      free(subset_string);
      break;
    }
    case EXP_MODELINTERCOMPARISON:
    {
      experiment_modelintercomparison *tmp_exp = (experiment_modelintercomparison *)experiment;

      char *subset_string = NULL;

      if(tmp_exp->indicators[0] == 0 && tmp_exp->indicators[1] == 0 && tmp_exp->indicators[2] == 0 && tmp_exp->indicators[3] == 0)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "No indicator selected in Model intercomparison\n" );
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }

      if( !strncasecmp(tmp_exp->basetime, DB_NULL_FIELD, MAX_LENGTH(tmp_exp->basetime, DB_NULL_FIELD)) || !strncasecmp(tmp_exp->units, DB_NULL_FIELD, MAX_LENGTH(tmp_exp->units, DB_NULL_FIELD)))
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to build subset string since some arguments are null\n" );
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }

      res = _build_subset_string(&(tmp_exp->bbox), tmp_exp->coordinate, tmp_exp->timeinterval, tmp_exp->basetime, tmp_exp->units, &subset_string); 
      if( res )
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to build subset string\n" );
        free_experiment(&experiment, type);
        return res;
      }

      int len = 0;
      unsigned int num_models = 0;
      while(tmp_exp->models[num_models] != NULL)
      {
        len += strlen(tmp_exp->models[num_models]);
        num_models++;
      }
      len += num_models - 1; //Add | character len;
      char *model = (char *)malloc(sizeof(char)*(len + 1));
      if(!model)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for string of models\n" );
        free(subset_string);
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }
      len = 0;
      for( i = 0; i < num_models; i++ )
      {
        snprintf(model + len, strlen(tmp_exp->models[i]) + 1, "%s", tmp_exp->models[i]);
        len += strlen(tmp_exp->models[i]);
        model[len++] = '|';
      }
      model[len-1] = '\0';

      len = 0;
      unsigned int num_scenarios = 0;
      while(tmp_exp->scenarios[num_scenarios] != NULL)
      {
        len += strlen(tmp_exp->scenarios[num_scenarios]);
        num_scenarios++;
      }
      len += num_scenarios - 1; //Add | character len;
      char *scenario = (char *)malloc(sizeof(char)*(len + 1));
      if(!scenario)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for string of scenarios\n" );
        free(model);
        free(subset_string);
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }
      len = 0;
      for( i = 0; i < num_scenarios; i++ )
      {
        snprintf(scenario + len, strlen(tmp_exp->scenarios[i]) + 1, "%s", tmp_exp->scenarios[i]);
        len += strlen(tmp_exp->scenarios[i]);
        scenario[len++] = '|';
      }
      scenario[len-1] = '\0';

      len = 0;
      unsigned int num_operations = 0;
      unsigned int num_variables = 0;
      unsigned int num_indicators = 0;
      char *variable = NULL;
      char *operation = NULL;
      /*Position of indicators are: TNn TNx TXn TXx */
      if((tmp_exp->indicators[0] != NULL || tmp_exp->indicators[1] != NULL) && (tmp_exp->indicators[2] != NULL || tmp_exp->indicators[3] != NULL) )
      { 
        variable = "tasmin|tasmax";
        num_variables = 2;
      }
      if((tmp_exp->indicators[0] != NULL || tmp_exp->indicators[1] != NULL) && !(tmp_exp->indicators[2] != NULL || tmp_exp->indicators[3] != NULL) )
      { 
        variable = "tasmin";
        num_variables = 1;
      }
      if(!(tmp_exp->indicators[0] != NULL || tmp_exp->indicators[1] != NULL) && (tmp_exp->indicators[2] != NULL || tmp_exp->indicators[3] != NULL) )
      { 
        variable = "tasmax";
        num_variables = 1;
      }
      if((tmp_exp->indicators[0] != NULL || tmp_exp->indicators[2] != NULL) && (tmp_exp->indicators[1] != NULL || tmp_exp->indicators[3] != NULL) )
      { 
        operation = "min|max";
        num_operations = 2;
      }
      if((tmp_exp->indicators[0] != NULL || tmp_exp->indicators[2] != NULL) && !(tmp_exp->indicators[1] != NULL || tmp_exp->indicators[3] != NULL) )
      { 
        operation = "min";
        num_operations = 1;
      }
      if(!(tmp_exp->indicators[0] != NULL || tmp_exp->indicators[2] != NULL) && (tmp_exp->indicators[1] != NULL || tmp_exp->indicators[3] != NULL) )
      { 
        operation = "max";
        num_operations = 1;
      }

      output_names = (char **)calloc(5,sizeof(char*));
      if(!output_names)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for output names string\n" );
        free(model);
        free(scenario);
        free(subset_string);
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }


      for(i = 0; i < 4; i++)
      { 
        if(tmp_exp->indicators[i] != NULL)
        {
          len += strlen(tmp_exp->indicators[i]);
          output_names[num_indicators] = (char * )strndup(tmp_exp->indicators[i], strlen(tmp_exp->indicators[i]));          
          num_indicators++;
        }
      }
      len += num_indicators - 1; //Add | character len;
      char *indicator = (char *)malloc(sizeof(char)*(len + 1));
      if(!indicator)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for indicator string\n" );
        free(model);
        free(scenario);
        free(subset_string);
        if(output_names)
        {
          i = 0;
          while(output_names[i]) free(output_names[i++]);
          free(output_names);
        }
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }
      len = 0;
      for(i = 0; i < 4; i++)
      { 
        if(tmp_exp->indicators[i] != NULL)
        {
          snprintf(indicator + len, strlen(tmp_exp->indicators[i]) + 1, "%s", tmp_exp->indicators[i]);
          len += strlen(tmp_exp->indicators[i]);
          indicator[len++] = '|';
        }
      }
      indicator[len-1] = '\0';

      command_pattern = COMMAND_MODELINTERCOMPARISON;            
      commandlen = snprintf(NULL, 0, command_pattern, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, conf->wf_path, tmp_exp->workflowfile, expid, model, num_models, scenario, num_scenarios, variable, num_variables, operation, num_operations, conf->data_path, tmp_exp->folder, subset_string, out_path, conf->wf_path, indicator);
      tmp_cmd = (char *)malloc(sizeof(char) * (commandlen + 1) );
      if(!tmp_cmd)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment command\n" );
        free(model);
        free(scenario);
        free(subset_string);
        free(indicator);
        if(output_names)
        {
          i = 0;
          while(output_names[i]) free(output_names[i++]);
          free(output_names);
        }
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }
      snprintf(tmp_cmd, commandlen + 1, command_pattern, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, conf->wf_path, tmp_exp->workflowfile, expid, model, num_models, scenario, num_scenarios, variable, num_variables, operation, num_operations, conf->data_path, tmp_exp->folder, subset_string, out_path, conf->wf_path, indicator);

      free(model);
      free(scenario);
      free(indicator);
      free(subset_string);
      break;
    }
    case EXP_RELATIVEHEIGHT:
    {
      experiment_relheight *tmp_exp = (experiment_relheight *)experiment;

      command_pattern = COMMAND_RELHEIGHT;            
      commandlen = snprintf(NULL, 0, command_pattern, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, conf->wf_path, tmp_exp->workflowfile, expid, conf->data_path, tmp_exp->folder, tmp_exp->filename, conf->wf_path, out_path);
      tmp_cmd = (char *)malloc(sizeof(char) * (commandlen + 1) );
      if(!tmp_cmd)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment command\n" );
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }
      snprintf(tmp_cmd, commandlen + 1,  command_pattern, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, conf->wf_path, tmp_exp->workflowfile, expid, conf->data_path, tmp_exp->folder, tmp_exp->filename, conf->wf_path, out_path);

      break;
    }
    case EXP_LIDARINTERCOMPARISON:
    {
      experiment_lidarintercomparison *tmp_exp = (experiment_lidarintercomparison *)experiment;

      output_names = (char **)calloc(3,sizeof(char*));
      if(!output_names)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for output names\n" );
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }

      int len = 0;
      unsigned int num_products = 0;

      for(i = 0; i < 9; i++)
      { 
        if(tmp_exp->products[i] != NULL)
        {
          len += strlen(tmp_exp->products[i]);
          output_names[num_products] = (char * )strndup(tmp_exp->products[i], strlen(tmp_exp->products[i]));          
          num_products++;
        }
      }
      len += 3*num_products - 1; //Add '', characters to len;
      char *product = (char *)malloc(sizeof(char)*(len + 1));
      if(!product)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for product names\n" );
        if(output_names)
        {
          i = 0;
          while(output_names[i]) free(output_names[i++]);
          free(output_names);
        }
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }
      len = 0;
      for(i = 0; i < 9; i++)
      { 
        if(tmp_exp->products[i] != NULL)
        {
          snprintf(product + len, strlen(tmp_exp->products[i]) + 2 + 1, "'%s'", tmp_exp->products[i]);
          len += strlen(tmp_exp->products[i]) + 2;
          product[len++] = ',';
        }
      }
      product[len -1] = '\0';


      command_pattern = COMMAND_LIDARINTERCOMPARISON;            
      commandlen = snprintf(NULL, 0, command_pattern, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, conf->wf_path, tmp_exp->workflowfile, expid, conf->data_path, tmp_exp->folder, tmp_exp->filename, conf->wf_path, out_path, product);
      tmp_cmd = (char *)malloc(sizeof(char) * (commandlen + 1) );
      if(!tmp_cmd)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment command\n" );
        if(output_names)
        {
          i = 0;
          while(output_names[i]) free(output_names[i++]);
          free(output_names);
        }
        free(product);
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }
      snprintf(tmp_cmd, commandlen + 1,  command_pattern, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, conf->wf_path, tmp_exp->workflowfile, expid, conf->data_path, tmp_exp->folder, tmp_exp->filename, conf->wf_path, out_path, product);

      free(product);
      break;
    }
    case EXP_ENM:
    {
      experiment_enm *tmp_exp = (experiment_enm *)experiment;

      int len = 0;
      char *occurrence_file = NULL;
      len = snprintf(NULL, 0, "%s/input.csv", out_path);

      occurrence_file = (char *)malloc(sizeof(char)*(len+1));
      if(!occurrence_file)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for occurence file name\n" );
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }
      snprintf(occurrence_file, len + 1, "%s/input.csv", out_path);

      FILE *fp = fopen(occurrence_file, "w");
      if (fp == NULL) 
      {
		    logging(LOG_ERROR, __FILE__, __LINE__, "Failed to open file: %d\n", errno );
        free_experiment(&experiment, type);
        free(occurrence_file);
        return AGENT_ERROR_QUIT;
      }

      /* Write x,y a line at a time */
      for ( i= 0; i < tmp_exp->num_occurrences; i++) 
      {
         fprintf (fp, "%f %f\n", tmp_exp->x[i], tmp_exp->y[i]);
      }

      fclose(fp);
      free(occurrence_file);

      //Remove & from species name
      char *orig = tmp_exp->species;
      char *new = tmp_exp->species;
      while(*orig){
        if (*orig != '&'){
          *new = *orig;
          new++;
        }
        orig++;
      }
      *new = '\0';

      command_pattern = COMMAND_ENM;            
      commandlen = snprintf(NULL, 0, command_pattern, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, conf->wf_path, tmp_exp->workflowfile, expid, tmp_exp->species, out_path, "input.csv", conf->wf_path, out_path);
      tmp_cmd = (char *)malloc(sizeof(char) * (commandlen + 1) );
      if(!tmp_cmd)
      {
        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment command\n" );
        free_experiment(&experiment, type);
        return AGENT_ERROR_QUIT;
      }
      snprintf(tmp_cmd, commandlen + 1, command_pattern, conf->term_exec, serv_addr, conf->term_port, conf->term_user, conf->term_pass, conf->wf_path, tmp_exp->workflowfile, expid, tmp_exp->species, out_path, "input.csv", conf->wf_path, out_path);

      break;
    }
    default:
    {
      logging(LOG_ERROR, __FILE__, __LINE__, "Unknown indicator\n" );
      free_experiment(&experiment, type);
      return AGENT_ERROR_COMMAND;
    }
  }

  free_experiment(&experiment, type);

  /*Insert output files of experiment*/
  if(add_experiment_output(con, expid, type, output_names))
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to add output of experiment\n" );
    free(tmp_cmd);
    if(output_names)
    {
      i = 0;
      while(output_names[i]) free(output_names[i++]);
      free(output_names);
    }
    return AGENT_ERROR_QUIT;
  }

  if(output_names)
  {
    i = 0;
    while(output_names[i]) free(output_names[i++]);
    free(output_names);
  }

  *command = tmp_cmd;

  return AGENT_SUCCESS;
}
