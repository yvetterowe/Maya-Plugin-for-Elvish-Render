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

#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_data_gen.h>
#include <eiCORE/ei_assert.h>

void ei_byteswap_array_typed(eiDatabase *db, const eiInt type, const eiInt count, void *arr)
{
	eiInt		i;
	eiByte		*data;
	eiSizet		type_size;

	data = arr;
	type_size = ei_db_type_size(db, type);

	for (i = 0; i < count; ++i)
	{
		ei_db_byteswap_typed(db, type, data, type_size);
		data += type_size;
	}
}

void byteswap_data_array(eiDatabase *db, void *ptr, const eiUint size)
{
	eiDataArray *arr;

	arr = (eiDataArray *)ptr;

	ei_byteswap_array_typed(db, arr->type, arr->size, arr + 1);

	/* must byte-swap these at the end because they 
	   will still be used in previous code. */
	ei_byteswap_int(&arr->type);
	ei_byteswap_int(&arr->size);
	ei_byteswap_int(&arr->cap);
}

static eiDataArray *ei_data_array_reserve_imp(eiDatabase *db, const eiTag tag, 
											  eiDataArray *arr, const eiInt n)
{
	eiSizet item_size;

	eiDBG_ASSERT(arr != NULL);

	/* got enough capacity case */
	if (arr->cap >= n || n <= 0)
	{
		return arr;
	}

	item_size = ei_db_type_size(db, arr->type);

	/* arr->cap < n case */
	/* resize the data, retrieve the data array pointer */
	arr = (eiDataArray *)ei_db_resize(db, tag, sizeof(eiDataArray) + item_size * (arr->size + n));

	arr->cap = n;

	return arr;
}

eiTag ei_create_data_array(eiDatabase *db, const eiInt type)
{
	eiTag tag;
	eiDataArray *arr;

	eiDBG_ASSERT(db != NULL);

	arr = (eiDataArray *)ei_db_create(db, &tag, EI_DATA_TYPE_ARRAY, sizeof(eiDataArray), EI_DB_FLUSHABLE);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	arr->type = type;
	arr->size = 0;
	arr->cap = 0;

	ei_db_end(db, tag);

	return tag;
}

void ei_delete_data_array(eiDatabase *db, const eiTag tag)
{
	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	ei_db_delete(db, tag);
}

void ei_data_array_clear(eiDatabase *db, const eiTag tag)
{
	eiDataArray *arr;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	arr = (eiDataArray *)ei_db_access(db, tag);

	arr = (eiDataArray *)ei_db_resize(db, tag, sizeof(eiDataArray));
	arr->size = 0;
	arr->cap = 0;

	ei_db_end(db, tag);

	/* dirt the array automatically */
	ei_db_dirt(db, tag);
}

static eiFORCEINLINE void *ei_data_array_access_imp(eiDatabase *db, const eiTag tag, const eiInt index)
{
	eiDataArray *arr;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	arr = (eiDataArray *)ei_db_access(db, tag);

	return (((eiByte *)(arr + 1)) + index * ei_db_type_size(db, arr->type));
}

void *ei_data_array_read(eiDatabase *db, const eiTag tag, const eiInt index)
{
	return ei_data_array_access_imp(db, tag, index);
}

void *ei_data_array_write(eiDatabase *db, const eiTag tag, const eiInt index)
{
	void *ptr;

	ptr = ei_data_array_access_imp(db, tag, index);

	/* dirt the data array automatically */
	ei_db_dirt(db, tag);

	return ptr;
}

void ei_data_array_end(eiDatabase *db, const eiTag tag, const eiInt index)
{
	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	ei_db_end(db, tag);
}

eiBool ei_data_array_empty(eiDatabase *db, const eiTag tag)
{
	eiDataArray *arr;
	eiBool empty;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	arr = (eiDataArray *)ei_db_access(db, tag);

	empty = (arr->size == 0);

	ei_db_end(db, tag);

	return empty;
}

eiInt ei_data_array_type(eiDatabase *db, const eiTag tag)
{
	eiDataArray *arr;
	eiInt type;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	arr = (eiDataArray *)ei_db_access(db, tag);

	type = arr->type;

	ei_db_end(db, tag);

	return type;
}

eiInt ei_data_array_size(eiDatabase *db, const eiTag tag)
{
	eiDataArray *arr;
	eiInt size;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	arr = (eiDataArray *)ei_db_access(db, tag);

	size = arr->size;

	ei_db_end(db, tag);

	return size;
}

eiInt ei_data_array_capacity(eiDatabase *db, const eiTag tag)
{
	eiDataArray *arr;
	eiInt cap;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	arr = (eiDataArray *)ei_db_access(db, tag);

	/* returns (size + cap) which is C++ STL compatible value */
	cap = (arr->size + arr->cap);

	ei_db_end(db, tag);

	return cap;
}

void ei_data_array_push_back(eiDatabase *db, const eiTag tag, const void *item)
{
	eiDataArray *arr;
	eiByte *data;
	eiSizet item_size;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);
	eiDBG_ASSERT(item != NULL);

	arr = (eiDataArray *)ei_db_access(db, tag);
	data = (eiByte *)(arr + 1);
	item_size = ei_db_type_size(db, arr->type);

	/* make sure we have enough space to hold the new item */
	if (arr->cap <= 0)
	{
		arr = ei_data_array_reserve_imp(db, tag, 
			arr, ei_reserve_size(arr->size));
		/* retrieve the data pointer */
		data = (eiByte *)(arr + 1);
	}

	/* copy the new item */
	memcpy(data + item_size * arr->size, item, item_size);
	++ arr->size;
	-- arr->cap;

	ei_db_end(db, tag);

	/* dirt the array automatically */
	ei_db_dirt(db, tag);
}

void ei_data_array_erase(eiDatabase *db, const eiTag tag, const eiInt index)
{
	eiDataArray *arr;
	eiByte *data;
	eiSizet item_size;
	eiInt i, end;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	arr = (eiDataArray *)ei_db_access(db, tag);

	eiDBG_ASSERT(arr != NULL);

	data = (eiByte *)(arr + 1);
	item_size = ei_db_type_size(db, arr->type);

	/* move the items after the deleting item */
	end = arr->size - 1;
	for (i = index; i < end; ++i)
	{
		memcpy(data + item_size * i, data + item_size * (i + 1), item_size);
	}

	/* we do not really free up memory, 
	   we just adjust the capacity */
	++ arr->cap;
	-- arr->size;

	ei_db_end(db, tag);

	/* dirt the array automatically */
	ei_db_dirt(db, tag);
}

void ei_data_array_reserve(eiDatabase *db, const eiTag tag, const eiInt n)
{
	eiDataArray *arr;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	arr = (eiDataArray *)ei_db_access(db, tag);

	/* retrieve the data array pointer just in case it 
	   will be used for some purposes in the future. */
	arr = ei_data_array_reserve_imp(db, tag, arr, n);

	ei_db_end(db, tag);

	/* dirt the array automatically */
	ei_db_dirt(db, tag);
}

void ei_data_array_resize(eiDatabase *db, const eiTag tag, const eiInt n)
{
	eiDataArray *arr;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	arr = (eiDataArray *)ei_db_access(db, tag);

	eiDBG_ASSERT(arr != NULL);

	/* already enough size case */
	if (arr->size == n)
	{
		ei_db_end(db, tag);
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
		/* TODO: should we reserve more than just 
		   the missing part? */
		arr = ei_data_array_reserve_imp(db, tag, 
			arr, n - arr->size);
	}

	arr->size = n;

	ei_db_end(db, tag);

	/* dirt the array automatically */
	ei_db_dirt(db, tag);
}

void ei_data_array_flush(eiDatabase *db, const eiTag tag)
{
	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	ei_db_flush(db, tag);
}
