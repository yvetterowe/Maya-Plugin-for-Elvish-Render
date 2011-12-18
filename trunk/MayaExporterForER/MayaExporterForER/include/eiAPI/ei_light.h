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
 
#ifndef EI_LIGHT_H
#define EI_LIGHT_H

/** \brief The light representation
 * \file ei_light.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_element.h>
#include <eiAPI/ei_base_bucket.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The class encapsulates the information of a light. */
#pragma pack(push, 1)
typedef struct eiLight {
	eiNode			node;
	eiTag			light_list;
	eiTag			emitter_list;
	eiVector		origin;
	eiVector		energy;
	eiInt			u_samples;
	eiInt			v_samples;
	eiInt			low_level;
	eiInt			low_u_samples;
	eiInt			low_v_samples;
} eiLight;
#pragma pack(pop)

void ei_light_init(eiNodeSystem *nodesys, eiNode *node);
void ei_light_exit(eiNodeSystem *nodesys, eiNode *node);

/** \brief Instance this element into global scene database or update 
 * the existing element */
void ei_light_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer);

void ei_light_add_light(eiLight *lgt, eiDatabase *db, const eiTag shader);
void ei_light_add_emitter(eiLight *lgt, eiDatabase *db, const eiTag shader);

/** \brief The light instance. */
typedef struct eiLightInstance {
	/* referenced light element */
	eiTag			light;
	/* transformation from light space to world space */
	eiMatrix		light_to_world;
	/* motion transformation from light space to world space */
	eiMatrix		motion_light_to_world;
	/* transformation from world space to light space */
	eiMatrix		world_to_light;
	/* the light origin in camera space */
	eiVector		origin;
} eiLightInstance;

void ei_light_instance_init(
	eiLightInstance *inst, 
	const eiTag ref_light, 
	const eiMatrix *light_to_world, 
	const eiMatrix *motion_light_to_world, 
	const eiVector *origin);
void ei_light_instance_exit(eiLightInstance *inst);

/** \brief Append transformations to the light instance. */
void ei_light_instance_transform(
	eiLightInstance *inst, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform);
eiScalar ei_light_instance_get_max_flux(
	eiLightInstance *inst, 
	eiDatabase *db);
void ei_light_instance_shoot_photon(
	eiLightInstance *light_inst, 
	const eiInt photon_type, 
	eiInt *halton_num, 
	eiBaseBucket *bucket);

/* for internal use only */
void byteswap_light_instance(eiDatabase *db, void *data, const eiUint size);

eiNodeObject *ei_create_light_node_object(void *param);
/** \brief Install the node into node system */
void ei_install_light_node(eiNodeSystem *nodesys);

#ifdef __cplusplus
}
#endif

#endif
