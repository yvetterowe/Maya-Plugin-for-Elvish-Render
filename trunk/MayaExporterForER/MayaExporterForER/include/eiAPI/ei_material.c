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

#include <eiAPI/ei_material.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_assert.h>

void ei_material_init(eiNodeSystem *nodesys, eiNode *node)
{
	eiMaterial	*mtl;

	mtl = (eiMaterial *)node;

	mtl->node.type = EI_ELEMENT_MATERIAL;

	mtl->surface_list = ei_create_data_array(nodesys->m_db, EI_DATA_TYPE_TAG);
	mtl->displace_list = ei_create_data_array(nodesys->m_db, EI_DATA_TYPE_TAG);
	mtl->shadow_list = ei_create_data_array(nodesys->m_db, EI_DATA_TYPE_TAG);
	mtl->volume_list = ei_create_data_array(nodesys->m_db, EI_DATA_TYPE_TAG);
	mtl->photon_list = ei_create_data_array(nodesys->m_db, EI_DATA_TYPE_TAG);

	mtl->env_list = eiNULL_TAG;
}

void ei_material_exit(eiNodeSystem *nodesys, eiNode *node)
{
	eiMaterial	*mtl;

	mtl = (eiMaterial *)node;

	if (mtl->env_list != eiNULL_TAG)
	{
		ei_delete_data_array(nodesys->m_db, mtl->env_list);
		mtl->env_list = eiNULL_TAG;
	}

	ei_delete_data_array(nodesys->m_db, mtl->photon_list);
	ei_delete_data_array(nodesys->m_db, mtl->volume_list);
	ei_delete_data_array(nodesys->m_db, mtl->shadow_list);
	ei_delete_data_array(nodesys->m_db, mtl->displace_list);
	ei_delete_data_array(nodesys->m_db, mtl->surface_list);
}

void ei_material_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer)
{
}

void ei_material_add_surface(eiMaterial *mtl, eiDatabase *db, const eiTag shader)
{
	ei_data_array_push_back(db, mtl->surface_list, &shader);
}

void ei_material_add_displace(eiMaterial *mtl, eiDatabase *db, const eiTag shader)
{
	ei_data_array_push_back(db, mtl->displace_list, &shader);
}

void ei_material_add_shadow(eiMaterial *mtl, eiDatabase *db, const eiTag shader)
{
	ei_data_array_push_back(db, mtl->shadow_list, &shader);
}

void ei_material_add_volume(eiMaterial *mtl, eiDatabase *db, const eiTag shader)
{
	ei_data_array_push_back(db, mtl->volume_list, &shader);
}

void ei_material_add_environment(eiMaterial *mtl, eiDatabase *db, const eiTag shader)
{
	if (mtl->env_list == eiNULL_TAG)
	{
		mtl->env_list = ei_create_data_array(db, EI_DATA_TYPE_TAG);
	}
	ei_data_array_push_back(db, mtl->env_list, &shader);
}

void ei_material_add_photon(eiMaterial *mtl, eiDatabase *db, const eiTag shader)
{
	ei_data_array_push_back(db, mtl->photon_list, &shader);
}

eiNodeObject *ei_create_material_node_object(void *param)
{
	eiElement	*element;

	element = ei_create_element();

	element->base.base.deletethis = ei_element_deletethis;
	element->base.init_node = ei_material_init;
	element->base.exit_node = ei_material_exit;
	element->base.node_changed = NULL;
	element->update_instance = ei_material_update_instance;

	return ((eiNodeObject *)element);
}

void ei_install_material_node(eiNodeSystem *nodesys)
{
	eiTag		desc_tag;
	eiNodeDesc	*desc;
	eiTag		default_tag;

	desc = ei_nodesys_node_desc(nodesys, &desc_tag, "material");
	if (desc == NULL)
	{
		return;
	}

	default_tag = eiNULL_TAG;

	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"surface_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"displace_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"shadow_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"volume_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"env_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"photon_list", 
		&default_tag);

	ei_nodesys_end_node_desc(nodesys, desc, desc_tag);
}
