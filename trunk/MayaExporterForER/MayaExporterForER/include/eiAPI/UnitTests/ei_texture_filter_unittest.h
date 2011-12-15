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

class ei_texture_filter_unittest : public CxxTest::TestSuite
{
public:
	void setUp();
	void tearDown();

	void testOverall();
};

void ei_texture_filter_unittest::setUp()
{
}

void ei_texture_filter_unittest::tearDown()
{
}

void ei_texture_filter_unittest::testOverall()
{
	char	cur_dir[ EI_MAX_FILE_NAME_LEN ];
	char	picturename[ EI_MAX_FILE_NAME_LEN ];
	char	texturename[ EI_MAX_FILE_NAME_LEN ];
	char	output_filename[ EI_MAX_FILE_NAME_LEN ];

	ei_get_current_directory(cur_dir);

	ei_context(ei_create_context());

	ei_verbose(EI_VERBOSE_ALL);
	ei_link("eiIMG");
	ei_link("eiSHADER");

	/* make the texture if we have never done so */
	ei_append_filename(picturename, cur_dir, "checker.bmp");
	ei_append_filename(texturename, cur_dir, "checker.tex");

	if (!ei_file_exists(texturename) && 
		ei_file_exists(picturename))
	{
		ei_make_texture(
			picturename, 
			texturename, 
			EI_TEX_WRAP_CLAMP, EI_TEX_WRAP_CLAMP, 
			EI_FILTER_BOX, 
			1.0f, 1.0f);
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
		ei_append_filename(output_filename, cur_dir, "frame_texture_filter01.bmp");
		ei_output(output_filename, "bmp", EI_IMG_DATA_RGB);
			ei_output_variable("color", EI_DATA_TYPE_VECTOR);
		ei_end_output();
		ei_focal(1.000000);
		ei_aperture(0.828427);
		ei_aspect(1.693122);
		ei_resolution(640, 378);
		ei_clip(0.100000, 1000000015047466200000000000000.000000);
	ei_end_camera();

	ei_instance("caminst1");
		ei_element("cam1");
		ei_transform(0.999962, -0.008727, 0.000000, 0.000000, 0.002405, 0.275627, 0.961262, 0.000000, -0.008389, -0.961225, 0.275637, 0.000000, 1.482874, -253.926529, 50.152733, 1.000000);
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
		ei_transform(1.000000, 0.000000, 0.000000, 0.000000, 0.000000, -0.000000, 1.000000, 0.000000, 0.000000, -1.000000, -0.000000, 0.000000, 0.000120, 6.860437, 142.963196, 1.000000);
	ei_end_instance();

	ei_shader("phong_shader");
		ei_shader_param_string("desc", "plastic_uv");
		ei_shader_param_vector("Cs", 1.0f, 0.2f, 0.3f);
		ei_shader_param_vector("Kd", 0.7f, 1.0f, 1.0f);
		ei_shader_param_scalar("Ks", 1.0f);
		ei_shader_param_scalar("roughness", 0.02f);
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
			ei_tab_add_vector(-114.343636, -345.448242, 0.000000);
			ei_tab_add_vector(-57.171818, -345.448242, 0.000000);
			ei_tab_add_vector(0.000000, -345.448242, 0.000000);
			ei_tab_add_vector(57.171818, -345.448242, 0.000000);
			ei_tab_add_vector(114.343636, -345.448242, 0.000000);
			ei_tab_add_vector(-114.343636, -172.724121, 0.000000);
			ei_tab_add_vector(-57.171818, -172.724121, 0.000000);
			ei_tab_add_vector(0.000000, -172.724121, 0.000000);
			ei_tab_add_vector(57.171818, -172.724121, 0.000000);
			ei_tab_add_vector(114.343636, -172.724121, 0.000000);
			ei_tab_add_vector(-114.343636, 0.000000, 0.000000);
			ei_tab_add_vector(-57.171818, 0.000000, 0.000000);
			ei_tab_add_vector(0.000000, 0.000000, 0.000000);
			ei_tab_add_vector(57.171818, 0.000000, 0.000000);
			ei_tab_add_vector(114.343636, 0.000000, 0.000000);
			ei_tab_add_vector(-114.343636, 172.724121, 0.000000);
			ei_tab_add_vector(-57.171818, 172.724121, 0.000000);
			ei_tab_add_vector(0.000000, 172.724121, 0.000000);
			ei_tab_add_vector(57.171818, 172.724121, 0.000000);
			ei_tab_add_vector(114.343636, 172.724121, 0.000000);
			ei_tab_add_vector(-114.343636, 345.448242, 0.000000);
			ei_tab_add_vector(-57.171818, 345.448242, 0.000000);
			ei_tab_add_vector(0.000000, 345.448242, 0.000000);
			ei_tab_add_vector(57.171818, 345.448242, 0.000000);
			ei_tab_add_vector(114.343636, 345.448242, 0.000000);
		ei_end_tab();
		eiTag tagVal = eiNULL_TAG;
		ei_declare("N", eiVARYING, EI_DATA_TYPE_TAG, &tagVal);
		tagVal = ei_tab(EI_DATA_TYPE_VECTOR, 1024);
		ei_variable("N", &tagVal);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
		ei_end_tab();
		tagVal = eiNULL_TAG;
		ei_declare("u", eiVARYING, EI_DATA_TYPE_TAG, &tagVal);
		tagVal = ei_tab(EI_DATA_TYPE_SCALAR, 1024);
		ei_variable("u", &tagVal);
			ei_tab_add_scalar(0.0f);
			ei_tab_add_scalar(0.25f);
			ei_tab_add_scalar(0.5f);
			ei_tab_add_scalar(0.75f);
			ei_tab_add_scalar(1.0f);
			ei_tab_add_scalar(0.0f);
			ei_tab_add_scalar(0.25f);
			ei_tab_add_scalar(0.5f);
			ei_tab_add_scalar(0.75f);
			ei_tab_add_scalar(1.0f);
			ei_tab_add_scalar(0.0f);
			ei_tab_add_scalar(0.25f);
			ei_tab_add_scalar(0.5f);
			ei_tab_add_scalar(0.75f);
			ei_tab_add_scalar(1.0f);
			ei_tab_add_scalar(0.0f);
			ei_tab_add_scalar(0.25f);
			ei_tab_add_scalar(0.5f);
			ei_tab_add_scalar(0.75f);
			ei_tab_add_scalar(1.0f);
			ei_tab_add_scalar(0.0f);
			ei_tab_add_scalar(0.25f);
			ei_tab_add_scalar(0.5f);
			ei_tab_add_scalar(0.75f);
			ei_tab_add_scalar(1.0f);
		ei_end_tab();
		tagVal = eiNULL_TAG;
		ei_declare("v", eiVARYING, EI_DATA_TYPE_TAG, &tagVal);
		tagVal = ei_tab(EI_DATA_TYPE_SCALAR, 1024);
		ei_variable("v", &tagVal);
			ei_tab_add_scalar(0.0f);
			ei_tab_add_scalar(0.0f);
			ei_tab_add_scalar(0.0f);
			ei_tab_add_scalar(0.0f);
			ei_tab_add_scalar(0.0f);
			ei_tab_add_scalar(0.25f);
			ei_tab_add_scalar(0.25f);
			ei_tab_add_scalar(0.25f);
			ei_tab_add_scalar(0.25f);
			ei_tab_add_scalar(0.25f);
			ei_tab_add_scalar(0.5f);
			ei_tab_add_scalar(0.5f);
			ei_tab_add_scalar(0.5f);
			ei_tab_add_scalar(0.5f);
			ei_tab_add_scalar(0.5f);
			ei_tab_add_scalar(0.75f);
			ei_tab_add_scalar(0.75f);
			ei_tab_add_scalar(0.75f);
			ei_tab_add_scalar(0.75f);
			ei_tab_add_scalar(0.75f);
			ei_tab_add_scalar(1.0f);
			ei_tab_add_scalar(1.0f);
			ei_tab_add_scalar(1.0f);
			ei_tab_add_scalar(1.0f);
			ei_tab_add_scalar(1.0f);
		ei_end_tab();
		ei_triangle_list(ei_tab(EI_DATA_TYPE_INDEX, 1024));
			ei_tab_add_index(5);
			ei_tab_add_index(0);
			ei_tab_add_index(6);
			ei_tab_add_index(1);
			ei_tab_add_index(6);
			ei_tab_add_index(0);
			ei_tab_add_index(6);
			ei_tab_add_index(1);
			ei_tab_add_index(7);
			ei_tab_add_index(2);
			ei_tab_add_index(7);
			ei_tab_add_index(1);
			ei_tab_add_index(7);
			ei_tab_add_index(2);
			ei_tab_add_index(8);
			ei_tab_add_index(3);
			ei_tab_add_index(8);
			ei_tab_add_index(2);
			ei_tab_add_index(8);
			ei_tab_add_index(3);
			ei_tab_add_index(9);
			ei_tab_add_index(4);
			ei_tab_add_index(9);
			ei_tab_add_index(3);
			ei_tab_add_index(10);
			ei_tab_add_index(5);
			ei_tab_add_index(11);
			ei_tab_add_index(6);
			ei_tab_add_index(11);
			ei_tab_add_index(5);
			ei_tab_add_index(11);
			ei_tab_add_index(6);
			ei_tab_add_index(12);
			ei_tab_add_index(7);
			ei_tab_add_index(12);
			ei_tab_add_index(6);
			ei_tab_add_index(12);
			ei_tab_add_index(7);
			ei_tab_add_index(13);
			ei_tab_add_index(8);
			ei_tab_add_index(13);
			ei_tab_add_index(7);
			ei_tab_add_index(13);
			ei_tab_add_index(8);
			ei_tab_add_index(14);
			ei_tab_add_index(9);
			ei_tab_add_index(14);
			ei_tab_add_index(8);
			ei_tab_add_index(15);
			ei_tab_add_index(10);
			ei_tab_add_index(16);
			ei_tab_add_index(11);
			ei_tab_add_index(16);
			ei_tab_add_index(10);
			ei_tab_add_index(16);
			ei_tab_add_index(11);
			ei_tab_add_index(17);
			ei_tab_add_index(12);
			ei_tab_add_index(17);
			ei_tab_add_index(11);
			ei_tab_add_index(17);
			ei_tab_add_index(12);
			ei_tab_add_index(18);
			ei_tab_add_index(13);
			ei_tab_add_index(18);
			ei_tab_add_index(12);
			ei_tab_add_index(18);
			ei_tab_add_index(13);
			ei_tab_add_index(19);
			ei_tab_add_index(14);
			ei_tab_add_index(19);
			ei_tab_add_index(13);
			ei_tab_add_index(20);
			ei_tab_add_index(15);
			ei_tab_add_index(21);
			ei_tab_add_index(16);
			ei_tab_add_index(21);
			ei_tab_add_index(15);
			ei_tab_add_index(21);
			ei_tab_add_index(16);
			ei_tab_add_index(22);
			ei_tab_add_index(17);
			ei_tab_add_index(22);
			ei_tab_add_index(16);
			ei_tab_add_index(22);
			ei_tab_add_index(17);
			ei_tab_add_index(23);
			ei_tab_add_index(18);
			ei_tab_add_index(23);
			ei_tab_add_index(17);
			ei_tab_add_index(23);
			ei_tab_add_index(18);
			ei_tab_add_index(24);
			ei_tab_add_index(19);
			ei_tab_add_index(24);
			ei_tab_add_index(18);
		ei_end_tab();
	ei_end_object();

	ei_instance("inst1");
		ei_add_material("mtl");
		ei_element("obj1");
		ei_transform(1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 2.742935, 1.037594, 0.000000, 1.000000);
	ei_end_instance();

	ei_instgroup("world");
		ei_add_instance("caminst1");
		ei_add_instance("lightinst1");
		ei_add_instance("inst1");
	ei_end_instgroup();

	// render frame 01
	ei_render("world", "caminst1", "opt");

	ei_delete_context(ei_context(NULL));
}
