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
 
#ifndef EI_DATA_BUFFER_H
#define EI_DATA_BUFFER_H

/** \brief The generic data buffer stored in database, and can be 
 * referenced by tag.
 * \file ei_data_buffer.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* forward declarations */
typedef struct eiDatabase	eiDatabase;

/* the byte-swapping callbacks for internal use only */
void byteswap_data_buffer(eiDatabase *db, void *ptr, const eiUint size);

/** \brief Create a buffer in database, returns its tag. */
eiAPI eiTag ei_create_data_buffer(
	eiDatabase *db, 
	const eiInt type);

/** \brief Delete the buffer from database. */
eiAPI void ei_delete_data_buffer(
	eiDatabase *db, 
	const eiTag tag);

/** \brief Allocate the buffer. */
eiAPI void ei_data_buffer_allocate(
	eiDatabase *db, 
	const eiTag tag, 
	const eiInt w, 
	const eiInt h);

/** \brief Get the size of buffer element in bytes. */
eiAPI eiInt ei_data_buffer_get_item_size(
	eiDatabase *db, 
	const eiTag tag);
/** \brief Get the data type of buffer element. */
eiAPI eiInt ei_data_buffer_get_type(
	eiDatabase *db, 
	const eiTag tag);
/** \brief Get the number of elements in this buffer. */
eiAPI eiInt ei_data_buffer_get_size(
	eiDatabase *db, 
	const eiTag tag);
/** \brief Get the width. */
eiAPI eiInt ei_data_buffer_get_width(
	eiDatabase *db, 
	const eiTag tag);
/** \brief Get the height. */
eiAPI eiInt ei_data_buffer_get_height(
	eiDatabase *db, 
	const eiTag tag);

/** \brief Get buffer element by coordinates. */
eiAPI void ei_data_buffer_get(
	eiDatabase *db, 
	const eiTag tag, 
	eiInt x, 
	eiInt y, 
	void *val);

/** \brief Get buffer element by coordinates in tiled mode. */
eiAPI void ei_data_buffer_get_tiled(
	eiDatabase *db, 
	const eiTag tag, 
	eiInt x, 
	eiInt y, 
	void *val);

/** \brief Set buffer element by coordinates. */
eiAPI void ei_data_buffer_set(
	eiDatabase *db, 
	const eiTag tag, 
	eiInt x, 
	eiInt y, 
	const void *val);

/** \brief Set buffer element by coordinates in tiled mode. */
eiAPI void ei_data_buffer_set_clamped(
	eiDatabase *db, 
	const eiTag tag, 
	eiInt x, 
	eiInt y, 
	const void *val);

/** \brief Zero the buffer memory. */
eiAPI void ei_data_buffer_zero_memory(
	eiDatabase *db, 
	const eiTag tag);

/** \brief Fill the buffer with an element. */
eiAPI void ei_data_buffer_fill(
	eiDatabase *db, 
	const eiTag tag, 
	const void *val);

/** \brief Clear the buffer. */
eiAPI void ei_data_buffer_clear(
	eiDatabase *db, 
	const eiTag tag);

#ifdef __cplusplus
}
#endif

#endif
