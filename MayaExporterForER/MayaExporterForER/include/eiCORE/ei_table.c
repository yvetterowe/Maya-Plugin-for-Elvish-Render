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

/** \brief The implementation of the dynamic table container.
 * \file ei_table.c
 * \author Elvic Liang
 */

#include <eiCORE/ei_table.h>
#include <eiCORE/ei_platform.h>
#include <eiCORE/ei_assert.h>

#define BLOCK_DIV				4

/** \brief Internal block header */
typedef struct eiBlock {
	/* the number of actually allocated items in 
	   this block, to save memory for large blocks, 
	   we don't allocate the entire block for the 
	   first time */
	eiIntptr	count;
} eiBlock;

static void ei_table_clear_items(ei_table *tab)
{
	eiIntptr i;

	for (i = 0; i < ei_array_size(&tab->tags); ++i)
	{
		eiByte *tag;

		tag = *((eiByte **)ei_array_get(&tab->tags, i));

		/* delete each data block */
		eiCHECK_FREE(tag);
	}
}

void ei_table_init(ei_table *tab, const eiIntptr item_size, const eiIntptr items_per_slot)
{
	/* create an array of tags */
	ei_array_init(&tab->tags, sizeof(eiByte *));
	tab->item_count = 0;
	tab->item_size = item_size;
	
	/* round to the nearest power of 2 */
	tab->items_per_slot = 1;
	tab->items_per_slot_shift = 0;
	for (; tab->items_per_slot < items_per_slot; ++ tab->items_per_slot_shift)
	{
		tab->items_per_slot <<= 1;
	}
	if (tab->items_per_slot > items_per_slot)
	{
		tab->items_per_slot >>= 1;
		-- tab->items_per_slot_shift;
	}

	tab->current_items_available = 0;
}

void ei_table_clear(ei_table *tab)
{
	/* clear data items */
	ei_table_clear_items(tab);

	/* delete the tag array */
	ei_array_clear(&tab->tags);
	tab->item_count = 0;
}

void *ei_table_get(ei_table *tab, const eiIntptr index)
{
	eiInt slot_index;
	eiInt sub_index;
	eiByte *current_items;

	/* calculate indices */
	slot_index = (index >> tab->items_per_slot_shift);
	sub_index = index - (slot_index << tab->items_per_slot_shift);
	
	/* get block tag */
	current_items = (*((eiByte **)ei_array_get(&tab->tags, slot_index))) + sizeof(eiBlock);

	return (current_items + tab->item_size * sub_index);
}

eiBool ei_table_empty(ei_table *tab)
{
	return (tab->item_count == 0);
}

eiIntptr ei_table_size(ei_table *tab)
{
	return tab->item_count;
}

void ei_table_push_back(ei_table *tab, const void *item)
{
	eiBlock	*pBlock;
	eiByte *current_items;

	/* when the back block fills up, 
	   create new block for writing */
	if (tab->current_items_available == 0)
	{
		eiSizet		reserve_size;

		reserve_size = MAX((eiSizet)tab->items_per_slot / BLOCK_DIV, ei_reserve_size(0));
		/* this implies that each block would at least have reserve_size items */
		pBlock = (eiBlock *)ei_allocate(sizeof(eiBlock) + tab->item_size * reserve_size);
		pBlock->count = reserve_size;

		ei_array_push_back(&tab->tags, &pBlock);

		tab->current_items_available = tab->items_per_slot;
	}
	else
	{
		eiIntptr slot_index;
		eiIntptr sub_index;

		/* get the back block */
		slot_index = ei_array_size(&tab->tags) - 1;
		pBlock = *((eiBlock **)ei_array_get(&tab->tags, slot_index));

		/* calculate the sub-index to fill with the new item */
		sub_index = tab->items_per_slot - tab->current_items_available;

		/* resize the block if no enough memory has been allocated */
		if (sub_index >= pBlock->count)
		{
			eiIntptr new_count;

			new_count = pBlock->count + MAX((eiSizet)tab->items_per_slot / BLOCK_DIV, ei_reserve_size(pBlock->count));

			pBlock = (eiBlock *)ei_reallocate(pBlock, sizeof(eiBlock) + tab->item_size * new_count);
			pBlock->count = new_count;

			/* set the new block into the tag array */
			*((eiBlock **)ei_array_get(&tab->tags, slot_index)) = pBlock;
		}
	}

	current_items = (eiByte *)(pBlock + 1);

	memcpy(current_items + tab->item_size * (tab->items_per_slot - tab->current_items_available), 
		item, tab->item_size);

	-- tab->current_items_available;
	++ tab->item_count;
}
