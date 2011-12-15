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
 
#ifndef EI_INSTGROUP_H
#define EI_INSTGROUP_H

/** \brief The instance group for grouping a list of instances as 
 * a scene element.
 * \file ei_instgroup.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_element.h>
#include <eiAPI/ei_attributes.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The only purpose of instance groups is as a 
 * container for instances. it doesn't need to free the 
 * instances it's containing, because the real storage 
 * of these instances is in the global scene database, 
 * it only references them here. */
#pragma pack(push, 1)
typedef struct eiInstgroup {
	eiNode			node;
	/* a data array of tags, each tag points to an instance */
	eiTag			instances;
} eiInstgroup;
#pragma pack(pop)

void ei_instgroup_init(eiNodeSystem *nodesys, eiNode *node);
void ei_instgroup_exit(eiNodeSystem *nodesys, eiNode *node);

/** \brief Add an instance into this instance group. */
void ei_instgroup_add_instance(eiInstgroup *instgroup, const eiTag inst, eiDatabase *db);

/** \brief Instance this element into global scene database or update 
 * the existing element.
 * Merge attributes down the DAG. children will inherit their 
 * parents' attributes if not overridden by themselves. */
void ei_instgroup_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer);

eiNodeObject *ei_create_instgroup_node_object(void *param);
/** \brief Install the node into node system */
void ei_install_instgroup_node(eiNodeSystem *nodesys);

#ifdef __cplusplus
}
#endif

#endif
