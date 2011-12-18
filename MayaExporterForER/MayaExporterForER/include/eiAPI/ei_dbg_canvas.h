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
 
#ifndef EI_DBG_CANVAS_H
#define EI_DBG_CANVAS_H

/** \brief The debug canvas for internal use only.
 * \file ei_dbg_canvas.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>

#ifdef __cplusplus
extern "C" {
#endif

eiAPI void ei_dbg_canvas_init();
eiAPI void ei_dbg_canvas_resize(const eiInt width, const eiInt height);
eiAPI void ei_dbg_canvas_set_pixel(
	const eiScalar x, const eiScalar y, 
	const eiScalar r, const eiScalar g, const eiScalar b);
eiAPI eiBool ei_dbg_canvas_output(const char *filename);
eiAPI void ei_dbg_canvas_clear();

#ifdef __cplusplus
}
#endif

#endif
