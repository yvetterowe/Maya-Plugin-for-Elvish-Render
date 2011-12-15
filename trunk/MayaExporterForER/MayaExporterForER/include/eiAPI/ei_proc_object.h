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
 
#ifndef EI_PROC_OBJECT_H
#define EI_PROC_OBJECT_H

/** \brief The procedural object representation
 * \file ei_proc_object.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_object.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The procedural object. */
#pragma pack(push, 1)
typedef struct eiProcObject {
	eiObject		base;
	/* a list of geometry shaders */
	eiTag			geometry_list;
} eiProcObject;
#pragma pack(pop)

void ei_proc_object_init(eiNodeSystem *nodesys, eiNode *node);
void ei_proc_object_exit(eiNodeSystem *nodesys, eiNode *node);

/** \brief Instance this element into global scene database or update 
 * the existing element */
void ei_proc_object_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer);

void ei_proc_object_add_geometry(eiProcObject *obj, eiDatabase *db, const eiTag shader);

#ifdef __cplusplus
}
#endif

#endif
