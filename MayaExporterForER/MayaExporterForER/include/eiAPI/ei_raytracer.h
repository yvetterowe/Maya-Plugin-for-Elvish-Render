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
 
#ifndef EI_RAYTRACER_H
#define EI_RAYTRACER_H

/** \brief The standard ray-tracing engine of the rendering core, this is 
 * independent of the scene description interface. The main idea for this 
 * module is divide-and-conquer.
 * \file ei_raytracer.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_bsp.h>
#include <eiAPI/ei_attributes.h>
#include <eiAPI/ei_state.h>
#include <eiCORE/ei_vector.h>
#include <eiCORE/ei_vector4.h>
#include <eiCORE/ei_matrix.h>
#include <eiCORE/ei_platform.h>
#include <eiCORE/ei_array.h>
#include <eiCORE/ei_slist.h>

#ifdef __cplusplus
extern "C" {
#endif

/* for fixing numerous error caused by t_near, t_far rejections. */
#define EI_DISTANCE_TOL					0.00005f
/* the hair BSP-tree must NOT be deeper than this */
#define EI_MAX_HAIR_BSP_DEPTH			40

/* return TRUE/FALSE in ray intersection test */
#define RETURN_TRUE(state)		{ (state)->found_hit |= eiTRUE; return; }
#define RETURN_FALSE(state)		{ (state)->found_hit |= eiFALSE; return; }

/* forward declarations */
typedef struct eiTLS						eiTLS;
typedef struct eiData						eiData;
typedef struct eiDatabase					eiDatabase;
typedef struct eiGlobals					eiGlobals;
typedef struct eiBSPNode					eiBSPNode;
typedef struct eiState						eiState;
typedef struct eiRayAccelTriangle			eiRayAccelTriangle;
typedef struct eiRayAccelMotionTriangle		eiRayAccelMotionTriangle;
typedef struct eiRayTracer					eiRayTracer;
typedef struct eiBSPTree					eiBSPTree;
typedef struct eiRaySubscene				eiRaySubscene;
typedef struct eiRayScene					eiRayScene;
typedef struct eiRayTessel					eiRayTessel;
typedef struct eiRayTesselInstance			eiRayTesselInstance;

/** \brief The thread local storage of ray-tracer. 
 * for internal use only. */
typedef struct eiRayTLS {
	ei_array		hit_info_array;
	eiBSPStack		bsp_stack;
	eiBSPStack		sub_bsp_stack;
	eiBSPStack		hair_bsp_stack;
	/* scene statistics */
	eiInt			num_tessellated_primitives;
	/* ray statistics */
	eiInt			num_rays[ eiRAY_TYPE_COUNT ];
	/* bsp-tree statistics */
	eiInt			average_bsp_size;
	eiInt			average_bsp_depth;
	eiInt			max_bsp_size;
	eiInt			max_bsp_depth;
	eiInt			num_bsp_averages;
	eiInt			num_bsp_leaves;
	eiInt			num_bsp_empty_leaves;
	eiInt			num_bsp_nodes;
	eiInt			num_bsp_bad_splits;
	eiInt			bsp_allocations;
	eiInt			bsp_temp_allocations;
	eiInt			bsp_temp_huge_allocations;
	eiInt			gpit_memory_used;
	eiInt			average_subtree_size;
	eiInt			max_subtree_size;
	eiInt			num_subtree_averages;
	eiInt			bsp_construction_time;
	/* bsp packed indices statistics */
	eiInt			bsp_indices;
	eiInt			bsp_byte_indices;
	eiInt			bsp_ushort_indices;
	eiInt			bsp_uint_indices;
	eiInt			bsp_subleaf_desc;
	eiInt			bsp_primlist_desc;
} eiRayTLS;

/** \brief Initialize thread local storage. for internal use only. */
eiAPI void ei_ray_tls_init(eiRayTLS *pTls, const eiInt bsp_depth);

/** \brief Cleanup thread local storage. for internal use only. */
eiAPI void ei_ray_tls_exit(eiRayTLS *pTls);

/** \brief hold hit info of intersections for 
 * sorting them by distance. */
typedef struct eiRayHitInfo {
	eiTag			hit_bsp;			/*  4 */
	eiIndex			hit_tessel_inst;	/*  4 */
	eiIndex			hit_tri;			/*  4 */
	eiIndex			hit_prim;			/*  4 */
	eiBool			hit_motion;			/*  4 */
	eiScalar		hit_t;				/*  4 */
	eiVector		bary;				/* 12 */
	eiScalar		bias;				/*  4 */
	eiScalar		bias_scale;			/*  4 */
	eiScalar		user_data[ EI_MAX_USER_DATA_SIZE ];
} eiRayHitInfo;

eiAPI void ei_ray_hit_info_init(eiRayHitInfo *hit_info);

/** \brief The ray-tracing global options. 
 * this data should be provided by users. */
typedef struct eiRayOptions {
	/* the acceleration mode */
	eiInt			acceleration;
	/* the maximum BSP leaf size */
	eiInt			bsp_size;
	/* the maximum BSP depth */
	eiInt			bsp_depth;
} eiRayOptions;

/** \brief The ray-tracing camera information. */
typedef struct eiRayCamera {
	eiBool			camera_to_world_moving;	/* for internal use only */
	eiMatrix		camera_to_world;		/* provided by users */
	eiMatrix		motion_camera_to_world;	/* provided by users */
	eiMatrix		world_to_camera;		/* for internal use only */
	eiMatrix		motion_world_to_camera;	/* for internal use only */
} eiRayCamera;

/** \brief The vertex for tessellated ray-traceable 
 * triangle, contains parametric coordinates for 
 * interpolating vertex attributes. this data should 
 * be provided by users, typically the geometry 
 * approximation module. */
typedef struct eiRayVertex {
	/* position */
	eiVector			pos;
	/* moving position */
	eiVector			m_pos;
} eiRayVertex;

/** \brief The tessellated ray-traceable triangle. */
typedef struct eiRayTriangle {
	/* the indices of the vertices that form this 
	   triangle, provided by users */
	eiIndex			v1;
	eiIndex			v2;
	eiIndex			v3;
	/* source primitive index */
	eiIndex			prim_index;
	/* for internal use only */
	eiByte			deform_moving;
} eiRayTriangle;

/** \brief Begin describing a tessellation, pre-allocate 
 * memory for vertex list and triangle list. this function 
 * will create a new tessellation in database. */
eiAPI eiRayTessel *ei_rt_tessel(
	eiRayTracer *rt, 
	eiTag * const tag, 
	const eiUint num_vertices, 
	const eiUint num_triangles, 
	const eiUint num_uniform_channels, 
	const eiUint num_vertex_channels);

/** \brief Add a new vertex to a tessellation. */
eiAPI void ei_rt_tessel_add_vertex(
	eiRayTessel *tessel, 
	const eiRayVertex *vtx);

/** \brief Add a new vertex attribute data to a specific vertex channel. */
eiAPI void ei_rt_tessel_add_vertex_data(
	eiRayTessel *tessel, 
	const eiUint index, 
	const void *data);

/** \brief Get the number of vertices in this tessellation. */
eiAPI eiUint ei_rt_tessel_get_num_vertices(eiRayTessel *tessel);

/** \brief Get an existing vertex by index from a tessellation. */
eiRayVertex *ei_rt_tessel_get_vertex(
	eiRayTessel *tessel, 
	const eiIndex index);

/** \brief Get the data table of a vertex channel by index. */
eiAPI eiScalar *ei_rt_tessel_get_vertex_channel(
	eiRayTessel *tessel, 
	const eiIndex index);

/** \brief Add a new triangle to a tessellation. */
eiAPI void ei_rt_tessel_add_triangle(
	eiRayTessel *tessel, 
	const eiRayTriangle *tri);

/** \brief Add a new uniform attribute data to a specific uniform channel. */
eiAPI void ei_rt_tessel_add_uniform_data(
	eiRayTessel *tessel, 
	const eiUint index, 
	const void *data);

/** \brief Get the number of triangles in this tessellation. */
eiAPI eiUint ei_rt_tessel_get_num_triangles(eiRayTessel *tessel);

/** \brief Get an existing triangle by index from a tessellation. */
eiRayTriangle *ei_rt_tessel_get_triangle(
	eiRayTessel *tessel, 
	const eiIndex index);

/** \brief Get the data table of an uniform channel by index. */
eiAPI eiScalar *ei_rt_tessel_get_uniform_channel(
	eiRayTessel *tessel, 
	const eiIndex index);

/** \brief End describing a tessellation. This function 
 * finishes the creation of the new tessellation. */
eiAPI void ei_rt_end_tessel(
	eiRayTracer *rt, 
	eiRayTessel *tessel, 
	const eiTag tag);

/** \brief Specify bounding box for deferred/procedural tessellation. */
eiAPI void ei_rt_tessel_box(
	eiRayTessel *tessel, 
	const eiBound *box);

/** \brief Begin describing a deferred tessellation. */
eiAPI eiRayTessel *ei_rt_defer_tessel(
	eiRayTracer *rt, 
	eiTag * const tag);

/** \brief When used for a deferred tessellation, it 
 * specifies the node which has a dice function to 
 * tessellate the high-level geometry data stored in this 
 * tesssellation into micro-triangles.
 * when used for a procedural tessellation, it specifies 
 * a list of geometry shaders which is used to create a 
 * sub-scene. */
eiAPI void ei_rt_tessel_job(
	eiRayTessel *tessel, 
	const eiTag job);

/** \brief End describing procedural tessellation. */
eiAPI void ei_rt_end_defer_tessel(
	eiRayTracer *rt, 
	eiRayTessel *tessel, 
	const eiTag tag);

/** \brief Resize a tessellation for vertex list and triangle list. */
eiAPI eiRayTessel *ei_rt_tessel_resize(
	eiRayTracer *rt, 
	const eiTag tag, 
	const eiUint num_vertices, 
	const eiUint num_triangles, 
	const eiUint num_uniform_channels, 
	const eiUint num_vertex_channels);

/** \brief Begin describing a procedural tessellation. */
eiAPI eiRayTessel *ei_rt_proc_tessel(
	eiRayTracer *rt, 
	eiTag * const tag);

/** \brief End describing procedural tessellation. */
eiAPI void ei_rt_end_proc_tessel(
	eiRayTracer *rt, 
	eiRayTessel *tessel, 
	const eiTag tag);

/** \brief An instance of a ray-traceable tessellation. */
struct eiRayTesselInstance {
	/* the tag of an eiRayTessel */
	eiTag			tessel;
	/* the tag of an eiRayObjectInstance */
	eiTag			object_instance;
};

/** \brief The tessellated ray-traceable object 
 * which consists of ray-traceable tessellations. */
typedef struct eiRayObject {
	/* the source object */
	eiTag			source;
	/* a data array of tags, each tag points to an eiRayTessel. */
	eiTag			tessels;
} eiRayObject;

/** \brief Initialize a ray-traceable object. */
eiAPI void ei_ray_object_init(
	eiRayObject *object, 
	eiDatabase *db, 
	const eiTag source);
/** \brief Cleanup a ray-traceable object. */
eiAPI void ei_ray_object_exit(
	eiRayTracer *rt, 
	eiRayObject *object);

/** \brief An instance of a ray-traceable object. */
typedef struct eiRayObjectInstance {
	/* the tag of an eiRayObject */
	eiTag			object;
	/* the merged instance attributes */
	eiAttributes	attr;
	eiBool			object_to_world_moving;
	eiMatrix		object_to_world;
	eiMatrix		world_to_object;
	eiMatrix		motion_object_to_world;
	eiMatrix		motion_world_to_object;
	/* motion_world_to_object * object_to_world 
	   this is used to transform ray for intersecting 
	   with moving objects in world space. */
	eiMatrix		world_to_motion;
} eiRayObjectInstance;

/** \brief Initialize a ray-traceable object instance. */
eiAPI void ei_ray_object_instance_init(
	eiRayObjectInstance *instance, 
	const eiTag object, 
	const eiAttributes *attr, 
	const eiMatrix *object_to_world, 
	const eiMatrix *motion_object_to_world);
/** \brief Cleanup a ray-traceable object instance. */
eiAPI void ei_ray_object_instance_exit(eiRayObjectInstance *instance);

/** \brief The callback for interpolating primitive variables */
typedef eiBool (*ei_get_prim_var_cb)(
	eiState * const state, 
	const char *name, 
	const eiInt type, 
	eiByte * const x, 
	eiByte * const dx1, 
	eiByte * const dx2);
/** \brief The callback for performing intersection tests with 
 * procedural objects */
typedef void (*ei_procedural_intersect_cb)(
	eiRayObjectInstance *ray_obj_inst, 
	const eiTag tessel_tag, 
	const eiIndex tessel_instance_index, 
	const eiIndex parent_bsptree, 
	eiState *state, 
	ei_array *hit_info_array, 
	const eiBool sort_by_distance);

/** \brief The standard interface class of ray-tracing engine. */
struct eiRayTracer {
	/* the local database on which we manipulate */
	eiDatabase					*db;
	/* the in-memory scene data, remains valid between 
	   ei_rt_scene and ei_rt_end_scene */
	eiRayScene					*scene;
	/* the tag of an eiRayScene */
	eiTag						scene_tag;
	/* the progress display		callback for building BSP */
	ei_bsp_build_progress		build_bsp_progress;
	/* the callback for interpolating primitive variables */
	ei_get_prim_var_cb			get_prim_var;
	/* the callback for intersecting with procedurals */
	ei_procedural_intersect_cb	procedural_intersect;
};

/** \brief The procedure to be called when hitting an object.
 * returning eiTRUE means we can exit the traversal immediately, 
 * e.g., early exit; returnning eiFALSE means we should 
 * continue tracing. */
typedef eiBool (*eiHitProc)(eiState *state);

/** \brief Construct the ray-tracing engine. */
eiAPI void ei_rt_init(eiRayTracer *rt);

/** \brief Destruct the ray-tracing engine. */
eiAPI void ei_rt_exit(eiRayTracer *rt);

/** \brief Begin describing/editing the scene. */
eiAPI void ei_rt_scene(
	eiRayTracer *rt, 
	eiDatabase *db);

/** \brief Begin describing/editing the ray-tracing options. */
eiAPI eiRayOptions *ei_rt_options(eiRayTracer *rt);

/** \brief End describing/editing the ray-tracing options. */
eiAPI void ei_rt_end_options(eiRayTracer *rt);

/** \brief Begin describing/editing the ray-tracing camera. */
eiAPI eiRayCamera *ei_rt_camera(eiRayTracer *rt);

/** \brief End describing/editing the ray-tracing camera. */
eiAPI void ei_rt_end_camera(eiRayTracer *rt);

/** \brief Get the scene root. */
eiAPI eiTag ei_rt_scene_root(eiRayTracer *rt);

/** \brief Add an existing tessellation to an object. this function 
 * is thread-safe, so each geometry approximation thread can build 
 * tessellation on its own, and add the tessellation to the scene 
 * concurrently.
 * @param object The tag of the ray-traceable object.
 * @param tessel The tag of the ray-traceable tessellation 
 * to be added. */
eiAPI void ei_rt_add_tessel(
	eiRayTracer *rt, 
	const eiTag object, 
	const eiTag tessel);

/** \brief Clear all tessellations attached to an object, this 
 * is called when the object is changed, and re-tessellation is 
 * needed for the object.
 * @param object The tag of the ray-traceable object. */
eiAPI void ei_rt_clear_tessels(
	eiRayTracer *rt, 
	const eiTag object);

/** \brief Set the ray-traceable object instances for a specfic sub-scene.
 * @param scene_tag The specific sub-scene which we will add the object 
 * instances into. 
 * @param object_instances The data array of object instances. */
eiAPI void ei_rt_set_instances(
	eiRayTracer *rt, 
	const eiTag scene_tag, 
	const eiTag object_instances);

/** \brief Ask all objects instances to unreference all objects for a 
 * specific sub-scene.
 * @param scene_tag The specific sub-scene to remove object references. */
eiAPI void ei_rt_instances_unref_objects(
	eiRayTracer *rt, 
	const eiTag scene_tag);

/** \brief Remove all ray-traceable object instances from a specific sub-scene.
 * @param scene_tag The specific sub-scene which we will remove the object 
 * instances from. */
eiAPI void ei_rt_remove_instances(
	eiRayTracer *rt, 
	const eiTag scene_tag);

/** \brief Begin ray-tracing mode, do pre-processing. */
eiAPI void ei_rt_tracing(eiRayTracer *rt);

/** \brief Get the bounding box of the entire scene in world space. */
eiAPI void ei_rt_scene_box(eiRayTracer *rt, eiBound * const box);
/** \brief Get the length of scene diagonal in world space. */
eiAPI eiScalar ei_rt_scene_diag(eiRayTracer *rt);

/** \brief Trace a generic ray. Returns whether there are 
 * any intersections satisfied hit_proc.
 * @param state The shading state for this ray sample.
 * @param hit_proc The callback when an intersection was found.
 * @param sort_by_distance Whether to sort intersections from 
 * near to far by distance. this is usually used when tracing 
 * a shadow ray with transparency support. */
eiAPI eiBool ei_rt_trace(
	eiRayTracer *rt, 
	eiState *state, 
	eiHitProc hit_proc, 
	const eiBool sort_by_distance);

/** \brief Compute details including differential geometry for 
 * an intersection with a specific sub-scene. */
eiAPI void ei_rt_compute_hit_details(
	eiRayTracer *rt, 
	eiState *state);

/** \brief End ray-tracing mode, do post-processing. */
eiAPI void ei_rt_end_tracing(eiRayTracer *rt);

/** \brief End describing/editing the scene. */
eiAPI void ei_rt_end_scene(eiRayTracer *rt);

typedef struct eiRayGPIT {
	ei_slist_node		node;
	void				*m_data;
	eiUint				m_size;
} eiRayGPIT;

typedef struct eiRayGPITList {
	ei_slist			m_list;
	eiUint				m_list_size;
} eiRayGPITList;

eiFORCEINLINE void ei_ray_gpit_init(
	eiRayGPIT *gpit, 
	void *data, 
	const eiUint size)
{
	gpit->m_data = data;
	gpit->m_size = size;
	/* initialize the next to NULL is 
	   required by ei_slist. */
	gpit->node.next = NULL;
}

eiFORCEINLINE void ei_ray_gpit_delete_node(ei_slist_node *node)
{
	eiCHECK_FREE(node);
}

eiFORCEINLINE void ei_ray_gpit_list_init(eiRayGPITList *list)
{
	ei_slist_init(&list->m_list, ei_ray_gpit_delete_node);
	list->m_list_size = 0;
}

eiFORCEINLINE void ei_ray_gpit_list_exit(eiRayGPITList *list)
{
	ei_slist_clear(&list->m_list);
}

/** \bried Generate tessellation, for internal use only. */
void generate_ray_tessel(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls);

/** \brief Generate accelerated triangles, for internal use only. */
void generate_ray_accel_triangles(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls);

/** \brief Generate sub-tree, for internal use only. */
void generate_ray_subtree(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls);

/* for internal use only */
void byteswap_bsptree(eiDatabase *db, void *data, const eiUint size);
void byteswap_ray_subscene(eiDatabase *db, void *data, const eiUint size);
void byteswap_ray_scene(eiDatabase *db, void *data, const eiUint size);
void byteswap_ray_object(eiDatabase *db, void *data, const eiUint size);
void byteswap_ray_object_inst(eiDatabase *db, void *data, const eiUint size);
void byteswap_ray_tessel(eiDatabase *db, void *data, const eiUint size);
void byteswap_ray_accel_triangles(eiDatabase *db, void *data, const eiUint size);
void byteswap_ray_subtree(eiDatabase *db, void *data, const eiUint size);
void byteswap_ray_tessel_inst(eiDatabase *db, void *data, const eiUint size);

#ifdef __cplusplus
}
#endif

#endif
