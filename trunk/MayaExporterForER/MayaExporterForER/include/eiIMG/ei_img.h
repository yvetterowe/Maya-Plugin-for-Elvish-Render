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
 
#ifndef EI_IMG_H
#define EI_IMG_H

/** \brief The header of the module.
 * \file ei_img.h
 * \author Elvic Liang
 */

#include <eiAPI/ei.h>

#ifdef __cplusplus
extern "C" {
#endif

/* exporting */
#ifdef _MSC_VER
#	if !defined UNIT_TESTS
#		define eiIMG_API		__declspec(dllexport)
#		define eiIMG_EXTERN		extern __declspec(dllexport)
#	else
#		define eiIMG_API		__declspec(dllimport)
#		define eiIMG_EXTERN		extern __declspec(dllimport)
#	endif
#elif __GNUC__ >= 4
#	if !defined UNIT_TESTS
#		define eiIMG_API		__attribute__((visibility("default")))
#		define eiIMG_EXTERN		extern
#	else
#		define eiIMG_API		
#		define eiIMG_EXTERN		extern
#	endif
#else
#	if !defined UNIT_TESTS
#		define eiIMG_API		
#		define eiIMG_EXTERN		extern
#	else
#		define eiIMG_API		
#		define eiIMG_EXTERN		extern
#	endif
#endif

/** \brief The function to initialize a module on loading. */
eiIMG_API void module_init();
/** \brief The function to cleanup a module on unloading. */
eiIMG_API void module_exit();

#ifdef __cplusplus
}
#endif

#endif
