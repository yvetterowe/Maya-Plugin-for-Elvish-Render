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

class ei_instancing_unittest : public CxxTest::TestSuite
{
public:
	void setUp();
	void tearDown();

	void testOverall();

private:
};

void ei_instancing_unittest::setUp()
{
}

void ei_instancing_unittest::tearDown()
{
}

void ei_instancing_unittest::testOverall()
{
	char	cur_dir[ EI_MAX_FILE_NAME_LEN ];
	char	output_filename[ EI_MAX_FILE_NAME_LEN ];

	ei_get_current_directory(cur_dir);

	ei_context(ei_create_context());

	ei_verbose(EI_VERBOSE_ALL);
	ei_link("eiIMG");
	ei_link("eiSHADER");

	ei_options("opt");
		ei_samples(0, 2);
		ei_contrast(0.05f, 0.05f, 0.05f, 0.05f);
		ei_filter(EI_FILTER_GAUSSIAN, 3.0f);
	ei_end_options();

	ei_camera("cam1");
		ei_append_filename(output_filename, cur_dir, "frame_instancing01.bmp");
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
		ei_transform(-0.000000, -1.000000, 0.000000, 0.000000, 0.374607, -0.000000, 0.927184, 0.000000, -0.927184, 0.000000, 0.374607, 0.000000, -495.413849, -1.083953, 269.927765, 1.000000);
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
		ei_transform(1.000000, 0.000000, 0.000000, 0.000000, 0.000000, -0.000000, 1.000000, 0.000000, 0.000000, -1.000000, -0.000000, 0.000000, -677.384033, -221.470703, 279.836304, 1.000000);
	ei_end_instance();

	ei_shader("phong_shader");
		ei_shader_param_string("desc", "plastic");
		ei_shader_param_vector("Cs", 1.0f, 0.2f, 0.3f);
		ei_shader_param_vector("Kd", 0.7f, 1.0f, 1.0f);
		ei_shader_param_scalar("Ks", 1.0f);
		ei_shader_param_scalar("roughness", 0.2f);
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
			ei_tab_add_vector(-17.859985, -16.176228, 0.000000);
			ei_tab_add_vector(-17.859985, -16.176228, 0.000000);
			ei_tab_add_vector(-17.859985, -16.176228, 0.000000);
			ei_tab_add_vector(17.859985, -16.176228, 0.000000);
			ei_tab_add_vector(17.859985, -16.176228, 0.000000);
			ei_tab_add_vector(17.859985, -16.176228, 0.000000);
			ei_tab_add_vector(-17.859985, 16.176228, 0.000000);
			ei_tab_add_vector(-17.859985, 16.176228, 0.000000);
			ei_tab_add_vector(-17.859985, 16.176228, 0.000000);
			ei_tab_add_vector(17.859985, 16.176228, 0.000000);
			ei_tab_add_vector(17.859985, 16.176228, 0.000000);
			ei_tab_add_vector(17.859985, 16.176228, 0.000000);
			ei_tab_add_vector(-17.859985, -16.176228, 47.707706);
			ei_tab_add_vector(-17.859985, -16.176228, 47.707706);
			ei_tab_add_vector(-17.859985, -16.176228, 47.707706);
			ei_tab_add_vector(17.859985, -16.176228, 47.707706);
			ei_tab_add_vector(17.859985, -16.176228, 47.707706);
			ei_tab_add_vector(17.859985, -16.176228, 47.707706);
			ei_tab_add_vector(-17.859985, 16.176228, 47.707706);
			ei_tab_add_vector(-17.859985, 16.176228, 47.707706);
			ei_tab_add_vector(-17.859985, 16.176228, 47.707706);
			ei_tab_add_vector(17.859985, 16.176228, 47.707706);
			ei_tab_add_vector(17.859985, 16.176228, 47.707706);
			ei_tab_add_vector(17.859985, 16.176228, 47.707706);
			ei_end_tab();
		eiTag tagVal = eiNULL_TAG;
		ei_declare("N", eiVARYING, EI_DATA_TYPE_TAG, &tagVal);
		tagVal = ei_tab(EI_DATA_TYPE_VECTOR, 1024);
		ei_variable("N", &tagVal);
			ei_tab_add_vector(0.000000, 0.000000, -1.000000);
			ei_tab_add_vector(0.000000, -1.000000, 0.000000);
			ei_tab_add_vector(-1.000000, 0.000000, 0.000000);
			ei_tab_add_vector(0.000000, 0.000000, -1.000000);
			ei_tab_add_vector(0.000000, -1.000000, 0.000000);
			ei_tab_add_vector(1.000000, 0.000000, 0.000000);
			ei_tab_add_vector(0.000000, 0.000000, -1.000000);
			ei_tab_add_vector(0.000000, 1.000000, 0.000000);
			ei_tab_add_vector(-1.000000, 0.000000, 0.000000);
			ei_tab_add_vector(0.000000, 0.000000, -1.000000);
			ei_tab_add_vector(1.000000, 0.000000, -0.000000);
			ei_tab_add_vector(0.000000, 1.000000, 0.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, -1.000000, 0.000000);
			ei_tab_add_vector(-1.000000, 0.000000, 0.000000);
			ei_tab_add_vector(0.000000, -0.000000, 1.000000);
			ei_tab_add_vector(0.000000, -1.000000, 0.000000);
			ei_tab_add_vector(1.000000, -0.000000, 0.000000);
			ei_tab_add_vector(-0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(0.000000, 1.000000, 0.000000);
			ei_tab_add_vector(-1.000000, -0.000000, -0.000000);
			ei_tab_add_vector(0.000000, 0.000000, 1.000000);
			ei_tab_add_vector(1.000000, 0.000000, 0.000000);
			ei_tab_add_vector(0.000000, 1.000000, 0.000000);
			ei_end_tab();
		ei_triangle_list(ei_tab(EI_DATA_TYPE_INDEX, 1024));
			ei_tab_add_index(0);
			ei_tab_add_index(6);
			ei_tab_add_index(9);
			ei_tab_add_index(9);
			ei_tab_add_index(3);
			ei_tab_add_index(0);
			ei_tab_add_index(12);
			ei_tab_add_index(15);
			ei_tab_add_index(21);
			ei_tab_add_index(21);
			ei_tab_add_index(18);
			ei_tab_add_index(12);
			ei_tab_add_index(1);
			ei_tab_add_index(4);
			ei_tab_add_index(16);
			ei_tab_add_index(16);
			ei_tab_add_index(13);
			ei_tab_add_index(1);
			ei_tab_add_index(5);
			ei_tab_add_index(10);
			ei_tab_add_index(22);
			ei_tab_add_index(22);
			ei_tab_add_index(17);
			ei_tab_add_index(5);
			ei_tab_add_index(11);
			ei_tab_add_index(7);
			ei_tab_add_index(19);
			ei_tab_add_index(19);
			ei_tab_add_index(23);
			ei_tab_add_index(11);
			ei_tab_add_index(8);
			ei_tab_add_index(2);
			ei_tab_add_index(14);
			ei_tab_add_index(14);
			ei_tab_add_index(20);
			ei_tab_add_index(8);
		ei_end_tab();
	ei_end_object();

	ei_instance("inst1");
		ei_add_material("mtl");
		ei_element("obj1");
		ei_transform(0.552933, 0.332917, 0.763827, 0.000000, -0.515813, 0.856701, 0.000000, 0.000000, -0.654371, -0.393992, 0.645421, 0.000000, -2.252075, -2.493015, 0.000000, 1.000000);
	ei_end_instance();

	ei_instance("inst2");
		ei_add_material("mtl");
		ei_element("obj1");
		ei_transform(-0.451175, 0.461530, 0.763827, 0.000000, -0.715082, -0.699040, 0.000000, 0.000000, 0.533945, -0.546199, 0.645421, 0.000000, -2.252075, -2.493015, 0.000000, 1.000000);
	ei_end_instance();

	ei_instance("inst3");
		ei_add_material("mtl");
		ei_element("obj1");
		ei_transform(0.019942, -0.645113, 0.763827, 0.000000, 0.999523, 0.030897, 0.000000, 0.000000, -0.023600, 0.763462, 0.645421, 0.000000, -2.252075, -2.493015, 0.000000, 1.000000);
	ei_end_instance();

	ei_instgroup("instgroup1");
		ei_add_instance("inst1");
		ei_add_instance("inst2");
		ei_add_instance("inst3");
	ei_end_instgroup();

	ei_instance("instgroup_inst1");
		ei_add_material("mtl");
		ei_element("instgroup1");
		ei_transform(0.552933, 0.332917, 0.763827, 0.000000, -0.515813, 0.856701, 0.000000, 0.000000, -0.654371, -0.393992, 0.645421, 0.000000, -91.301430, -44.680016, 87.791328, 1.000000);
	ei_end_instance();

	ei_instance("instgroup_inst2");
		ei_add_material("mtl");
		ei_element("instgroup1");
		ei_transform(-0.451175, 0.461530, 0.763827, 0.000000, -0.715082, -0.699040, 0.000000, 0.000000, 0.533945, -0.546199, 0.645421, 0.000000, 82.963608, -44.680016, 85.988586, 1.000000);
	ei_end_instance();

	ei_instance("instgroup_inst3");
		ei_add_material("mtl");
		ei_element("instgroup1");
		ei_transform(0.019942, -0.645113, 0.763827, 0.000000, 0.999523, 0.030897, 0.000000, 0.000000, -0.023600, 0.763462, 0.645421, 0.000000, -1.106793, 73.903511, 88.692696, 1.000000);
	ei_end_instance();

	ei_instgroup("world");
		ei_add_instance("caminst1");
		ei_add_instance("lightinst1");
		ei_add_instance("instgroup_inst1");
		ei_add_instance("instgroup_inst2");
		ei_add_instance("instgroup_inst3");
	ei_end_instgroup();

	// render frame 01
	ei_render("world", "caminst1", "opt");

	ei_camera("cam1");
		ei_append_filename(output_filename, cur_dir, "frame_instancing02.bmp");
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
