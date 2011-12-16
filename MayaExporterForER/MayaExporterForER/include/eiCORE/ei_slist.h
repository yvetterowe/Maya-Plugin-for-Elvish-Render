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
 
#ifndef EI_SLIST_H
#define EI_SLIST_H

/** \brief Intrusive non-thread-safe single linked list.
 * \file ei_slist.h
 * \author Bo Zhou
 *
 * This container is very similar to ei_list, except that it's 
 * single linked, which saves memory space by reducing one pointer.
 */

#include <eiCORE/ei_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The node in single linked list.
 */
typedef struct ei_slist_node {
	struct ei_slist_node	*next;
} ei_slist_node;

/** \brief The function callback for deleting a node in a list.
 */
typedef void (*ei_slist_delete_node)(ei_slist_node *node);

/** \brief The generic single linked list container.
 */
typedef struct ei_slist {
	ei_slist_node			head;
	ei_slist_node			*tail;
	eiSizet					size;
	ei_slist_delete_node	delete_node;
} ei_slist;

/** \brief Initialize the node.
 */
eiCORE_API void ei_slist_node_init(ei_slist_node *node);

/** \brief Clear the node.
 */
eiCORE_API void ei_slist_node_clear(ei_slist_node *node);

/** \brief Initialize a single linked list.
 */
eiCORE_API void ei_slist_init(ei_slist *slist, ei_slist_delete_node delete_node);

/** \brief Returns the number of nodes in a list.
 */
eiCORE_API eiSizet ei_slist_size(const ei_slist *slist);

/** \brief Returns whether the list is empty.
 */
eiCORE_API eiBool ei_slist_empty(const ei_slist *slist);

/** \brief Insert a new node after an existing node.
 */
eiCORE_API void ei_slist_insert(ei_slist *list, ei_slist_node *pnode, ei_slist_node *newnode);

/** \brief Push a node at the end of the list.
 */
eiCORE_API void ei_slist_push_back(ei_slist *slist, ei_slist_node *node);

/** \brief Push a node at the beginning of the list.
 */
eiCORE_API void ei_slist_push_front(ei_slist *slist, ei_slist_node *node);

/** \brief Pop a node after an existing node.
 */
eiCORE_API ei_slist_node *ei_slist_pop(ei_slist *list, ei_slist_node *pnode_prev);

/** \brief Pop a node after an existing node, then call delete_node callback on the node.
 */
eiCORE_API void ei_slist_erase(ei_slist *list, ei_slist_node *pnode_prev);

/** \brief Get the beginning of the list.
 */
eiCORE_API ei_slist_node *ei_slist_begin(ei_slist *slist);

/** \brief Get the end of the list.
 */
eiCORE_API ei_slist_node *ei_slist_end(ei_slist *slist);

/** \brief Get the first node in the list.
 */
eiCORE_API ei_slist_node *ei_slist_front(ei_slist *slist);

/** \brief Get the last node in the list.
 */
eiCORE_API ei_slist_node *ei_slist_back(ei_slist *slist);

/** \brief Clear a single linked list.
 */
eiCORE_API void ei_slist_clear(ei_slist *slist);

#ifdef __cplusplus
}
#endif

#endif
