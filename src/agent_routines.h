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

#ifndef __AGENT_ROUTINES_H
#define __AGENT_ROUTINES_H

#include "agent_common.h"
#include "agent_db.h"

/**
 * \brief               Function to execute experiment indicators.
 * \param con           Pointer to a connection parameters structure
 * \param conf          Pointer to configuration parameters structure
 * \param serv_addr     Address of server to be used to execute jobs
 * \param expid         Id of experiment to execute
 * \param exp_status    Return experiment status
 * \param exp_weight    Weight of the experiment being executed
 * \return 0 if successfull, another error code otherwise
 */
int exec_experiment(configuration_struct *conf, connection_struct *con, char * serv_addr, long long expid, short int *exp_status, int *exp_weight);

/**
 * \brief                 Function to store experiment indicators into clearing house.
 * \param con             Pointer to a connection parameters structure
 * \param conf            Pointer to configuration parameters structure
 * \param submissiondate  String with job submission date
 * \param expid           Id of experiment to store
 * \param exp_status      Return experiment status
 * \return 0 if successfull, another error code otherwise
 */
int store_experiment(configuration_struct *conf, connection_struct *con, char *submissiondate, long long expid, short int *exp_status);

/**
 * \brief                 Function to delete experiment files from tmp path.
 * \param con             Pointer to a connection parameters structure
 * \param conf            Pointer to configuration parameters structure
 * \param expid           Id of experiment to delete
 * \param expchstatus     Clearing house status of experiment to delete
 * \param submissiondate  String with job submission date
 * \return 0 if successfull, another error code otherwise
 */
int delete_experiment(configuration_struct *conf, connection_struct *con, long long expid, short expchstatus, char *submissiondate);

/**
 * \brief                 Function to delete transient/urecoverable data from system catalog and file system.
 * \param con             Pointer to a connection parameters structure
 * \param conf            Pointer to configuration parameters structure
 * \return 0 if successfull, another error code otherwise
 */
int delete_transient(configuration_struct *conf, connection_struct *con);


#endif  //__AGENT_ROUTINES_H
