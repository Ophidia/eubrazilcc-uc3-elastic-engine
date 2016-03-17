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
#include "debug.h"
#include <errno.h>
#include "agent_error.h" 
#include "agent_queue.h"

extern int msglevel;

int enqueue(exp_queue *q, long long expid)
{
	if(!q || !expid)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}
  
  queue_elem *temp = (queue_elem *)malloc(1*sizeof(queue_elem));
  if(!temp)
  {
	  logging(LOG_ERROR, __FILE__, __LINE__, "Unable to alloc memory for new queue element\n" );
	  return AGENT_ERROR_QUIT;  
  }

  temp->expid = expid;
  temp->next = NULL;

  if( q->head == NULL )
  {
    q->head = temp;
  }
  else
  {
    q->tail->next = (struct queue_elem *)temp;
  }
  q->tail = temp;
  q->num_elements++;

  return AGENT_SUCCESS;
}


int dequeue(exp_queue *q, long long *expid)
{
	if(!q || !expid)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}
  
  queue_elem *temp = NULL;

  if( q->head == NULL )
  {
	  logging(LOG_WARNING, __FILE__, __LINE__, "Empty queue\n" );
    *expid = 0;
	  return AGENT_SUCCESS;
  }
 
  temp = q->head;
  *expid = temp->expid;

  q->head = (queue_elem *)q->head->next;
  free(temp);
  q->num_elements--;

  if( q->head == NULL )  
    q->tail = NULL; 

  return AGENT_SUCCESS;
}

int init_queue(exp_queue *q)
{
	if(!q)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  q->head = NULL;
  q->tail = NULL;
  q->num_elements = 0;

  return AGENT_SUCCESS;
}

int free_queue(exp_queue *q)
{
	if(!q)
  {
		logging(LOG_ERROR, __FILE__, __LINE__, "Missing function input\n" );
		return AGENT_ERROR_QUIT;
	}

  long long temp_id = 0;

  while(q->num_elements > 0)
  {
    dequeue(q, &temp_id);
  }

  return AGENT_SUCCESS;
}

