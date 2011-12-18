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

#include <eiAPI/ei_texture.h>
#include <eiAPI/ei_data_buffer.h>
#include <eiAPI/ei_image.h>
#include <eiCORE/ei_dataflow.h>
#include <eiCORE/ei_filesys.h>
#include <eiCORE/ei_assert.h>

/** \brief This class represents a texture tile at 
 * certain MIP-MAP level, it's a cache slot. */
typedef struct eiTextureTile {
	/* the tag of the texture map which contains 
	   this texture tile. */
	eiTag		m_map_tag;
	eiInt		m_data_offset;
	eiInt		m_data_size;
	eiUint		padding;
} eiTextureTile;

/** \brief This class represents a single MIP-MAP 
 * level consists of many texture tiles. */
typedef struct eiTextureLayer {
	/* a data buffer of tags */
	eiTag		m_tiles;
	eiInt		m_data_offset;
	eiScalar	m_width;
	eiScalar	m_height;
	eiInt		m_x_tiles;
	eiInt		m_y_tiles;
	eiInt		m_tiled_width;
	eiInt		m_tiled_height;
} eiTextureLayer;

/** \brief This class represents a texture map 
 * consists of many MIP-MAP levels. */
typedef struct eiTextureMap {
	/* the resolved texture file name */
	char		m_name[ EI_MAX_FILE_NAME_LEN ];
	/* if this texture map is local, it should be 
	   loaded from file in local disk for both rendering 
	   manager and rendering servers, otherwise in 
	   the case of non-local texture map, it should be 
	   loaded from remote host over the network for 
	   rendering servers, and loaded from local disk 
	   for rendering manager. */
	eiBool		m_local;
	eiInt		m_tile_size;
	eiInt		m_num_channels;
	eiInt		m_channel_size;
	eiInt		m_swrap;
	eiInt		m_twrap;
	eiScalar	m_width;
	eiScalar	m_height;
	eiUint		m_num_layers;
} eiTextureMap;

/* compute number of tiles, we use one more full-sized tile if cannot divide 
   exactly, in order to equalize the cache slot size. */
void ei_compute_num_tiles(
	eiInt *x_tiles, eiInt *y_tiles, 
	const eiScalar width, const eiScalar height, const eiInt tile_size)
{
	*x_tiles = lceilf(width / (eiScalar)tile_size);
	*y_tiles = lceilf(height / (eiScalar)tile_size);
}

static void ei_texture_tile_init(
	eiTextureTile *tile, 
	const eiTag map_tag, const eiInt data_offset, const eiInt data_size)
{
	tile->m_map_tag = map_tag;
	tile->m_data_offset = data_offset;
	tile->m_data_size = data_size;
}

static void ei_texture_tile_lookup_scalar(
	eiTextureTile *tile, 
	eiTextureMap *map, 
	eiScalar *value, 
	const eiUint channel, 
	const eiInt x, 
	const eiInt y)
{
	eiByte	*slot_mem;
	eiByte	*ptr;

	slot_mem = (eiByte *)(tile + 1);
	ptr = slot_mem + (x + y * map->m_tile_size) * map->m_num_channels * map->m_channel_size;

	if (channel < (eiUint)map->m_num_channels)
	{
		switch (map->m_channel_size)
		{
		case 1:
			/* 8 bits per channel */
			*value = ((eiScalar)((eiByte *)ptr)[ channel ]) * (1.0f / 255.0f);
			break;
		case 2:
			/* 16 bits per channel */
			*value = ((eiScalar)((eiUshort *)ptr)[ channel ]) * (1.0f / 65535.0f);
			break;
		case 4:
			/* 32 bits per channel */
			*value = ((eiScalar *)ptr)[ channel ];
			break;
		default:
			*value = 0.0f;
			break;
		}
	}
	else
	{
		*value = 0.0f;
	}
}

static void ei_texture_tile_lookup_vector(
	eiTextureTile *tile, 
	eiTextureMap *map, 
	eiVector *value, 
	const eiUint channel, 
	const eiInt x, 
	const eiInt y)
{
	ei_texture_tile_lookup_scalar(tile, map, &value->comp[0], channel + 0, x, y);
	ei_texture_tile_lookup_scalar(tile, map, &value->comp[1], channel + 1, x, y);
	ei_texture_tile_lookup_scalar(tile, map, &value->comp[2], channel + 2, x, y);
}

static void ei_texture_layer_init(
	eiTextureLayer *layer, 
	eiTextureMap *map, 
	const eiTag map_tag, 
	const eiInt data_offset, const eiScalar width, const eiScalar height, 
	eiDatabase *db)
{
	eiInt	offset;
	eiInt	data_size;
	eiInt	i, j;

	layer->m_data_offset = data_offset;
	layer->m_width = width;
	layer->m_height = height;

	ei_compute_num_tiles(&layer->m_x_tiles, &layer->m_y_tiles, layer->m_width, layer->m_height, map->m_tile_size);

	/* tiled width and height for fast wrapping */
	layer->m_tiled_width = layer->m_x_tiles * map->m_tile_size;
	layer->m_tiled_height = layer->m_y_tiles * map->m_tile_size;

	/* create texture tiles */
	layer->m_tiles = ei_create_data_buffer(db, EI_DATA_TYPE_TAG);
	ei_data_buffer_allocate(db, layer->m_tiles, layer->m_x_tiles, layer->m_y_tiles);

	offset = layer->m_data_offset;
	data_size = map->m_tile_size * map->m_tile_size * map->m_num_channels * map->m_channel_size;

	for (j = 0; j < layer->m_y_tiles; ++j)
	{
		for (i = 0; i < layer->m_x_tiles; ++i)
		{
			eiTextureTile	*tile;
			eiTag			tile_tag;

			tile = (eiTextureTile *)ei_db_create(
				db, 
				&tile_tag, 
				EI_DATA_TYPE_TEXTURE_TILE, 
				sizeof(eiTextureTile), 
				EI_DB_FLUSHABLE | EI_DB_GEN_LOCAL | EI_DB_GEN_ALWAYS);

			ei_texture_tile_init(
				tile, 
				map_tag, offset, data_size);

			ei_db_end(db, tile_tag);

			ei_data_buffer_set(db, layer->m_tiles, i, j, &tile_tag);

			offset += data_size;
		}
	}
}

static void ei_texture_layer_exit(eiTextureLayer *layer, eiDatabase *db)
{
	/* delete all texture tiles in this texture layer */
	eiTag		tile_tag;
	eiInt		i, j;

	for (j = 0; j < layer->m_y_tiles; ++j)
	{
		for (i = 0; i < layer->m_x_tiles; ++i)
		{
			ei_data_buffer_get(db, layer->m_tiles, i, j, &tile_tag);

			ei_db_delete(db, tile_tag);
		}
	}
	ei_delete_data_buffer(db, layer->m_tiles);
}

static void ei_texture_layer_lookup_scalar(
	eiTextureLayer *layer, 
	eiTextureMap *map, 
	eiScalar *value, 
	const eiUint channel, 
	eiInt x, 
	eiInt y, 
	eiDatabase *db)
{
	eiInt			tile_x;
	eiInt			tile_y;
	eiInt			sub_x;
	eiInt			sub_y;
	eiTag			tile_tag;
	eiTextureTile	*tile;

	ei_wrap_texcoord(&x, map->m_swrap, layer->m_tiled_width);
	ei_wrap_texcoord(&y, map->m_twrap, layer->m_tiled_height);

	tile_x = x / map->m_tile_size;
	tile_y = y / map->m_tile_size;
	sub_x = x - tile_x * map->m_tile_size;
	sub_y = y - tile_y * map->m_tile_size;

	ei_data_buffer_get(db, layer->m_tiles, tile_x, tile_y, &tile_tag);

	tile = (eiTextureTile *)ei_db_access(db, tile_tag);

	ei_texture_tile_lookup_scalar(tile, map, value, channel, sub_x, sub_y);

	ei_db_end(db, tile_tag);
}

static void ei_texture_layer_lookup_vector(
	eiTextureLayer *layer, 
	eiTextureMap *map, 
	eiVector *value, 
	const eiUint channel, 
	eiInt x, 
	eiInt y, 
	eiDatabase *db)
{
	eiInt			tile_x;
	eiInt			tile_y;
	eiInt			sub_x;
	eiInt			sub_y;
	eiTag			tile_tag;
	eiTextureTile	*tile;

	ei_wrap_texcoord(&x, map->m_swrap, layer->m_tiled_width);
	ei_wrap_texcoord(&y, map->m_twrap, layer->m_tiled_height);

	tile_x = x / map->m_tile_size;
	tile_y = y / map->m_tile_size;
	sub_x = x - tile_x * map->m_tile_size;
	sub_y = y - tile_y * map->m_tile_size;

	ei_data_buffer_get(db, layer->m_tiles, tile_x, tile_y, &tile_tag);

	tile = (eiTextureTile *)ei_db_access(db, tile_tag);

	ei_texture_tile_lookup_vector(tile, map, value, channel, sub_x, sub_y);

	ei_db_end(db, tile_tag);
}

static void ei_texture_map_exit(
	eiTextureMap *map, 
	eiDatabase *db)
{
	eiUint			i;
	eiTextureLayer	*layer;

	/* destruct all texture layers */
	layer = (eiTextureLayer *)(map + 1);

	for (i = 0; i < map->m_num_layers; ++i)
	{
		ei_texture_layer_exit(layer, db);
		++ layer;
	}

	/* the texture file will be automatically closed 
	   by the database system, no need to handle it here. */
}

static eiTag ei_create_texture_map_from_file(
	eiDatabase *db, 
	const char *filename, 
	const eiBool local)
{
	eiUint				i;
	eiTag				tag;
	eiTextureMap		*map;
	eiFileSystem		*pFileSystem;
	eiFile				*pFile;
	eiTextureHeader		header;
	eiTextureLayer		*layer;
	eiTextureLayerInfo	layerInfo;

	/* open texture file */
	pFileSystem = ei_db_file_system(db);
	pFile = ei_filesys_open(pFileSystem, filename, EI_FILE_READ);

	ei_file_read(pFile, &header, sizeof(eiTextureHeader));

	if (header.format_code != EI_TEXTURE_FILE_CODE)
	{
		ei_error("Invalid texture file %s\n", filename);

		/* close the texture file */
		ei_filesys_close(pFileSystem, pFile);

		return eiNULL_TAG;
	}

	map = (eiTextureMap *)ei_db_create(db, 
		&tag, 
		EI_DATA_TYPE_TEXTURE_MAP, 
		sizeof(eiTextureMap) + 
		header.num_layers * sizeof(eiTextureLayer), 
		EI_DB_FLUSHABLE | EI_DB_GEN_LOCAL);

	strncpy(map->m_name, filename, EI_MAX_FILE_NAME_LEN - 1);
	map->m_local = local;
	map->m_tile_size = header.tile_size;
	map->m_num_channels = header.num_channels;
	map->m_channel_size = header.channel_size;
	map->m_swrap = header.swrap;
	map->m_twrap = header.twrap;
	map->m_num_layers = header.num_layers;

	/* construct all texture layers */
	layer = (eiTextureLayer *)(map + 1);

	for (i = 0; i < map->m_num_layers; ++i)
	{
		ei_file_read(pFile, &layerInfo, sizeof(eiTextureLayerInfo));

		ei_texture_layer_init(
			layer, 
			map, 
			tag, 
			layerInfo.data_offset, layerInfo.width, layerInfo.height, 
			db);

		if (i == 0)
		{
			map->m_width = layerInfo.width;
			map->m_height = layerInfo.height;
		}

		++ layer;
	}

	ei_db_end(db, tag);

	/* close the texture file */
	ei_filesys_close(pFileSystem, pFile);

	return tag;
}

static void ei_delete_texture_map(eiDatabase *db, const eiTag tag)
{
	eiTextureMap	*map;

	/* destruct the texture map */
	map = (eiTextureMap *)ei_db_access(db, tag);

	ei_texture_map_exit(map, db);

	ei_db_end(db, tag);

	/* delete the texture map */
	ei_db_delete(db, tag);
}

static void ei_texture_map_lookup_scalar_lod(
	eiTextureMap *map, 
	eiScalar *value, 
	eiUint layer, 
	const eiUint channel, 
	const eiScalar s, const eiScalar t, 
	eiDatabase *db)
{
	eiTextureLayer	*pLayer;
	eiScalar		x;
	eiScalar		y;
	eiInt			u;
	eiInt			v;
	eiScalar		du;
	eiScalar		dv;
	eiScalar		f1, f2, f3, f4, r1, r2;

	if (layer >= map->m_num_layers)
	{
		/* clamp the layer to biggest available layer */
		layer = map->m_num_layers - 1;
	}

	pLayer = ((eiTextureLayer *)(map + 1)) + layer;

	x = s * pLayer->m_width;
	y = t * pLayer->m_height;
	u = truncf(x);
	v = truncf(y);
	du = curve(x - u);
	dv = curve(y - v);

	ei_texture_layer_lookup_scalar(pLayer, map, &f1, channel, u + 0, v + 0, db);
	ei_texture_layer_lookup_scalar(pLayer, map, &f2, channel, u + 1, v + 0, db);
	ei_texture_layer_lookup_scalar(pLayer, map, &f3, channel, u + 0, v + 1, db);
	ei_texture_layer_lookup_scalar(pLayer, map, &f4, channel, u + 1, v + 1, db);

	lerp(&r1, f1, f2, du);
	lerp(&r2, f3, f4, du);
	lerp(value, r1, r2, dv);
}

static void ei_texture_map_lookup_vector_lod(
	eiTextureMap *map, 
	eiVector *value, 
	eiUint layer, 
	const eiUint channel, 
	const eiScalar s, const eiScalar t, 
	eiDatabase *db)
{
	eiTextureLayer	*pLayer;
	eiScalar		x;
	eiScalar		y;
	eiInt			u;
	eiInt			v;
	eiScalar		du;
	eiScalar		dv;
	eiVector		f1, f2, f3, f4, r1, r2;

	if (layer >= map->m_num_layers)
	{
		/* clamp the layer to biggest available layer */
		layer = map->m_num_layers - 1;
	}

	pLayer = ((eiTextureLayer *)(map + 1)) + layer;

	x = s * pLayer->m_width;
	y = t * pLayer->m_height;
	u = truncf(x);
	v = truncf(y);
	du = curve(x - u);
	dv = curve(y - v);

	ei_texture_layer_lookup_vector(pLayer, map, &f1, channel, u + 0, v + 0, db);
	ei_texture_layer_lookup_vector(pLayer, map, &f2, channel, u + 1, v + 0, db);
	ei_texture_layer_lookup_vector(pLayer, map, &f3, channel, u + 0, v + 1, db);
	ei_texture_layer_lookup_vector(pLayer, map, &f4, channel, u + 1, v + 1, db);

	lerp3(&r1, &f1, &f2, du);
	lerp3(&r2, &f3, &f4, du);
	lerp3(value, &r1, &r2, dv);
}

static void ei_texture_map_lookup_scalar(
	eiTextureMap *map, 
	eiScalar *value, 
	const eiUint channel, 
	const eiScalar s, const eiScalar t, 
	eiDatabase *db)
{
	ei_texture_map_lookup_scalar_lod(
		map, 
		value, 
		0, 
		channel, 
		s, t, 
		db);
}

static void ei_texture_map_lookup_vector(
	eiTextureMap *map, 
	eiVector *value, 
	const eiUint channel, 
	const eiScalar s, const eiScalar t, 
	eiDatabase *db)
{
	ei_texture_map_lookup_vector_lod(
		map, 
		value, 
		0, 
		channel, 
		s, t, 
		db);
}

static void ei_texture_map_lookup_scalar_filtered(
	eiTextureMap *map, 
	eiScalar *value, 
	const eiUint channel, 
	const eiScalar s1, const eiScalar t1, 
	const eiScalar s2, const eiScalar t2, 
	const eiScalar s3, const eiScalar t3, 
	const eiScalar s4, const eiScalar t4, 
	eiDatabase *db)
{
	eiScalar	s, t, ds, dt, d, c1, c2;
	eiInt		di;

	s = MIN(s1, MIN(s2, MIN(s3, s4)));
	t = MIN(t1, MIN(t2, MIN(t3, t4)));
	ds = MAX(s1, MAX(s2, MAX(s3, s4))) - s;
	dt = MAX(t1, MAX(t2, MAX(t3, t4))) - t;
	ds = MAX(ds * map->m_width, 1.0f);
	dt = MAX(dt * map->m_height, 1.0f);
	d = MAX(log2f(ds), log2f(dt));
	di = truncf(d);
	d = curve(d - (eiScalar)di);
	ei_texture_map_lookup_scalar_lod(map, &c1, di, channel, s, t, db);
	ei_texture_map_lookup_scalar_lod(map, &c2, di + 1, channel, s, t, db);
	lerp(value, c1, c2, d);
}

static void ei_texture_map_lookup_vector_filtered(
	eiTextureMap *map, 
	eiVector *value, 
	const eiUint channel, 
	const eiScalar s1, const eiScalar t1, 
	const eiScalar s2, const eiScalar t2, 
	const eiScalar s3, const eiScalar t3, 
	const eiScalar s4, const eiScalar t4, 
	eiDatabase *db)
{
	eiScalar	s, t, ds, dt, d;
	eiVector	c1, c2;
	eiInt		di;

	s = MIN(s1, MIN(s2, MIN(s3, s4)));
	t = MIN(t1, MIN(t2, MIN(t3, t4)));
	ds = MAX(s1, MAX(s2, MAX(s3, s4))) - s;
	dt = MAX(t1, MAX(t2, MAX(t3, t4))) - t;
	ds = MAX(ds * map->m_width, 1.0f);
	dt = MAX(dt * map->m_height, 1.0f);
	d = MAX(log2f(ds), log2f(dt));
	di = truncf(d);
	d = curve(d - (eiScalar)di);
	ei_texture_map_lookup_vector_lod(map, &c1, di, channel, s, t, db);
	ei_texture_map_lookup_vector_lod(map, &c2, di + 1, channel, s, t, db);
	lerp3(value, &c1, &c2, d);
}

void ei_lookup_scalar_texture(
	eiDatabase *db, 
	eiScalar *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s, const eiScalar t)
{
	eiTextureMap	*map;

	map = (eiTextureMap *)ei_db_access(db, tag);

	ei_texture_map_lookup_scalar(
		map, 
		value, 
		channel, 
		s, t, 
		db);

	ei_db_end(db, tag);
}

void ei_lookup_vector_texture(
	eiDatabase *db, 
	eiVector *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s, const eiScalar t)
{
	eiTextureMap	*map;

	map = (eiTextureMap *)ei_db_access(db, tag);

	ei_texture_map_lookup_vector(
		map, 
		value, 
		channel, 
		s, t, 
		db);

	ei_db_end(db, tag);
}

void ei_lookup_scalar_texture_filtered(
	eiDatabase *db, 
	eiScalar *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s1, const eiScalar t1, 
	const eiScalar s2, const eiScalar t2, 
	const eiScalar s3, const eiScalar t3, 
	const eiScalar s4, const eiScalar t4)
{
	eiTextureMap	*map;

	map = (eiTextureMap *)ei_db_access(db, tag);

	ei_texture_map_lookup_scalar_filtered(
		map, 
		value, 
		channel, 
		s1, t1, 
		s2, t2, 
		s3, t3, 
		s4, t4, 
		db);

	ei_db_end(db, tag);
}

void ei_lookup_vector_texture_filtered(
	eiDatabase *db, 
	eiVector *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s1, const eiScalar t1, 
	const eiScalar s2, const eiScalar t2, 
	const eiScalar s3, const eiScalar t3, 
	const eiScalar s4, const eiScalar t4)
{
	eiTextureMap	*map;

	map = (eiTextureMap *)ei_db_access(db, tag);

	ei_texture_map_lookup_vector_filtered(
		map, 
		value, 
		channel, 
		s1, t1, 
		s2, t2, 
		s3, t3, 
		s4, t4, 
		db);

	ei_db_end(db, tag);
}

void generate_texture_tile(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls)
{
	eiTextureTile	*tile;
	eiData			*map_data;
	eiFile			*pFile;
	eiBool			is_local;
	eiByte			*slot_mem;
	eiFileSystem	*pFileSystem;

	eiDBG_ASSERT(pData != NULL && pData->ptr != NULL);
	tile = (eiTextureTile *)pData->ptr;

	/* get texture file handle from texture map */
	map_data = ei_db_access_info(db, tile->m_map_tag);
	pFile = map_data->file;
	is_local = ((eiTextureMap *)map_data->ptr)->m_local;
	ei_db_end(db, tile->m_map_tag);

	/* if the texture map is non-local, and we are not the host 
	   that generated this data, we can receive it from the host 
	   on which it's generated. */
	if (!is_local && pData->host != ei_tls_get_host(pTls))
	{
		/* we are already in data generator, so we don't want to 
		   defer the initialization, we want the sender to generate 
		   it immediately. */
		ei_db_net_recv_from_host(db, data_tag, pData, pTls, eiFALSE);
		return;
	}

	/* resize the texture tile for holding pixels, 
	   if the texture tile has been in the size we 
	   want, no redundant resizing will be performed. */
	tile = (eiTextureTile *)ei_db_resize(db, 
		data_tag, 
		sizeof(eiTextureTile) + tile->m_data_size);
	/* get pixel data pointer */
	slot_mem = (eiByte *)(tile + 1);

	/* load the texture tile from file, must lock the file system 
	   for synchronization. */
	pFileSystem = ei_db_file_system(db);

	ei_filesys_lock(pFileSystem);
	{
		ei_file_seek(pFile, tile->m_data_offset);
		ei_file_read(pFile, slot_mem, tile->m_data_size);
	}
	ei_filesys_unlock(pFileSystem);
}

void generate_texture_map(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls)
{
	eiTextureMap	*map;

	eiDBG_ASSERT(pData != NULL && pData->ptr != NULL);
	map = (eiTextureMap *)pData->ptr;

	/* if the texture map is non-local, and we are not the host 
	   that generated this data, we can receive it from the host 
	   on which it's generated. therefore, no file loading is 
	   needed. */
	if (!map->m_local && pData->host != ei_tls_get_host(pTls))
	{
		return;
	}

	/* no locking on file system is needed because the function 
	   will do the locking by itself. */
	ei_db_create_file_chunk_from_file(db, pData, map->m_name, EI_FILE_READ);
}

void byteswap_texture_tile(eiDatabase *db, void *data, const eiUint size)
{
	eiTextureTile *tile = (eiTextureTile *)data;

	ei_byteswap_int(&tile->m_map_tag);
	ei_byteswap_int(&tile->m_data_offset);
	ei_byteswap_int(&tile->m_data_size);
}

static void byteswap_texture_layer(eiTextureLayer *layer)
{
	ei_byteswap_int(&layer->m_tiles);
	ei_byteswap_int(&layer->m_data_offset);
	ei_byteswap_scalar(&layer->m_width);
	ei_byteswap_scalar(&layer->m_height);
	ei_byteswap_int(&layer->m_x_tiles);
	ei_byteswap_int(&layer->m_y_tiles);
	ei_byteswap_int(&layer->m_tiled_width);
	ei_byteswap_int(&layer->m_tiled_height);
}

void byteswap_texture_map(eiDatabase *db, void *data, const eiUint size)
{
	eiTextureMap	*map;
	eiTextureLayer	*layer;
	eiUint			i;

	map = (eiTextureMap *)data;

	/* byte-swap all texture layers */
	layer = (eiTextureLayer *)(map + 1);

	for (i = 0; i < map->m_num_layers; ++i)
	{
		byteswap_texture_layer(layer);
		++ layer;
	}

	/* must byte-swap these at the end because they 
	   will still be used in previous code. */
	ei_byteswap_int(&map->m_local);
	ei_byteswap_int(&map->m_tile_size);
	ei_byteswap_int(&map->m_num_channels);
	ei_byteswap_int(&map->m_channel_size);
	ei_byteswap_int(&map->m_swrap);
	ei_byteswap_int(&map->m_twrap);
	ei_byteswap_scalar(&map->m_width);
	ei_byteswap_scalar(&map->m_height);
	ei_byteswap_int(&map->m_num_layers);
}

void ei_texture_init(eiNodeSystem *nodesys, eiNode *node)
{
	eiTexture	*tex;

	tex = (eiTexture *)node;

	tex->tag = eiNULL_TAG;
}

void ei_texture_exit(eiNodeSystem *nodesys, eiNode *node)
{
	eiTexture	*tex;

	tex = (eiTexture *)node;

	if (tex->tag != eiNULL_TAG)
	{
		ei_delete_texture_map(nodesys->m_db, tex->tag);
		tex->tag = eiNULL_TAG;
	}
}

void ei_texture_create_from_file(
	eiTexture *tex, 
	eiDatabase *db, 
	const char *filename, 
	const eiBool local)
{
	if (tex->tag != eiNULL_TAG)
	{
		ei_delete_texture_map(db, tex->tag);
		tex->tag = eiNULL_TAG;
	}

	tex->tag = ei_create_texture_map_from_file(db, filename, local);
}

void ei_texture_deletethis(eiPluginObject *object)
{
	if (object == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_node_object_exit((eiNodeObject *)object);

	ei_free(object);
}

eiNodeObject *ei_create_texture_node_object(void *param)
{
	eiNodeObject	*object;

	object = (eiNodeObject *)ei_allocate(sizeof(eiNodeObject));

	ei_node_object_init(object);

	object->base.deletethis = ei_texture_deletethis;
	object->init_node = ei_texture_init;
	object->exit_node = ei_texture_exit;
	object->node_changed = NULL;

	return object;
}

void ei_install_texture_node(eiNodeSystem *nodesys)
{
	eiTag		desc_tag;
	eiNodeDesc	*desc;
	eiTag		default_tag;

	desc = ei_nodesys_node_desc(nodesys, &desc_tag, "texture");
	if (desc == NULL)
	{
		return;
	}

	default_tag = eiNULL_TAG;

	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"tag", 
		&default_tag);

	ei_nodesys_end_node_desc(nodesys, desc, desc_tag);
}
