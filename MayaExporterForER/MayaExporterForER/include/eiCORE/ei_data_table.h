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

#ifndef EI_DATA_TABLE_H
#define EI_DATA_TABLE_H

/** \brief The generic data table which is stored in database, and 
 * can be referenced by tag. It's actually a block-based array which 
 * is good at dynamic increasing without huge memory allocations.
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>
#include <eiCORE/ei_dataflow.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Internal representation of data table.
 */
typedef struct eiDataTable {
	/* the tag of this data table */
	eiTag			tag;
	/* the number of items in tag array */
	eiInt			tags_size;
	/* the capacity of tag array */
	eiInt			tags_cap;
	eiInt			item_count;
	eiInt			current_items_available;
	/* constants describe this array */
	eiInt			item_type;
	eiInt			items_per_slot;
	eiInt			items_per_slot_shift;
} eiDataTable;

/** \brief The iterator for speeding up data table accesses.
 */
typedef struct eiDataTableIterator {
	/* the current database */
	eiDatabase		*db;
	/* the current data table */
	eiDataTable		*tab;
	/* the tag of current data block */
	eiTag			block_tag;
	/* the current data block */
	eiByte			*block;
} eiDataTableIterator;

/* the byte-swapping callbacks for internal use only */
void byteswap_data_block(eiDatabase *db, void *ptr, const eiUint size);
void byteswap_data_table(eiDatabase *db, void *ptr, const eiUint size);

/** \brief Initialize a data table in database, and returns its tag.
 */
eiCORE_API eiTag ei_create_data_table(eiDatabase *db, const eiInt type, const eiInt items_per_slot);

/** \brief Delete a data table by its tag.
 */
eiCORE_API void ei_delete_data_table(eiDatabase *db, const eiTag tag);

/** \brief Clear all data in the data table.
 */
eiCORE_API void ei_data_table_clear(eiDatabase *db, const eiTag tag);

/** \brief Begin accessing the data table.
 */
eiCORE_API void ei_data_table_begin(eiDatabase *db, const eiTag tag, eiDataTableIterator * const iter);

/** \brief Begin reading a data item by index.
 */
eiCORE_API void *ei_data_table_read(eiDataTableIterator * const iter, const eiInt index);

/** \brief Begin reading/writing a data item by index.
 */
eiCORE_API void *ei_data_table_write(eiDataTableIterator * const iter, const eiInt index);

/** \brief End accessing the data table.
 */
eiCORE_API void ei_data_table_end(eiDataTableIterator * const iter);

/** \brief Return true when the data table is empty.
 */
eiCORE_API eiBool ei_data_table_empty(eiDataTable *tab);

/** \brief Get the type of the data table.
 */
eiCORE_API eiInt ei_data_table_type(eiDataTable *tab);

/** \brief Get the current size of the data table.
 */
eiCORE_API eiInt ei_data_table_size(eiDataTable *tab);

/** \brief Get the number of items per slot.
 */
eiCORE_API eiInt ei_data_table_items_per_slot(eiDataTable *tab);

/** \brief Push a data item at the back.
 */
eiCORE_API void ei_data_table_push_back(
	eiDatabase *db, 
	eiDataTable **tab, 
	const void *item);

/** \brief Reset the data table iterator to an empty one.
 */
eiCORE_API void ei_data_table_reset_iterator(eiDataTableIterator *iter);

#ifdef __cplusplus
}
#endif

#endif
