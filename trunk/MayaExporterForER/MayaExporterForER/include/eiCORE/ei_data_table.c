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

#include <eiCORE/ei_data_table.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_assert.h>

#define MIN_TAG_INCREMENT		10
#define BLOCK_DIV				4

/** \brief Internal header of data block which 
 * consists of basic database elements, and can be 
 * byte-swapped independently.
 */
typedef struct eiDataBlock {
	eiInt			type;
	/* the number of actually allocated items in 
	   this block, to save memory for large blocks, 
	   we don't allocate the entire block for the 
	   first time */
	eiInt			count;
} eiDataBlock;

#define TAG_ARRAY_SIZE(tab)			(tab)->tags_size
#define TAG_ARRAY_GET(tab, index)	(((eiByte *)((tab) + 1)) + (index) * sizeof(eiTag))
#define TAG_ARRAY_INIT(tab)			(tab)->tags_size = 0; (tab)->tags_cap = 0;
#define TAG_ARRAY_CLEAR(tab)		(tab)->tags_size = 0;

static eiFORCEINLINE eiDataTable *tag_array_reserve_imp(
	eiDatabase *db, const eiTag tag, 
	eiDataTable *tab, const eiInt n)
{
	eiDBG_ASSERT(tab != NULL);

	/* got enough capacity case */
	if (tab->tags_cap >= n || n <= 0)
	{
		return tab;
	}

	/* tab->cap < n case */
	/* resize the data, retrieve the data table pointer */
	tab = (eiDataTable *)ei_db_resize(db, tag, sizeof(eiDataTable) + sizeof(eiTag) * (tab->tags_size + n));

	tab->tags_cap = n;

	return tab;
}

static eiFORCEINLINE eiDataTable *tag_array_add_imp(eiDatabase *db, const eiTag tag, eiDataTable *tab, const void *item)
{
	eiByte *data;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);
	eiDBG_ASSERT(item != NULL);

	data = (eiByte *)(tab + 1);

	/* make sure we have enough space to hold the new item */
	if (tab->tags_cap <= 0)
	{
		tab = tag_array_reserve_imp(db, tag, 
			tab, MAX(MIN_TAG_INCREMENT, ei_reserve_size(tab->tags_size)));
		/* retrieve the data pointer */
		data = (eiByte *)(tab + 1);
	}

	/* copy the new item */
	memcpy(data + sizeof(eiTag) * tab->tags_size, item, sizeof(eiTag));
	++ tab->tags_size;
	-- tab->tags_cap;

	/* dirt the table automatically */
	ei_db_dirt(db, tag);

	return tab;
}

#define TAG_ARRAY_ADD(db, tag, tab, item)	tag_array_add_imp(db, tag, tab, item)

void byteswap_data_block(eiDatabase *db, void *ptr, const eiUint size)
{
	eiDataBlock *pBlock;

	pBlock = (eiDataBlock *)ptr;

	ei_byteswap_array_typed(db, pBlock->type, pBlock->count, pBlock + 1);

	/* must byte-swap these at the end because they 
	   will still be used in previous code. */
	ei_byteswap_int(&pBlock->type);
	ei_byteswap_int(&pBlock->count);
}

void byteswap_data_table(eiDatabase *db, void *ptr, const eiUint size)
{
	eiDataTable *tab;

	tab = (eiDataTable *)ptr;

	ei_byteswap_int(&tab->tag);
	ei_byteswap_int(&tab->tags_size);
	ei_byteswap_int(&tab->tags_cap);
	ei_byteswap_int(&tab->item_count);
	ei_byteswap_int(&tab->current_items_available);
	ei_byteswap_int(&tab->item_type);
	ei_byteswap_int(&tab->items_per_slot);
	ei_byteswap_int(&tab->items_per_slot_shift);
}

static void ei_data_table_clear_items(eiDatabase *db, eiDataTable *tab)
{
	eiInt i;

	for (i = 0; i < TAG_ARRAY_SIZE(tab); ++i)
	{
		eiTag tag;
		
		tag = *((eiTag *)TAG_ARRAY_GET(tab, i));

		/* delete each data block */
		ei_db_delete(db, tag);
	}
}

eiTag ei_create_data_table(eiDatabase *db, const eiInt type, const eiInt items_per_slot)
{
	eiTag tag;
	eiDataTable *tab;

	eiDBG_ASSERT(db != NULL);

	tab = (eiDataTable *)ei_db_create(db, &tag, EI_DATA_TYPE_TABLE, sizeof(eiDataTable), EI_DB_FLUSHABLE);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	/* create an array of tags */
	TAG_ARRAY_INIT(tab);
	tab->tag = tag;
	tab->item_count = 0;
	tab->item_type = type;
	
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

	ei_db_end(db, tag);

	return tag;
}

void ei_delete_data_table(eiDatabase *db, const eiTag tag)
{
	eiDataTable *tab;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	tab = (eiDataTable *)ei_db_access(db, tag);

	/* clear data items */
	ei_data_table_clear_items(db, tab);

	/* clear the tag array */
	TAG_ARRAY_CLEAR(tab);

	ei_db_end(db, tag);

	ei_db_delete(db, tag);
}

void ei_data_table_clear(eiDatabase *db, const eiTag tag)
{
	eiDataTable *tab;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	tab = (eiDataTable *)ei_db_access(db, tag);

	/* clear data items */
	ei_data_table_clear_items(db, tab);

	/* clear the tag array */
	TAG_ARRAY_CLEAR(tab);
	tab->item_count = 0;

	ei_db_end(db, tag);

	/* dirt the table automatically */
	ei_db_dirt(db, tag);
}

void ei_data_table_begin(eiDatabase *db, const eiTag tag, eiDataTableIterator * const iter)
{
	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(iter != NULL);

	iter->db = db;
	/* access the table */
	/* we allow the tag to be eiNULL_TAG, in which case we will do nothing, 
	   so we can save the outside code for checking */
	iter->tab = NULL;
	if (tag != eiNULL_TAG)
	{
		iter->tab = (eiDataTable *)ei_db_access(db, tag);
	}
	iter->block_tag = eiNULL_TAG;
	iter->block = NULL;
}

static eiFORCEINLINE void *ei_data_table_access_imp(eiDataTableIterator * const iter, const eiInt index)
{
	eiInt slot_index;
	eiInt sub_index;
	eiTag block_tag;
	eiSizet item_size;

	eiDBG_ASSERT(iter != NULL);
	eiDBG_ASSERT(index >= 0 && index < iter->tab->item_count);

	/* calculate indices */
	slot_index = (index >> iter->tab->items_per_slot_shift);
	sub_index = index - (slot_index << iter->tab->items_per_slot_shift);
	
	/* get block tag */
	block_tag = *((eiTag *)TAG_ARRAY_GET(iter->tab, slot_index));

	/* get item size */
	item_size = ei_db_type_size(iter->db, iter->tab->item_type);

	/* access the block if it's not the current block */
	if (block_tag != iter->block_tag)
	{
		iter->block_tag = block_tag;
		iter->block = ((eiByte *)ei_db_access(iter->db, block_tag)) + sizeof(eiDataBlock);
	}

	return (iter->block + item_size * sub_index);
}

void *ei_data_table_read(eiDataTableIterator * const iter, const eiInt index)
{
	return ei_data_table_access_imp(iter, index);
}

void *ei_data_table_write(eiDataTableIterator * const iter, const eiInt index)
{
	void *ptr;
	
	ptr = ei_data_table_access_imp(iter, index);

	/* dirt the block automatically */
	eiDBG_ASSERT(iter->block_tag != eiNULL_TAG);

	ei_db_dirt(iter->db, iter->block_tag);

	return ptr;
}

void ei_data_table_end(eiDataTableIterator * const iter)
{
	eiDBG_ASSERT(iter != NULL);

	/* done access the block */
	if (iter->block_tag != eiNULL_TAG)
	{
		ei_db_end(iter->db, iter->block_tag);
		iter->block_tag = eiNULL_TAG;
		iter->block = NULL;
	}

	/* done access the table */
	if (iter->tab != NULL)
	{
		eiTag tab_tag;

		tab_tag = iter->tab->tag;

		if (tab_tag != eiNULL_TAG)
		{
			ei_db_end(iter->db, tab_tag);
		}

		iter->tab = NULL;
	}
}

eiBool ei_data_table_empty(eiDataTable *tab)
{
	eiDBG_ASSERT(tab != NULL);
	
	return (tab->item_count == 0);
}

eiInt ei_data_table_type(eiDataTable *tab)
{
	eiDBG_ASSERT(tab != NULL);

	return tab->item_type;
}

eiInt ei_data_table_size(eiDataTable *tab)
{
	eiDBG_ASSERT(tab != NULL);

	return tab->item_count;
}

eiInt ei_data_table_items_per_slot(eiDataTable *tab)
{
	eiDBG_ASSERT(tab != NULL);
	
	return tab->items_per_slot;
}

void ei_data_table_push_back(
	eiDatabase *db, 
	eiDataTable **tab, 
	const void *item)
{
	eiTag tag;
	eiSizet item_size;
	eiTag data_tag;
	eiDataBlock *pBlock;
	eiByte *current_items;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tab != NULL);
	eiDBG_ASSERT(*tab != NULL);
	eiDBG_ASSERT(item != NULL);

	tag = (*tab)->tag;
	eiDBG_ASSERT(tag != eiNULL_TAG);

	/* get item size */
	item_size = ei_db_type_size(db, (*tab)->item_type);

	/* when the back block fills up, 
	   create new block for writing */
	if ((*tab)->current_items_available == 0)
	{
		eiSizet		reserve_size;

		reserve_size = MAX((eiSizet)(*tab)->items_per_slot / BLOCK_DIV, ei_reserve_size(0));
		/* this implies that each block would at least have reserve_size items */
		/* the create will access the data implicitly */
		pBlock = (eiDataBlock *)ei_db_create(
			db, 
			&data_tag, 
			EI_DATA_TYPE_BLOCK, 
			sizeof(eiDataBlock) + item_size * reserve_size, 
			EI_DB_FLUSHABLE);

		pBlock->type = (*tab)->item_type;
		pBlock->count = reserve_size;

		*tab = TAG_ARRAY_ADD(db, tag, *tab, &data_tag);

		(*tab)->current_items_available = (*tab)->items_per_slot;
	}
	else
	{
		eiInt slot_index;
		eiInt sub_index;

		/* get the back block */
		slot_index = TAG_ARRAY_SIZE(*tab) - 1;
		data_tag = *((eiTag *)TAG_ARRAY_GET(*tab, slot_index));

		/* access current block */
		pBlock = (eiDataBlock *)ei_db_access(db, data_tag);

		/* calculate the sub-index to fill with the new item */
		sub_index = (*tab)->items_per_slot - (*tab)->current_items_available;

		/* resize the block if no enough memory has been allocated */
		if (sub_index >= pBlock->count)
		{
			eiInt new_count;

			new_count = pBlock->count + MAX((eiSizet)(*tab)->items_per_slot / BLOCK_DIV, ei_reserve_size(pBlock->count));

			pBlock = (eiDataBlock *)ei_db_resize(db, data_tag, sizeof(eiDataBlock) + item_size * new_count);
			pBlock->count = new_count;

			/* since the data tag remains unchanged, we don't 
			   need to set the new block to the tag array */
		}

		/* dirt the block automatically */
		ei_db_dirt(db, data_tag);
	}

	current_items = (eiByte *)(pBlock + 1);

	memcpy(current_items + item_size * ((*tab)->items_per_slot - (*tab)->current_items_available), 
		item, item_size);

	/* done access current block */
	ei_db_end(db, data_tag);

	-- (*tab)->current_items_available;
	++ (*tab)->item_count;

	/* dirt the table automatically */
	ei_db_dirt(db, tag);
}

void ei_data_table_reset_iterator(eiDataTableIterator *iter)
{
	iter->db = NULL;
	iter->tab = NULL;
	iter->block_tag = eiNULL_TAG;
	iter->block = NULL;
}
