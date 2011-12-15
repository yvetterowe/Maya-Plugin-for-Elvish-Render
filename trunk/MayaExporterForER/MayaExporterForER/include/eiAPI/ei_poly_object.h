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
 
#ifndef EI_POLY_OBJECT_H
#define EI_POLY_OBJECT_H

/** \brief The polygon object representation
 * \file ei_poly_object.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_object.h>
#include <eiCORE/ei_data_table.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The polygon object. */
#pragma pack(push, 1)
typedef struct eiPolyObject {
	eiObject			base;
	eiTag				pos_list;
	eiTag				motion_pos_list;
	eiTag				triangle_list;
} eiPolyObject;
#pragma pack(pop)

void ei_poly_object_init(eiNodeSystem *nodesys, eiNode *node);
void ei_poly_object_exit(eiNodeSystem *nodesys, eiNode *node);

/** \brief Create a tessellable sub-object from this source object. */
eiTag ei_poly_object_create(
	eiDatabase *db, 
	eiObject *src_obj, 
	eiTesselJob *job);
/** \brief Delete a tessellable sub-object for a source object. */
void ei_poly_object_delete(
	eiDatabase *db, 
	eiTesselJob *job);
/** \brief Bound the sub-object in object space including motion blur(no displacement). */
void ei_poly_object_bound(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	eiBound *box);
/** \brief Test if the sub-object is small enough to be diced. */
eiBool ei_poly_object_diceable(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box);
/** \brief Dice the sub-object into micro-triangle grids, return the tag of 
 * the created tessellation */
eiTag ei_poly_object_dice(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box);
/** \brief Deferred dice the sub-object into micro-triangle grids when the 
 * bounding box of the sub-object is hit by a ray for the first time. */
void ei_poly_object_deferred_dice(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box, 
	const eiTag deferred_tessel_tag);
/** \brief Split the sub-object into smaller sub-objects, add back generated 
 * sub-objects into job queue. */
void ei_poly_object_split(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box, 
	eiJobQueue *queue);
/** \brief Perform intersection test directly with this object. in nearest 
 * mode, the intersection test should return the nearest intersection between 
 * t_near and t_far; in sort mode, it should return all intersections between 
 * t_near and t_far. */
void ei_poly_object_intersect(
	eiObject *obj, 
	const eiTag tessel_tag, 
	const eiIndex tessel_instance_index, 
	const eiIndex parent_bsptree, 
	eiState *state, 
	ei_array *hit_info_array, 
	const eiBool sort_by_distance);
/** \brief Interpolate varying or vertex primitive variables. */
eiBool ei_poly_object_interp_varying(
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

eiNodeObject *ei_create_poly_object_node_object(void *param);
/** \brief Install the node into node system */
void ei_install_poly_object_node(eiNodeSystem *nodesys);

/** \brief The internal tessellable representation(sub-object) of 
 * polygon object. */
typedef struct eiPolyTessel {
	/* the source object node */
	eiTag		object;
	eiTag		pos_list;
	eiTag		motion_pos_list;
	eiInt		varying_dim;
	eiTag		varying_list;
	eiInt		vertex_dim;
	eiTag		vertex_list;
	eiTag		triangle_list;
} eiPolyTessel;

/* exported unit testing functions for internal use only */
eiAPI void connect_two_edges_exported(
	ei_array *triangles, 
	const eiIndex prim_index, 
	eiIndex *e1, const eiInt e1_size, 
	eiIndex *e2, const eiInt e2_size, 
	eiDatabase *db, 
	eiPolyTessel *poly);

eiAPI void stitch_two_edges_exported(
	ei_array *vertices, 
	ei_array *triangles, 
	const eiIndex prim_index, 
	eiIndex *e1, const eiInt e1_size, 
	eiIndex *e2, const eiInt e2_size, 
	eiDatabase *db, 
	eiPolyTessel *poly);

eiAPI void build_triangle_grid_exported(
	ei_array *vertices, 
	ei_array *vertex_data, 
	eiDataTableIterator *varying_list_iter, 
	eiDataTableIterator *vertex_list_iter, 
	ei_array *triangles, 
	const eiIndex prim_index, 
	const eiIndex v1, const eiIndex v2, const eiIndex v3, 
	const eiVector *pos1, const eiVector *pos2, const eiVector *pos3, 
	const eiVector *motion_pos1, const eiVector *motion_pos2, const eiVector *motion_pos3, 
	const eiInt num_edges, 
	const eiIndex vertex_offset, 
	eiDatabase *db, 
	eiPolyTessel *poly);

#ifdef __cplusplus
}
#endif

#endif
