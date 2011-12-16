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

#ifndef EI_POOL_H
#define EI_POOL_H

/** \brief Generic memory pool for fast allocations of 
 * varying-sized small objects.
 * \file ei_pool.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EI_POOL_NUM_CHUNKS		32768

/** \brief Declaration of generic memory pool.
 */
typedef struct ei_pool ei_pool;

/** \brief Declaration of memory chunk header.
 */
typedef union ei_chunk_header ei_chunk_header;

/** \brief Generic memory pool object.
 */
struct ei_pool {
	ei_chunk_header		*memoryChunks[ EI_POOL_NUM_CHUNKS ];
	eiByte				*memoryPage;
	eiSizet				memoryAvailable;
	eiSizet				memoryUsage;
	ei_chunk_header		*memoryAllPages;
	/* statistics */
	eiUint				page_allocations;
	eiUint				huge_allocations;
};

/** \brief Initialize a pool.
 */
eiCORE_API void ei_pool_init(ei_pool *pool);

/** \brief Cleanup a pool.
 */
eiCORE_API void ei_pool_clear(ei_pool *pool);

/** \brief Allocate a chunk of memory from the pool.
 */
eiCORE_API void *ei_pool_allocate(ei_pool *pool, eiSizet size);

/** \brief Free a chunk of memory to the pool.
 */
eiCORE_API void ei_pool_free(ei_pool *pool, void *ptr);

#ifdef __cplusplus
}
#endif

#endif
