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
#include <eiCORE/ei_data_table.h>
#include <eiCORE/ei_random.h>

class ei_hair_unittest : public CxxTest::TestSuite
{
public:
	void setUp();
	void tearDown();

	void testOverall();

private:
};

void ei_hair_unittest::setUp()
{
}

void ei_hair_unittest::tearDown()
{
}

/** \brief Grow some random hairs on a polygon object */
static void generate_hair(
	eiDatabase *db, 
	const eiTag pos_list, 
	const eiTag tri_list, 
	const eiScalar density, 
	const eiInt num_segments, 
	const eiScalar segment_length, 
	const eiScalar radius, 
	const eiScalar randomness)
{
	eiDataTableIterator	tri_list_iter;
	eiDataTableIterator	pos_list_iter;
	
	ei_data_table_begin(db, tri_list, &tri_list_iter);
	ei_data_table_begin(db, pos_list, &pos_list_iter);

	eiInt num_triangles = tri_list_iter.tab->item_count / 3;

	eiTag vtx_list = ei_tab(EI_DATA_TYPE_VECTOR4, 100000);
	ei_end_tab();
	eiTag hair_list = ei_tab(EI_DATA_TYPE_INDEX, 100000);
	ei_end_tab();
	TS_ASSERT(vtx_list != eiNULL_TAG);
	TS_ASSERT(hair_list != eiNULL_TAG);

	eiRandomGen		randGen;

	ei_random_reset(&randGen, EI_DEFAULT_RANDOM_SEED);

	eiInt num_total_hairs = 0;
	eiInt num_total_vertices = 0;

	eiDataTable *vtx_tab = (eiDataTable *)ei_db_access(db, vtx_list);
	eiDataTable *hair_tab = (eiDataTable *)ei_db_access(db, hair_list);

	for (eiInt i = 0; i < num_triangles; ++i)
	{
		eiIndex		v1, v2, v3;
		eiVector	pos1, pos2, pos3;
		eiVector4	normal;

		/* access 3 vertex indices of triangle */
		v1 = *((eiIndex *)ei_data_table_read(&tri_list_iter, i * 3 + 0));
		v2 = *((eiIndex *)ei_data_table_read(&tri_list_iter, i * 3 + 1));
		v3 = *((eiIndex *)ei_data_table_read(&tri_list_iter, i * 3 + 2));

		/* access 3 vertices */
		movv(&pos1, (eiVector *)ei_data_table_read(&pos_list_iter, v1));
		movv(&pos2, (eiVector *)ei_data_table_read(&pos_list_iter, v2));
		movv(&pos3, (eiVector *)ei_data_table_read(&pos_list_iter, v3));

		get_normal(&pos1, &pos2, &pos3, &normal);

		eiScalar area = absf(calc_tri_area(&pos1, &pos2, &pos3));
		eiInt num_hairs = (eiInt)(area * density);
		num_total_hairs += num_hairs;

		for (eiInt j = 0; j < num_hairs; ++j)
		{
			eiVector	bary;
			eiVector	P;
			eiScalar	R;
			eiVector	N, X, Y;
			eiVector4	vtx;
			eiIndex		index;

			ei_uniform_sample_triangle(&bary, (eiScalar)ei_random(&randGen), (eiScalar)ei_random(&randGen));

			interp_point(&P, &pos1, &pos2, &pos3, &bary);

			index = ei_data_table_size(vtx_tab);
			ei_data_table_push_back(db, &hair_tab, &index);
			index = num_segments;
			ei_data_table_push_back(db, &hair_tab, &index);

			R = radius;
			vtx.xyz = P;
			vtx.w = R;
			ei_data_table_push_back(db, &vtx_tab, &vtx);
			++ num_total_vertices;

			N = normal.xyz;
			ortho_basis(&N, &X, &Y);
			mulvfi(&N, segment_length);
			mulvfi(&X, segment_length * randomness * ((eiScalar)ei_random(&randGen) * 2.0f - 1.0f));
			mulvfi(&Y, segment_length * randomness * ((eiScalar)ei_random(&randGen) * 2.0f - 1.0f));
			addi(&N, &X);
			addi(&N, &Y);

			for (eiInt k = 0; k < num_segments; ++k)
			{
				for (eiInt l = 0; l < 3; ++l)
				{
					addi(&P, &N);

					R *= 0.9f;
					vtx.xyz = P;
					vtx.w = R;
					ei_data_table_push_back(db, &vtx_tab, &vtx);
					++ num_total_vertices;

					// don't do it for the end point to make the curve smooth
					if (l != 2)
					{
						normalizei(&N);
						ortho_basis(&N, &X, &Y);
						mulvfi(&N, segment_length);
						mulvfi(&X, segment_length * randomness * ((eiScalar)ei_random(&randGen) * 2.0f - 1.0f));
						mulvfi(&Y, segment_length * randomness * ((eiScalar)ei_random(&randGen) * 2.0f - 1.0f));
						addi(&N, &X);
						addi(&N, &Y);
					}
				}
			}
		}
	}

	ei_db_end(db, hair_list);
	ei_db_end(db, vtx_list);

	ei_data_table_end(&pos_list_iter);
	ei_data_table_end(&tri_list_iter);
	
	ei_vertex_list(vtx_list);
	ei_hair_list(hair_list);

	ei_info("Number of generated hairs: %d\n", num_total_hairs);
	ei_info("Number of generated hair vertices: %d\n", num_total_vertices);
}

void ei_hair_unittest::testOverall()
{
	char	cur_dir[ EI_MAX_FILE_NAME_LEN ];
	char	output_filename[ EI_MAX_FILE_NAME_LEN ];

	ei_get_current_directory(cur_dir);

	eiContext *context = ei_create_context();
	ei_context(context);

	ei_verbose(EI_VERBOSE_ALL);
	ei_link("eiIMG");
	ei_link("eiSHADER");

	ei_options("opt");
		ei_samples(0, 2);
		ei_contrast(0.05f, 0.05f, 0.05f, 0.05f);
		ei_filter(EI_FILTER_GAUSSIAN, 3.0f);
	ei_end_options();

	ei_camera("cam1");
		ei_append_filename(output_filename, cur_dir, "frame_hair01.bmp");
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
	ei_end_shader();

	ei_shader("opaque_shadow");
		ei_shader_param_string("desc", "opaque");
	ei_end_shader();

	ei_material("mtl");
		ei_add_surface("phong_shader");
		ei_add_shadow("opaque_shadow");
	ei_end_material();

	ei_object("obj1", "poly");
		eiTag pos_list = ei_tab(EI_DATA_TYPE_VECTOR, 1024);
		ei_pos_list(pos_list);
			ei_tab_add_vector(-7.068787f, -4.155799f, -22.885710f);
			ei_tab_add_vector(-0.179573f, -7.973234f, -16.724060f);
			ei_tab_add_vector(-7.068787f, 4.344949f, -17.619093f);
			ei_tab_add_vector(-0.179573f, 0.527515f, -11.457443f);
			ei_tab_add_vector(0.179573f, -0.527514f, -28.742058f);
			ei_tab_add_vector(7.068787f, -4.344948f, -22.580408f);
			ei_tab_add_vector(0.179573f, 7.973235f, -23.475441f);
			ei_tab_add_vector(7.068787f, 4.155800f, -17.313791f);
		ei_end_tab();
		eiTag tri_list = ei_tab(EI_DATA_TYPE_INDEX, 1024);
		ei_triangle_list(tri_list);
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

	ei_shader("hair_shader");
		ei_shader_param_string("desc", "simple_hair");
		ei_shader_param_vector("Cs", 0.426f, 0.355f, 0.113f);
		ei_shader_param_vector("Kd", 0.7f, 0.7f, 0.7f);
		ei_shader_param_scalar("Ks", 1.0f);
		ei_shader_param_vector("specularcolor", 1.0f, 1.0f, 1.0f);
	ei_end_shader();

	ei_shader("hair_shadow");
		ei_shader_param_string("desc", "opaque");
	ei_end_shader();

	ei_material("hair_mtl");
		ei_add_surface("hair_shader");
		ei_add_shadow("hair_shadow");
	ei_end_material();

	ei_object("obj2", "hair");
		ei_degree(3);
		generate_hair(ei_context_database(context), pos_list, tri_list, 6.0f, 2, 1.0f, 0.01f, 0.25f);
	ei_end_object();

	ei_instance("inst2");
		ei_add_material("hair_mtl");
		ei_element("obj2");
	ei_end_instance();

	ei_instgroup("world");
		ei_add_instance("caminst1");
		ei_add_instance("lightinst1");
		ei_add_instance("inst1");
		ei_add_instance("inst2");
	ei_end_instgroup();

	// render frame 01
	ei_render("world", "caminst1", "opt");

	ei_camera("cam1");
		ei_append_filename(output_filename, cur_dir, "frame_hair02.bmp");
		ei_output(output_filename, "bmp", EI_IMG_DATA_RGB);
			ei_output_variable("color", EI_DATA_TYPE_VECTOR);
		ei_end_output();
		ei_aperture(100.0f);
	ei_end_camera();

	ei_shader("point_light_shader");
		ei_shader_param_vector("lightcolor", 1.0f, 0.5f, 1.0f);
	ei_end_shader();

	// render frame 02
	//ei_render("world", "caminst1", "opt");

	ei_delete_context(ei_context(NULL));
}
