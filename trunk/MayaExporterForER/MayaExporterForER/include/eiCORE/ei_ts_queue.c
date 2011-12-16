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

#include <eiCORE/ei_ts_queue.h>
#include <eiCORE/ei_assert.h>

void ei_ts_queue_node_init(ei_ts_queue_node *node)
{
	ei_list_node_init(node);
}

void ei_ts_queue_node_clear(ei_ts_queue_node *node)
{
	ei_list_node_clear(node);
}

void ei_ts_queue_init(ei_ts_queue *queue, ei_ts_queue_delete_node delete_node)
{
	eiDBG_ASSERT(queue != NULL);

	ei_create_rwlock(&queue->lock);
	ei_list_init(&queue->q, delete_node);
}

void ei_ts_queue_clear(ei_ts_queue *queue)
{
	eiDBG_ASSERT(queue != NULL);

	ei_list_clear(&queue->q);
	ei_delete_rwlock(&queue->lock);
}

eiSizet ei_ts_queue_size(ei_ts_queue *queue)
{
	eiSizet size;

	eiDBG_ASSERT(queue != NULL);

	ei_read_lock(&queue->lock);
	size = ei_list_size(&queue->q);
	ei_read_unlock(&queue->lock);

	return size;
}

eiBool ei_ts_queue_empty(ei_ts_queue *queue)
{
	eiBool empty;

	eiDBG_ASSERT(queue != NULL);

	ei_read_lock(&queue->lock);
	empty = ei_list_empty(&queue->q);
	ei_read_unlock(&queue->lock);

	return empty;
}

void ei_ts_queue_push(ei_ts_queue *queue, ei_ts_queue_node *node)
{
	eiDBG_ASSERT(queue != NULL);

	ei_write_lock(&queue->lock);
	ei_list_push_back(&queue->q, node);
	ei_write_unlock(&queue->lock);
}

ei_ts_queue_node *ei_ts_queue_pop(ei_ts_queue *queue)
{
	ei_ts_queue_node *node;

	eiDBG_ASSERT(queue != NULL);

	ei_read_lock(&queue->lock);
	if (ei_list_empty(&queue->q))
	{
		ei_read_unlock(&queue->lock);
		return NULL;
	}

	ei_upgrade_lock(&queue->lock);
	node = ei_list_front(&queue->q);
	if (node == NULL)
	{
		ei_write_unlock(&queue->lock);
		return NULL;
	}

	ei_list_pop(&queue->q, node);
	ei_write_unlock(&queue->lock);

	return node;
}

void ei_ts_queue_traverse(ei_ts_queue *queue, ei_ts_queue_proc proc, void *param)
{
	ei_ts_queue_node *node;

	eiDBG_ASSERT(queue != NULL);

	for (node = ei_list_begin(&queue->q); 
		node != ei_list_end(&queue->q); 
		node = node->next)
	{
		if (!proc(node, param))
		{
			break;
		}
	}
}
