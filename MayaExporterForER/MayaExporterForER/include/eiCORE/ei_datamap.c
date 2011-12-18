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

/** \brief Generic data map container implementation.
 * \file ei_datamap.c
 * \author Elvic Liang
 */

#include <eiCORE/ei_datamap.h>
#include <string.h>

#define DATA_MEM_POOL_BANK_SIZE		4096

void ei_datamap_init(eiDataMap *map, const eiIntptr item_size)
{
	eiDBG_ASSERT(map != NULL);

	ei_create_lock(&map->lock);
	memset(map->blocks, 0, sizeof(eiAtomicptr) * EI_DATAMAP_NUM_BLOCKS);
	ei_atomic_set(&map->size, 0);
	ei_fixed_pool_init(&map->pool, item_size, DATA_MEM_POOL_BANK_SIZE);
}

void ei_datamap_clear(eiDataMap *map)
{
	eiInt i;

	eiDBG_ASSERT(map != NULL);

	ei_fixed_pool_clear(&map->pool);
	for (i = 0; i < EI_DATAMAP_NUM_BLOCKS; ++i)
	{
		void *block;

		block = (void *)ei_atomic_read(&map->blocks[i]);
		
		if (block != NULL)
		{
			ei_free(block);
			ei_atomic_set(&map->blocks[i], (eiIntptr)NULL);
		}
	}
	ei_delete_lock(&map->lock);
}

void *ei_datamap_insert(eiDataMap *map, const eiTag tag)
{
	eiTag block_index;
	eiTag sub_index;
	void **iter;

	eiDBG_ASSERT(map != NULL && tag != eiNULL_TAG);
	/* must NOT exceed the maximum tag available */
	eiASSERT(tag < (EI_DATAMAP_NUM_BLOCKS * EI_DATAMAP_BLOCK_SIZE));

	block_index = (tag >> EI_DATAMAP_BLOCK_SIZE_SHIFT);
	sub_index = tag - (block_index << EI_DATAMAP_BLOCK_SIZE_SHIFT);

	if (ei_atomic_read(&map->blocks[block_index]) == (eiIntptr)NULL)
	{
		void *new_block;

		ei_lock(&map->lock);

		/* check again in lock scope to ensure there 
		   is no duplicated initialization */
		if (ei_atomic_read(&map->blocks[block_index]) == (eiIntptr)NULL)
		{
			new_block = (void *)ei_allocate(sizeof(void *) * EI_DATAMAP_BLOCK_SIZE);
			memset(new_block, 0, sizeof(void *) * EI_DATAMAP_BLOCK_SIZE);

			ei_atomic_swap_ptr(&map->blocks[block_index], (eiIntptr)new_block);
		}

		ei_unlock(&map->lock);
	}

	iter = ((void **)ei_atomic_read(&map->blocks[block_index])) + sub_index;

	ei_lock(&map->lock);

	if (*iter == NULL)
	{
		*iter = ei_fixed_pool_allocate(&map->pool);
	}

	/* the size is the largest tag, to make sure all 
	   tags can be iterated later */
	if (tag >= (eiTag)ei_atomic_read(&map->size))
	{
		ei_atomic_swap(&map->size, tag + 1);
	}

	ei_unlock(&map->lock);

	return *iter;
}

void ei_datamap_erase(eiDataMap *map, const eiTag tag)
{
	eiTag block_index;
	eiTag sub_index;
	void **iter;

	eiDBG_ASSERT(map != NULL && tag != eiNULL_TAG);

	block_index = (tag >> EI_DATAMAP_BLOCK_SIZE_SHIFT);
	sub_index = tag - (block_index << EI_DATAMAP_BLOCK_SIZE_SHIFT);

	iter = ((void **)ei_atomic_read(&map->blocks[block_index])) + sub_index;

	ei_lock(&map->lock);

	if (*iter != NULL)
	{
		ei_fixed_pool_free(&map->pool, *iter);
		*iter = NULL;
	}

	ei_unlock(&map->lock);
}
