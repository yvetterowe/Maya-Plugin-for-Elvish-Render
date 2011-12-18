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

#ifndef EI_DATA_ARRAY_H
#define EI_DATA_ARRAY_H

/** \brief The generic data array which is stored in database, and 
 * can be referenced by tag.
 * \file ei_data_array.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>
#include <eiCORE/ei_dataflow.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Internal header of data array.
 */
typedef struct eiDataArray {
	/* the type of each item which is used for byte-swapping 
	   and getting item size. we are using types here for 
	   byte-swapping because we should not transfer local 
	   callback function pointer over network. */
	eiInt		type;
	/* the number of items in this data array */
	eiInt		size;
	/* the capacity of this data array */
	eiInt		cap;
} eiDataArray;

/* the byte-swapping callbacks for internal use only */
eiCORE_API void ei_byteswap_array_typed(eiDatabase *db, const eiInt type, const eiInt count, void *arr);
void byteswap_data_array(eiDatabase *db, void *ptr, const eiUint size);

/** \brief Initialize a data array in database, and returns its tag.
 */
eiCORE_API eiTag ei_create_data_array(eiDatabase *db, const eiInt type);

/** \brief Delete a data array by its tag.
 */
eiCORE_API void ei_delete_data_array(eiDatabase *db, const eiTag tag);

/** \brief Clear all data in the data array.
 */
eiCORE_API void ei_data_array_clear(eiDatabase *db, const eiTag tag);

/** \brief Get data item by index.
 */
eiFORCEINLINE void *ei_data_array_get(eiDatabase *db, eiDataArray *arr, const eiInt index)
{
	return (((eiByte *)(arr + 1)) + index * ei_db_type_size(db, arr->type));
}

/** \brief Begin reading a data item by index.
 */
eiCORE_API void *ei_data_array_read(eiDatabase *db, const eiTag tag, const eiInt index);

/** \brief Begin reading/writing a data item by index.
 */
eiCORE_API void *ei_data_array_write(eiDatabase *db, const eiTag tag, const eiInt index);

/** \brief End accessing a data item by index.
 */
eiCORE_API void ei_data_array_end(eiDatabase *db, const eiTag tag, const eiInt index);

/** \brief Return true when the data array is empty.
 */
eiCORE_API eiBool ei_data_array_empty(eiDatabase *db, const eiTag tag);

/** \brief Get the type of the data array.
 */
eiCORE_API eiInt ei_data_array_type(eiDatabase *db, const eiTag tag);

/** \brief Get the current size of the data array.
 */
eiCORE_API eiInt ei_data_array_size(eiDatabase *db, const eiTag tag);

/** \brief Get the capacity of the data array, returns the C++ STL compatible value.
 */
eiCORE_API eiInt ei_data_array_capacity(eiDatabase *db, const eiTag tag);

/** \brief Push a data item at the back.
 */
eiCORE_API void ei_data_array_push_back(eiDatabase *db, const eiTag tag, const void *item);

/** \brief Erase a data item by index.
 */
eiCORE_API void ei_data_array_erase(eiDatabase *db, const eiTag tag, const eiInt index);

/** \brief Reserve the size of the data array for at least n elements.
 * This function is NOT C++ STL compatible, the n is the size to 
 * increase, not the resulting capacity.
 */
eiCORE_API void ei_data_array_reserve(eiDatabase *db, const eiTag tag, const eiInt n);

/** \brief Resize the array to n elements. This function is C++ STL 
 * compatible, the n is the resulting size of this container.
 */
eiCORE_API void ei_data_array_resize(eiDatabase *db, const eiTag tag, const eiInt n);

/** \brief Flush this data array with all hosts.
 */
eiCORE_API void ei_data_array_flush(eiDatabase *db, const eiTag tag);

#ifdef __cplusplus
}
#endif

#endif
