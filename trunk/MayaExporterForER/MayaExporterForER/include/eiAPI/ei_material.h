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
 
#ifndef EI_MATERIAL_H
#define EI_MATERIAL_H

/** \brief The material representation
 * \file ei_material.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_element.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The class encapsulates the information of a material. */
#pragma pack(push, 1)
typedef struct eiMaterial {
	eiNode			node;
	eiTag			surface_list;
	eiTag			displace_list;
	eiTag			shadow_list;
	eiTag			volume_list;
	eiTag			env_list;
	eiTag			photon_list;
} eiMaterial;
#pragma pack(pop)

void ei_material_init(eiNodeSystem *nodesys, eiNode *node);
void ei_material_exit(eiNodeSystem *nodesys, eiNode *node);

/** \brief Instance this element into global scene database or update 
 * the existing element */
void ei_material_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer);

void ei_material_add_surface(eiMaterial *mtl, eiDatabase *db, const eiTag shader);
void ei_material_add_displace(eiMaterial *mtl, eiDatabase *db, const eiTag shader);
void ei_material_add_shadow(eiMaterial *mtl, eiDatabase *db, const eiTag shader);
void ei_material_add_volume(eiMaterial *mtl, eiDatabase *db, const eiTag shader);
void ei_material_add_environment(eiMaterial *mtl, eiDatabase *db, const eiTag shader);
void ei_material_add_photon(eiMaterial *mtl, eiDatabase *db, const eiTag shader);

eiNodeObject *ei_create_material_node_object(void *param);
/** \brief Install the node into node system */
void ei_install_material_node(eiNodeSystem *nodesys);

#ifdef __cplusplus
}
#endif

#endif
