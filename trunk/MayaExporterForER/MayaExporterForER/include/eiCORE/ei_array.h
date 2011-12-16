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

#ifndef EI_ARRAY_H
#define EI_ARRAY_H

/** \brief Dynamic array container.
 * \file ei_array.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>
#include <eiCORE/ei_assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Declaration of dynamic array container.
 */
typedef struct ei_array ei_array;

/** \brief Array book-keeping object.
 */
struct ei_array {
	eiByte		*data;		/** Data block pointer */
	eiIntptr	item_size;	/** Size of a data item in bytes */
	eiIntptr	size;		/** Number of items in this array */
	eiIntptr	cap;		/** Capacity of this array */
};

/** \brief Initialize an array.
 */
eiCORE_API void ei_array_init(ei_array *arr, const eiIntptr item_size);

/** \brief Clear the data in an array.
 */
eiCORE_API void ei_array_clear(ei_array *arr);

/** \brief Get a data item by index.
 */
eiFORCEINLINE void *ei_array_get(ei_array *arr, const eiIntptr index)
{
	eiDBG_ASSERT(arr != NULL);

	return (arr->data + arr->item_size * index);
}

/** \brief Return true when the array is empty.
 */
eiCORE_API eiBool ei_array_empty(ei_array *arr);

/** \brief Get the current size of the array.
 */
eiCORE_API eiIntptr ei_array_size(ei_array *arr);

/** \brief Get the capacity of the array, returns the C++ STL compatible value.
 */
eiCORE_API eiIntptr ei_array_capacity(ei_array *arr);

/** \brief Push a data item at the back.
 */
eiCORE_API void ei_array_push_back(ei_array *arr, const void *item);

/** \brief Erase a data item by index.
 */
eiCORE_API void ei_array_erase(ei_array *arr, const eiIntptr index);

/** \brief Copy from an existing array.
 */
eiCORE_API void ei_array_copy(ei_array *dest, ei_array *src);

/** \brief Reserve the size of the array for at least n elements.
 * This function is NOT C++ STL compatible, the n is the size to 
 * increase, not the resulting capacity.
 */
eiCORE_API void ei_array_reserve(ei_array *arr, const eiIntptr n);

/** \brief Resize the array to n elements. This function is C++ STL 
 * compatible, the n is the resulting size of this container.
 */
eiCORE_API void ei_array_resize(ei_array *arr, const eiIntptr n);

/** \brief Get the front data item.
 */
eiCORE_API void *ei_array_front(ei_array *arr);

/** \brief Get the back data item.
 */
eiCORE_API void *ei_array_back(ei_array *arr);

/** \brief Get the data buffer of the array.
 */
eiCORE_API void *ei_array_data(ei_array *arr);

#ifdef __cplusplus
}
#endif

#endif
