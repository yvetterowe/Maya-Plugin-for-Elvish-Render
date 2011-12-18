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

/** \brief Fixed-sized memory pool implementation.
 * \file ei_fixed_pool.c
 * \author Elvic Liang
 */

#include <eiCORE/ei_fixed_pool.h>
#include <eiCORE/ei_platform.h>

/** \brief Memory chunk node.
 */
struct ei_chunk_node {
	struct ei_chunk_node	*next;
};

void ei_fixed_pool_init(ei_fixed_pool *pool, const eiInt dataSize, const eiInt itemsPerBank)
{
	pool->dataSize = dataSize;
	pool->itemsPerBank = itemsPerBank; /* The number of items to allocate at a time */
	pool->allocatedBanks = NULL;
	pool->freeItems = NULL;
}

void ei_fixed_pool_clear(ei_fixed_pool *pool)
{
	ei_chunk_node *cBank;

	while (pool->allocatedBanks != NULL)
	{
		cBank = pool->allocatedBanks;
		pool->allocatedBanks = cBank->next;

		eiCHECK_FREE(cBank);
	}

	pool->freeItems = NULL;
}

void *ei_fixed_pool_allocate(ei_fixed_pool *pool)
{
	ei_chunk_node *cBank;
	eiInt i;
	eiByte *cItem;

	if (pool->freeItems != NULL)
	{
		cItem = (eiByte *)pool->freeItems;
		pool->freeItems = pool->freeItems->next;

		return cItem;
	}
	else
	{
		// Compute the number of words we want to allocate
		i = pool->dataSize * pool->itemsPerBank / sizeof(ei_chunk_node) + 1;

		// Insert the new bank into the linked list
		cBank =	(ei_chunk_node *)ei_allocate(sizeof(ei_chunk_node) * i);
		cBank->next = pool->allocatedBanks;
		pool->allocatedBanks = cBank;

		for (++ cBank, i = pool->itemsPerBank; i > 0; --i)
		{
			cBank->next = pool->freeItems;
			pool->freeItems = cBank;
			cItem = (eiByte *)cBank;
			cItem += pool->dataSize;
			cBank = (ei_chunk_node *)cItem;
		}

		cItem =	(eiByte *)pool->freeItems;
		pool->freeItems = pool->freeItems->next;

		return cItem;
	}
}

void ei_fixed_pool_free(ei_fixed_pool *pool, void *item)
{
	ei_chunk_node *tmp = (ei_chunk_node *)item;

	tmp->next = pool->freeItems;
	pool->freeItems = tmp;
}
