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

#include <eiAPI/ei_dbg_canvas.h>
#include <eiCORE/ei_vector.h>
#include <eiCORE/ei_platform.h>
#include <eiCORE/ei_assert.h>

/** \brief The debug canvas class */
typedef struct eiDbgCanvas {
	eiInt		width;
	eiInt		height;
	eiVector	*data;
} eiDbgCanvas;

/* the global debug canvas object */
eiDbgCanvas g_DbgCanvas;

void ei_dbg_canvas_init()
{
	g_DbgCanvas.width = 0;
	g_DbgCanvas.height = 0;
	g_DbgCanvas.data = NULL;
}

void ei_dbg_canvas_resize(const eiInt width, const eiInt height)
{
	eiCHECK_FREE(g_DbgCanvas.data);

	g_DbgCanvas.width = width;
	g_DbgCanvas.height = height;
	g_DbgCanvas.data = (eiVector *)ei_allocate(sizeof(eiVector) * width * height);
	memset(g_DbgCanvas.data, 0, sizeof(eiVector) * width * height);
}

void ei_dbg_canvas_set_pixel(
	const eiScalar x, const eiScalar y, 
	const eiScalar r, const eiScalar g, const eiScalar b)
{
	eiInt		xi, yi;
	eiVector	*ptr;

	xi = (eiInt)x;
	yi = (eiInt)y;
	clampi(xi, 0, g_DbgCanvas.width - 1);
	clampi(yi, 0, g_DbgCanvas.height - 1);

	ptr = g_DbgCanvas.data + (yi * g_DbgCanvas.width + xi);

	ptr->x = r;
	ptr->y = g;
	ptr->z = b;
}

eiBool ei_dbg_canvas_output(const char *filename)
{
	FILE				*bmpFile;
	BITMAPFILEHEADER	bfh;
	BITMAPINFOHEADER	bih;
	eiInt				blanks;
	eiInt				i, j;

	if (filename == NULL)
	{
		return eiFALSE;
	}

	// prepare bitmap file header
	memset(&bfh, 0, sizeof(bfh));
	bfh.bfType = 0x4d42; // "BM"
	bfh.bfSize = sizeof(bfh) + sizeof(bih) + ((g_DbgCanvas.width * 3 + 3) & ~3) * g_DbgCanvas.height;
	bfh.bfOffBits = sizeof(bfh) + sizeof(bih);

	// prepare bitmap info header
	memset(&bih, 0, sizeof(bih));
	bih.biSize = sizeof(bih);
	bih.biWidth = g_DbgCanvas.width;
	bih.biHeight = g_DbgCanvas.height;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;

	// the .bmp requests the width of scanlines must be multiply of 4
	blanks = ((g_DbgCanvas.width * 3 + 3) & ~3) - g_DbgCanvas.width * 3;

	// create bitmap file
	bmpFile = fopen(filename, "wb");
	if (bmpFile == NULL)
	{
		return eiFALSE;
	}
	fwrite(&bfh, sizeof(bfh), 1, bmpFile);
	fwrite(&bih, sizeof(bih), 1, bmpFile);
	for (j = (g_DbgCanvas.height - 1); j >= 0; --j)
	{
		for (i = 0; i < g_DbgCanvas.width; ++i)
		{
			eiVector *ptr;
			eiByte bCol[3];

			ptr = g_DbgCanvas.data + (j * g_DbgCanvas.width + i);
			bCol[0] = (eiByte)(ptr->x * 255.0f);
			bCol[1] = (eiByte)(ptr->y * 255.0f);
			bCol[2] = (eiByte)(ptr->z * 255.0f);
			fwrite(bCol + 2, 1, 1, bmpFile);
			fwrite(bCol + 1, 1, 1, bmpFile);
			fwrite(bCol + 0, 1, 1, bmpFile);
		}
		if (blanks > 0)
		{
			fwrite(g_DbgCanvas.data, blanks, 1, bmpFile);
		}
	}
	fclose(bmpFile);

	return eiTRUE;
}

void ei_dbg_canvas_clear()
{
	g_DbgCanvas.width = 0;
	g_DbgCanvas.height = 0;
	eiCHECK_FREE(g_DbgCanvas.data);
}
