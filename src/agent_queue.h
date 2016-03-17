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

#ifndef __AGENT_QUEUE_H
#define __AGENT_QUEUE_H

/**
 * \brief           Structure that contains queue elements info
 * \param expid     Id stored in the queue
 * \param next      pointer to next queue element
 */
typedef struct
{
  long long expid;
  struct queue_elem *next;
} queue_elem;


/**
 * \brief               Structure to contain a queue 
 * \param head          head of queue (extraction)
 * \param tail          tail of queue (insertion)
 * \param num_elements  number of elements in queue
 */
typedef struct
{
  queue_elem *head;
  queue_elem *tail;
  int num_elements;
} exp_queue;

/**
 * \brief       Function to add element to the queue.
 * \param q     Pointer to queue
 * \param expid Value of Id to be added
 * \return 0 if successfull, another error code otherwise
 */
int enqueue(exp_queue *q, long long expid);

/**
 * \brief       Function to remove element from the queue.
 * \param q     Pointer to queue
 * \param expid Value of Id to be removed
 * \return 0 if successfull, another error code otherwise
 */
int dequeue(exp_queue *q, long long *expid);

/**
 * \brief     Function to initialize the queue.
 * \param q   Pointer to queue
 * \return 0 if successfull, another error code otherwise
 */
int init_queue(exp_queue *q);

/**
 * \brief     Function to free queue resources.
 * \param q   Pointer to queue
 * \return 0 if successfull, another error code otherwise
 */
int free_queue(exp_queue *q);

#endif  //__AGENT_QUEUE_H
