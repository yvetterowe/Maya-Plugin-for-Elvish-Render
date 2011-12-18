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
 
#ifndef EI_ELEMENT_H
#define EI_ELEMENT_H

/** \brief The base class for all scene elements.
 * \file ei_element.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_nodesys.h>
#include <eiCORE/ei_matrix.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The flags to identify element types */
enum {
	EI_ELEMENT_NONE = 0, 
	EI_ELEMENT_INSTANCE, 
	EI_ELEMENT_INSTGROUP, 
	EI_ELEMENT_OPTIONS, 
	EI_ELEMENT_CAMERA, 
	EI_ELEMENT_MATERIAL, 
	EI_ELEMENT_LIGHT, 
	EI_ELEMENT_OBJECT, 
	EI_ELEMENT_TYPE_COUNT, 
};

/* forward declarations */
typedef struct eiInstance			eiInstance;
typedef struct eiAttributes			eiAttributes;

/** \brief The global cache of object representations which will 
 * be used during scene pre-processing to avoid unnecessary 
 * tessellations. */
typedef struct eiObjectRepCache {
	/* current camera for view-dependent tessellation */
	eiTag				cam_tag;
	/* the temporary set of tags of objects that were travesed 
	   during scene DAG traversal */
	ei_btree			objects;
	/* the temporary job queue of object representations that 
	   were scheduled for tessellation during scene DAG traversal */
	eiJobQueue			*object_rep_jobs;
	/* the newly built renderable object instances during 
	   scene pre-processing, this will replace the old renderable 
	   object instances built in previous scene pre-processing */
	eiTag				object_instances;
	/* the light instances created during scene pre-processing */
	eiTag				light_instances;
} eiObjectRepCache;

/** \brief The callback for instancing this element into 
 * global renderable scene, or updating this element if it 
 * was modified and already created in the renderable scene. */
typedef void (*ei_element_update_instance)(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer);

/** \brief The base class for all scene elements. */
typedef struct eiElement {
	/* the base node object */
	eiNodeObject				base;
	/* the method for instancing this element or 
	   updating the created instance. */
	ei_element_update_instance	update_instance;
} eiElement;

void ei_element_init(eiElement *element);
void ei_element_exit(eiElement *element);
/** \brief Create node object of element type. */
eiElement *ei_create_element();
/** \brief The callback to delete node object of element type. */
void ei_element_deletethis(eiPluginObject *object);

#ifdef __cplusplus
}
#endif

#endif
