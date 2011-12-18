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
 
#ifndef EI_DISC_OBJECT_H
#define EI_DISC_OBJECT_H

/** \brief The disc object representation
 * \file ei_disc_object.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_object.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The disc object. */
#pragma pack(push, 1)
typedef struct eiDiscObject {
	eiObject		base;
	/* a list of eiVector4, XYZ is position, W is radius */
	eiTag			disc_list;
} eiDiscObject;
#pragma pack(pop)

void ei_disc_object_init(eiNodeSystem *nodesys, eiNode *node);
void ei_disc_object_exit(eiNodeSystem *nodesys, eiNode *node);

/** \brief Instance this element into global scene database or update 
 * the existing element */
void ei_disc_object_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer);

#ifdef __cplusplus
}
#endif

#endif
