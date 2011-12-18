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

#ifndef EI_TIMER_H
#define EI_TIMER_H

/** \brief This file contains utilities for timing and profiling.
 * \file ei_timer.h
 */

#include <eiCORE/ei_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Timer for multiple time segments.
 */
typedef struct eiTimer {
	eiInt		start_time;	/** The starting time record */
	eiInt		duration;	/** The accumulated duration in milliseconds */
} eiTimer;

/** \brief Restart a timer.
 */
eiCORE_API void ei_timer_reset(eiTimer *timer);

/** \brief Start a timer.
 */
eiCORE_API void ei_timer_start(eiTimer *timer);

/** \brief Stop a timer, accumulate passed duration.
 */
eiCORE_API void ei_timer_stop(eiTimer *timer);

/** \brief Format the duration of a timer into hours, minutes, seconds.
 */
eiCORE_API void ei_timer_format(eiTimer *timer, 
								eiInt *hours, 
								eiInt *minutes, 
								eiScalar *seconds);

#ifdef __cplusplus
}
#endif

#endif
