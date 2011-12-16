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

#include <eiCORE/ei_slist.h>
#include <eiCORE/ei_assert.h>

void ei_slist_node_init(ei_slist_node *node)
{
	node->next = NULL;
}

void ei_slist_node_clear(ei_slist_node *node)
{
	node->next = NULL;
}

void ei_slist_init(ei_slist *slist, ei_slist_delete_node delete_node)
{
	eiDBG_ASSERT(slist != NULL);
	
	slist->head.next = &slist->head;
	slist->tail = &slist->head;
	slist->size = 0;
	slist->delete_node = delete_node;
}

eiSizet ei_slist_size(const ei_slist *slist)
{
	eiDBG_ASSERT(slist != NULL);

	return slist->size;
}

eiBool ei_slist_empty(const ei_slist *slist)
{
	return (slist->size == 0);
}

void ei_slist_insert(ei_slist *list, ei_slist_node *pnode, ei_slist_node *newnode)
{
	eiDBG_ASSERT(list != NULL && pnode != NULL && newnode != NULL);

	newnode->next = pnode->next;
	pnode->next = newnode;
	if (pnode == list->tail)
	{
		list->tail = newnode;
	}
	++ list->size;
}

void ei_slist_push_back(ei_slist *slist, ei_slist_node *node)
{
	ei_slist_insert(slist, slist->tail, node);
}

void ei_slist_push_front(ei_slist *slist, ei_slist_node *node)
{
	ei_slist_insert(slist, &slist->head, node);
}

ei_slist_node *ei_slist_pop(ei_slist *list, ei_slist_node *pnode_prev)
{
	ei_slist_node *pnode = NULL;

	eiDBG_ASSERT(list != NULL && pnode_prev != NULL);

	pnode = pnode_prev->next;

	eiDBG_ASSERT(pnode != NULL && pnode != &list->head);

	pnode_prev->next = pnode->next;
	if (pnode == list->tail)
	{
		list->tail = pnode_prev;
	}
	pnode->next = NULL;
	-- list->size;

	return pnode;
}

void ei_slist_erase(ei_slist *list, ei_slist_node *pnode_prev)
{
	ei_slist_node *pnode = NULL;

	eiDBG_ASSERT(list != NULL && pnode_prev != NULL);

	pnode = ei_slist_pop(list, pnode_prev);
	
	if (pnode != NULL && list->delete_node != NULL)
	{
		list->delete_node(pnode);
	}
}

ei_slist_node *ei_slist_begin(ei_slist *slist)
{
	eiDBG_ASSERT(slist != NULL);

	return slist->head.next;
}

ei_slist_node *ei_slist_end(ei_slist *slist)
{
	eiDBG_ASSERT(slist != NULL);

	return &slist->head;
}

ei_slist_node *ei_slist_front(ei_slist *slist)
{
	eiDBG_ASSERT(slist != NULL);

	return slist->head.next;
}

ei_slist_node *ei_slist_back(ei_slist *slist)
{
	eiDBG_ASSERT(slist != NULL);

	return slist->tail;
}

void ei_slist_clear(ei_slist *slist)
{
	ei_slist_node *p = NULL;
	ei_slist_node *next = NULL;

	eiDBG_ASSERT(slist != NULL);
	
	p = slist->head.next;
	slist->head.next = &slist->head;
	slist->tail = &slist->head;
	slist->size = 0;

	for (; p != &slist->head; p = next)
	{
		next = p->next;
		
		p->next = NULL;
		if (slist->delete_node != NULL)
		{
			slist->delete_node(p);
		}
	}
}
