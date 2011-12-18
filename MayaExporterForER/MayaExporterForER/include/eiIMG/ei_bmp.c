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

#include <eiIMG/ei_bmp.h>
#include <eiAPI/ei_image.h>
#include <eiCORE/ei_assert.h>

typedef struct eiBMPReader {
	eiImageReader		base;
	BITMAPFILEHEADER	hdr;
    BITMAPINFOHEADER	bmi;
	/* RGB color palette */
	RGBQUAD				*rgb;
	eiByte				*p;
	eiInt				wb;
	eiInt				data_offset;
} eiBMPReader;

static eiBool ei_bmp_read_header(eiBMPReader *reader)
{
	eiSizet		res;

	/* read file header */
	res = ei_image_reader_read_data(&reader->base, 
		&reader->hdr, sizeof(BITMAPFILEHEADER));

	if (res != sizeof(BITMAPFILEHEADER))
	{
		return eiFALSE;
	}

	/* read bitmap info header */
	res = ei_image_reader_read_data(&reader->base, 
		&reader->bmi, sizeof(BITMAPINFOHEADER));

	if (res != sizeof(BITMAPINFOHEADER))
	{
		return eiFALSE;
	}

	/* validate */
	if (reader->hdr.bfType != 0x4d42)
	{
		return eiFALSE;
	}

	/* done */
	return eiTRUE;
}

static void ei_bmp_reader_init(
	eiBMPReader *reader, 
	const char *filename)
{
	ei_image_reader_init(&reader->base, filename);

	if (!ei_bmp_read_header(reader))
	{
		/* error */
		return;
	}

	reader->base.m_width = reader->bmi.biWidth;
	reader->base.m_height = reader->bmi.biHeight;
	reader->rgb = NULL;
	reader->p = NULL;

	if ((reader->bmi.biBitCount != 32 && 
		reader->bmi.biBitCount != 24 && 
		reader->bmi.biBitCount != 8 && 
		reader->bmi.biBitCount != 4) || 
		reader->bmi.biCompression != BI_RGB)
	{
		/* error */
		return;
	}

	switch (reader->bmi.biBitCount)
	{
	case 4:
		/* read 4 bit palette */
		reader->base.m_num_channels = 3;
		reader->base.m_channel_size	= 1;

		if (!reader->bmi.biClrUsed)
		{
			reader->bmi.biClrUsed = 16;
		}

		reader->rgb = (RGBQUAD *)ei_allocate(reader->bmi.biClrUsed * sizeof(RGBQUAD));

		ei_image_reader_read_data(&reader->base, 
			reader->rgb, sizeof(RGBQUAD) * reader->bmi.biClrUsed);

		/* read image (4 bits) */
		reader->wb = (((reader->base.m_width + 1) / 2) + 3) & ~3; /* width must be multiple of 4 */

		reader->p = (eiByte *)ei_allocate(reader->wb);

		break;

	case 8:
		/* read 8 bit palette */
		reader->base.m_num_channels = 3;
		reader->base.m_channel_size = 1;

		if (!reader->bmi.biClrUsed)
		{
			reader->bmi.biClrUsed = 256;
		}

		reader->rgb = (RGBQUAD *)ei_allocate(reader->bmi.biClrUsed * sizeof(RGBQUAD));

		ei_image_reader_read_data(&reader->base, 
			reader->rgb, sizeof(RGBQUAD) * reader->bmi.biClrUsed);

		/* read image (8 bits) */
		reader->wb = (reader->base.m_width + 3) & ~3; /* width must be multiple of 4 */

		reader->p = (eiByte *)ei_allocate(reader->wb);

		break;

	case 24:
		/* read image 24 bits */
		reader->base.m_num_channels = 3;
		reader->base.m_channel_size = 1;

		reader->wb = (reader->base.m_width * 3 + 3) & ~3; /* width bytes must be multiple of 4 */

		reader->p = (eiByte *)ei_allocate(reader->wb);

		break;

	case 32:
		/* read image 32 bits */
		reader->base.m_num_channels	= 4;
		reader->base.m_channel_size = 1;

		reader->wb = reader->base.m_width * 4;

		reader->p = (eiByte *)ei_allocate(reader->wb);

		break;
	}

	reader->data_offset = reader->hdr.bfOffBits;
}

static void ei_bmp_reader_exit(eiBMPReader *reader)
{
	eiCHECK_FREE(reader->p);
	eiCHECK_FREE(reader->rgb);

	ei_image_reader_exit(&reader->base);
}

static void ei_bmp_reader_deletethis(eiPluginObject *object)
{
	if (object == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_bmp_reader_exit((eiBMPReader *)object);

	ei_free(object);
}

static void ei_bmp_reader_read_scanline(
	eiImageReader *reader, 
	eiScalar *buffer, 
	eiInt y, 
	eiInt x1, 
	eiInt x2, 
	const eiInt swrap, 
	const eiInt twrap)
{
	eiBMPReader		*child_reader;
	eiInt			i;

	child_reader = (eiBMPReader *)reader;

	ei_wrap_texcoord(&y, twrap, reader->m_height);

	ei_image_reader_seek_data(reader, 
		child_reader->data_offset + (reader->m_height - 1 - y) * child_reader->wb);

	switch (child_reader->bmi.biBitCount)
	{
	case 4:
		/* read 4 bit palette */
		ei_image_reader_read_data(reader, child_reader->p, child_reader->wb);

		/* the 4 bit buffer has two pixels per byte, 
		   convert it to 8 bit buffer that has one pixel per byte */
		for (i = x1; i < x2; ++i)
		{
			eiInt	x;
			eiByte	index;

			x = i;
			ei_wrap_texcoord(&x, swrap, reader->m_width);

			index = (x%2) ? (child_reader->p[x/2] & 0x0f) : (child_reader->p[x/2] >> 4);

			buffer[(i - x1) * 3 + 0] = (eiScalar)child_reader->rgb[index].rgbRed * (1.0f / 255.0f);
			buffer[(i - x1) * 3 + 1] = (eiScalar)child_reader->rgb[index].rgbGreen * (1.0f / 255.0f);
			buffer[(i - x1) * 3 + 2] = (eiScalar)child_reader->rgb[index].rgbBlue * (1.0f / 255.0f);
		}

		break;

	case 8:
		/* read 8 bit palette */
		ei_image_reader_read_data(reader, child_reader->p, child_reader->wb);

		/* the 8 bit buffer has one pixel per byte */
		for (i = x1; i < x2; ++i)
		{
			eiInt	x;
			eiByte	index;
			
			x = i;
			ei_wrap_texcoord(&x, swrap, reader->m_width);

			index = child_reader->p[x];

			buffer[(i - x1) * 3 + 0] = (eiScalar)child_reader->rgb[index].rgbRed * (1.0f / 255.0f);
			buffer[(i - x1) * 3 + 1] = (eiScalar)child_reader->rgb[index].rgbGreen * (1.0f / 255.0f);
			buffer[(i - x1) * 3 + 2] = (eiScalar)child_reader->rgb[index].rgbBlue * (1.0f / 255.0f);
		}

		break;

	case 24:
		/* read image 24 bits */
		ei_image_reader_read_data(reader, child_reader->p, child_reader->wb);

		for (i = x1; i < x2; ++i)
		{
			eiInt	x;
			
			x = i;
			ei_wrap_texcoord(&x, swrap, reader->m_width);

			buffer[(i - x1 ) * 3 + 0] = (eiScalar)child_reader->p[x * 3 + 2] * (1.0f / 255.0f);
			buffer[(i - x1 ) * 3 + 1] = (eiScalar)child_reader->p[x * 3 + 1] * (1.0f / 255.0f);
			buffer[(i - x1 ) * 3 + 2] = (eiScalar)child_reader->p[x * 3 + 0] * (1.0f / 255.0f);
		}

		break;

	case 32:
		/* read image 32 bits */
		ei_image_reader_read_data(reader, child_reader->p, child_reader->wb);

		for (i = x1; i < x2; ++i)
		{
			eiInt	x;
			
			x = i;
			ei_wrap_texcoord(&x, swrap, reader->m_width);

			buffer[(i - x1 ) * 4 + 0] = (eiScalar)child_reader->p[x * 4 + 2] * (1.0f / 255.0f);
			buffer[(i - x1 ) * 4 + 1] = (eiScalar)child_reader->p[x * 4 + 1] * (1.0f / 255.0f);
			buffer[(i - x1 ) * 4 + 2] = (eiScalar)child_reader->p[x * 4 + 0] * (1.0f / 255.0f);
			buffer[(i - x1 ) * 4 + 3] = (eiScalar)child_reader->p[x * 4 + 3] * (1.0f / 255.0f);
		}

		break;
	}
}

eiPluginObject *create_bmp_reader(void *param)
{
	eiBMPReader		*reader;

	reader = (eiBMPReader *)ei_allocate(sizeof(eiBMPReader));

	ei_bmp_reader_init(reader, (char *)param);

	reader->base.base.deletethis = ei_bmp_reader_deletethis;
	reader->base.read_scanline = ei_bmp_reader_read_scanline;

	return ((eiPluginObject *)reader);
}

typedef struct eiBMPWriter {
	eiImageWriter		base;
} eiBMPWriter;

static void ei_bmp_writer_init(
	eiBMPWriter *writer, 
	const char *filename)
{
	ei_image_writer_init(&writer->base, filename);
}

static void ei_bmp_writer_exit(eiBMPWriter *writer)
{
	ei_image_writer_exit(&writer->base);
}

static void ei_bmp_writer_deletethis(eiPluginObject *object)
{
	if (object == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_bmp_writer_exit((eiBMPWriter *)object);

	ei_free(object);
}

static void ei_bmp_writer_output_rgb(
	eiImageWriter *writer, 
	eiDatabase *db, 
	eiFrameBuffer *rgb, 
	eiOptions *opt, 
	eiCamera *cam)
{
	BITMAPFILEHEADER	bfh;
	BITMAPINFOHEADER	bih;
	/* .bmp requests the width of scanlines must be multiply of 4 */
	eiInt				blanks;
	eiByte				chan;
	eiInt				x, y;
	
	blanks = ((rgb->m_width * 3 + 3) & ~3) - rgb->m_width * 3;

	/* prepare bitmap file header */
	memset(&bfh, 0, sizeof(bfh));
	bfh.bfType = 0x4d42; /* "BM" */
	bfh.bfSize = sizeof(bfh) + sizeof(bih) 
		+ ((rgb->m_width * 3 + 3) & ~3) * rgb->m_height;
	bfh.bfOffBits = sizeof(bfh) + sizeof(bih);
	
	/* prepare bitmap info header */
	memset(&bih, 0, sizeof(bih));
	bih.biSize = sizeof(bih);
	bih.biWidth = rgb->m_width;
	bih.biHeight = rgb->m_height;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;

	ei_image_writer_write_data(writer, &bfh, sizeof(bfh));
	ei_image_writer_write_data(writer, &bih, sizeof(bih));

	chan = 0;

	for (y = (rgb->m_height - 1); y >= 0; --y)
	{
		for (x = 0; x < rgb->m_width; ++x)
		{
			switch (rgb->m_type)
			{
			case EI_DATA_TYPE_INT:
				{
					eiInt	ival;
					
					ival = 0;

					ei_framebuffer_get(db, rgb, x, y, &ival);
					ei_image_writer_quantize(writer, &chan, ((eiScalar)ival) * (1.0f / 255.0f), opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
				}
				break;

			case EI_DATA_TYPE_BOOL:
				{
					eiBool	bval;
					
					bval = eiFALSE;

					ei_framebuffer_get(db, rgb, x, y, &bval);
					ei_image_writer_quantize(writer, &chan, (eiScalar)bval, opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
				}
				break;

			case EI_DATA_TYPE_SCALAR:
				{
					eiScalar	fval;
					
					fval = 0.0f;

					ei_framebuffer_get(db, rgb, x, y, &fval);
					ei_image_writer_quantize(writer, &chan, fval, opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
				}
				break;

			case EI_DATA_TYPE_VECTOR:
				{
					eiVector	cval;

					initv(&cval);

					ei_framebuffer_get(db, rgb, x, y, &cval);
					/* .bmp writes in BGR order */
					ei_image_writer_quantize(writer, &chan, cval.z, opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_quantize(writer, &chan, cval.y, opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_quantize(writer, &chan, cval.x, opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
				}
				break;

			default:
				/* error */
				break;
			}
		}

		ei_image_writer_fill_data(writer, blanks);
	}
}

static void ei_bmp_writer_output_rgba(
	eiImageWriter *writer, 
	eiDatabase *db, 
	eiFrameBuffer *rgb, 
	eiFrameBuffer *a, 
	eiOptions *opt, 
	eiCamera *cam)
{
	BITMAPFILEHEADER	bfh;
	BITMAPINFOHEADER	bih;
	eiByte				chan;
	eiInt				x, y;

	if (rgb->m_width != a->m_width || 
		rgb->m_height != a->m_height)
	{
		/* error */
		return;
	}

	/* prepare bitmap file header */
	memset(&bfh, 0, sizeof(bfh));
	bfh.bfType = 0x4d42; /* "BM" */
	bfh.bfSize = sizeof(bfh) + sizeof(bih) 
		+ (rgb->m_width * 4) * rgb->m_height;
	bfh.bfOffBits = sizeof(bfh) + sizeof(bih);
	
	/* prepare bitmap info header */
	memset(&bih, 0, sizeof(bih));
	bih.biSize = sizeof(bih);
	bih.biWidth = rgb->m_width;
	bih.biHeight = rgb->m_height;
	bih.biPlanes = 1;
	bih.biBitCount = 32;
	bih.biCompression = BI_RGB;

	ei_image_writer_write_data(writer, &bfh, sizeof(bfh));
	ei_image_writer_write_data(writer, &bih, sizeof(bih));

	chan = 0;

	for (y = (rgb->m_height - 1); y >= 0; --y)
	{
		for (x = 0; x < rgb->m_width; ++x)
		{
			switch (rgb->m_type)
			{
			case EI_DATA_TYPE_INT:
				{
					eiInt	ival;
					
					ival = 0;

					ei_framebuffer_get(db, rgb, x, y, &ival);
					ei_image_writer_quantize(writer, &chan, ((eiScalar)ival) * (1.0f / 255.0f), opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
				}
				break;

			case EI_DATA_TYPE_BOOL:
				{
					eiBool	bval;
					
					bval = eiFALSE;

					ei_framebuffer_get(db, rgb, x, y, &bval);
					ei_image_writer_quantize(writer, &chan, (eiScalar)bval, opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
				}
				break;

			case EI_DATA_TYPE_SCALAR:
				{
					eiScalar	fval;
					
					fval = 0.0f;

					ei_framebuffer_get(db, rgb, x, y, &fval);
					ei_image_writer_quantize(writer, &chan, fval, opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
				}
				break;

			case EI_DATA_TYPE_VECTOR:
				{
					eiVector	cval;

					initv(&cval);

					ei_framebuffer_get(db, rgb, x, y, &cval);
					/* .bmp writes in BGR order */
					ei_image_writer_quantize(writer, &chan, cval.z, opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_quantize(writer, &chan, cval.y, opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
					ei_image_writer_quantize(writer, &chan, cval.x, opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
				}
				break;

			default:
				/* error */
				break;
			}

			switch (a->m_type)
			{
			case EI_DATA_TYPE_INT:
				{
					eiInt	ival;
					
					ival = 0;

					ei_framebuffer_get(db, a, x, y, &ival);
					ei_image_writer_quantize(writer, &chan, ((eiScalar)ival) * (1.0f / 255.0f), opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
				}
				break;

			case EI_DATA_TYPE_BOOL:
				{
					eiBool	bval;
					
					bval = eiFALSE;

					ei_framebuffer_get(db, a, x, y, &bval);
					ei_image_writer_quantize(writer, &chan, (eiScalar)bval, opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
				}
				break;

			case EI_DATA_TYPE_SCALAR:
				{
					eiScalar	fval;
					
					fval = 0.0f;

					ei_framebuffer_get(db, a, x, y, &fval);
					ei_image_writer_quantize(writer, &chan, fval, opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
				}
				break;

			case EI_DATA_TYPE_VECTOR:
				{
					eiVector	cval;

					initv(&cval);

					ei_framebuffer_get(db, a, x, y, &cval);
					ei_image_writer_quantize(writer, &chan, average(&cval), opt);
					ei_image_writer_write_data(writer, &chan, sizeof(eiByte));
				}
				break;

			default:
				/* error */
				break;
			}
		}
	}
}

static void ei_bmp_writer_output_rgbaz(
	eiImageWriter *writer, 
	eiDatabase *db, 
	eiFrameBuffer *rgb, 
	eiFrameBuffer *a, 
	eiFrameBuffer *z, 
	eiOptions *opt, 
	eiCamera *cam)
{
	/* .bmp doesn't support RGBA-Z, just output the RGBA components */
	ei_bmp_writer_output_rgba(writer, db, rgb, a, opt, cam);
}

eiPluginObject *create_bmp_writer(void *param)
{
	eiBMPWriter		*writer;

	writer = (eiBMPWriter *)ei_allocate(sizeof(eiBMPWriter));

	ei_bmp_writer_init(writer, (char *)param);

	writer->base.base.deletethis = ei_bmp_writer_deletethis;
	writer->base.output_rgb = ei_bmp_writer_output_rgb;
	writer->base.output_rgba = ei_bmp_writer_output_rgba;
	writer->base.output_rgbaz = ei_bmp_writer_output_rgbaz;

	return ((eiPluginObject *)writer);
}
