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
#include "debug.h"
#include <errno.h>

#include "agent_error.h"
#include "agent_commands.h"
#include "agent_folder.h"

#include <dirent.h>

extern int msglevel;

/* Make recursive directory function*/
int folder_rec_mkdir(const char *dir) 
{
	if(!dir)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  char tmp_path[FS_MAX_PATH_LEN];
  char *c = NULL;
	struct stat st;
	int res;
  size_t len = snprintf(tmp_path, sizeof(tmp_path),"%s",dir);

  /* Check if path exists */
  if(stat(dir,&st))
  {
    /* Strip last slash */    
    if(tmp_path[len - 1] == '/')
        tmp_path[len - 1] = 0;

    /* Create recursively folders */  
    for(c = tmp_path + 1; *c != 0 ; c++){
      if(*c == '/') {
    		*c = 0;
	      if(stat(tmp_path,&st)){
        	if((res = mkdir(tmp_path, S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH)))
          {
        		logging(LOG_ERROR, __FILE__, __LINE__, "Error %d while creating %s\n", res, tmp_path);
		        return res;
          }
        }
        *c = '/';
      }
	  }
    
    /* Create last folder */  
	  if(stat(tmp_path,&st)){
    	if((res = mkdir(tmp_path, S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH)))
      {
    		logging(LOG_ERROR, __FILE__, __LINE__, "Error %d while creating %s\n", res, tmp_path);
		    return res;
      }
	  }
  }
  else
  {
		logging(LOG_WARNING, __FILE__, __LINE__, "Directory %s exists already\n", dir);
  }
      
  return AGENT_SUCCESS;
}

/* List files in directory function*/
int folder_get_files_in_dir(char *dirpath, char ***file_list, int *file_number) 
{
	if(!dirpath || !file_list || !file_number)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  int n, i, j;
  struct dirent **ents;
  DIR *dir;
  struct dirent *ent;
  int file_count = 0;

  /* Assure file_list is NULL until end */
  *file_list = NULL;
  *file_number = 0;

  /* Count regular files into directory */
  if ((dir = opendir (dirpath)) != NULL) 
  {
    while ((ent = readdir (dir)) != NULL) 
    {
      if (ent->d_type == DT_REG)  
        file_count++;
    }
    closedir (dir);
  }
  else 
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Could not open directory %s\n", dirpath );
	  return AGENT_ERROR_QUIT;
  }

  /* No files found */
  if( file_count == 0 )
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "No files found in directory %s\n", dirpath );
	  return AGENT_SUCCESS;

  }

  char **tmp_files = (char **)calloc((file_count + 1), sizeof(char *));
  if(!tmp_files){
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment file list\n" );
	  return AGENT_ERROR_QUIT;
  }

  if ((n = scandir(dirpath,&ents,0,versionsort)) >= 0) 
  {
    j = 0;
    for (i = 0; i < n; i++) 
    {
      if (ents[i]->d_type == DT_REG)
      {  
        tmp_files[j] = (char * )strndup(ents[i]->d_name, strlen(ents[i]->d_name));
        if(!tmp_files[j]){
	        logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for experiment file %s in list\n", ents[i]->d_name);
          for (i = 0; i < j; i++) free(tmp_files[j]);
          free(tmp_files);
	        return AGENT_ERROR_QUIT;
        }
        j++;
      }
      free(ents[i]);
    }
    free(ents);
  } 
  else 
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Could not open directory %s\n", dirpath );
    if(tmp_files) free(tmp_files);
    return AGENT_ERROR_QUIT;
  }

  *file_list = tmp_files;
  *file_number = file_count;

  return AGENT_SUCCESS;
}

/* Callback function (nftw) to delete a file or empty folder */
int folder_delete_files(const char *path, const struct stat *st, int flag, struct FTW *ftw)
{
  int res = 0;
  
  if ((res = remove(path)) != 0)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Unable to remove path %s, error: %d\n", path, errno );
	}
  return res;
}

/* Check that experiment output is available in the folder */
int folder_check_file_existance(connection_struct *con, long long expid, char *out_path)
{
	if(!con || !expid || !out_path)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  char **file_list1 = NULL;
  char **ext_list1 = NULL;
  char **file_list2 = NULL;
  int file_num1 = 0;
  int file_num2 = 0;
  int i = 0, j = 0;
  int res = 0;

  res = get_exp_files(con, expid, &file_list1, &ext_list1, &file_num1);
  /* If list is empty set error */
  if(res == 0 && file_num1 == 0 ) res = AGENT_ERROR_EXECUTION;
  if( res )
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to get output file names\n" );
    return res;
  }

  res = folder_get_files_in_dir(out_path, &file_list2, &file_num2);  
  /* If list is empty set error */
  if(res == 0 && file_num2 == 0 ) res = AGENT_ERROR_EXECUTION;
  if( res )
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to get output files from path\n" );
    for(i = 0; i < file_num1; i++) 
    {
      if(file_list1[i]) free(file_list1[i]);
      if(ext_list1[i]) free(ext_list1[i]);
    }          
    free(file_list1);
    free(ext_list1);
    return res;
  }


  /* Compare output lists */
  for(j = 0; j < file_num1; j++) 
  {
    for(i = 0; i < file_num2; i++) 
    {
      if(!strncasecmp(file_list2[i], file_list1[j], MAX_LENGTH(file_list2[i], file_list1[j])))
        break;
    }

    if(i == file_num2)
    {
      logging(LOG_ERROR, __FILE__, __LINE__, "Unable to match output files\n" );
      for(i = 0; i < file_num2; i++) 
      {
        if(file_list2[i]) free(file_list2[i]);
      }          
      free(file_list2);
      for(i = 0; i < file_num1; i++) 
      {
        if(file_list1[i]) free(file_list1[i]);
        if(ext_list1[i]) free(ext_list1[i]);
      }          
      free(file_list1);
      free(ext_list1);
      return AGENT_ERROR_EXECUTION;
    }
  }

  for(i = 0; i < file_num2; i++) 
  {
    if(file_list2[i]) free(file_list2[i]);
  }          
  free(file_list2);
  for(i = 0; i < file_num1; i++) 
  {
    if(file_list1[i]) free(file_list1[i]);
    if(ext_list1[i]) free(ext_list1[i]);
  }          
  free(file_list1);
  free(ext_list1);
   
  return AGENT_SUCCESS;
}

/* Maove file from in to out path */
int folder_move_files(connection_struct *con, long long expid, char *in_path, char *out_path)
{
	if(!con || !expid || !in_path || !out_path)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  char **file_list1 = NULL;
  char **file_list2 = NULL;
  char **ext_list1 = NULL;
  int file_num2 = 0;
  int file_num1 = 0;
  int i = 0, j = 0;
  char tmp_path_in[FS_MAX_PATH_LEN] = {0};
  char tmp_path_out[FS_MAX_PATH_LEN] = {0};
  int res = 0;

  res = get_exp_files(con, expid, &file_list1, &ext_list1, &file_num1);
  /* If list is empty set error */
  if(res == 0 && file_num1 == 0) res = AGENT_ERROR_EXECUTION;
  if( res )
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to get file names\n" );
    return res;
  }

  res = folder_get_files_in_dir(in_path, &file_list2, &file_num2);  
  /* If list is empty set error */
  if(res == 0 && file_num2 == 0 ) res = AGENT_ERROR_EXECUTION;
  if( res )
  {
    logging(LOG_ERROR, __FILE__, __LINE__, "Unable to get files from path\n" );
    for(i = 0; i < file_num2; i++) 
    {
      if(file_list2[i]) free(file_list2[i]);
    }          
    free(file_list2);
    for(i = 0; i < file_num1; i++) 
    {
      if(file_list1[i]) free(file_list1[i]);
      if(ext_list1[i]) free(ext_list1[i]);
    }          
    free(file_list1);
    free(ext_list1);
    return res;
  }


  /* Compare file lists and move files */
  for(j = 0; j < file_num1; j++) 
  {
    for(i = 0; i < file_num2; i++) 
    {
      if(!strncasecmp(file_list2[i], file_list1[j], MAX_LENGTH(file_list2[i], file_list1[j])))
      {
        snprintf(tmp_path_in, sizeof(tmp_path_in),"%s/%s", in_path, file_list2[i]);
        snprintf(tmp_path_out, sizeof(tmp_path_out),"%s/%s", out_path, file_list2[i]);
        logging(LOG_DEBUG, __FILE__, __LINE__, "Matched file! Move from %s -> %s\n",(const char *)tmp_path_in, (const char *)tmp_path_out );
        if(rename((const char *)tmp_path_in, (const char *)tmp_path_out))
        {
          logging(LOG_ERROR, __FILE__, __LINE__, "Failure while moving files: %d\n", errno );
          for(i = 0; i < file_num2; i++) 
          {
            if(file_list2[i]) free(file_list2[i]);
          }          
          free(file_list2);
          for(i = 0; i < file_num1; i++) 
          {
            if(file_list1[i]) free(file_list1[i]);
            if(ext_list1[i]) free(ext_list1[i]);
          }          
          free(file_list1);
          free(ext_list1);
          return AGENT_ERROR_QUIT;
        }
        break;
      }
    }

    if(i == file_num2)
    {
      logging(LOG_ERROR, __FILE__, __LINE__, "Unable to match experiment files\n" );
      for(i = 0; i < file_num2; i++) 
      {
        if(file_list2[i]) free(file_list2[i]);
      }          
      free(file_list2);
      for(i = 0; i < file_num1; i++) 
      {
        if(file_list1[i]) free(file_list1[i]);
        if(ext_list1[i]) free(ext_list1[i]);
      }          
      free(file_list1);
      free(ext_list1);
      return AGENT_ERROR_EXECUTION;
    }
  }

  for(i = 0; i < file_num2; i++) 
  {
    if(file_list2[i]) free(file_list2[i]);
  }          
  free(file_list2);
  for(i = 0; i < file_num1; i++) 
  {
    if(file_list1[i]) free(file_list1[i]);
    if(ext_list1[i]) free(ext_list1[i]);
  }          
  free(file_list1);
  free(ext_list1);
   
  return AGENT_SUCCESS;
}
