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

#ifndef EI_FIXED_POOL_H
#define EI_FIXED_POOL_H

/** \brief Memory pool for allocating fixed-sized blocks.
 * \file ei_fixed_pool.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Declaration of fixed memory pool.
 */
typedef struct ei_fixed_pool ei_fixed_pool;

/** \brief Declaration of memory chunk node.
 */
typedef struct ei_chunk_node ei_chunk_node;

/** \brief Fixed memory pool object.
 */
struct ei_fixed_pool {
	ei_chunk_node	*freeItems;			/** Linked list of free items */
	ei_chunk_node	*allocatedBanks;	/** Linked list of allocated banks */
	eiInt			dataSize;			/** Data size of each item */
	eiInt			itemsPerBank;		/** The number of items per bank */
};

/** \brief Initialize a fixed pool.
 */
eiCORE_API void ei_fixed_pool_init(ei_fixed_pool *pool, const eiInt dataSize, const eiInt itemsPerBank);

/** \brief Cleanup a fixed pool.
 */
eiCORE_API void ei_fixed_pool_clear(ei_fixed_pool *pool);

/** \brief Allocate a chunk of memory from the fixed pool.
 */
eiCORE_API void *ei_fixed_pool_allocate(ei_fixed_pool *pool);

/** \brief Free a chunk of memory to the fixed pool.
 */
eiCORE_API void ei_fixed_pool_free(ei_fixed_pool *pool, void *item);

#ifdef __cplusplus
}
#endif

#endif
