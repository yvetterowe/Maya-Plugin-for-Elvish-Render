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

#include <eiAPI/ei_buffer.h>
#include <eiCORE/ei_dataflow.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_assert.h>
#include <eiCORE/ei_util.h>

/** \brief A frame buffer tile stored in database. */
typedef struct eiFrameBufferTile {
	eiInt			type;
	eiInt			width;
	eiInt			height;
	eiUint			padding;
} eiFrameBufferTile;

static void ei_buffer_default_copy_item(void *dst_item, const void *src_item, const eiInt item_size)
{
	memcpy(dst_item, src_item, item_size);
}

void ei_buffer_init(
	eiBuffer *buffer, 
	const eiInt item_size, 
	ei_buffer_init_item init_item, 
	ei_buffer_exit_item exit_item, 
	ei_buffer_copy_item copy_item, 
	ei_buffer_zero_item zero_item, 
	ei_buffer_add_item add_item, 
	ei_buffer_mul_item mul_item)
{
	buffer->m_init_item = init_item;
	buffer->m_exit_item = exit_item;
	buffer->m_copy_item = copy_item;
	/* use a default copy item if no explicit callback is specified */
	if (buffer->m_copy_item == NULL)
	{
		buffer->m_copy_item = ei_buffer_default_copy_item;
	}
	buffer->m_zero_item = zero_item;
	buffer->m_add_item = add_item;
	buffer->m_mul_item = mul_item;

	buffer->m_data = NULL;
	buffer->m_item_size = item_size;
	buffer->m_size = 0;
	buffer->m_width = 0;
	buffer->m_height = 0;
	buffer->m_width1 = 0;
	buffer->m_height1 = 0;
}

void ei_buffer_allocate(
	eiBuffer *buffer, 
	const eiInt w, 
	const eiInt h)
{
	buffer->m_size = w * h;
	buffer->m_data = (eiByte *)ei_allocate(buffer->m_item_size * buffer->m_size);
	buffer->m_width = w;
	buffer->m_height = h;
	buffer->m_width1 = w - 1;
	buffer->m_height1 = h - 1;

	/* call constructor for all items if available */
	if (buffer->m_init_item != NULL)
	{
		eiInt	i;

		for (i = 0; i < buffer->m_size; ++i)
		{
			buffer->m_init_item(buffer->m_data + i * buffer->m_item_size);
		}
	}
}

eiInt ei_buffer_get_item_size(eiBuffer *buffer)
{
	return buffer->m_item_size;
}

eiInt ei_buffer_get_size(eiBuffer *buffer)
{
	return buffer->m_size;
}

eiInt ei_buffer_get_width(eiBuffer *buffer)
{
	return buffer->m_width;
}

eiInt ei_buffer_get_height(eiBuffer *buffer)
{
	return buffer->m_height;
}

void *ei_buffer_getptr(
	eiBuffer *buffer, 
	eiInt x, 
	eiInt y)
{
	clampi(x, 0, buffer->m_width1);
	clampi(y, 0, buffer->m_height1);
	return (buffer->m_data + (x + y * buffer->m_width) * buffer->m_item_size);
}

void ei_buffer_get(
	eiBuffer *buffer, 
	eiInt x, 
	eiInt y, 
	void *val)
{
	buffer->m_copy_item(val, ei_buffer_getptr(buffer, x, y), buffer->m_item_size);
}

void *ei_buffer_getptr_tiled(
	eiBuffer *buffer, 
	eiInt x, 
	eiInt y)
{
	if (x < 0) {
		x = buffer->m_width - ((-x) % buffer->m_width) - 1;
	} else if (x >= buffer->m_width) {
		x = x % buffer->m_width;
	}
	if (y < 0) {
		y = buffer->m_height - ((-y) % buffer->m_height) - 1;
	} else if (y >= buffer->m_height) {
		y = y % buffer->m_height;
	}
	return (buffer->m_data + (x + y * buffer->m_width) * buffer->m_item_size);
}

void ei_buffer_get_tiled(
	eiBuffer *buffer, 
	eiInt x, 
	eiInt y, 
	void *val)
{
	buffer->m_copy_item(val, ei_buffer_getptr_tiled(buffer, x, y), buffer->m_item_size);
}

void ei_buffer_set(
	eiBuffer *buffer, 
	eiInt x, 
	eiInt y, 
	const void *val)
{
	if (x >= 0 && x < buffer->m_width && y >= 0 && y < buffer->m_height)
	{
		buffer->m_copy_item(buffer->m_data + (x + y * buffer->m_width) * buffer->m_item_size, val, buffer->m_item_size);
	}
}

void ei_buffer_set_clamped(
	eiBuffer *buffer, 
	eiInt x, 
	eiInt y, 
	const void *val)
{
	clampi(x, 0, buffer->m_width1);
	clampi(y, 0, buffer->m_height1);
	buffer->m_copy_item(buffer->m_data + (x + y * buffer->m_width) * buffer->m_item_size, val, buffer->m_item_size);
}

void ei_buffer_zero_memory(eiBuffer *buffer)
{
	memset(buffer->m_data, 0, buffer->m_item_size * buffer->m_size);
}

void ei_buffer_fill(
	eiBuffer *buffer, 
	const void *val)
{
	eiInt	i;

	for (i = 0; i < buffer->m_size; ++i)
	{
		buffer->m_copy_item(buffer->m_data + i * buffer->m_item_size, val, buffer->m_item_size);
	}
}

void ei_buffer_clear(eiBuffer *buffer)
{
	/* call destructor for all items if available */
	if (buffer->m_exit_item != NULL)
	{
		eiInt	i;

		for (i = 0; i < buffer->m_size; ++i)
		{
			buffer->m_exit_item(buffer->m_data + i * buffer->m_item_size);
		}
	}

	eiCHECK_FREE(buffer->m_data);
	buffer->m_size = 0;
	buffer->m_width = 0;
	buffer->m_height = 0;
	buffer->m_width1 = 0;
	buffer->m_height1 = 0;
}

void ei_buffer_filter(
	eiBuffer *buffer, 
	const eiInt radius)
{
	eiScalar	inv;
	eiInt		i, j, k;
	void		*avg;

	eiASSERT(buffer->m_zero_item != NULL);
	eiASSERT(buffer->m_add_item != NULL);
	eiASSERT(buffer->m_mul_item != NULL);

	avg = _alloca(buffer->m_item_size);

	inv = 1.0f / (eiScalar)(2 * radius + 1);
	for (j = 0; j < buffer->m_height; ++j) {
		for (i = 0; i < buffer->m_width; ++i) {
			buffer->m_copy_item(avg, ei_buffer_getptr(buffer, i, j), buffer->m_item_size);
			buffer->m_zero_item(avg, buffer->m_item_size);
			for (k = - radius; k <= radius; ++k) {
				buffer->m_add_item(avg, ei_buffer_getptr_tiled(buffer, i, j + k), buffer->m_item_size);
			}
			buffer->m_mul_item(avg, inv, buffer->m_item_size);
			ei_buffer_set(buffer, i, j, avg);
		}
	}
	for (j = 0; j < buffer->m_height; ++j) {
		for (i = 0; i < buffer->m_width; ++i) {
			buffer->m_copy_item(avg, ei_buffer_getptr(buffer, i, j), buffer->m_item_size);
			buffer->m_zero_item(avg, buffer->m_item_size);
			for (k = - radius; k <= radius; ++k) {
				buffer->m_add_item(avg, ei_buffer_getptr_tiled(buffer, i + k, j), buffer->m_item_size);
			}
			buffer->m_mul_item(avg, inv, buffer->m_item_size);
			ei_buffer_set(buffer, i, j, avg);
		}
	}
}

void ei_framebuffer_cache_init(
	eiFrameBufferCache *cache, 
	eiDatabase *db, 
	const eiTag fb, 
	const eiInt w, 
	const eiInt h, 
	const eiInt i, 
	const eiInt j)
{
	eiFrameBuffer	*frameBuffer;

	frameBuffer = (eiFrameBuffer *)ei_db_access(db, fb);

	strncpy(cache->m_name, frameBuffer->m_name, EI_MAX_NODE_NAME_LEN - 1);
	cache->m_db = db;
	cache->m_fb = fb;
	cache->m_width = w;
	cache->m_height = h;
	cache->m_type = frameBuffer->m_type;
	cache->m_data_size = ei_db_type_size(db, frameBuffer->m_type);
	cache->m_data_offset = frameBuffer->m_data_offset;
	cache->m_width1 = cache->m_width - 1;
	cache->m_height1 = cache->m_height - 1;
	cache->m_i = i;
	cache->m_j = j;

	cache->m_ptr = ei_framebuffer_access_tile(
		cache->m_db, frameBuffer, cache->m_i, cache->m_j, &cache->m_tile_tag);

	ei_db_end(db, fb);
}

void ei_framebuffer_cache_exit(
	eiFrameBufferCache *cache)
{
	eiFrameBuffer	*frameBuffer;

	frameBuffer = (eiFrameBuffer *)ei_db_access(cache->m_db, cache->m_fb);

	ei_framebuffer_end_access_tile(cache->m_db, frameBuffer, cache->m_i, cache->m_j);

	ei_db_end(cache->m_db, cache->m_fb);
}

void ei_framebuffer_cache_flush(
	eiFrameBufferCache *cache)
{
	eiFrameBuffer	*frameBuffer;

	frameBuffer = (eiFrameBuffer *)ei_db_access(cache->m_db, cache->m_fb);

	ei_framebuffer_flush_tile(cache->m_db, frameBuffer, cache->m_i, cache->m_j);

	ei_db_end(cache->m_db, cache->m_fb);
}

char *ei_framebuffer_cache_get_name(
	eiFrameBufferCache *cache)
{
	return cache->m_name;
}

eiInt ei_framebuffer_cache_get_width(
	eiFrameBufferCache *cache)
{
	return cache->m_width;
}

eiInt ei_framebuffer_cache_get_height(
	eiFrameBufferCache *cache)
{
	return cache->m_height;
}

eiInt ei_framebuffer_cache_get_type(
	eiFrameBufferCache *cache)
{
	return cache->m_type;
}

eiInt ei_framebuffer_cache_get_data_size(
	eiFrameBufferCache *cache)
{
	return cache->m_data_size;
}

eiUint ei_framebuffer_cache_get_data_offset(
	eiFrameBufferCache *cache)
{
	return cache->m_data_offset;
}

eiByte *ei_framebuffer_cache_get_scanline(
	eiFrameBufferCache *cache, 
	eiInt y)
{
	return ei_framebuffer_cache_get_base_ptr(cache) + (y * cache->m_width) * cache->m_data_size;
}

eiInt ei_framebuffer_cache_get_scanline_size(
	eiFrameBufferCache *cache)
{
	return cache->m_width * cache->m_data_size;
}

void ei_framebuffer_cache_get(
	eiFrameBufferCache *cache, 
	eiInt x, 
	eiInt y, 
	void *data)
{
	clampi(x, 0, cache->m_width1);
	clampi(y, 0, cache->m_height1);
	memcpy(data, 
		ei_framebuffer_cache_get_base_ptr(cache) 
		+ (x + y * cache->m_width) * cache->m_data_size, 
		cache->m_data_size);
}

void ei_framebuffer_cache_set(
	eiFrameBufferCache *cache, 
	eiInt x, 
	eiInt y, 
	const void *data)
{
	if (x >= 0 && x < cache->m_width && y >= 0 && y < cache->m_height)
	{
		memcpy(ei_framebuffer_cache_get_base_ptr(cache) 
			+ (x + y * cache->m_width) * cache->m_data_size, 
			data, 
			cache->m_data_size);

		/* dirt the tile automatically */
		ei_db_dirt(cache->m_db, cache->m_tile_tag);
	}
}

void ei_framebuffer_cache_paint(
	eiFrameBufferCache *cache, 
	eiInt x, 
	eiInt y, 
	eiInt r, 
	const void *data)
{
	/* draw a hexagon */
	eiInt h = r;
	eiInt w = 0;
	eiInt h1 = (eiInt)((0.25f + 0.375f) * (eiScalar)h);
	eiInt h2 = (eiInt)(0.375f * (eiScalar)h);
	eiInt i, j;

	for (j = -h1; j < -h2; ++j)
	{
		for (i = -w; i <= w; ++i)
		{
			ei_framebuffer_cache_set(cache, x + i, y + j, data);
		}
		w += 2;
	}
	for (j = -h2; j < h2; ++j)
	{
		for (i = -w; i <= w; ++i)
		{
			ei_framebuffer_cache_set(cache, x + i, y + j, data);
		}
	}
	for (j = h2; j <= h1; ++j)
	{
		for (i = -w; i <= w; ++i)
		{
			ei_framebuffer_cache_set(cache, x + i, y + j, data);
		}
		w -= 2;
	}
}

eiByte *ei_framebuffer_cache_get_base_ptr(
	eiFrameBufferCache *cache)
{
	return cache->m_ptr;
}

void byteswap_framebuffer_tile(eiDatabase *db, void *ptr, const eiUint size)
{
	eiFrameBufferTile	*tile;

	tile = (eiFrameBufferTile *)ptr;

	ei_byteswap_array_typed(db, tile->type, tile->width * tile->height, tile + 1);

	/* must byte-swap these at the end because they 
	   will still be used in previous code. */
	ei_byteswap_int(&tile->type);
	ei_byteswap_int(&tile->width);
	ei_byteswap_int(&tile->height);
}

void byteswap_framebuffer(eiDatabase *db, void *ptr, const eiUint size)
{
	eiFrameBuffer	*fb;

	fb = (eiFrameBuffer *)ptr;

	ei_byteswap_int(&fb->m_type);
	ei_byteswap_int(&fb->m_data_offset);
	ei_byteswap_int(&fb->m_width);
	ei_byteswap_int(&fb->m_height);
	ei_byteswap_int(&fb->m_bucket_size);
	ei_byteswap_int(&fb->m_num_xbuckets);
	ei_byteswap_int(&fb->m_num_ybuckets);
	ei_byteswap_int(&fb->m_num_xbuckets1);
	ei_byteswap_int(&fb->m_num_ybuckets1);
	ei_byteswap_int(&fb->m_buckets);
}

eiTag ei_create_framebuffer(
	eiDatabase *db, 
	const char *name, 
	const eiInt type, 
	const eiUint data_offset, 
	const eiInt w, 
	const eiInt h, 
	const eiInt bs)
{
	eiTag			tag;
	eiFrameBuffer	*fb;
	eiInt			data_size;
	eiInt			i, j;

	fb = (eiFrameBuffer *)ei_db_create(db, 
		&tag, 
		EI_DATA_TYPE_FRAMEBUFFER, 
		sizeof(eiFrameBuffer), 
		EI_DB_FLUSHABLE);

	strncpy(fb->m_name, name, EI_MAX_NODE_NAME_LEN - 1);

	fb->m_type = type;
	fb->m_data_offset = data_offset;
	fb->m_width = w;
	fb->m_height = h;
	fb->m_bucket_size = bs;
	fb->m_num_xbuckets = fb->m_width / fb->m_bucket_size;
	fb->m_num_ybuckets = fb->m_height / fb->m_bucket_size;
	if (fb->m_num_xbuckets == 0) {
		fb->m_num_xbuckets = 1;
	}
	if (fb->m_num_ybuckets == 0) {
		fb->m_num_ybuckets = 1;
	}
	fb->m_num_xbuckets1 = fb->m_num_xbuckets - 1;
	fb->m_num_ybuckets1 = fb->m_num_ybuckets - 1;

	fb->m_buckets = ei_create_data_buffer(db, EI_DATA_TYPE_TAG);
	ei_data_buffer_allocate(db, fb->m_buckets, fb->m_num_xbuckets, fb->m_num_ybuckets);

	data_size = ei_db_type_size(db, fb->m_type);

	for (j = 0; j < fb->m_num_ybuckets; ++j)
	{
		for (i = 0; i < fb->m_num_xbuckets; ++i)
		{
			eiInt				bucket_width, bucket_height;
			eiInt				size;
			eiTag				tile_tag;
			eiFrameBufferTile	*tile;

			if (i == fb->m_num_xbuckets1)
			{
				bucket_width = fb->m_width - (i * fb->m_bucket_size);
			}
			else
			{
				bucket_width = fb->m_bucket_size;
			}

			if (j == fb->m_num_ybuckets1)
			{
				bucket_height = fb->m_height - (j * fb->m_bucket_size);
			}
			else
			{
				bucket_height = fb->m_bucket_size;
			}

			size = bucket_width * bucket_height * data_size;

			tile = (eiFrameBufferTile *)ei_db_create(
				db, 
				&tile_tag, 
				EI_DATA_TYPE_FRAMEBUFFER_TILE, 
				sizeof(eiFrameBufferTile) + size, 
				EI_DB_FLUSHABLE);

			tile->type = fb->m_type;
			tile->width = bucket_width;
			tile->height = bucket_height;

			/* zero the memory */
			memset((eiByte *)(tile + 1), 0, size);

			ei_db_end(db, tile_tag);

			ei_data_buffer_set(db, fb->m_buckets, i, j, &tile_tag);
		}
	}

	ei_db_end(db, tag);

	return tag;
}

void ei_delete_framebuffer(
	eiDatabase *db, 
	const eiTag tag)
{
	eiFrameBuffer	*fb;
	eiTag			tile_tag;
	eiInt			i, j;

	fb = (eiFrameBuffer *)ei_db_access(db, tag);

	for (j = 0; j < fb->m_num_ybuckets; ++j)
	{
		for (i = 0; i < fb->m_num_xbuckets; ++i)
		{
			ei_data_buffer_get(db, fb->m_buckets, i, j, &tile_tag);

			ei_db_delete(db, tile_tag);
		}
	}
	ei_delete_data_buffer(db, fb->m_buckets);

	ei_db_end(db, tag);

	ei_db_delete(db, tag);
}

eiByte *ei_framebuffer_access_tile(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt i, 
	eiInt j, 
	eiTag * const tile_tag)
{
	eiFrameBufferTile	*tile;

	ei_data_buffer_get(db, fb->m_buckets, i, j, tile_tag);

	tile = (eiFrameBufferTile *)ei_db_access(db, *tile_tag);

	return (eiByte *)(tile + 1);
}

void ei_framebuffer_end_access_tile(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt i, 
	eiInt j)
{
	eiTag	tile_tag;

	ei_data_buffer_get(db, fb->m_buckets, i, j, &tile_tag);

	ei_db_end(db, tile_tag);
}

void ei_framebuffer_flush_tile(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt i, 
	eiInt j)
{
	eiTag	tile_tag;

	ei_data_buffer_get(db, fb->m_buckets, i, j, &tile_tag);

	ei_db_flush(db, tile_tag);
}

static void ei_framebuffer_get_data(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt i, 
	eiInt j, 
	eiInt x, 
	eiInt y, 
	eiInt data_size, 
	eiInt scanline_size, 
	eiByte *scanline)
{
	eiFrameBufferTile	*tile;
	eiByte				*ptr;
	eiTag				tile_tag;

	ei_data_buffer_get(db, fb->m_buckets, i, j, &tile_tag);

	tile = (eiFrameBufferTile *)ei_db_access(db, tile_tag);
	ptr = (eiByte *)(tile + 1);

	memcpy(scanline, ptr + (x + y * tile->width) * data_size, scanline_size);

	ei_db_end(db, tile_tag);
}

static void ei_framebuffer_set_data(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt i, 
	eiInt j, 
	eiInt x, 
	eiInt y, 
	eiInt data_size, 
	eiInt scanline_size, 
	eiByte *scanline)
{
	eiFrameBufferTile	*tile;
	eiByte				*ptr;
	eiTag				tile_tag;

	ei_data_buffer_get(db, fb->m_buckets, i, j, &tile_tag);

	tile = (eiFrameBufferTile *)ei_db_access(db, tile_tag);
	ptr = (eiByte *)(tile + 1);

	memcpy(ptr + (x + y * tile->width) * data_size, scanline, scanline_size);

	ei_db_end(db, tile_tag);

	/* dirt the tile automatically */
	ei_db_dirt(db, tile_tag);
}

void ei_framebuffer_get(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt x, 
	eiInt y, 
	void *val)
{
	eiInt			data_size;
	eiInt			i, j;

	data_size = (eiInt)ei_db_type_size(db, fb->m_type);

	i = x / fb->m_bucket_size;
	j = y / fb->m_bucket_size;
	clampi(i, 0, fb->m_num_xbuckets1);
	clampi(j, 0, fb->m_num_ybuckets1);
	x = x - i * fb->m_bucket_size;
	y = y - j * fb->m_bucket_size;

	ei_framebuffer_get_data(db, fb, i, j, x, y, data_size, data_size, (eiByte *)val);
}

void ei_framebuffer_get_scanline(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt x, 
	eiInt y, 
	eiInt scanline_size, 
	eiByte *scanline)
{
	eiInt			data_size;
	eiInt			i, j;

	data_size = (eiInt)ei_db_type_size(db, fb->m_type);

	i = x / fb->m_bucket_size;
	j = y / fb->m_bucket_size;
	clampi(i, 0, fb->m_num_xbuckets1);
	clampi(j, 0, fb->m_num_ybuckets1);
	x = x - i * fb->m_bucket_size;
	y = y - j * fb->m_bucket_size;
	
	ei_framebuffer_get_data(db, fb, i, j, x, y, data_size, scanline_size, scanline);
}

void ei_framebuffer_set_scanline(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt x, 
	eiInt y, 
	eiInt scanline_size, 
	eiByte *scanline)
{
	eiInt			data_size;
	eiInt			i, j;

	data_size = (eiInt)ei_db_type_size(db, fb->m_type);

	i = x / fb->m_bucket_size;
	j = y / fb->m_bucket_size;
	clampi(i, 0, fb->m_num_xbuckets1);
	clampi(j, 0, fb->m_num_ybuckets1);
	x = x - i * fb->m_bucket_size;
	y = y - j * fb->m_bucket_size;

	ei_framebuffer_set_data(db, fb, i, j, x, y, data_size, scanline_size, scanline);
}
