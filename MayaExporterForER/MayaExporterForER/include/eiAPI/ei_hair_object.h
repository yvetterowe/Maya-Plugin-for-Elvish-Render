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
 
#ifndef EI_HAIR_OBJECT_H
#define EI_HAIR_OBJECT_H

/** \brief The hair object representation
 * \file ei_hair_object.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_object.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The hair object. */
#pragma pack(push, 1)
typedef struct eiHairObject {
	eiObject		base;
	eiInt           degree;
	eiTag			vertex_list;
	eiTag			motion_vertex_list;
	eiTag			hair_list;
	eiInt			bsp_size;
	eiInt			bsp_depth;
	eiTag			bsptree;
} eiHairObject;
#pragma pack(pop)

void ei_hair_object_init(eiNodeSystem *nodesys, eiNode *node);
void ei_hair_object_exit(eiNodeSystem *nodesys, eiNode *node);

/** \brief Create a tessellable sub-object from this source object. */
eiTag ei_hair_object_create(
	eiDatabase *db, 
	eiObject *src_obj, 
	eiTesselJob *job);
/** \brief Delete a tessellable sub-object for a source object. */
void ei_hair_object_delete(
	eiDatabase *db, 
	eiTesselJob *job);
/** \brief Bound the sub-object in object space including motion blur(no displacement). */
void ei_hair_object_bound(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	eiBound *box);
/** \brief Test if the sub-object is small enough to be diced. */
eiBool ei_hair_object_diceable(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box);
/** \brief Dice the sub-object into micro-triangle grids, return the tag of 
 * the created tessellation */
eiTag ei_hair_object_dice(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box);
/** \brief Deferred dice the sub-object into micro-triangle grids when the 
 * bounding box of the sub-object is hit by a ray for the first time. */
void ei_hair_object_deferred_dice(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box, 
	const eiTag deferred_tessel_tag);
/** \brief Split the sub-object into smaller sub-objects, add back generated 
 * sub-objects into job queue. */
void ei_hair_object_split(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box, 
	eiJobQueue *queue);
/** \brief Interpolate varying primitive variables. */
eiBool ei_hair_object_interp_varying(
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
/** \brief Interpolate vertex primitive variables. */
eiBool ei_hair_object_interp_vertex(
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

eiNodeObject *ei_create_hair_object_node_object(void *param);
/** \brief Install the node into node system */
void ei_install_hair_object_node(eiNodeSystem *nodesys);

#ifdef __cplusplus
}
#endif

#endif
