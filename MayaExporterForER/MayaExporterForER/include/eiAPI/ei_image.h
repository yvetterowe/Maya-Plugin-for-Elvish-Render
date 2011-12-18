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
 
#ifndef EI_IMAGE_H
#define EI_IMAGE_H

/** \brief The image I/O interfaces for generic loading and 
 * outputing of miscellaneous image formats.
 * \file ei_image.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_buffer.h>
#include <eiAPI/ei_options.h>
#include <eiAPI/ei_camera.h>
#include <eiCORE/ei_platform.h>
#include <eiCORE/ei_plugsys.h>
#include <eiCORE/ei_random.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Texture wrap types */
enum {
	EI_TEX_WRAP_NONE = 0, 
	EI_TEX_WRAP_CLAMP, 
	EI_TEX_WRAP_PERIODIC, 
	EI_TEX_WRAP_COUNT, 
};

/* forward declarations */
typedef struct eiImageReader	eiImageReader;
typedef struct eiImageWriter	eiImageWriter;

/** \brief Wrap the texture coordinates as specified by the wrapping mode. */
eiAPI void ei_wrap_texcoord(
	eiInt *texcoord, 
	const eiInt wrap_mode, 
	const eiInt width);

/** \brief Read a segment on a scanline. */
typedef void (*ei_image_reader_read_scanline)(
	eiImageReader *reader, 
	eiScalar *buffer, 
	eiInt y, 
	eiInt x1, 
	eiInt x2, 
	const eiInt swrap, 
	const eiInt twrap);

/** \brief The class for reading image files from the disk. 
 * this provides an uniform interface so that you can derive 
 * your own classes to support more image file formats 
 * without changing the rendering core. */
struct eiImageReader {
	eiPluginObject		base;
	ei_image_reader_read_scanline	read_scanline;

	eiFileHandle		m_file;
	eiInt				m_width;
	eiInt				m_height;
	eiInt				m_num_channels;
	eiInt				m_channel_size;
};

/** \brief Create image reader plugin object by name. */
eiAPI eiImageReader *ei_create_image_reader(
	eiPluginSystem *plugin_system, 
	const char *image_type, 
	const char *filename);
/** \brief Delete image reader plugin object. */
eiAPI void ei_delete_image_reader(
	eiPluginSystem *plugin_system, 
	eiImageReader *reader);

/** \brief Initialize the base image reader. open 
 * the image file for reading. */
eiAPI void ei_image_reader_init(
	eiImageReader *reader, 
	const char *filename);
/** \brief Cleanup the base image reader. close the 
 * image file for reading. */
eiAPI void ei_image_reader_exit(eiImageReader *reader);

/** \brief Get image width. */
eiAPI eiInt ei_image_reader_get_width(eiImageReader *reader);
/** \brief Get image height. */
eiAPI eiInt ei_image_reader_get_height(eiImageReader *reader);
/** \brief Get the number of color channels. */
eiAPI eiInt ei_image_reader_get_num_channels(eiImageReader *reader);
/** \brief Get the size per color channel in bytes. */
eiAPI eiInt ei_image_reader_get_channel_size(eiImageReader *reader);
/** \brief Read data from the image file. */
eiAPI eiSizet ei_image_reader_read_data(
	eiImageReader *reader, 
	void *buf, 
	const eiSizet size);
/** \brief Seek in the image file. */
eiAPI eiInt64 ei_image_reader_seek_data(
	eiImageReader *reader, 
	const eiInt64 offset);

/** \brief The callback for outputing RGB color buffer. */
typedef void (*ei_image_writer_output_rgb)(
	eiImageWriter *writer, 
	eiDatabase *db, 
	eiFrameBuffer *rgb, 
	eiOptions *opt, 
	eiCamera *cam);
/** \brief The callback for outputing RGB color buffer, 
 * alpha buffer. */
typedef void (*ei_image_writer_output_rgba)(
	eiImageWriter *writer, 
	eiDatabase *db, 
	eiFrameBuffer *rgb, 
	eiFrameBuffer *a, 
	eiOptions *opt, 
	eiCamera *cam);
/** \brief The callback for outputing RGB color buffer, 
 * alpha buffer, Z depth buffer. */
typedef void (*ei_image_writer_output_rgbaz)(
	eiImageWriter *writer, 
	eiDatabase *db, 
	eiFrameBuffer *rgb, 
	eiFrameBuffer *a, 
	eiFrameBuffer *z, 
	eiOptions *opt, 
	eiCamera *cam);

/** \brief The image writer for writing images to files, and 
 * supporting arbitrary output variables, by allowing writing 
 * some basic types of shader parameters. different image 
 * formats derived from this class can support writing 
 * full-precision data or quantized data with precision loss, 
 * depends on their capabilities and implementations. */
struct eiImageWriter {
	eiPluginObject		base;
	ei_image_writer_output_rgb		output_rgb;
	ei_image_writer_output_rgba		output_rgba;
	ei_image_writer_output_rgbaz	output_rgbaz;

	eiFileHandle		m_file;
	eiRandomGen			m_randGen;
};

/** \brief Create image writer plugin object by name. */
eiAPI eiImageWriter *ei_create_image_writer(
	eiPluginSystem *plugin_system, 
	const char *image_type, 
	const char *filename);
/** \brief Delete image writer plugin object. */
eiAPI void ei_delete_image_writer(
	eiPluginSystem *plugin_system, 
	eiImageWriter *writer);

/** \brief Initialize the base image writer. open 
 * the image file for writing. */
eiAPI void ei_image_writer_init(
	eiImageWriter *writer, 
	const char *filename);
/** \brief Cleanup the base image writer. close the 
 * image file for writing. */
eiAPI void ei_image_writer_exit(eiImageWriter *writer);

/** \brief Fill the image file with specified number 
 * of zero bytes. */
eiAPI void ei_image_writer_fill_data(
	eiImageWriter *writer, 
	const eiSizet size);
/** \brief Write data into the image file. */
eiAPI eiSizet ei_image_writer_write_data(
	eiImageWriter *writer, 
	const void *buf, 
	const eiSizet size);
/** \brief Perform gamma correction and dithering. */
eiAPI void ei_image_writer_quantize(
	eiImageWriter *writer, 
	eiByte *dst, 
	const eiScalar src, 
	const eiOptions *opt);

#ifdef __cplusplus
}
#endif

#endif
