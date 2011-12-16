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

#include <eiCORE/ei_list.h>

void ei_list_node_init(ei_list_node *node)
{
	node->prev = NULL;
	node->next = NULL;
}

void ei_list_node_clear(ei_list_node *node)
{
	node->prev = NULL;
	node->next = NULL;
}

void ei_list_init(ei_list *list, ei_list_delete_node delete_node)
{
	eiDBG_ASSERT(list != NULL);

	list->head.prev = NULL;
	list->head.next = NULL;
	list->tail = &list->head;
	list->size = 0;
	list->delete_node = delete_node;
}

eiSizet ei_list_size(const ei_list* list)
{
	eiDBG_ASSERT(list != NULL);

	return list->size;
}

eiBool ei_list_empty(const ei_list *list)
{
	eiDBG_ASSERT(list != NULL);

	return (list->size == 0);
}

void ei_list_insert(ei_list *list, ei_list_node *pnode, ei_list_node *newnode)
{
	eiDBG_ASSERT(list != NULL && pnode != NULL && newnode != NULL);

	/* we should only insert the node when it doesn't 
	   belong to this list. */
	if (ei_list_contains(list, newnode))
	{
		return;
	}

	newnode->prev = pnode;
	newnode->next = pnode->next;
	if (pnode->next != NULL)
	{
		pnode->next->prev = newnode;
	}
	pnode->next = newnode;
	if (pnode == list->tail)
	{
		list->tail = newnode;
	}
	++ list->size;
}

void ei_list_push_back(ei_list *list, ei_list_node *node)
{
	ei_list_insert(list, list->tail, node);
}

void ei_list_push_front(ei_list *list, ei_list_node *node)
{
	ei_list_insert(list, &list->head, node);
}

void ei_list_pop(ei_list *list, ei_list_node *pnode)
{
	eiDBG_ASSERT(list != NULL && pnode != NULL && pnode != &list->head);

	/* pop the node only when it belongs to 
	   this list. */
	if (!ei_list_contains(list, pnode))
	{
		return;
	}

	pnode->prev->next = pnode->next;
	if (pnode->next != NULL)
	{
		pnode->next->prev = pnode->prev;
	}
	if (pnode == list->tail)
	{
		list->tail = pnode->prev;
	}
	pnode->prev = NULL;
	pnode->next = NULL;
	-- list->size;
}

void ei_list_erase(ei_list *list, ei_list_node *node)
{
	eiDBG_ASSERT(list != NULL && node != NULL);

	ei_list_pop(list, node);
	
	if (list->delete_node != NULL)
	{
		list->delete_node(node);
	}
}

ei_list_node *ei_list_begin(ei_list *list)
{
	eiDBG_ASSERT(list != NULL);

	return list->head.next;
}

ei_list_node *ei_list_end(ei_list *list)
{
	eiDBG_ASSERT(list != NULL);

	return NULL;
}

ei_list_node *ei_list_front(ei_list *list)
{
	eiDBG_ASSERT(list != NULL);

	return list->head.next;
}

ei_list_node *ei_list_back(ei_list *list)
{
	eiDBG_ASSERT(list != NULL);

	return list->tail;
}

void ei_list_clear(ei_list *list)
{
	ei_list_node *p = NULL;
	ei_list_node *ptr = NULL;
	
	eiDBG_ASSERT(list != NULL);

	p = list->head.next;

	while (p != NULL)
	{
		ptr = p;
		p = p->next;

		ptr->prev = NULL;
		ptr->next = NULL;
		if (list->delete_node != NULL)
		{
			list->delete_node(ptr);
		}
	}

	list->head.prev = NULL;
	list->head.next = NULL;
	list->tail = &list->head;
	list->size = 0;
}
