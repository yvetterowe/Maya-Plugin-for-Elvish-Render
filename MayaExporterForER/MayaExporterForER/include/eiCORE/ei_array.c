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

/** \brief Dynamic array container implementation.
 * \file ei_array.c
 * \author Elvic Liang
 */

#include <eiCORE/ei_array.h>
#include <eiCORE/ei_platform.h>

void ei_array_init(ei_array *arr, const eiIntptr item_size)
{
	eiDBG_ASSERT(arr != NULL);

	arr->data = NULL;
	arr->size = 0;
	arr->cap = 0;
	arr->item_size = item_size;
}

void ei_array_clear(ei_array *arr)
{
	eiDBG_ASSERT(arr != NULL);

	eiCHECK_FREE(arr->data);
	arr->size = 0;
	arr->cap = 0;
}

eiBool ei_array_empty(ei_array *arr)
{
	eiDBG_ASSERT(arr != NULL);

	return (arr->size == 0);
}

eiIntptr ei_array_size(ei_array *arr)
{
	eiDBG_ASSERT(arr != NULL);

	return arr->size;
}

eiIntptr ei_array_capacity(ei_array *arr)
{
	eiDBG_ASSERT(arr != NULL);

	/* returns (size + cap) which is C++ STL compatible value */
	return (arr->size + arr->cap);
}

void ei_array_push_back(ei_array *arr, const void *item)
{
	eiDBG_ASSERT(arr != NULL);

	/* make sure we have enough space to hold the new item */
	if (arr->cap <= 0)
	{
		ei_array_reserve(arr, ei_reserve_size(arr->size));
	}

	/* copy the new item */
	memcpy(arr->data + arr->item_size * arr->size, item, arr->item_size);
	++ arr->size;
	-- arr->cap;
}

void ei_array_erase(ei_array *arr, const eiIntptr index)
{
	eiIntptr i, end;

	eiDBG_ASSERT(arr != NULL);

	/* move the items after the deleting item */
	end = arr->size - 1;
	for (i = index; i < end; ++i)
	{
		memcpy(arr->data + arr->item_size * i, arr->data + arr->item_size * (i + 1), arr->item_size);
	}

	/* we do not really free up memory, 
	   we just adjust the capacity */
	++ arr->cap;
	-- arr->size;
}

void ei_array_copy(ei_array *dest, ei_array *src)
{
	/* being pure C, we cannot ensure data type 
	   matches, we just ensure data size matches */
	eiDBG_ASSERT(dest != NULL && src != NULL && 
		dest->item_size == src->item_size);

	/* copy to itself case, ignore */
	if (dest == src)
	{
		return;
	}

	if (ei_array_empty(src))
	{
		/* source is empty, clear the destination */
		ei_array_clear(dest);
	}
	else if (ei_array_capacity(dest) >= src->size)
	{
		/* destination has enough space, just copy */
		memcpy(dest->data, src->data, src->item_size * src->size);

		dest->cap = ei_array_capacity(dest) - src->size;
		dest->size = src->size;
	}
	else
	{
		/* destination got no enough space, 
		   allocation is required */
		eiCHECK_FREE(dest->data);

		dest->size = src->size;
		dest->data = ei_allocate(dest->item_size * dest->size);

		memcpy(dest->data, src->data, src->item_size * src->size);

		dest->cap = 0;
	}
}

void ei_array_reserve(ei_array *arr, const eiIntptr n)
{
	eiDBG_ASSERT(arr != NULL);

	/* got enough capacity case */
	if (arr->cap >= n || n <= 0)
	{
		return;
	}

	/* arr->cap < n case */
	if (arr->data == NULL)
	{
		/* data has not been allocated before, use allocate */
		arr->data = ei_allocate(arr->item_size * (arr->size + n));
		memset(arr->data, 0, arr->item_size * (arr->size + n));
	}
	else
	{
		/* data has been allocated before, use reallocate for efficiency */
		arr->data = ei_reallocate(arr->data, arr->item_size * (arr->size + n));
		memset(arr->data + arr->item_size * arr->size, 0, arr->item_size * n);
	}
	arr->cap = n;
}

void ei_array_resize(ei_array *arr, const eiIntptr n)
{
	eiDBG_ASSERT(arr != NULL);

	/* already enough size case */
	if (arr->size == n)
	{
		return;
	}
	
	if (arr->size > n)
	{
		/* size too large case, 
		   shrink the array, capacity increased */
		arr->cap += (arr->size - n);
	}
	else /* arr->size < n */
	{
		/* size not enough case, 
		   reserve for the missing part */
		/* we reserve more than just the missing part */
		eiSizet		reserve_size;
		
		reserve_size = ei_reserve_size(arr->size);
		ei_array_reserve(arr, n - arr->size + reserve_size);
		arr->cap = reserve_size;
	}

	arr->size = n;
}

void *ei_array_front(ei_array *arr)
{
	eiDBG_ASSERT(arr != NULL);

	return arr->data;
}

void *ei_array_back(ei_array *arr)
{
	eiDBG_ASSERT(arr != NULL);

	return (arr->data + arr->item_size * (arr->size - 1));
}

/* this function is identical to ei_array_front, 
   being here just for semantic sake */
void *ei_array_data(ei_array *arr)
{
	eiDBG_ASSERT(arr != NULL);

	return arr->data;
}
