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

#include <eiAPI/ei_proc_object.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_assert.h>

void ei_proc_object_init(eiNodeSystem *nodesys, eiNode *node)
{
	eiProcObject	*proc;

	ei_object_init(nodesys, node);

	proc = (eiProcObject *)node;

	proc->geometry_list = ei_create_data_array(nodesys->m_db, EI_DATA_TYPE_TAG);
}

void ei_proc_object_exit(eiNodeSystem *nodesys, eiNode *node)
{
	eiProcObject	*proc;

	proc = (eiProcObject *)node;

	ei_delete_data_array(nodesys->m_db, proc->geometry_list);

	ei_object_exit(nodesys, node);
}

void ei_proc_object_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer)
{
}

void ei_proc_object_add_geometry(eiProcObject *obj, eiDatabase *db, const eiTag shader)
{
	ei_data_array_push_back(db, obj->geometry_list, &shader);
}
