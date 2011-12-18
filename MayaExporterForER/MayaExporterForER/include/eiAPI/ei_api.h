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
 
#ifndef EI_API_H
#define EI_API_H

/** \brief The header of the module.
 * \file ei_api.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_types.h>
#include <eiCOMMON/ei_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/* exporting and importing */
/* eiAPI_EXPORTS should NOT be defined by users */
#ifdef _MSC_VER
#	if defined eiAPI_EXPORTS && !defined UNIT_TESTS
#		define eiAPI			__declspec(dllexport)
#		define eiAPI_EXTERN		extern __declspec(dllexport)
#	else
#		define eiAPI			__declspec(dllimport)
#		define eiAPI_EXTERN		extern __declspec(dllimport)
#	endif
#elif __GNUC__ >= 4
#	if defined eiAPI_EXPORTS && !defined UNIT_TESTS
#		define eiAPI			__attribute__((visibility("default")))
#		define eiAPI_EXTERN		extern
#	else
#		define eiAPI		
#		define eiAPI_EXTERN		extern
#	endif
#else
#	if defined eiAPI_EXPORTS && !defined UNIT_TESTS
#		define eiAPI		
#		define eiAPI_EXTERN		extern
#	else
#		define eiAPI		
#		define eiAPI_EXTERN		extern
#	endif
#endif

/** \brief Initialize and register the module.
 */
eiAPI void ei_api_init();

/** \brief Cleanup and exit the module.
 */
eiAPI void ei_api_exit();

#ifdef __cplusplus
}
#endif

#endif
