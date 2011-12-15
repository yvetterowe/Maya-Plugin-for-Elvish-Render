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

#include <eiAPI/ei_instance.h>
#include <eiAPI/ei_instgroup.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_assert.h>

void ei_instance_init(eiNodeSystem *nodesys, eiNode *node)
{
	eiInstance	*inst;

	inst = (eiInstance *)node;

	inst->node.type = EI_ELEMENT_INSTANCE;

	ei_attr_init(&inst->attr);
	inst->attr.material_list = ei_create_data_array(nodesys->m_db, EI_DATA_TYPE_TAG);
}

void ei_instance_exit(eiNodeSystem *nodesys, eiNode *node)
{
	eiInstance	*inst;

	inst = (eiInstance *)node;

	ei_attr_exit(&inst->attr, nodesys->m_db);
	ei_delete_data_array(nodesys->m_db, inst->attr.material_list);
}

void ei_instance_add_material(eiInstance *inst, const eiTag mtl, eiDatabase *db)
{
	ei_data_array_push_back(db, inst->attr.material_list, &mtl);
}

/* instance this instance, concatenate transform matrix and motion transform 
   matrix, pass them to the element which this instance is referencing.
   The matrices establish the transformation from the object space of the 
   instanced element to the parent coordinate space. */
void ei_instance_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer)
{
	eiInstance	*instance;

	instance = (eiInstance *)node;

	if (instance->element != eiNULL_TAG)
	{
		eiAttributes	merged_attr;
		eiMatrix		mat;
		eiMatrix		motion_mat;
		eiNode			*element_node;
		eiElement		*element;

		/* inherit from parent attributes */
		ei_attr_copy(&merged_attr, &instance->attr);
		ei_attr_inherit(&merged_attr, attr);

		mulmm(&mat, &instance->transform, transform);
		mulmm(&motion_mat, &instance->motion_transform, motion_transform);

		element_node = (eiNode *)ei_db_access(nodesys->m_db, instance->element);

		element = (eiElement *)element_node->object;

		element->update_instance(nodesys, cache, element_node, &merged_attr, &mat, &motion_mat, node);

		ei_db_end(nodesys->m_db, instance->element);
	}
}

eiNodeObject *ei_create_instance_node_object(void *param)
{
	eiElement	*element;

	element = ei_create_element();

	element->base.base.deletethis = ei_element_deletethis;
	element->base.init_node = ei_instance_init;
	element->base.exit_node = ei_instance_exit;
	element->base.node_changed = NULL;
	element->update_instance = ei_instance_update_instance;

	return ((eiNodeObject *)element);
}

void ei_install_instance_node(eiNodeSystem *nodesys)
{
	eiTag		desc_tag;
	eiNodeDesc	*desc;
	eiTag		default_tag;
	eiShort		default_short;
	eiScalar	default_scalar;

	desc = ei_nodesys_node_desc(nodesys, &desc_tag, "instance");
	if (desc == NULL)
	{
		return;
	}

	default_tag = eiNULL_TAG;
	default_short = 0;
	default_scalar = 0.0f;

	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"element", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_MATRIX, 
		"transform", 
		&g_IdentityMatrix);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_MATRIX, 
		"motion_transform", 
		&g_IdentityMatrix);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SHORT, 
		"flags", 
		&default_short);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SHORT, 
		"face", 
		&default_short);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"approx", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"max_displace", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SHORT, 
		"min_samples", 
		&default_short);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SHORT, 
		"max_samples", 
		&default_short);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"material_list", 
		&default_tag);

	ei_nodesys_end_node_desc(nodesys, desc, desc_tag);
}