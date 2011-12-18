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
 
#ifndef EI_LIST_H
#define EI_LIST_H

/** \brief Intrusive non-thread-safe double linked list.
 * \file ei_list.h
 * \author Bo Zhou
 *
 * "Intrusive" means this container does not carry data as the member 
 * of its node, instead, it only operates on the linkages of nodes. 
 * As a consequence, users should explicitly allocate/free the nodes  
 * on their own.
 */

#include <eiCORE/ei_core.h>
#include <eiCORE/ei_assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Declaration of the node of double linked list.
 */
typedef struct ei_list_node ei_list_node;

/** \brief Declaration of double linked list.
 */
typedef struct ei_list ei_list;

/** \brief ei_list_node should be part of user's data structure.
 */
struct ei_list_node {
	struct ei_list_node		*prev;
	struct ei_list_node		*next;
};

/** \brief The function callback for deleting a node in a list.
 */
typedef void (*ei_list_delete_node)(ei_list_node *node);

/** \brief The generic double link list container.
 */
struct ei_list {
	ei_list_node		head;
	ei_list_node		*tail;
	eiSizet				size;
	ei_list_delete_node	delete_node;
};

/** \brief Initialize the node.
 */
eiCORE_API void ei_list_node_init(ei_list_node *node);

/** \brief Clear the node.
 */
eiCORE_API void ei_list_node_clear(ei_list_node *node);

/** \brief Initialize a double linked list.
 */
eiCORE_API void ei_list_init(ei_list *list, ei_list_delete_node delete_node);

/** \brief Get the number of nodes in a double linked list.
 */
eiCORE_API eiSizet ei_list_size(const ei_list *list);

/** \brief Returns whether a list is empty.
 */
eiCORE_API eiBool ei_list_empty(const ei_list *list);

/** \brief Returns whether the list contains a specific node.
 */
eiFORCEINLINE eiBool ei_list_contains(const ei_list *list, const ei_list_node *node)
{
	/* a node belongs to this list only when 
	   its prev is non-null. */
	eiDBG_ASSERT(list != NULL && node != NULL);

	if (node->prev != NULL)
	{
		return eiTRUE;
	}
	else
	{
		return eiFALSE;
	}
}

/** \brief Insert a new node after an existing node.
 */
eiCORE_API void ei_list_insert(ei_list *list, ei_list_node *pnode, ei_list_node *newnode);

/** \brief Push a node at the end of the list.
 */
eiCORE_API void ei_list_push_back(ei_list *list, ei_list_node *node);

/** \brief Push a node at the beginning of the list.
 */
eiCORE_API void ei_list_push_front(ei_list *list, ei_list_node *node);

/** \brief Move a node to the end of the list.
 */
eiFORCEINLINE void ei_list_move_back(ei_list *list, ei_list_node *node)
{
	eiDBG_ASSERT(list != NULL && node != NULL);

	/* move the node to back when it belongs to 
	   this list. */
	if (!ei_list_contains(list, node))
	{
		return;
	}

	if (node != list->tail)
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;

		node->prev = list->tail;
		node->next = NULL;

		list->tail->next = node;
		list->tail = node;
	}
}

/** \brief Pop a node from the list.
 */
eiCORE_API void ei_list_pop(ei_list *list, ei_list_node *pnode);

/** \brief Pop a node from the list, then call delete_node callback on the node.
 */
eiCORE_API void ei_list_erase(ei_list *list, ei_list_node *node);

/** \brief Get the beginning of the list.
 */
eiCORE_API ei_list_node *ei_list_begin(ei_list *list);

/** \brief Get the end of the list.
 */
eiCORE_API ei_list_node *ei_list_end(ei_list *list);

/** \brief Get the first node in the list.
 */
eiCORE_API ei_list_node *ei_list_front(ei_list *list);

/** \brief Get the last node in the list.
 */
eiCORE_API ei_list_node *ei_list_back(ei_list *list);

/** \brief Clear a double linked list.
 */
eiCORE_API void ei_list_clear(ei_list *list);

#ifdef __cplusplus
}
#endif

#endif
