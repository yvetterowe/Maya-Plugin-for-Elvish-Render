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

#include <eiAPI/ei_element.h>
#include <eiCORE/ei_assert.h>

void ei_element_init(eiElement *element)
{
	ei_node_object_init(&element->base);
}

void ei_element_exit(eiElement *element)
{
	ei_node_object_exit(&element->base);
}

eiElement *ei_create_element()
{
	eiElement	*element;

	element = (eiElement *)ei_allocate(sizeof(eiElement));

	ei_element_init(element);

	return element;
}

void ei_element_deletethis(eiPluginObject *object)
{
	if (object == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_element_exit((eiElement *)object);

	ei_free(object);
}
