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

#include <eiIMG/ei_hdr.h>
#include <eiAPI/ei_image.h>

#include <eiIMG/rgbe.h>

eiPluginObject *create_hdr_reader(void *param)
{
	return NULL;
}

//

typedef struct eiHDRWriter {
	eiImageWriter		base;
} eiHDRWriter;

static void ei_hdr_writer_init(
	eiHDRWriter *writer, 
	const char *filename)
{
	ei_image_writer_init(&writer->base, filename);
}

static void ei_hdr_writer_exit(eiHDRWriter *writer)
{
	ei_image_writer_exit(&writer->base);
}

static void ei_hdr_writer_deletethis(eiPluginObject *object)
{
	if (object == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_hdr_writer_exit((eiHDRWriter *)object);

	ei_free(object);
}

static void ei_hdr_writer_output_rgb(
	eiImageWriter *writer, 
	eiDatabase *db, 
	eiFrameBuffer *rgb, 
	eiOptions *opt, 
	eiCamera *cam)
{
	eiInt x, y;
	eiInt e;
	eiScalar v;
	eiByte rgbe[4];

	fseek(writer->m_file, 0, SEEK_SET);
	RGBE_WriteHeader(writer->m_file, rgb->m_width, rgb->m_height, NULL);

	for (y = 0; y < rgb->m_height; ++ y)
	{
		for (x = 0; x < rgb->m_width; ++ x)
		{
			eiVector color;
			ei_framebuffer_get(db, rgb, x, y, &color);

			v = color.r;
			if (color.g > v)
			{
				v = color.g;
			}
			if (color.b > v)
			{
				v = color.b;
			}
			if (v < 1e-32)
			{
				rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
			}
			else
			{
				v = frexpf(v, &e) * 256.0f / v;
				rgbe[0] = (unsigned char) (color.r * v);
				rgbe[1] = (unsigned char) (color.g * v);
				rgbe[2] = (unsigned char) (color.b * v);
				rgbe[3] = (unsigned char) (e + 128);
			}

			fwrite(rgbe, 4, 1, writer->m_file);
		}
	}
}

static void ei_hdr_writer_output_rgba(
	eiImageWriter *writer, 
	eiDatabase *db, 
	eiFrameBuffer *rgb, 
	eiFrameBuffer *a, 
	eiOptions *opt, 
	eiCamera *cam)
{
	ei_hdr_writer_output_rgb(writer, db, rgb, opt, cam);
}

static void ei_hdr_writer_output_rgbaz(
	eiImageWriter *writer, 
	eiDatabase *db, 
	eiFrameBuffer *rgb, 
	eiFrameBuffer *a, 
	eiFrameBuffer *z, 
	eiOptions *opt, 
	eiCamera *cam)
{
	ei_hdr_writer_output_rgba(writer, db, rgb, a, opt, cam);
}

eiPluginObject *create_hdr_writer(void *param)
{
	eiHDRWriter		*writer;

	writer = (eiHDRWriter *)ei_allocate(sizeof(eiHDRWriter));
	ei_hdr_writer_init(writer, (char *)param);
	writer->base.base.deletethis = ei_hdr_writer_deletethis;
	writer->base.output_rgb = ei_hdr_writer_output_rgb;
	writer->base.output_rgba = ei_hdr_writer_output_rgba;
	writer->base.output_rgbaz = ei_hdr_writer_output_rgbaz;

	return ((eiPluginObject *)writer);
}
