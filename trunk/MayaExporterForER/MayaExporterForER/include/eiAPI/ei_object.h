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
 
#ifndef EI_OBJECT_H
#define EI_OBJECT_H

/** \brief The common object representation
 * \file ei_object.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_element.h>
#include <eiAPI/ei_raytracer.h>
#include <eiCORE/ei_bound.h>
#include <eiCORE/ei_btree.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EI_MAX_USER_CHANNEL_DIM		16

/** \brief The tessellation job stored in database */
typedef struct eiTesselJob {
	/* current camera for view-dependent tessellation */
	eiTag			cam;
	/* the tag of the renderable object instance which 
	   instances this object representation. when it's 
	   eiNULL_TAG, this is an object space representation 
	   (view-independent) which is instanced by multiple 
	   renderable object instances that shares the identical 
	   approximation parameters; when it's not eiNULL_TAG, 
	   this is a world space representation(view-dependent) 
	   which depends on the world transformation of the 
	   renderable object instance, it's very unlikely to be 
	   shared by multiple renderable object instances unless 
	   their world transformations are identical, which 
	   should be practically useless. */
	eiTag			inst;
	/* the approximation statement which is used to generate 
	   this representation */
	eiTag			approx;
	/* is motion blur enabled */
	eiBool			motion;
	/* displacement shader list */
	eiTag			displace_list;
	/* the node description of source object */
	eiTag			object_desc;
	/* the tessellable sub-object for geometric approximation */
	eiTag			tessellable;
	/* the ray-traceable object representation */
	eiTag			raytraceable;
	/* the current subdivision level */
	eiUint			subdiv;
	/* whether this job is a deferred dicing job */
	eiBool			deferred_dice;
} eiTesselJob;

/** \brief The node for mapping object representation key to 
 * object representation, keep it small for memory efficiency */
typedef struct eiObjectRepNode {
	ei_btree_node	node;
	/* the tag of the renderable object instance which 
	   instances this object representation. when it's 
	   eiNULL_TAG, this is an object space representation 
	   (view-independent) which is instanced by multiple 
	   renderable object instances that shares the identical 
	   approximation parameters; when it's not eiNULL_TAG, 
	   this is a world space representation(view-dependent) 
	   which depends on the world transformation of the 
	   renderable object instance, it's very unlikely to be 
	   shared by multiple renderable object instances unless 
	   their world transformations are identical, which 
	   should be practically useless. */
	eiTag			inst;
	/* the approximation statement which is used to generate 
	   this representation */
	eiTag			approx;
	/* is motion blur enabled */
	eiBool			motion;
	/* displacement shader list */
	eiTag			displace_list;
	/* the time-stamp of the last evaluation of the object */
	eiUint			time;
	/* when this representation is view-dependent, 
	   the time-stamp of view should also be included */
	eiUint			view_time;
	/* the object representation, as the value */
	eiTag			rep;
} eiObjectRepNode;

/** \brief The mapping from instance and approximation setting 
 * to renderable object representation. */
typedef struct eiObjectRepMap {
	ei_btree			map;
} eiObjectRepMap;

void ei_object_rep_map_init(eiObjectRepMap *map);
void ei_object_rep_map_exit(eiObjectRepMap *map);

void ei_object_rep_map_add(
	eiObjectRepMap *map, 
	const eiTag inst, 
	const eiTag approx, 
	const eiBool motion, 
	const eiTag displace_list, 
	const eiUint time, 
	const eiUint view_time, 
	const eiTag rep, 
	eiDatabase *db);
eiTag ei_object_rep_map_find(
	eiObjectRepMap *map, 
	const eiTag inst, 
	const eiTag approx, 
	const eiBool motion, 
	const eiTag displace_list, 
	const eiUint time, 
	const eiUint view_time, 
	eiDatabase *db);

void ei_object_rep_cache_init(
	eiObjectRepCache *cache, 
	eiDatabase *db, 
	const eiTag cam_tag, 
	const eiTag light_insts);
void ei_object_rep_cache_exit(eiObjectRepCache *cache, eiDatabase *db);

void ei_object_rep_cache_add_object(eiObjectRepCache *cache, const eiTag tag);
void ei_object_rep_cache_clear_unref_object_reps(eiObjectRepCache *cache, eiDatabase *db);

/** \brief The class encapsulates the information of a 
 * geometric object. */
#pragma pack(push, 1)
typedef struct eiObject {
	eiNode				node;
	/* the box enclosing all vertices */
	eiBound				box;
	/* the box enclosing all vertices and motion vertices */
	eiBound				motion_box;
	/* renderable object representations for different 
	   instances and approximation settings */
	eiObjectRepMap		object_reps;
} eiObject;
#pragma pack(pop)

void ei_object_init(eiNodeSystem *nodesys, eiNode *node);
void ei_object_exit(eiNodeSystem *nodesys, eiNode *node);

/** \brief Interpolate varying primitive variable, returns true if 
 * the primitive varible exists, and was interpolated successfully, 
 * otherwise returns false. */
eiBool ei_get_prim_var(
	eiState * const state, 
	const char *name, 
	const eiInt type, 
	eiByte * const x, 
	eiByte * const dx1, 
	eiByte * const dx2);

/** \brief Instance this element into global scene database or update 
 * the existing element */
void ei_object_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer);

/** \brief Create a tessellable sub-object from this source object. */
typedef eiTag (*ei_object_element_create)(
	eiDatabase *db, 
	eiObject *src_obj, 
	eiTesselJob *job);
/** \brief Delete a tessellable sub-object for a source object. */
typedef void (*ei_object_element_delete)(
	eiDatabase *db, 
	eiTesselJob *job);
/** \brief Bound the sub-object in object space including motion blur(no displacement). */
typedef void (*ei_object_element_bound)(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	eiBound *box);
/** \brief Test if the sub-object is small enough to be diced. */
typedef eiBool (*ei_object_element_diceable)(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box);
/** \brief Dice the sub-object into micro-triangle grids, return the tag of 
 * the created tessellation */
typedef eiTag (*ei_object_element_dice)(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box);
/** \brief Deferred dice the sub-object into micro-triangle grids when the 
 * bounding box of the sub-object is hit by a ray for the first time. */
typedef void (*ei_object_element_deferred_dice)(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box, 
	const eiTag deferred_tessel_tag);
/** \brief Split the sub-object into smaller sub-objects, add back generated 
 * sub-objects into job queue. */
typedef void (*ei_object_element_split)(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box, 
	eiJobQueue *queue);
/** \brief Perform intersection test directly with this object. in nearest 
 * mode, the intersection test should return the nearest intersection between 
 * t_near and t_far; in sort mode, it should return all intersections between 
 * t_near and t_far. */
typedef void (*ei_object_element_intersect)(
	eiObject *obj, 
	const eiTag tessel_tag, 
	const eiIndex tessel_instance_index, 
	const eiIndex parent_bsptree, 
	eiState *state, 
	ei_array *hit_info_array, 
	const eiBool sort_by_distance);
/** \brief Interpolate varying or vertex primitive variables. 
 * the node parameter pointer can be NULL when interpolating 
 * system built-in parameters, the implementation should check 
 * parameter name in this case. */
typedef eiBool (*ei_object_element_interp)(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	eiNodeParam *param, 
	const eiTag tessel_tag, 
	const eiIndex tri_index, 
	const eiIndex prim_index, 
	const eiVector *bary, 
	const eiScalar *user_data, 
	const char *name, 
	eiScalar * const x, 
	eiScalar * const dx1, 
	eiScalar * const dx2);

/** \brief The object element which defines common object methods */
typedef struct eiObjectElement {
	eiElement								base;
	ei_object_element_create				create_obj;
	ei_object_element_delete				delete_obj;
	ei_object_element_bound					bound;
	ei_object_element_diceable				diceable;
	ei_object_element_dice					dice;
	ei_object_element_deferred_dice			deferred_dice;
	ei_object_element_split					split;
	ei_object_element_intersect				intersect;
	ei_object_element_interp				interp_varying;
	ei_object_element_interp				interp_vertex;
} eiObjectElement;

/** \brief Create node object of element type. */
eiObjectElement *ei_create_object_element();
/** \brief The callback to delete node object of element type. */
void ei_object_element_deletethis(eiPluginObject *object);

/** \brief The internal representation of user-defined data. */
typedef struct eiUserData {
	/* the index of the node parameter */
	eiIndex			param_index;
	/* the data table tag of this vertex data */
	eiTag			tag;
	/* the data type of the table */
	eiInt			type;
	/* the offset in scalar channel */
	eiUint			channel_offset;
	/* the dimension in scalar channel */
	eiUint			channel_dim;
} eiUserData;

/** \brief The dimension of this type after converting to scalars, 
 * returning 0 means this data cannot be converted to scalars. */
eiAPI eiUint ei_type_dim(const eiInt type);
/** \brief Convert a data to scalars by type. */
eiAPI void ei_type_to_scalars(
	const eiInt type, 
	eiScalar *sval, 
	void *data);

void append_user_data_array(
	eiDatabase *db, 
	const eiTag tab_tag, 
	ei_array *user_data_array, 
	const eiInt num_elements);
void ei_user_data_array_init(
	ei_array *varyings, 
	ei_array *vertices, 
	eiUint * const varying_dim, 
	eiUint * const vertex_dim, 
	eiNodeSystem *nodesys, 
	eiNode *node);
void ei_user_data_array_exit(
	ei_array *varyings, 
	ei_array *vertices);

/** \brief Call this function to apply displacement 
 * to tessellations. */
eiAPI void ei_displace_tessel(
	eiDatabase *db, 
	const eiTag displace_list, 
	const eiTag tessel_tag, 
	const eiTag obj_tag, 
	const eiBool motion);

/* for internal use only */
void byteswap_job_tessel(eiDatabase *db, void *data, const eiUint size);
eiBool execute_job_tessel(eiDatabase *db, eiBaseWorker *pWorker, void *job, void *param);
eiUint count_job_tessel(eiDatabase *db, void *job);

/** \brief Perform intersection tests with procedural objects. */
void ei_procedural_intersect(
	eiRayObjectInstance *ray_obj_inst, 
	const eiTag tessel_tag, 
	const eiIndex tessel_instance_index, 
	const eiIndex parent_bsptree, 
	eiState *state, 
	ei_array *hit_info_array, 
	const eiBool sort_by_distance);

/** \brief Install the node into node system */
void ei_install_object_node(eiNodeSystem *nodesys, eiNodeDesc *desc);

#ifdef __cplusplus
}
#endif

#endif
