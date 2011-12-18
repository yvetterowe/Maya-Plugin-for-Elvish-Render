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
 
#ifndef EI_INSTANCE_H
#define EI_INSTANCE_H

/** \brief The instance for instancing scene element.
 * \file ei_instance.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_element.h>
#include <eiAPI/ei_attributes.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Every instance references exactly one 
 * element element, which must be the name of a 
 * camera, a light, an object, or an instance 
 * group. If the instanced item is a geometry 
 * shader function, the scene element created 
 * by this special shader is actually used as 
 * the instanced item. */
#pragma pack(push, 1)
struct eiInstance {
	eiNode			node;
	eiTag			element;
	eiMatrix		transform;
	eiMatrix		motion_transform;
	eiAttributes	attr;
};
#pragma pack(pop)

void ei_instance_init(eiNodeSystem *nodesys, eiNode *node);
void ei_instance_exit(eiNodeSystem *nodesys, eiNode *node);

/** \brief Add material to the material list of this instance. */
void ei_instance_add_material(eiInstance *inst, const eiTag mtl, eiDatabase *db);

/** \brief Instance this element into global scene database or update 
 * the existing element.
 * Merge attributes down the DAG. children will inherit their 
 * parents' attributes if not overridden by themselves. */
void ei_instance_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer);

eiNodeObject *ei_create_instance_node_object(void *param);
/** \brief Install the node into node system */
void ei_install_instance_node(eiNodeSystem *nodesys);

#ifdef __cplusplus
}
#endif

#endif
