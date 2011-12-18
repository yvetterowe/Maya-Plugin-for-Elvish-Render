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
 
#ifndef EI_TEXMAKE_H
#define EI_TEXMAKE_H

/** \brief Texture maker utilities for converting miscellaneous image 
 * formats into our internal texture format.
 * \file ei_texmake.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiCORE/ei_plugsys.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief This function converts a picture file to a texture file 
 * and pre-filter the texture. */
void ei_make_texture_imp(
	eiPluginSystem *plugsys, 
	const char *picturename, const char *texturename, 
	eiInt swrap, eiInt twrap, eiInt filter, eiScalar swidth, eiScalar twidth);

#ifdef __cplusplus
}
#endif

#endif
