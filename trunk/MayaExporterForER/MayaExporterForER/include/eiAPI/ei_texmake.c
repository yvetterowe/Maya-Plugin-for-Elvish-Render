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

#include <eiAPI/ei_texmake.h>
#include <eiAPI/ei_texture.h>
#include <eiAPI/ei_image.h>
#include <eiCORE/ei_platform.h>
#include <eiCORE/ei_assert.h>

/* write into a stream with n bytes and increment the pointer with n bytes */
#define WRITE_STREAM(dest, src, n) { memcpy((dest), (src), (n)); (dest) += (n); }

/** \brief This class represents a texture buffer stored in the disk, 
 * provides the functionality for manipulating a single channel. */
typedef struct eiTextureBuffer {
	char			m_filename[ EI_MAX_FILE_NAME_LEN ];
	eiInt			m_width;
	eiInt			m_height;
	eiInt			m_num_channels;
	eiInt			m_swrap;
	eiInt			m_twrap;
	eiFileHandle	m_file;
	eiFileMap		m_file_map;
	eiByte			*m_data;
} eiTextureBuffer;

typedef float	eiTextureBufferDataType;

static void ei_texture_buffer_data_interp(
	eiTextureBufferDataType *r, 
	const eiTextureBufferDataType *a, 
	const eiTextureBufferDataType *b, 
	const eiScalar t)
{
	lerp(r, *a, *b, t);
}

static void ei_texture_buffer_init(
	eiTextureBuffer *buf, 
	const eiInt w, 
	const eiInt h, 
	const eiInt num_channels, 
	const eiInt swrap, 
	const eiInt twrap)
{
	char	cur_dir[ EI_MAX_FILE_NAME_LEN ];

	ei_get_current_directory(cur_dir);
	ei_get_temp_filename(buf->m_filename, cur_dir, "eiTB");

	buf->m_width = w;
	buf->m_height = h;
	buf->m_num_channels = num_channels;
	buf->m_swrap = swrap;
	buf->m_twrap = twrap;

	buf->m_file = ei_open_file(buf->m_filename, EI_FILE_WRITE_UPDATE);

	/* TODO: we map the whole file here, maybe it's required to map 
	   just one segment each time for large files. */
	ei_map_file(&buf->m_file_map, buf->m_file, EI_FILE_WRITE_UPDATE, 
		0, buf->m_width * buf->m_height * sizeof(eiTextureBufferDataType) * buf->m_num_channels);

	buf->m_data = (eiByte *)buf->m_file_map.data;

	memset(buf->m_data, 0, buf->m_width * buf->m_height * sizeof(eiTextureBufferDataType) * buf->m_num_channels);
}

static void ei_texture_buffer_exit(eiTextureBuffer *buf)
{
	ei_unmap_file(&buf->m_file_map);
	ei_close_file(buf->m_file);

	if (ei_file_exists(buf->m_filename))
	{
		ei_delete_file(buf->m_filename);
	}
}

static eiTextureBuffer *ei_create_texture_buffer(
	const eiInt w, 
	const eiInt h, 
	const eiInt num_channels, 
	const eiInt swrap, 
	const eiInt twrap)
{
	eiTextureBuffer	*buf;

	buf = ei_allocate(sizeof(eiTextureBuffer));

	ei_texture_buffer_init(buf, w, h, num_channels, swrap, twrap);

	return buf;
}

static void ei_delete_texture_buffer(eiTextureBuffer *buf)
{
	ei_texture_buffer_exit(buf);

	eiCHECK_FREE(buf);
}

static void ei_texture_buffer_get(
	eiTextureBuffer *buf, eiInt x, eiInt y, eiTextureBufferDataType *val, const eiInt channel)
{
	ei_wrap_texcoord(&x, buf->m_swrap, buf->m_width);
	ei_wrap_texcoord(&y, buf->m_twrap, buf->m_height);

	memcpy(val, 
		buf->m_data + ((x + y * buf->m_width) * buf->m_num_channels + channel) * sizeof(eiTextureBufferDataType), 
		sizeof(eiTextureBufferDataType));
}

static void ei_texture_buffer_get_bilinear(
	eiTextureBuffer *buf, eiScalar x, eiScalar y, eiTextureBufferDataType *val, const eiInt channel)
{
	eiInt						u, v;
	eiScalar					du, dv;
	eiTextureBufferDataType		f1, f2, f3, f4, r1, r2;

	u = truncf(x);
	v = truncf(y);
	du = curve(x - u);
	dv = curve(y - v);

	ei_texture_buffer_get(buf, u + 0, v + 0, &f1, channel);
	ei_texture_buffer_get(buf, u + 1, v + 0, &f2, channel);
	ei_texture_buffer_get(buf, u + 0, v + 1, &f3, channel);
	ei_texture_buffer_get(buf, u + 1, v + 1, &f4, channel);

	ei_texture_buffer_data_interp(&r1, &f1, &f2, du);
	ei_texture_buffer_data_interp(&r2, &f3, &f4, du);
	ei_texture_buffer_data_interp(val, &r1, &r2, dv);
}

static void ei_texture_buffer_set(
	eiTextureBuffer *buf, eiInt x, eiInt y, const eiTextureBufferDataType *val, const eiInt channel)
{
	memcpy(buf->m_data + ((x + y * buf->m_width) * buf->m_num_channels + channel) * sizeof(eiTextureBufferDataType), 
		val, 
		sizeof(eiTextureBufferDataType));
}

static void make_topmost_texture_layer(
	eiTextureHeader *header, eiImageReader *reader, 
	eiByte **pData, eiTextureBuffer *texbuf, 
	const eiInt x_tiles, const eiInt y_tiles, 
	const eiScalar width, const eiScalar height, 
	const eiInt swrap, const eiInt twrap, 
	const eiInt filter, const eiScalar swidth, const eiScalar twidth)
{
	eiScalar	*buffer;
	eiInt		x, y, i, j, k;
	
	buffer = ei_allocate(sizeof(eiScalar) * header->tile_size * header->num_channels);

	for (y = 0; y < y_tiles; ++y)
	{
		for (x = 0; x < x_tiles; ++x)
		{
			for (j = 0; j < header->tile_size; ++j)
			{
				memset(buffer, 0, sizeof(eiScalar) * header->tile_size * header->num_channels);

				reader->read_scanline(
					reader, 
					buffer, 
					y * header->tile_size + j, 
					x * header->tile_size, 
					(x + 1) * header->tile_size, 
					swrap, 
					twrap);

				switch (header->channel_size)
				{
				case 1:
					for (i = 0; i < header->tile_size; ++i)
					{
						for (k = 0; k < header->num_channels; ++k)
						{
							eiScalar	fcolor = buffer[i * header->num_channels + k];
							eiByte		color = (eiByte)(fcolor * 255.0f);

							WRITE_STREAM(*pData, &color, 1);
							ei_texture_buffer_set(texbuf, x * header->tile_size + i, y * header->tile_size + j, &fcolor, k);
						}
					}
					break;

				case 2:
					for (i = 0; i < header->tile_size; ++i)
					{
						for (k = 0; k < header->num_channels; ++k)
						{
							eiScalar	fcolor = buffer[i * header->num_channels + k];
							eiUshort	color = (eiUshort)(fcolor * 65535.0f);

							WRITE_STREAM(*pData, &color, 2);
							ei_texture_buffer_set(texbuf, x * header->tile_size + i, y * header->tile_size + j, &fcolor, k);
						}
					}
					break;

				case 4:
					for (i = 0; i < header->tile_size; ++i)
					{
						for (k = 0; k < header->num_channels; ++k)
						{
							eiScalar	fcolor = buffer[i * header->num_channels + k];

							WRITE_STREAM(*pData, &fcolor, 4);
							ei_texture_buffer_set(texbuf, x * header->tile_size + i, y * header->tile_size + j, &fcolor, k);
						}
					}
					break;

				default:
					/* error */
					break;
				}
			}
		}
	}

	eiCHECK_FREE(buffer);
}

static void make_texture_layer(
	eiTextureHeader *header, 
	eiTextureBuffer *reader, 
	eiByte **pData, eiTextureBuffer *texbuf, 
	const eiInt layer, 
	const eiInt x_tiles, const eiInt y_tiles, 
	const eiScalar width, const eiScalar height, 
	const eiInt swrap, const eiInt twrap)
{
	eiScalar	fc, fc1, fc2, fc3, fc4;
	eiScalar	x_scale, y_scale;
	eiInt		x, y, i, j, k;

	x_scale = width / (eiScalar)lceilf(width);
	y_scale = height / (eiScalar)lceilf(height);

	for (y = 0; y < y_tiles; ++y)
	{
		for (x = 0; x < x_tiles; ++x)
		{
			for (j = 0; j < header->tile_size; ++j)
			{
				for (i = 0; i < header->tile_size; ++i)
				{
					eiInt		pos_x = x * header->tile_size + i;
					eiInt		pos_y = y * header->tile_size + j;
					eiScalar	fpos_x = (eiScalar)pos_x * 2.0f;
					eiScalar	fpos_y = (eiScalar)pos_y * 2.0f;

					switch (header->channel_size)
					{
					case 1:
						for (k = 0; k < header->num_channels; ++k)
						{
							eiByte		bc;

							ei_texture_buffer_get_bilinear(reader, (fpos_x + 0.0f) * x_scale, (fpos_y + 0.0f) * y_scale, &fc1, k);
							ei_texture_buffer_get_bilinear(reader, (fpos_x + 1.0f) * x_scale, (fpos_y + 0.0f) * y_scale, &fc2, k);
							ei_texture_buffer_get_bilinear(reader, (fpos_x + 0.0f) * x_scale, (fpos_y + 1.0f) * y_scale, &fc3, k);
							ei_texture_buffer_get_bilinear(reader, (fpos_x + 1.0f) * x_scale, (fpos_y + 1.0f) * y_scale, &fc4, k);
							fc = (fc1 + fc2 + fc3 + fc4) * 0.25f;
							bc = (eiByte)(fc * 255.0f);
							WRITE_STREAM(*pData, &bc, 1);
							ei_texture_buffer_set(texbuf, pos_x, pos_y, &fc, k);
						}
						break;

					case 2:
						for (k = 0; k < header->num_channels; ++k)
						{
							eiUshort	sc;

							ei_texture_buffer_get_bilinear(reader, (fpos_x + 0.0f) * x_scale, (fpos_y + 0.0f) * y_scale, &fc1, k);
							ei_texture_buffer_get_bilinear(reader, (fpos_x + 1.0f) * x_scale, (fpos_y + 0.0f) * y_scale, &fc2, k);
							ei_texture_buffer_get_bilinear(reader, (fpos_x + 0.0f) * x_scale, (fpos_y + 1.0f) * y_scale, &fc3, k);
							ei_texture_buffer_get_bilinear(reader, (fpos_x + 1.0f) * x_scale, (fpos_y + 1.0f) * y_scale, &fc4, k);
							fc = (fc1 + fc2 + fc3 + fc4) * 0.25f;
							sc = (eiUshort)(fc * 65535.0f);
							WRITE_STREAM(*pData, &sc, 2);
							ei_texture_buffer_set(texbuf, pos_x, pos_y, &fc, k);
						}
						break;

					case 4:
						for (k = 0; k < header->num_channels; ++k)
						{
							ei_texture_buffer_get_bilinear(reader, (fpos_x + 0.0f) * x_scale, (fpos_y + 0.0f) * y_scale, &fc1, k);
							ei_texture_buffer_get_bilinear(reader, (fpos_x + 1.0f) * x_scale, (fpos_y + 0.0f) * y_scale, &fc2, k);
							ei_texture_buffer_get_bilinear(reader, (fpos_x + 0.0f) * x_scale, (fpos_y + 1.0f) * y_scale, &fc3, k);
							ei_texture_buffer_get_bilinear(reader, (fpos_x + 1.0f) * x_scale, (fpos_y + 1.0f) * y_scale, &fc4, k);
							fc = (fc1 + fc2 + fc3 + fc4) * 0.25f;
							WRITE_STREAM(*pData, &fc, 4);
							ei_texture_buffer_set(texbuf, pos_x, pos_y, &fc, k);
						}
						break;

					default:
						/* error */
						break;
					}
				}
			}
		}
	}
}

void ei_make_texture_imp(
	eiPluginSystem *plugsys, 
	const char *picturename, const char *texturename, 
	eiInt swrap, eiInt twrap, eiInt filter, eiScalar swidth, eiScalar twidth)
{
	char				ext[ EI_MAX_FILE_NAME_LEN ];
	eiImageReader		*reader;
	eiFileHandle		pFile;
	eiScalar			width, height;
	eiInt				channel_size;
	eiTextureHeader		header;
	eiInt				x_tiles, y_tiles;
	eiInt				num_layers;
	eiSizet				file_length;
	eiFileMap			file_map;
	eiByte				*pData;
	eiInt				data_offset;
	eiInt				layer;
	eiTextureBuffer		*texbuf, *texbuf1;

	ei_get_file_extension(ext, picturename);
	
	reader = ei_create_image_reader(plugsys, ext, picturename);
	
	if (reader == NULL)
	{
		ei_error("Unsupported image file format %s\n", picturename);
		return;
	}

	/* create the texture file for writing */
	pFile = ei_open_file(texturename, EI_FILE_WRITE_UPDATE);

	if (pFile == NULL)
	{
		ei_error("Cannot open the texture file for writing %s\n", texturename);
		return;
	}

	/* get basic information from source texture map file */
	width = (eiScalar)ei_image_reader_get_width(reader);
	height = (eiScalar)ei_image_reader_get_height(reader);
	channel_size = ei_image_reader_get_channel_size(reader);

	/* check channel size, can be 1, 2, 4 bytes */
	clampi(channel_size, 1, 4);
	if (channel_size == 3)
	{
		channel_size = 4;
	}

	/* build texture header */
	header.format_code = EI_TEXTURE_FILE_CODE;
	header.tile_size = EI_TEXTURE_TILE_SIZE;
	header.num_channels = ei_image_reader_get_num_channels(reader);
	header.channel_size = channel_size;
	header.swrap = swrap;
	header.twrap = twrap;

	/* compute the number of MIP-MAP levels and file length */
	x_tiles = 0;
	y_tiles = 0;
	num_layers = 0;
	file_length = sizeof(eiTextureHeader);

	while (width >= 1.0f && height >= 1.0f)
	{
		++ num_layers;

		/* accumulate file length for current level */
		ei_compute_num_tiles(&x_tiles, &y_tiles, width, height, header.tile_size);

		file_length += sizeof(eiTextureLayerInfo);
		file_length += (x_tiles * y_tiles 
			* header.tile_size * header.tile_size 
			* header.num_channels * header.channel_size);

		/* loop to next level */
		width *= 0.5f;
		height *= 0.5f;
	}

	/* we've got number of MIP-MAP levels and file length, write them */
	header.num_layers = num_layers;
	header.file_length = file_length;

	/* now we can do file mapping since we know the file length */
	ei_map_file(&file_map, pFile, EI_FILE_WRITE_UPDATE, 0, file_length);

	pData = (eiByte *)file_map.data;

	if (pData == NULL)
	{
		ei_error("Failed to access the texture file %s\n", texturename);
		return;
	}

	WRITE_STREAM(pData, &header, sizeof(eiTextureHeader));

	width = (eiScalar)ei_image_reader_get_width(reader);
	height = (eiScalar)ei_image_reader_get_height(reader);

	data_offset = (eiInt)(sizeof(eiTextureHeader) + sizeof(eiTextureLayerInfo) * num_layers);

	for (layer = 0; layer < num_layers; ++ layer)
	{
		eiTextureLayerInfo	layerInfo;

		layerInfo.width = width;
		layerInfo.height = height;
		layerInfo.data_offset = data_offset;

		WRITE_STREAM(pData, &layerInfo, sizeof(eiTextureLayerInfo));

		ei_compute_num_tiles(&x_tiles, &y_tiles, width, height, header.tile_size);
		data_offset += (x_tiles * y_tiles * header.tile_size * header.tile_size * header.num_channels * header.channel_size);

		width *= 0.5f;
		height *= 0.5f;
	}

	width = (eiScalar)ei_image_reader_get_width(reader);
	height = (eiScalar)ei_image_reader_get_height(reader);

	/* make the topmost layer */
	ei_compute_num_tiles(&x_tiles, &y_tiles, width, height, header.tile_size);

	texbuf = ei_create_texture_buffer(
		x_tiles * header.tile_size, 
		y_tiles * header.tile_size, 
		header.num_channels, 
		header.swrap, 
		header.twrap);

	/* output topmost layer both in pData and texbuf */
	make_topmost_texture_layer(
		&header, reader, &pData, texbuf, 
		x_tiles, y_tiles, width, height, 
		header.swrap, header.twrap, filter, swidth, twidth);

	/* reader is useless now, delete it at once */
	ei_delete_image_reader(plugsys, reader);

	width *= 0.5f;
	height *= 0.5f;

	/* make sub-layers */
	for (layer = 1; layer < num_layers; ++ layer)
	{
		ei_compute_num_tiles(&x_tiles, &y_tiles, width, height, header.tile_size);

		texbuf1 = ei_create_texture_buffer(
			x_tiles * header.tile_size, 
			y_tiles * header.tile_size, header.num_channels, 
			header.swrap, header.twrap);

		make_texture_layer(
			&header, texbuf, &pData, texbuf1, layer, 
			x_tiles, y_tiles, width, height, 
			header.swrap, header.twrap);

		ei_delete_texture_buffer(texbuf);

		texbuf = texbuf1;
		width *= 0.5f;
		height *= 0.5f;
	}

	ei_delete_texture_buffer(texbuf);

	/* unmap the texture file */
	ei_unmap_file(&file_map);

	/* close the texture file */
	ei_close_file(pFile);
}
