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
 
#ifndef EI_BUFFER_H
#define EI_BUFFER_H

/** \brief Generic buffers and frame buffers.
 * \file ei_buffer.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_data_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The callback to construct an item */
typedef void (*ei_buffer_init_item)(void *item);
/** \brief The callback to destruct an item */
typedef void (*ei_buffer_exit_item)(void *item);
/** \brief The callback to perform generic dst_item = src_item */
typedef void (*ei_buffer_copy_item)(void *dst_item, const void *src_item, const eiInt item_size);
/** \brief The callback to perform generic item = 0 */
typedef void (*ei_buffer_zero_item)(void *item, const eiInt item_size);
/** \brief The callback to perform generic item += value */
typedef void (*ei_buffer_add_item)(void *item, void *value, const eiInt item_size);
/** \brief The callback to perform generic item *= scale */
typedef void (*ei_buffer_mul_item)(void *item, const eiScalar scale, const eiInt item_size);

/** \brief This class represents a generic 
 * simple 2D array/grid. */
typedef struct eiBuffer {
	ei_buffer_init_item		m_init_item;
	ei_buffer_exit_item		m_exit_item;
	ei_buffer_copy_item		m_copy_item;
	ei_buffer_zero_item		m_zero_item;
	ei_buffer_add_item		m_add_item;
	ei_buffer_mul_item		m_mul_item;

	eiByte		*m_data;
	eiInt		m_item_size;
	eiInt		m_size;
	eiInt		m_width;
	eiInt		m_height;
	/* m_width - 1 */
	eiInt		m_width1;
	/* m_height - 1 */
	eiInt		m_height1;
} eiBuffer;

/** \brief Initialize a buffer. */
eiAPI void ei_buffer_init(
	eiBuffer *buffer, 
	const eiInt item_size, 
	ei_buffer_init_item init_item, 
	ei_buffer_exit_item exit_item, 
	ei_buffer_copy_item copy_item, 
	ei_buffer_zero_item zero_item, 
	ei_buffer_add_item add_item, 
	ei_buffer_mul_item mul_item);

/** \brief Allocate the buffer. */
eiAPI void ei_buffer_allocate(
	eiBuffer *buffer, 
	const eiInt w, 
	const eiInt h);

/** \brief Get size of each buffer element in bytes. */
eiAPI eiInt ei_buffer_get_item_size(eiBuffer *buffer);
/** \brief Get the number of elements in this buffer. */
eiAPI eiInt ei_buffer_get_size(eiBuffer *buffer);
/** \brief Get the width. */
eiAPI eiInt ei_buffer_get_width(eiBuffer *buffer);
/** \brief Get the height. */
eiAPI eiInt ei_buffer_get_height(eiBuffer *buffer);

/** \brief Get data pointer by coordinates. */
eiAPI void *ei_buffer_getptr(
	eiBuffer *buffer, 
	eiInt x, 
	eiInt y);

/** \brief Get data by coordinates. */
eiAPI void ei_buffer_get(
	eiBuffer *buffer, 
	eiInt x, 
	eiInt y, 
	void *val);

/** \brief Get data pointer by coordinates in tiled mode. */
eiAPI void *ei_buffer_getptr_tiled(
	eiBuffer *buffer, 
	eiInt x, 
	eiInt y);

/** \brief Get data by coordinates in tiled mode. */
eiAPI void ei_buffer_get_tiled(
	eiBuffer *buffer, 
	eiInt x, 
	eiInt y, 
	void *val);

/** \brief Set data by coordinates. */
eiAPI void ei_buffer_set(
	eiBuffer *buffer, 
	eiInt x, 
	eiInt y, 
	const void *val);

/** \brief Set data by coordinates with clamping. */
eiAPI void ei_buffer_set_clamped(
	eiBuffer *buffer, 
	eiInt x, 
	eiInt y, 
	const void *val);

/** \brief Zero the buffer memory. */
eiAPI void ei_buffer_zero_memory(eiBuffer *buffer);

/** \brief Fill the buffer with an element. */
eiAPI void ei_buffer_fill(
	eiBuffer *buffer, 
	const void *val);

/** \brief Clear the buffer. */
eiAPI void ei_buffer_clear(eiBuffer *buffer);

/** \brief Do rectangular filtering within radius. */
eiAPI void ei_buffer_filter(
	eiBuffer *buffer, 
	const eiInt radius);

/** \brief The interface for accessing a tile on 
 * a frame buffer. */
typedef struct eiFrameBufferCache {
	char				m_name[ EI_MAX_NODE_NAME_LEN ];
	eiDatabase			*m_db;
	eiTag				m_fb;
	eiInt				m_width;
	eiInt				m_height;
	/* data type of frame buffer elements, copied 
	   from frame buffer for fast access. */
	eiInt				m_type;
	eiInt				m_data_size;
	/* data offset in bytes in sample info block, 
	   copied from frame buffer for fast access. */
	eiUint				m_data_offset;
	eiInt				m_width1;
	eiInt				m_height1;
	eiInt				m_i;
	eiInt				m_j;
	char				*m_ptr;
	eiTag				m_tile_tag;
} eiFrameBufferCache;

eiAPI void ei_framebuffer_cache_init(
	eiFrameBufferCache *cache, 
	eiDatabase *db, 
	const eiTag fb, 
	const eiInt w, 
	const eiInt h, 
	const eiInt i, 
	const eiInt j);
eiAPI void ei_framebuffer_cache_exit(
	eiFrameBufferCache *cache);
eiAPI void ei_framebuffer_cache_flush(
	eiFrameBufferCache *cache);

eiAPI char *ei_framebuffer_cache_get_name(
	eiFrameBufferCache *cache);
eiAPI eiInt ei_framebuffer_cache_get_width(
	eiFrameBufferCache *cache);
eiAPI eiInt ei_framebuffer_cache_get_height(
	eiFrameBufferCache *cache);
eiAPI eiInt ei_framebuffer_cache_get_type(
	eiFrameBufferCache *cache);
eiAPI eiInt ei_framebuffer_cache_get_data_size(
	eiFrameBufferCache *cache);
eiAPI eiUint ei_framebuffer_cache_get_data_offset(
	eiFrameBufferCache *cache);
eiAPI eiByte *ei_framebuffer_cache_get_scanline(
	eiFrameBufferCache *cache, 
	eiInt y);
eiAPI eiInt ei_framebuffer_cache_get_scanline_size(
	eiFrameBufferCache *cache);
eiAPI void ei_framebuffer_cache_get(
	eiFrameBufferCache *cache, 
	eiInt x, 
	eiInt y, 
	void *data);
eiAPI void ei_framebuffer_cache_set(
	eiFrameBufferCache *cache, 
	eiInt x, 
	eiInt y, 
	const void *data);
eiAPI void ei_framebuffer_cache_paint(
	eiFrameBufferCache *cache, 
	eiInt x, 
	eiInt y, 
	eiInt r, 
	const void *data);
eiAPI eiByte *ei_framebuffer_cache_get_base_ptr(
	eiFrameBufferCache *cache);

/** \brief This class represents a frame buffer 
 * stored in database. */
typedef struct eiFrameBuffer {
	char			m_name[ EI_MAX_NODE_NAME_LEN ];
	eiInt			m_type;
	/* data offset in bytes in sample info block */
	eiUint			m_data_offset;
	eiInt			m_width;
	eiInt			m_height;
	eiInt			m_bucket_size;
	eiInt			m_num_xbuckets;
	eiInt			m_num_ybuckets;
	eiInt			m_num_xbuckets1;
	eiInt			m_num_ybuckets1;
	/* a data buffer of tags, each tag points to 
	   a frame buffer tile */
	eiTag			m_buckets;
} eiFrameBuffer;

/* the byte-swapping callbacks for internal use only */
void byteswap_framebuffer_tile(eiDatabase *db, void *ptr, const eiUint size);
void byteswap_framebuffer(eiDatabase *db, void *ptr, const eiUint size);

/** \brief Create a frame buffer in database. */
eiAPI eiTag ei_create_framebuffer(
	eiDatabase *db, 
	const char *name, 
	const eiInt type, 
	const eiUint data_offset, 
	const eiInt w, 
	const eiInt h, 
	const eiInt bs);
/** \brief Delete the frame buffer from database. */
eiAPI void ei_delete_framebuffer(
	eiDatabase *db, 
	const eiTag tag);

/** \brief Access a tile on frame buffer by tile indices. */
eiAPI eiByte *ei_framebuffer_access_tile(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt i, 
	eiInt j, 
	eiTag * const tile_tag);
/** \brief End access a tile on frame buffer by tile indices. */
eiAPI void ei_framebuffer_end_access_tile(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt i, 
	eiInt j);
/** \brief Flush a tile to make changes visible to all hosts. */
void ei_framebuffer_flush_tile(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt i, 
	eiInt j);

/** \brief Get buffer element by coordinates. */
eiAPI void ei_framebuffer_get(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt x, 
	eiInt y, 
	void *val);
/** \brief Get a segment on a scanline. */
eiAPI void ei_framebuffer_get_scanline(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt x, 
	eiInt y, 
	eiInt scanline_size, 
	eiByte *scanline);
/** \brief Set a segment on a scanline. */
eiAPI void ei_framebuffer_set_scanline(
	eiDatabase *db, 
	eiFrameBuffer *fb, 
	eiInt x, 
	eiInt y, 
	eiInt scanline_size, 
	eiByte *scanline);

#ifdef __cplusplus
}
#endif

#endif
