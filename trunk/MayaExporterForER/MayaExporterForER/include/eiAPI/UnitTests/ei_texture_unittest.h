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

#include <cxxtest/TestSuite.h>

#include <eiAPI/ei.h>
#include <eiAPI/ei_texture.h>
#include <vector>

class ei_texture_unittest : public CxxTest::TestSuite
{
public:
	void setUp();
	void tearDown();

	void testOverall();

private:
};

void ei_texture_unittest::setUp()
{
}

void ei_texture_unittest::tearDown()
{
}

/** \brief Convert internal texture file to bitmap file. */
void tex2bmp(const char *tex_filename, const char *bmp_filename)
{
	eiFileHandle texFile = ei_open_file(tex_filename, EI_FILE_READ);
	if (texFile == NULL)
	{
		ei_error("Failed to open the texture %s\n", tex_filename);
		return;
	}

	eiFileHandle bmpFile = ei_open_file(bmp_filename, EI_FILE_WRITE);
	if (bmpFile == NULL)
	{
		ei_error("Failed to create the bitmap %s\n", bmp_filename);
		return;
	}

	// first, read the file header
	eiTextureHeader header;
	ei_read_file(texFile, &header, sizeof(eiTextureHeader));

	// then read texture layer infos
	std::vector< eiTextureLayerInfo > layerInfos;

	layerInfos.resize(header.num_layers);

	for (eiInt i = 0; i < header.num_layers; ++i)
	{
		ei_read_file(texFile, &layerInfos[i], sizeof(eiTextureLayerInfo));
	}

	// process the texture data, just care about the first layer
	eiTextureLayerInfo layerInfo = layerInfos[0];

	eiInt x_tiles = 0, y_tiles = 0;
	ei_compute_num_tiles(&x_tiles, &y_tiles, layerInfo.width, layerInfo.height, header.tile_size);

	eiBuffer	buffer;

	ei_buffer_init(&buffer, sizeof(eiVector), NULL, NULL, NULL, NULL, NULL, NULL);
	ei_buffer_allocate(&buffer, x_tiles * header.tile_size, y_tiles * header.tile_size);

	eiByte *tile = new eiByte [header.tile_size * header.tile_size * header.num_channels * header.channel_size];

	for (eiInt y = 0; y < y_tiles; ++y)
	{
		for (eiInt x = 0; x < x_tiles; ++x)
		{
			ei_read_file(texFile, tile, header.tile_size * header.tile_size * header.num_channels * header.channel_size);
			eiByte *pval = tile;

			for (eiInt j = 0; j < header.tile_size; ++j)
			{
				for (eiInt i = 0; i < header.tile_size; ++i)
				{
					eiVector vec3;
					initv(&vec3);

					for (eiInt k = 0; k < header.num_channels; ++k)
					{
						switch (header.channel_size)
						{
						case 1:
							if (k < 3)
							{
								vec3.comp[k] = (eiScalar)(*pval) / 255.0f;
							}
							break;
						case 2:
							if (k < 3)
							{
								vec3.comp[k] = (eiScalar)(*((eiUshort *)pval)) / 65535.0f;
							}
							break;
						case 4:
							if (k < 3)
							{
								vec3.comp[k] = (*((eiScalar *)pval));
							}
							break;
						}
						pval += header.channel_size;
					}

					ei_buffer_set(&buffer, x * header.tile_size + i, y * header.tile_size + j, &vec3);
				}
			}
		}
	}

	delete [] tile;
	ei_close_file(texFile);

	// data is ready, write to bitmap file
	BITMAPFILEHEADER	bfh;
	BITMAPINFOHEADER	bih;
	// the .bmp requests the width of scanlines must be multiply of 4.
	eiInt	blanks = ((ei_buffer_get_width(&buffer) * 3 + 3) & ~3) - ei_buffer_get_width(&buffer) * 3;

	// prepare bitmap file header
	memset(&bfh, 0, sizeof(bfh));
	bfh.bfType			= 0x4d42;	// "BM"
	bfh.bfSize			= sizeof(bfh) + sizeof(bih)
							+ ((ei_buffer_get_width(&buffer) * 3 + 3) & ~3) * ei_buffer_get_height(&buffer);
	bfh.bfOffBits		= sizeof(bfh) + sizeof(bih);
	
	// prepare bitmap info header
	memset(&bih, 0, sizeof(bih));
	bih.biSize			= sizeof(bih);
	bih.biWidth			= ei_buffer_get_width(&buffer);
	bih.biHeight		= ei_buffer_get_height(&buffer);
	bih.biPlanes		= 1;
	bih.biBitCount		= 24;
	bih.biCompression	= BI_RGB;

	ei_write_file(bmpFile, &bfh, sizeof(bfh));
	ei_write_file(bmpFile, &bih, sizeof(bih));

	eiByte	chan = 0;

	for (eiInt y = (ei_buffer_get_height(&buffer) - 1); y >= 0; --y)
	{
		for (eiInt x = 0; x < ei_buffer_get_width(&buffer); ++x)
		{
			eiVector	cval;
			initv(&cval);

			ei_buffer_get(&buffer, x, y, &cval);
			// .bmp writes in BGR order
			chan = (eiByte)(cval.b * 255.0f);
			ei_write_file(bmpFile, &chan, sizeof(eiByte));
			chan = (eiByte)(cval.g * 255.0f);
			ei_write_file(bmpFile, &chan, sizeof(eiByte));
			chan = (eiByte)(cval.r * 255.0f);
			ei_write_file(bmpFile, &chan, sizeof(eiByte));
		}

		if (blanks > 0)
		{
			const eiInt	zero = 0;

			for (eiInt i = 0; i < blanks; ++i)
			{
				ei_write_file(bmpFile, (void *)&zero, sizeof(eiByte));
			}
		}
	}

	ei_close_file(bmpFile);
	ei_info("Finished conversion.\n");
}

void ei_texture_unittest::testOverall()
{
	char	cur_dir[ EI_MAX_FILE_NAME_LEN ];
	char	picturename[ EI_MAX_FILE_NAME_LEN ];
	char	texturename[ EI_MAX_FILE_NAME_LEN ];
	char	check_picturename[ EI_MAX_FILE_NAME_LEN ];
	char	output_filename[ EI_MAX_FILE_NAME_LEN ];

	ei_get_current_directory(cur_dir);

	ei_context(ei_create_context());

	ei_verbose(EI_VERBOSE_ALL);
	ei_link("eiIMG");
	ei_link("eiSHADER");

	/* make the texture if we have never done so */
	ei_append_filename(picturename, cur_dir, "leopard.bmp");
	ei_append_filename(texturename, cur_dir, "leopard.tex");
	ei_append_filename(check_picturename, cur_dir, "leopard_check.bmp");

	if (!ei_file_exists(texturename) && 
		ei_file_exists(picturename))
	{
		ei_make_texture(
			picturename, 
			texturename, 
			EI_TEX_WRAP_CLAMP, EI_TEX_WRAP_CLAMP, 
			EI_FILTER_BOX, 
			1.0f, 1.0f);

		tex2bmp(texturename, check_picturename);
	}

	if (ei_file_exists(texturename))
	{
		ei_texture(texturename);
			ei_file_texture(texturename, eiFALSE);
		ei_end_texture();
	}

	ei_options("opt");
		ei_samples(0, 2);
		ei_contrast(0.05f, 0.05f, 0.05f, 0.05f);
		ei_filter(EI_FILTER_GAUSSIAN, 3.0f);	
	ei_end_options();

	ei_camera("cam1");
		ei_append_filename(output_filename, cur_dir, "frame_tex01.bmp");
		ei_output(output_filename, "bmp", EI_IMG_DATA_RGB);
			ei_output_variable("color", EI_DATA_TYPE_VECTOR);
		ei_end_output();
		ei_focal(100.0f);
		ei_aperture(144.724029f);
		ei_aspect(800.0f / 600.0f);
		ei_resolution(800, 600);
	ei_end_camera();

	ei_instance("caminst1");
		ei_element("cam1");
	ei_end_instance();

	ei_shader("point_light_shader");
		ei_shader_param_string("desc", "pointlight");
		ei_shader_param_scalar("intensity", 1.0f);
		ei_shader_param_vector("lightcolor", 1.0f, 1.0f, 1.0f);
	ei_end_shader();

	ei_light("light1");
		ei_add_light("point_light_shader");
		ei_origin(141.375732f, 83.116005f, 35.619434f);
	ei_end_light();

	ei_instance("lightinst1");
		ei_element("light1");
	ei_end_instance();

	ei_shader("phong_shader");
		ei_shader_param_string("desc", "plastic");
		ei_shader_param_vector("Cs", 1.0f, 0.2f, 0.3f);
		ei_shader_param_vector("Kd", 0.7f, 1.0f, 1.0f);
		ei_shader_param_scalar("Ks", 1.0f);
		ei_shader_param_scalar("roughness", 0.2f);
		ei_shader_param_texture("Cs_tex", texturename);
	ei_end_shader();

	ei_shader("opaque_shadow");
		ei_shader_param_string("desc", "opaque");
	ei_end_shader();

	ei_material("mtl");
		ei_add_surface("phong_shader");
		ei_add_shadow("opaque_shadow");
	ei_end_material();

	ei_object("obj1", "poly");
		ei_pos_list(ei_tab(EI_DATA_TYPE_VECTOR, 1024));
			ei_tab_add_vector(-7.068787f, -4.155799f, -22.885710f);
			ei_tab_add_vector(-0.179573f, -7.973234f, -16.724060f);
			ei_tab_add_vector(-7.068787f, 4.344949f, -17.619093f);
			ei_tab_add_vector(-0.179573f, 0.527515f, -11.457443f);
			ei_tab_add_vector(0.179573f, -0.527514f, -28.742058f);
			ei_tab_add_vector(7.068787f, -4.344948f, -22.580408f);
			ei_tab_add_vector(0.179573f, 7.973235f, -23.475441f);
			ei_tab_add_vector(7.068787f, 4.155800f, -17.313791f);
		ei_end_tab();
		ei_triangle_list(ei_tab(EI_DATA_TYPE_INDEX, 1024));
			ei_tab_add_index(0); ei_tab_add_index(1); ei_tab_add_index(3);
			ei_tab_add_index(0); ei_tab_add_index(3); ei_tab_add_index(2);
			ei_tab_add_index(1); ei_tab_add_index(5); ei_tab_add_index(7);
			ei_tab_add_index(1); ei_tab_add_index(7); ei_tab_add_index(3);
			ei_tab_add_index(5); ei_tab_add_index(4); ei_tab_add_index(6);
			ei_tab_add_index(5); ei_tab_add_index(6); ei_tab_add_index(7);
			ei_tab_add_index(4); ei_tab_add_index(0); ei_tab_add_index(2);
			ei_tab_add_index(4); ei_tab_add_index(2); ei_tab_add_index(6);
			ei_tab_add_index(4); ei_tab_add_index(5); ei_tab_add_index(1);
			ei_tab_add_index(4); ei_tab_add_index(1); ei_tab_add_index(0);
			ei_tab_add_index(2); ei_tab_add_index(3); ei_tab_add_index(7);
			ei_tab_add_index(2); ei_tab_add_index(7); ei_tab_add_index(6);
		ei_end_tab();
	ei_end_object();

	ei_instance("inst1");
		ei_add_material("mtl");
		ei_element("obj1");
	ei_end_instance();

	ei_instgroup("world");
		ei_add_instance("caminst1");
		ei_add_instance("lightinst1");
		ei_add_instance("inst1");
	ei_end_instgroup();

	// render frame 01
	ei_render("world", "caminst1", "opt");

	ei_camera("cam1");
		ei_append_filename(output_filename, cur_dir, "frame_tex02.bmp");
		ei_output(output_filename, "bmp", EI_IMG_DATA_RGB);
			ei_output_variable("color", EI_DATA_TYPE_VECTOR);
		ei_end_output();
		ei_aperture(100.0f);
	ei_end_camera();

	ei_shader("point_light_shader");
		ei_shader_param_vector("lightcolor", 1.0f, 0.5f, 1.0f);
	ei_end_shader();

	// render frame 02
	ei_render("world", "caminst1", "opt");

	ei_delete_context(ei_context(NULL));
}
