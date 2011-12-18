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

#include <eiAPI/ei_disc_object.h>
#include <eiCORE/ei_assert.h>

void ei_disc_object_init(eiNodeSystem *nodesys, eiNode *node)
{
	eiDiscObject	*disc;

	ei_object_init(nodesys, node);

	disc = (eiDiscObject *)node;
}

void ei_disc_object_exit(eiNodeSystem *nodesys, eiNode *node)
{
	ei_object_exit(nodesys, node);
}

void ei_disc_object_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer)
{
}
