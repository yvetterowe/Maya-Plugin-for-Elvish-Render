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
 
#ifndef EI_TEXTURE_H
#define EI_TEXTURE_H

/** \brief Texture mapping utilities.
 * \file ei_texture.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_nodesys.h>
#include <eiCORE/ei_vector.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EI_TEXTURE_TILE_SIZE		64
#define EI_TEXTURE_FILE_CODE		0xA7D4CA1

/* forward declarations */
typedef struct eiTLS		eiTLS;
typedef struct eiData		eiData;
typedef struct eiDatabase	eiDatabase;

/** \brief The layer information of internal texture format. */
typedef struct eiTextureLayerInfo {
	/* width in pixels */
	eiScalar	width;
	/* height in pixels */
	eiScalar	height;
	/* data offset of this layer from the file beginning */
	eiInt		data_offset;
} eiTextureLayerInfo;

/** \brief The header of internal texture format. */
typedef struct eiTextureHeader {
	/* code for verifying the format */
	eiInt		format_code;
	/* edge size of each tile in pixels */
	eiInt		tile_size;
	/* number of color channels */
	eiInt		num_channels;
	/* size of each color channel in bytes */
	eiInt		channel_size;
	/* type of wrapping */
	eiInt		swrap;
	eiInt		twrap;
	/* number of MIP-MAP layers */
	eiInt		num_layers;
	/* file length of this texture map in bytes */
	eiUint64	file_length;
} eiTextureHeader;

/* for internal use only */
eiAPI void ei_compute_num_tiles(
	eiInt *x_tiles, eiInt *y_tiles, 
	const eiScalar width, const eiScalar height, const eiInt tile_size);

/** \brief Lookup a scalar texture at one point. */
eiAPI void ei_lookup_scalar_texture(
	eiDatabase *db, 
	eiScalar *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s, const eiScalar t);
/** \brief Lookup a vector texture at one point. */
eiAPI void ei_lookup_vector_texture(
	eiDatabase *db, 
	eiVector *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s, const eiScalar t);
/** \brief Lookup a scalar texture by filtering over 
 * a quad. */
eiAPI void ei_lookup_scalar_texture_filtered(
	eiDatabase *db, 
	eiScalar *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s1, const eiScalar t1, 
	const eiScalar s2, const eiScalar t2, 
	const eiScalar s3, const eiScalar t3, 
	const eiScalar s4, const eiScalar t4);
/** \brief Lookup a vector texture by filtering over 
 * a quad. */
eiAPI void ei_lookup_vector_texture_filtered(
	eiDatabase *db, 
	eiVector *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s1, const eiScalar t1, 
	const eiScalar s2, const eiScalar t2, 
	const eiScalar s3, const eiScalar t3, 
	const eiScalar s4, const eiScalar t4);

/** \brief Generate texture tile by loading the 
 * tile from texture file. for internal use only. */
void generate_texture_tile(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls);

/** \brief Generate texture map by opening the 
 * texture file from local disk, or making the texture 
 * data to be received over network. for internal use only. */
void generate_texture_map(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls);

/* for internal use only */
void byteswap_texture_tile(eiDatabase *db, void *data, const eiUint size);
void byteswap_texture_map(eiDatabase *db, void *data, const eiUint size);

/** \brief The node for texture map. */
typedef struct eiTexture {
	/* the base node */
	eiNode			node;
	/* the tag of the texture map */
	eiTag			tag;
} eiTexture;

void ei_texture_init(eiNodeSystem *nodesys, eiNode *node);
void ei_texture_exit(eiNodeSystem *nodesys, eiNode *node);

/** \brief Create a texture from file. all textures being 
 * looked up later must have been created in scene 
 * pre-processing in single threaded mode.
 * @param filename The texture file name.
 * @param local If true, the texture will only be loaded 
 * from local host, otherwise, it will be received from 
 * remote host over network. */
void ei_texture_create_from_file(
	eiTexture *tex, 
	eiDatabase *db, 
	const char *filename, 
	const eiBool local);

eiNodeObject *ei_create_texture_node_object(void *param);
/** \brief Install the node into node system */
void ei_install_texture_node(eiNodeSystem *nodesys);

#ifdef __cplusplus
}
#endif

#endif
