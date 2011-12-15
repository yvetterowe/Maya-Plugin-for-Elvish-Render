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

#include <eiCORE/ei_dataflow.h>
#include <eiCORE/ei_data_gen.h>
#include <eiCORE/ei_message.h>
#include <eiCORE/ei_atomic_ops.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_config.h>
#include <eiCORE/ei_assert.h>
#include <eiAPI/ei_raytracer.h>
#include <eiAPI/ei_state.h>
#include <eiAPI/ei_dbg_canvas.h>
#include <eiAPI/ei_base_bucket.h>
#include <eiAPI/ei.h>

#define IMAGE_WIDTH		640
#define IMAGE_HEIGHT	480

static void init_globals(eiGlobals *globals, eiDatabase *db)
{
	globals->interfaces = (eiInterface *)ei_allocate(sizeof(eiInterface) * EI_INTERFACE_TYPE_COUNT);
	globals->num_interfaces = EI_INTERFACE_TYPE_COUNT;

	globals->interfaces[ EI_INTERFACE_TYPE_NONE ] = NULL;

	globals->interfaces[ EI_INTERFACE_TYPE_RAYTRACER ] = (eiInterface)ei_allocate(sizeof(eiRayTracer));
	ei_rt_init((eiRayTracer *)globals->interfaces[ EI_INTERFACE_TYPE_RAYTRACER ]);
}

static void exit_globals(eiGlobals *globals, eiDatabase *db)
{
	ei_rt_exit((eiRayTracer *)globals->interfaces[ EI_INTERFACE_TYPE_RAYTRACER ]);
	eiCHECK_FREE(globals->interfaces[ EI_INTERFACE_TYPE_RAYTRACER ]);

	eiCHECK_FREE(globals->interfaces);
	globals->num_interfaces = 0;
}

static void init_tls(eiTLS *tls)
{
	ei_tls_allocate_interfaces(tls, EI_TLS_TYPE_COUNT);

	ei_tls_set_interface(tls, EI_TLS_TYPE_NONE, NULL);

	ei_tls_set_interface(tls, EI_TLS_TYPE_RAYTRACER, (eiInterface)ei_allocate(sizeof(eiRayTLS)));
	ei_ray_tls_init((eiRayTLS *)ei_tls_get_interface(tls, EI_TLS_TYPE_RAYTRACER), EI_MAX_BSP_DEPTH);
}

static void exit_tls(eiTLS *tls)
{
	ei_ray_tls_exit((eiRayTLS *)ei_tls_get_interface(tls, EI_TLS_TYPE_RAYTRACER));
	ei_tls_free_interface(tls, EI_TLS_TYPE_RAYTRACER);

	ei_tls_clear_interfaces(tls);
}

static eiBool ei_eye_ray_hit_proc(eiState *state)
{
	// we don't handle transparency here because we are 
	// just a simple test, we assume all objects are opaque.
	return eiTRUE;
}

typedef struct eiDbgCamera {
	eiScalar	image_center_x;
	eiScalar	image_center_y;
	eiScalar	pixel_to_camera_x;
	eiScalar	pixel_to_camera_y;
	eiScalar	focal;
} eiDbgCamera;

eiDbgCamera g_Camera;

static void ei_get_ray_dir(eiVector *dir, const eiScalar sx, const eiScalar sy)
{
	dir->x = (sx - g_Camera.image_center_x) * g_Camera.pixel_to_camera_x;
	dir->y = (sy - g_Camera.image_center_y) * g_Camera.pixel_to_camera_y;
	dir->z = - g_Camera.focal;
}

static void ei_simple_shade(eiVector *color, eiState *state)
{
	eiVector light_pos, light_dir, norm_light_dir;

	setv(color, 0.8f, 0.5f, 0.5f);

	// get light position.
	setv(&light_pos, 1000.0f, 1000.0f, 1000.0f);
	sub(&light_dir, &light_pos, &state->P);

	// apply Lambert cosine law.
	norm_light_dir = light_dir;
	normalizei(&norm_light_dir);
	mulvfi(color, MAX(0.0f, dot(&norm_light_dir, &state->Ng)));
}

static void ei_render_image(eiDatabase *db, eiRayTracer *rt)
{
	eiBaseBucket	approx_bucket;
	eiState			state;
	eiVector		color;

	ei_build_approx_bucket(
		&approx_bucket, 
		db, 
		0);

	for (eiInt j = 0; j < IMAGE_HEIGHT; ++j)
	{
		for (eiInt i = 0; i < IMAGE_WIDTH; ++i)
		{
			ei_state_init(&state, eiRAY_EYE, &approx_bucket);

			setv2(&state.raster, (eiScalar)i, (eiScalar)j);
			initv(&state.org);
			ei_get_ray_dir(&state.dir, state.raster.x, state.raster.y);

			if (ei_rt_trace(rt, &state, ei_eye_ray_hit_proc, eiFALSE))
			{
				ei_rt_compute_hit_details(rt, &state);
				ei_simple_shade(&color, &state);
				ei_dbg_canvas_set_pixel(state.raster.x, state.raster.y, color.x, color.y, color.z);
			}
			else
			{
				ei_dbg_canvas_set_pixel(state.raster.x, state.raster.y, 0.0f, 0.0f, 0.0f);
			}

			ei_state_exit(&state);
		}
	}
}

static eiTag ei_rt_add_object(
	eiRayTracer *rt, 
	const eiTag object)
{
	eiTag				object_tag;
	eiRayObject			*pObject;

	/* allocate a new ray-traceable object. */
	pObject = (eiRayObject *)ei_db_create(rt->db, 
		&object_tag, 
		EI_DATA_TYPE_RAY_OBJECT, 
		sizeof(eiRayObject), 
		EI_DB_FLUSHABLE);

	ei_ray_object_init(pObject, rt->db, object);

	ei_db_end(rt->db, object_tag);

	return object_tag;
}

static eiTag ei_rt_add_instance(
	eiRayTracer *rt, 
	const eiTag object_instances, 
	const eiTag object, 
	const eiBool face, 
	const eiMatrix *object_to_world, 
	const eiMatrix *motion_object_to_world)
{
	eiTag					instance_tag;
	eiRayObjectInstance		*pInstance;
	eiData					*pData;
	eiAttributes			attr;

	/* add reference to the ray-traceable object. */
	pData = ei_db_access_info(rt->db, object);

	++ pData->ref_count;

	ei_db_end(rt->db, object);

	/* allocate a new ray-traceable object instance. */
	pInstance = (eiRayObjectInstance *)ei_db_create(rt->db, 
		&instance_tag, 
		EI_DATA_TYPE_RAY_OBJECT_INST, 
		sizeof(eiRayObjectInstance), 
		EI_DB_FLUSHABLE);

	ei_attr_set_nulls(&attr);
	attr.face = face;

	ei_ray_object_instance_init(pInstance, object, &attr, object_to_world, motion_object_to_world);

	ei_db_end(rt->db, instance_tag);

	/* append it into the object instance list of a specific sub-scene. */
	ei_data_array_push_back(rt->db, object_instances, &instance_tag);

	return instance_tag;
}

class ei_raytracer_unittest : public CxxTest::TestSuite
{
public:
	void setUp();
	void tearDown();

	void testOverall();

private:
};

void ei_raytracer_unittest::setUp()
{
	// set global callbacks
	g_InitGlobals = init_globals;
	g_ExitGlobals = exit_globals;
	g_InitTLS = init_tls;
	g_ExitTLS = exit_tls;

	// allocate data generators
	g_DataGenTable.data_gens = (eiDataGen *)ei_allocate(sizeof(eiDataGen) * EI_DATA_TYPE_COUNT);
	g_DataGenTable.num_data_gens = EI_DATA_TYPE_COUNT;

	// initialize the libraries we are linking to...
	ei_core_init();
	ei_api_init();
}

void ei_raytracer_unittest::tearDown()
{
	// exit the libraries we are linking to...
	ei_api_exit();
	ei_core_exit();

	// free data generators
	eiCHECK_FREE(g_DataGenTable.data_gens);
	g_DataGenTable.num_data_gens = 0;
}

void ei_raytracer_unittest::testOverall()
{
	char			cur_dir[ EI_MAX_FILE_NAME_LEN ];
	char			config_filename[ EI_MAX_FILE_NAME_LEN ];
	eiConfig		config;

	ei_info("------------------------------------------------------\n");
	ei_info("Testing ray-tracer...\n");

	ei_info("Setting up execution environment...\n");

	//------------------------------------------------------------------------
	// read host settings from configuration file
	ei_get_current_directory(cur_dir);
	ei_append_filename(config_filename, cur_dir, "manager.ini");

	ei_config_init(&config);
	ei_config_load(&config, config_filename);

	//------------------------------------------------------------------------
	// general initialization steps for rendering manager.
	eiMaster *pMaster = ei_create_master();
	eiExecutor *pExecutor = ei_create_exec(EI_EXECUTOR_TYPE_MANAGER, g_InitTLS);
	ei_master_set_executor(pMaster, pExecutor);
	eiDatabase *pDatabase = ei_create_db(config.memlimit, 
		EI_DEFAULT_FILE_SIZE_LIMIT, 
		EI_DEFAULT_PURGE_RATE, pExecutor, pMaster);
	ei_master_set_database(pMaster, pDatabase);

	ei_config_exit(&config);

	//------------------------------------------------------------------------
	// set user data generators immediately for this renderer.
	ei_db_data_gen_table(pDatabase, &g_DataGenTable);

	// create global object and set it to database.
	eiGlobals globals;

	ei_globals_init(&globals);

	if (g_InitGlobals != NULL)
	{
		g_InitGlobals(&globals, pDatabase);
	}
	else
	{
		ei_warning("Init globals is NULL.\n");
	}
	ei_db_globals(pDatabase, &globals);

	// call the generated dump data to test ray-tracer.
	eiRayTracer *rt;
	eiDatabase *db;
	eiRayOptions *opt;
	eiRayCamera *cam;
	// the temporary pointer to the current tessellation for editing
	eiRayTessel *tessel;
	// the tag to the current tessellation created by raytracer
	eiTag tesselTag;
	eiRayVertex vtx;
	eiRayTriangle tri;
	eiMatrix object_to_world;
	eiMatrix motion_object_to_world;
	
	rt = (eiRayTracer *)globals.interfaces[ EI_INTERFACE_TYPE_RAYTRACER ];
	db = pDatabase;

	ei_dbg_canvas_init();
	ei_dbg_canvas_resize(IMAGE_WIDTH, IMAGE_HEIGHT);

	// inculde the dumped scene and samples
#ifdef _DEBUG
#include "rtdump.txt"
#else
#include "rtdump_rel.txt"
#endif

	char filename[ EI_MAX_FILE_NAME_LEN ];

	ei_append_filename(filename, cur_dir, "rtoutput.bmp");

	ei_dbg_canvas_output(filename);
	ei_dbg_canvas_clear();

	ei_info("Cleaning up...\n");

	// garbage collection, release all data
	ei_db_gc(pDatabase);

	// delete global object before database being deleted.
	if (g_ExitGlobals != NULL)
	{
		g_ExitGlobals(&globals, pDatabase);
	}
	else
	{
		ei_warning("Exit globals is NULL.\n");
	}

	ei_globals_exit(&globals);

	//------------------------------------------------------------------------
	// general shutdown steps for rendering manager.
	ei_delete_db(pDatabase);
	ei_delete_exec(pExecutor, g_ExitTLS);
	// delete pMaster after pExecutor because pExecutor may 
	// have some dependencies on pMaster.
	ei_delete_master(pMaster);
	//------------------------------------------------------------------------

	ei_info("Rendering manager shutdown.\n");
}
