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

#include <eiCORE/ei_timer.h>
#include <eiCORE/ei_platform.h>
#include <eiCORE/ei_assert.h>

void ei_timer_reset(eiTimer *timer)
{
	eiDBG_ASSERT(timer != NULL);

	timer->duration = 0;
}

void ei_timer_start(eiTimer *timer)
{
	eiDBG_ASSERT(timer != NULL);

	timer->start_time = ei_get_time();
}

void ei_timer_stop(eiTimer *timer)
{
	eiDBG_ASSERT(timer != NULL);

	timer->duration += (ei_get_time() - timer->start_time);
}

void ei_timer_format(eiTimer *timer, 
					 eiInt *hours, 
					 eiInt *minutes, 
					 eiScalar *seconds)
{
	eiScalar dur;

	eiDBG_ASSERT(timer != NULL);

	dur = (eiScalar)timer->duration / 1000.0f;

	*minutes = (eiInt)dur / 60;
	*seconds = dur - (eiScalar)(*minutes) * 60.0f;
	*hours = *minutes / 60;
	*minutes = *minutes % 60;
}
