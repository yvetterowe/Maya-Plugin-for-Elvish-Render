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

/** \brief Varying-sized memory pool implementation.
 * \file ei_pool.c
 * \author Elvic Liang
 */

#include <eiCORE/ei_pool.h>
#include <eiCORE/ei_platform.h>

#define	EI_POOL_MEMORY_PAGE_SIZE		(1 << 19)

/** \brief Memory chunk header, padded to 16 bytes.
 */
union ei_chunk_header {
	eiUint					index;
	union ei_chunk_header	*next;
	eiByte					padding[16];
};

void ei_pool_init(ei_pool *pool)
{
	eiInt i;

	for (i = 0; i < EI_POOL_NUM_CHUNKS; ++i)
	{
		pool->memoryChunks[i] = NULL;
	}
	pool->memoryAvailable = 0;
	pool->memoryUsage = 0;
	pool->memoryPage = NULL;
	pool->memoryAllPages = NULL;
	pool->page_allocations = 0;
	pool->huge_allocations = 0;
}

void ei_pool_clear(ei_pool *pool)
{
	eiInt i;
	ei_chunk_header *cPage;

	while ((cPage = pool->memoryAllPages) != NULL)
	{
		pool->memoryAllPages = cPage->next;
		eiCHECK_FREE(cPage);
	}

	for (i = 0; i < EI_POOL_NUM_CHUNKS; ++i)
	{
		pool->memoryChunks[i] = NULL;
	}
	pool->memoryAvailable = 0;
	pool->memoryUsage = 0;
	pool->memoryPage = NULL;
	pool->memoryAllPages = NULL;
	pool->page_allocations = 0;
	pool->huge_allocations = 0;
}

void ei_pool_new_page(ei_pool *pool, const eiUint size)
{
	eiUint asize;
	ei_chunk_header *cPage;

	++ pool->page_allocations;

	asize = MAX(size, EI_POOL_MEMORY_PAGE_SIZE);
	cPage = (ei_chunk_header *)ei_allocate(asize + sizeof(ei_chunk_header));
	cPage->next = pool->memoryAllPages;
	pool->memoryAllPages = cPage;
	pool->memoryPage = (eiByte *)(cPage + 1);
	pool->memoryAvailable = asize;
}

void *ei_pool_allocate(ei_pool *pool, eiSizet size)
{
	ei_chunk_header *ptri;
	eiUint index;

	if (size == 0)
	{
		return NULL;
	}
		
	index = (eiUint)(size >> 3);
	if (size & 7)
	{
		++ index;
	}

	if (index >= EI_POOL_NUM_CHUNKS)
	{
		++ pool->huge_allocations;
		ptri = (ei_chunk_header *)ei_allocate(size + sizeof(ei_chunk_header));
	}
	else
	{
		if ((ptri = pool->memoryChunks[index]) == NULL)
		{
			size = (index << 3) + sizeof(ei_chunk_header);
				
			if (size > pool->memoryAvailable)
			{
				ei_pool_new_page(pool, (eiUint)size);
			}

			ptri = (ei_chunk_header *)pool->memoryPage;
			pool->memoryPage += size;
			pool->memoryAvailable -= (eiUint)size;
		}
		else
		{
			pool->memoryChunks[index] = ptri->next;
		}
	}

	ptri->index = index;
	pool->memoryUsage += index;

	return (ptri + 1);
}

void ei_pool_free(ei_pool *pool, void *ptr)
{
	eiUint index;
	ei_chunk_header *ptri;

	if (ptr == NULL)
	{
		return;
	}

	ptri = (ei_chunk_header*)ptr;
	-- ptri;
	pool->memoryUsage -= ptri->index;

	if (ptri->index >= EI_POOL_NUM_CHUNKS)
	{
		eiCHECK_FREE(ptri);
	}
	else
	{
		index = ptri->index;
		ptri->next = pool->memoryChunks[index];
		pool->memoryChunks[index] = ptri;
	}
}
