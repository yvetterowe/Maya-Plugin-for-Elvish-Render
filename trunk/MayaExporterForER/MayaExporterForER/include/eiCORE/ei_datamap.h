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

#ifndef EI_DATAMAP_H
#define EI_DATAMAP_H

/** \brief Generic data map container, a special hash map.
 * \file ei_datamap.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_platform.h>
#include <eiCORE/ei_atomic_ops.h>
#include <eiCORE/ei_fixed_pool.h>
#include <eiCORE/ei_assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EI_DATAMAP_NUM_BLOCKS			10000
/* IMPORTANCE: this must be changed together with 
   EI_DATAMAP_BLOCK_SIZE_SHIFT */
#define EI_DATAMAP_BLOCK_SIZE			65536
#define EI_DATAMAP_BLOCK_SIZE_SHIFT		16

/** \brief The generic data map container.
 */
typedef struct eiDataMap {
	eiLock			lock;
	eiAtomicptr		blocks[ EI_DATAMAP_NUM_BLOCKS ];
	eiAtomic		size;
	ei_fixed_pool	pool;
} eiDataMap;

/** \brief Initialize the data map.
 */
eiCORE_API void ei_datamap_init(eiDataMap *map, const eiIntptr item_size);

/** \brief Clear all data in the map.
 */
eiCORE_API void ei_datamap_clear(eiDataMap *map);

/** \brief Insert a data into the map and return the allocated tag.
 */
eiCORE_API void *ei_datamap_insert(eiDataMap *map, const eiTag tag);

/** \brief Lookup a data by tag and return the data pointer.
 */
eiFORCEINLINE void *ei_datamap_find(eiDataMap *map, const eiTag tag)
{
	eiTag block_index;
	eiTag sub_index;

	/* TODO: we can remove the check of the tag by choosing data structure 
	carefully, we can set eiNULL_TAG to 0, and set the 0-th element of 
	data_list[] always NULL, then we don't need to check. */
	eiDBG_ASSERT(map != NULL && tag != eiNULL_TAG);

	block_index = (tag >> EI_DATAMAP_BLOCK_SIZE_SHIFT);
	sub_index = tag - (block_index << EI_DATAMAP_BLOCK_SIZE_SHIFT);

	return ((void **)ei_atomic_read(&map->blocks[block_index]))[sub_index];
}

/** \brief Delete a data by tag.
 */
eiCORE_API void ei_datamap_erase(eiDataMap *map, const eiTag tag);

/** \brief Get the number of data items, some data items may be NULL.
 */
eiFORCEINLINE eiTag ei_datamap_size(eiDataMap *map)
{
	eiDBG_ASSERT(map != NULL);

	return (eiTag)ei_atomic_read(&map->size);
}

#ifdef __cplusplus
}
#endif

#endif
