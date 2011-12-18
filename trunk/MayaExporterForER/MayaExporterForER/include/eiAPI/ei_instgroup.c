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

#include <eiAPI/ei_instgroup.h>
#include <eiAPI/ei_instance.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_assert.h>

void ei_instgroup_init(eiNodeSystem *nodesys, eiNode *node)
{
	eiInstgroup	*instgroup;

	instgroup = (eiInstgroup *)node;

	instgroup->node.type = EI_ELEMENT_INSTGROUP;

	instgroup->instances = ei_create_data_array(nodesys->m_db, EI_DATA_TYPE_TAG);
}

void ei_instgroup_exit(eiNodeSystem *nodesys, eiNode *node)
{
	eiInstgroup	*instgroup;

	instgroup = (eiInstgroup *)node;

	ei_delete_data_array(nodesys->m_db, instgroup->instances);
}

void ei_instgroup_add_instance(eiInstgroup *instgroup, const eiTag inst, eiDatabase *db)
{
	ei_data_array_push_back(db, instgroup->instances, &inst);
}

/* instance the children instances of this instgroup, simply pass transform 
   matrix and motion transform matrix to all of them. */
void ei_instgroup_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer)
{
	eiInstgroup		*instgroup;
	eiInt			num_instances;
	eiInt			i;

	instgroup = (eiInstgroup *)node;

	num_instances = ei_data_array_size(nodesys->m_db, instgroup->instances);

	for (i = 0; i < num_instances; ++i)
	{
		eiTag		instance_tag;
		eiInstance	*instance;
		eiElement	*element;

		instance_tag = *((eiTag *)ei_data_array_read(nodesys->m_db, instgroup->instances, i));
		ei_data_array_end(nodesys->m_db, instgroup->instances, i);

		if (instance_tag != eiNULL_TAG)
		{
			instance = (eiInstance *)ei_db_access(nodesys->m_db, instance_tag);

			element = (eiElement *)instance->node.object;

			element->update_instance(
				nodesys, 
				cache, 
				(eiNode *)instance, 
				attr, 
				transform, 
				motion_transform, 
				node);

			ei_db_end(nodesys->m_db, instance_tag);
		}
	}
}

eiNodeObject *ei_create_instgroup_node_object(void *param)
{
	eiElement	*element;

	element = ei_create_element();

	element->base.base.deletethis = ei_element_deletethis;
	element->base.init_node = ei_instgroup_init;
	element->base.exit_node = ei_instgroup_exit;
	element->base.node_changed = NULL;
	element->update_instance = ei_instgroup_update_instance;

	return ((eiNodeObject *)element);
}

void ei_install_instgroup_node(eiNodeSystem *nodesys)
{
	eiTag		desc_tag;
	eiNodeDesc	*desc;
	eiTag		default_tag;

	desc = ei_nodesys_node_desc(nodesys, &desc_tag, "instgroup");
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
		"instances", 
		&default_tag);

	ei_nodesys_end_node_desc(nodesys, desc, desc_tag);
}
