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

#include <eiAPI/ei_image.h>
#include <eiCORE/ei_assert.h>
#include <eiCORE/ei_util.h>

void ei_wrap_texcoord(eiInt *texcoord, const eiInt wrap_mode, const eiInt width)
{
	switch (wrap_mode)
	{
	case EI_TEX_WRAP_CLAMP:
		clampi(*texcoord, 0, width - 1);
		break;

	case EI_TEX_WRAP_PERIODIC:
		if (*texcoord < 0)
		{
			*texcoord = width - ((-*texcoord) % width) - 1;
		}
		else
		{
			*texcoord = (*texcoord % width);
		}
		break;

	default:
		/* error */
		break;
	}
}

eiImageReader *ei_create_image_reader(
	eiPluginSystem *plugin_system, 
	const char *image_type, 
	const char *filename)
{
	char	plugin_name[ EI_MAX_FILE_NAME_LEN ];

	sprintf(plugin_name, "%s_reader", image_type);

	return (eiImageReader *)ei_plugsys_create(plugin_system, plugin_name, (void *)filename);
}

void ei_delete_image_reader(
	eiPluginSystem *plugin_system, 
	eiImageReader *reader)
{
	ei_plugsys_delete(plugin_system, &reader->base);
}

void ei_image_reader_init(
	eiImageReader *reader, 
	const char *filename)
{
	ei_plugin_object_init(&reader->base);

	reader->read_scanline = NULL;

	reader->m_width = 0;
	reader->m_height = 0;
	reader->m_num_channels = 0;
	reader->m_channel_size = 0;
	reader->m_file = ei_open_file(filename, EI_FILE_READ);
}

void ei_image_reader_exit(eiImageReader *reader)
{
	if (reader->m_file != NULL)
	{
		ei_close_file(reader->m_file);
		reader->m_file = NULL;
	}

	ei_plugin_object_exit(&reader->base);
}

eiInt ei_image_reader_get_width(eiImageReader *reader)
{
	return reader->m_width;
}

eiInt ei_image_reader_get_height(eiImageReader *reader)
{
	return reader->m_height;
}

eiInt ei_image_reader_get_num_channels(eiImageReader *reader)
{
	return reader->m_num_channels;
}

eiInt ei_image_reader_get_channel_size(eiImageReader *reader)
{
	return reader->m_channel_size;
}

eiSizet ei_image_reader_read_data(
	eiImageReader *reader, 
	void *buf, 
	const eiSizet size)
{
	eiDBG_ASSERT(reader->m_file != NULL);

	return ei_read_file(reader->m_file, buf, size);
}

eiInt64 ei_image_reader_seek_data(
	eiImageReader *reader, 
	const eiInt64 offset)
{
	eiDBG_ASSERT(reader->m_file != NULL);

	return ei_seek_file(reader->m_file, offset);
}

eiImageWriter *ei_create_image_writer(
	eiPluginSystem *plugin_system, 
	const char *image_type, 
	const char *filename)
{
	char	plugin_name[ EI_MAX_FILE_NAME_LEN ];

	sprintf(plugin_name, "%s_writer", image_type);

	return (eiImageWriter *)ei_plugsys_create(plugin_system, plugin_name, (void *)filename);
}

void ei_delete_image_writer(
	eiPluginSystem *plugin_system, 
	eiImageWriter *writer)
{
	ei_plugsys_delete(plugin_system, &writer->base);
}

void ei_image_writer_init(
	eiImageWriter *writer, 
	const char *filename)
{
	ei_plugin_object_init(&writer->base);

	writer->output_rgb = NULL;
	writer->output_rgba = NULL;
	writer->output_rgbaz = NULL;

	writer->m_file = ei_open_file(filename, EI_FILE_WRITE);
	ei_random_reset(&writer->m_randGen, EI_DEFAULT_RANDOM_SEED);
}

void ei_image_writer_exit(eiImageWriter *writer)
{
	if (writer->m_file != NULL)
	{
		ei_close_file(writer->m_file);
		writer->m_file = NULL;
	}

	ei_plugin_object_exit(&writer->base);
}

void ei_image_writer_fill_data(
	eiImageWriter *writer, 
	const eiSizet size)
{
	eiInt		zero;
	eiSizet		i;

	if (size == 0)
	{
		return;
	}

	eiDBG_ASSERT(writer->m_file != NULL);

	zero = 0;

	for (i = 0; i < size; ++i)
	{
		ei_write_file(writer->m_file, (void *)&zero, sizeof(eiByte));
	}
}

eiSizet ei_image_writer_write_data(
	eiImageWriter *writer, 
	const void *buf, 
	const eiSizet size)
{
	eiDBG_ASSERT(writer->m_file != NULL);

	return ei_write_file(writer->m_file, buf, size);
}

void ei_image_writer_quantize(
	eiImageWriter *writer, 
	eiByte *dst, 
	const eiScalar src, 
	const eiOptions *opt)
{
	eiScalar color = powf(src * opt->exposure_gain, 1.0f / opt->exposure_gamma);
	/* random is between [-1, 1) */
	eiScalar random = (eiScalar)(ei_random(&writer->m_randGen) * 2.0 - 1.0);

	color = (eiScalar)(roundf(opt->quantize_one * color + opt->quantize_dither_amplitude * random));
	clampi(color, opt->quantize_min, opt->quantize_max);

	*dst = (eiByte)color;
}
