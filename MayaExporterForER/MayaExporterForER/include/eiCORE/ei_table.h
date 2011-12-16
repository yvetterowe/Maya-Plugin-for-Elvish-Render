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

#ifndef EI_TABLE_H
#define EI_TABLE_H

/** \brief The generic data table. It's actually a block-based 
 * array which is good at dynamic increasing without huge 
 * memory allocations.
 * \file ei_table.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>
#include <eiCORE/ei_array.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Declaration of dynamic table container.
 */
typedef struct ei_table ei_table;

/** \brief Table book-keeping object.
 */
struct ei_table {
	/* varying member variables */
	ei_array		tags;
	eiIntptr		item_count;
	eiIntptr		current_items_available;
	/* constants describe this array */
	eiIntptr		item_size;
	eiIntptr		items_per_slot;
	eiIntptr		items_per_slot_shift;
};

/** \brief Initialize a table.
 */
eiCORE_API void ei_table_init(ei_table *tab, const eiIntptr item_size, const eiIntptr items_per_slot);

/** \brief Clear the data in a table.
 */
eiCORE_API void ei_table_clear(ei_table *tab);

/** \brief Get a data item by index.
 */
eiCORE_API void *ei_table_get(ei_table *tab, const eiIntptr index);

/** \brief Return true when the table is empty.
 */
eiCORE_API eiBool ei_table_empty(ei_table *tab);

/** \brief Get the current size of the table.
 */
eiCORE_API eiIntptr ei_table_size(ei_table *tab);

/** \brief Push a data item at the back.
 */
eiCORE_API void ei_table_push_back(ei_table *tab, const void *item);

#ifdef __cplusplus
}
#endif

#endif
