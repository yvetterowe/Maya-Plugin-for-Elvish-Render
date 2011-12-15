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

#include <eiAPI/ei_data_buffer.h>
#include <eiCORE/ei_dataflow.h>
#include <eiCORE/ei_data_gen.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_assert.h>
#include <eiCORE/ei_util.h>

/** \brief Internal header of data buffer. */
typedef struct eiDataBuffer {
	/* the type of each item which is used for byte-swapping 
	   and getting item size. we are using types here for 
	   byte-swapping because we should not transfer local 
	   callback function pointer over network. */
	eiInt		m_type;
	eiInt		m_size;
	eiInt		m_width;
	eiInt		m_height;
	/* m_width - 1 */
	eiInt		m_width1;
	/* m_height - 1 */
	eiInt		m_height1;
} eiDataBuffer;

void byteswap_data_buffer(eiDatabase *db, void *ptr, const eiUint size)
{
	eiDataBuffer *buf;

	buf = (eiDataBuffer *)ptr;

	ei_byteswap_array_typed(db, buf->m_type, buf->m_size, buf + 1);

	/* must byte-swap these at the end because they 
	   will still be used in previous code. */
	ei_byteswap_int(&buf->m_type);
	ei_byteswap_int(&buf->m_size);
	ei_byteswap_int(&buf->m_width);
	ei_byteswap_int(&buf->m_height);
	ei_byteswap_int(&buf->m_width1);
	ei_byteswap_int(&buf->m_height1);
}

eiTag ei_create_data_buffer(
	eiDatabase *db, 
	const eiInt type)
{
	eiTag			tag;
	eiDataBuffer	*buf;

	eiDBG_ASSERT(db != NULL);

	buf = (eiDataBuffer *)ei_db_create(db, &tag, EI_DATA_TYPE_BUFFER, sizeof(eiDataBuffer), EI_DB_FLUSHABLE);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf->m_type = type;
	buf->m_size = 0;
	buf->m_width = 0;
	buf->m_height = 0;
	buf->m_width1 = 0;
	buf->m_height1 = 0;

	ei_db_end(db, tag);

	return tag;
}

void ei_delete_data_buffer(
	eiDatabase *db, 
	const eiTag tag)
{
	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	ei_db_delete(db, tag);
}

void ei_data_buffer_allocate(
	eiDatabase *db, 
	const eiTag tag, 
	const eiInt w, 
	const eiInt h)
{
	eiDataBuffer	*buf;
	eiSizet			item_size;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);
	eiDBG_ASSERT(buf != NULL);

	buf->m_size = w * h;
	buf->m_width = w;
	buf->m_height = h;
	buf->m_width1 = w - 1;
	buf->m_height1 = h - 1;

	item_size = ei_db_type_size(db, buf->m_type);

	/* resize the data, retrieve the data buffer pointer */
	buf = (eiDataBuffer *)ei_db_resize(db, tag, sizeof(eiDataBuffer) + item_size * buf->m_size);

	ei_db_end(db, tag);

	/* dirt the buffer automatically */
	ei_db_dirt(db, tag);
}

eiInt ei_data_buffer_get_item_size(
	eiDatabase *db, 
	const eiTag tag)
{
	eiDataBuffer	*buf;
	eiInt			item_size;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);

	item_size = (eiInt)ei_db_type_size(db, buf->m_type);

	ei_db_end(db, tag);

	return item_size;
}

eiInt ei_data_buffer_get_type(
	eiDatabase *db, 
	const eiTag tag)
{
	eiDataBuffer	*buf;
	eiInt			type;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);

	type = buf->m_type;

	ei_db_end(db, tag);

	return type;
}

eiInt ei_data_buffer_get_size(
	eiDatabase *db, 
	const eiTag tag)
{
	eiDataBuffer	*buf;
	eiInt			size;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);

	size = buf->m_size;

	ei_db_end(db, tag);

	return size;
}

eiInt ei_data_buffer_get_width(
	eiDatabase *db, 
	const eiTag tag)
{
	eiDataBuffer	*buf;
	eiInt			width;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);

	width = buf->m_width;

	ei_db_end(db, tag);

	return width;
}

eiInt ei_data_buffer_get_height(
	eiDatabase *db, 
	const eiTag tag)
{
	eiDataBuffer	*buf;
	eiInt			height;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);

	height = buf->m_height;

	ei_db_end(db, tag);

	return height;
}

void ei_data_buffer_get(
	eiDatabase *db, 
	const eiTag tag, 
	eiInt x, 
	eiInt y, 
	void *val)
{
	eiDataBuffer	*buf;
	eiByte			*data;
	eiSizet			item_size;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);

	data = (eiByte *)(buf + 1);
	item_size = ei_db_type_size(db, buf->m_type);

	clampi(x, 0, buf->m_width1);
	clampi(y, 0, buf->m_height1);
	memcpy(val, data + (x + y * buf->m_width) * item_size, item_size);

	ei_db_end(db, tag);
}

void ei_data_buffer_get_tiled(
	eiDatabase *db, 
	const eiTag tag, 
	eiInt x, 
	eiInt y, 
	void *val)
{
	eiDataBuffer	*buf;
	eiByte			*data;
	eiSizet			item_size;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);

	data = (eiByte *)(buf + 1);
	item_size = ei_db_type_size(db, buf->m_type);

	if (x < 0) {
		x = buf->m_width - ((-x) % buf->m_width) - 1;
	} else if (x >= buf->m_width) {
		x = x % buf->m_width;
	}
	if (y < 0) {
		y = buf->m_height - ((-y) % buf->m_height) - 1;
	} else if (y >= buf->m_height) {
		y = y % buf->m_height;
	}
	memcpy(val, data + (x + y * buf->m_width) * item_size, item_size);

	ei_db_end(db, tag);
}

void ei_data_buffer_set(
	eiDatabase *db, 
	const eiTag tag, 
	eiInt x, 
	eiInt y, 
	const void *val)
{
	eiDataBuffer	*buf;
	eiByte			*data;
	eiSizet			item_size;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);

	data = (eiByte *)(buf + 1);
	item_size = ei_db_type_size(db, buf->m_type);

	if (x >= 0 && x < buf->m_width && y >= 0 && y < buf->m_height)
	{
		memcpy(data + (x + y * buf->m_width) * item_size, val, item_size);
	}

	ei_db_end(db, tag);

	/* dirt the buffer automatically */
	ei_db_dirt(db, tag);
}

void ei_data_buffer_set_clamped(
	eiDatabase *db, 
	const eiTag tag, 
	eiInt x, 
	eiInt y, 
	const void *val)
{
	eiDataBuffer	*buf;
	eiByte			*data;
	eiSizet			item_size;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);

	data = (eiByte *)(buf + 1);
	item_size = ei_db_type_size(db, buf->m_type);

	clampi(x, 0, buf->m_width1);
	clampi(y, 0, buf->m_height1);
	memcpy(data + (x + y * buf->m_width) * item_size, val, item_size);

	ei_db_end(db, tag);

	/* dirt the buffer automatically */
	ei_db_dirt(db, tag);
}

void ei_data_buffer_zero_memory(
	eiDatabase *db, 
	const eiTag tag)
{
	eiDataBuffer	*buf;
	eiByte			*data;
	eiSizet			item_size;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);

	data = (eiByte *)(buf + 1);
	item_size = ei_db_type_size(db, buf->m_type);

	memset(data, 0, item_size * buf->m_size);

	ei_db_end(db, tag);

	/* dirt the buffer automatically */
	ei_db_dirt(db, tag);
}

void ei_data_buffer_fill(
	eiDatabase *db, 
	const eiTag tag, 
	const void *val)
{
	eiDataBuffer	*buf;
	eiByte			*data;
	eiSizet			item_size;
	eiInt			i;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);

	data = (eiByte *)(buf + 1);
	item_size = ei_db_type_size(db, buf->m_type);

	for (i = 0; i < buf->m_size; ++i)
	{
		memcpy(data + i * item_size, val, item_size);
	}

	ei_db_end(db, tag);

	/* dirt the buffer automatically */
	ei_db_dirt(db, tag);
}

void ei_data_buffer_clear(
	eiDatabase *db, 
	const eiTag tag)
{
	eiDataBuffer *buf;

	eiDBG_ASSERT(db != NULL);
	eiDBG_ASSERT(tag != eiNULL_TAG);

	buf = (eiDataBuffer *)ei_db_access(db, tag);

	buf = (eiDataBuffer *)ei_db_resize(db, tag, sizeof(eiDataBuffer));
	buf->m_size = 0;
	buf->m_width = 0;
	buf->m_height = 0;
	buf->m_width1 = 0;
	buf->m_height1 = 0;

	ei_db_end(db, tag);

	/* dirt the buffer automatically */
	ei_db_dirt(db, tag);
}
