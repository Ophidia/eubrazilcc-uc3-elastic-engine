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

#ifndef __AGENT_THREAD_H
#define __AGENT_THREAD_H

#include "agent_db.h"
#include "agent_queue.h"

/* Used for clusters which have not been deployed yet */
#define NO_ADDRESS      "NONE"

/**
 * \brief                   Structure to manage threads information
 * \param static_flag       if cluster is static or dynamic  
 * \param job_num           number of jobs assigned to cluster
 * \param active            array of active/inactive flag for clusters
 * \param infrastruct_url   array of infrastructure urls
 * \param server_address    array of server_addresses
 * \param queue             pointer to thread queues
 * \param exec_start        array of flags for experiment execution
 * \param exec_stop         array of counters to coordinate experiment execution stop (1 flag for each experiment thread)
 */
typedef struct
{
    short int *static_flag;
    int       *job_num;
    short int *active;
    char      **infrastruct_url;
    char      **server_address;
    exp_queue **queue;
    short int *exec_start;
    short int *exec_stop;
} cluster_data;

/**
 * \brief       Function to initialize cluster data content.
 * \param data  Pointer to an allocated cluster data structure
 * \return 0 if successfull, another error code otherwise
 */
int init_cluster_data(cluster_data *data);

/**
 * \brief       Function to free cluster data content.
 * \param data  Pointer to an allocated cluster data structure
 * \return 0 if successfull, another error code otherwise
 */
int free_cluster_data(cluster_data *data);

/**
 * \brief           Function to assign experiments to thread queues
 * \param con       Pointer to structure containing database connection
 * \param expids    Pointer to null-terminated array of experiment to be distributed
 * \param expsizes  Pointer to null-terminated array of experiment to be distributed
 * \return 0 if successfull, another error code otherwise
 */
int assign_experiments(connection_struct *con, long long *expids, int *expsizes);

/**
 * \brief           Function to update cluster status tables
 * \param con       Pointer to structure containing database connection
 * \return 0 if successfull, another error code otherwise
 */
int update_cluster_status(connection_struct *con);

/**
 * \brief           Function to manage delete thread
 * \param arg       Thread ID
 * \return 0 if successfull, another error code otherwise
 */
void *delete_storage_manager(void *arg);

/**
 * \brief           Function to manage clearing-house thread
 * \param arg       Thread ID
 * \return 0 if successfull, another error code otherwise
 */
void *clearing_house_manager(void *arg);

/**
 * \brief           Function to manage cluster threads
 * \param arg       Thread ID
 * \return 0 if successfull, another error code otherwise
 */
void *cluster_manager(void *arg);

#endif  //__AGENT_THREAD_H
