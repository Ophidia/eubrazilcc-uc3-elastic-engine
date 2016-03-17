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

#ifndef __AGENT_FOLDER_H
#define __AGENT_FOLDER_H

#include "agent_common.h"
#include <ftw.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * \brief                 Function to make recursive directory function.
 * \param dir             Folder to be created recursively 
 * \return 0 if successfull, another error code otherwise
 */
int folder_rec_mkdir(const char *dir); 

/**
 * \brief                 Function to list files in directory function.
 * \param dirpath         Folder where to look for files 
 * \param file_list       Nul-terminated list of files in the folder
 * \param file_number     Number of files found 
 * \return 0 if successfull, another error code otherwise
 */
int folder_get_files_in_dir(char *dirpath, char ***file_list, int *file_number); 

/**
 * \brief                 Callback function (nftw) to delete a file or empty folder.
 * \return 0 if successfull, another error code otherwise
 */
int folder_delete_files(const char *path, const struct stat *st, int flag, struct FTW *ftw);

/**
 * \brief                 Function to check that experiment output is available in the folder.
 * \param con             Pointer to a connection parameters structure
 * \param expid           Id of experiment to store
 * \param out_path        Output path 
 * \return 0 if successfull, another error code otherwise
 */
int folder_check_file_existance(connection_struct *con, long long expid, char *out_path);

/**
 * \brief                 Function to move file from in to out path.
 * \param con             Pointer to a connection parameters structure
 * \param expid           Id of experiment to store
 * \param in_path         Input path
 * \param out_path        Output path 
 * \return 0 if successfull, another error code otherwise
 */
int folder_move_files(connection_struct *con, long long expid, char *in_path, char *out_path);

#endif  //__AGENT_FOLDER_H
