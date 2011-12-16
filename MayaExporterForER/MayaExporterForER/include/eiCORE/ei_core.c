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

#include <eiCORE/ei_core.h>
#include <eiCORE/ei_network.h>
#include <eiCORE/ei_data_gen.h>
#include <eiCORE/ei_assert.h>

#define EI_RESERVED_SIZE		16
#define EI_MAX_RESERVED_SIZE	256

void ei_core_init()
{
	ei_verbose_init();

	ei_init_default_data_gen_table();

	ei_info("Starting up network...\n");
	ei_net_startup();
}

void ei_core_exit()
{
	ei_net_shutdown();
	ei_verbose_exit();
}

eiSizet ei_reserve_size(const eiSizet count)
{
	if (count == 0)
	{
		return EI_RESERVED_SIZE;
	}
	else
	{
		return MIN(count * 2, EI_MAX_RESERVED_SIZE);
	}
}
