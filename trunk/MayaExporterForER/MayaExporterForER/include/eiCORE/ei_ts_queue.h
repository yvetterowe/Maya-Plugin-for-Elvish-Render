/*
 * Copyright 2010 elvish render Team
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef EI_TS_QUEUE_H
#define EI_TS_QUEUE_H

/** \brief This file contains a thread-safe queue container.
 * \file ei_ts_queue.h
 */

#include <eiCORE/ei_list.h>
#include <eiCORE/ei_rwlock.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The book keeping structure for thread-safe queue.
 */
typedef struct ei_ts_queue {
	ei_list			q;
	eiRWLock		lock;
} ei_ts_queue;

typedef ei_list_node			ei_ts_queue_node;
typedef ei_list_delete_node		ei_ts_queue_delete_node;
/** \brief The procedure used to traverse the thread-safe queue, if zero is 
 * returned while traversing nodes, the traversal will be terminated.
 */
typedef eiInt (*ei_ts_queue_proc)(ei_ts_queue_node *node, void *param);

/** \brief Initialize the node.
 */
eiCORE_API void ei_ts_queue_node_init(ei_ts_queue_node *node);

/** \brief Clear the node.
 */
eiCORE_API void ei_ts_queue_node_clear(ei_ts_queue_node *node);

/** \brief Initialize a thread-safe queue.
 */
eiCORE_API void ei_ts_queue_init(ei_ts_queue *queue, ei_ts_queue_delete_node delete_node);

/** \brief Clear a thread-safe queue.
 */
eiCORE_API void ei_ts_queue_clear(ei_ts_queue *queue);

/** \brief Returns the number of nodes in a thread-safe queue.
 */
eiCORE_API eiSizet ei_ts_queue_size(ei_ts_queue *queue);

/** \brief Returns whether the thread-safe queue is empty.
 */
eiCORE_API eiBool ei_ts_queue_empty(ei_ts_queue *queue);

/** \brief Push a node into the queue.
 */
eiCORE_API void ei_ts_queue_push(ei_ts_queue *queue, ei_ts_queue_node *node);

/** \brief Pop a node from the queue, returns NULL if the queue is empty.
 */
eiCORE_API ei_ts_queue_node *ei_ts_queue_pop(ei_ts_queue *queue);

/** \brief Traverse all nodes in the thread-safe queue.
 */
eiCORE_API void ei_ts_queue_traverse(ei_ts_queue *queue, ei_ts_queue_proc proc, void *param);

#ifdef __cplusplus
}
#endif

#endif
