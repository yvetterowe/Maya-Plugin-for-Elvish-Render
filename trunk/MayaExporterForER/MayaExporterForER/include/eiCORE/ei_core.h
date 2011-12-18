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
 
#ifndef EI_CORE_H
#define EI_CORE_H

/** \brief The header of the module.
 * \file ei_core.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_types.h>
/* must not include ei_common.h here because 
   the specific data types should not be known 
   to core module since this is a very 
   fundamental infrastructure. */

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The data types */
enum {
	EI_DATA_TYPE_NONE = 0,	/* none data type, must be 0, should not be used actually. */
	/* atomic types */
	EI_DATA_TYPE_BYTE,		/* eiByte */
	EI_DATA_TYPE_SHORT,		/* eiShort */
	EI_DATA_TYPE_INT,		/* eiInt */
	EI_DATA_TYPE_BOOL,		/* eiBool */
	EI_DATA_TYPE_TAG,		/* eiTag */
	EI_DATA_TYPE_INDEX,		/* eiIndex */
	EI_DATA_TYPE_LONG,		/* eiLong */
	EI_DATA_TYPE_SCALAR,	/* eiScalar */
	EI_DATA_TYPE_GEOSCALAR,	/* eiGeoScalar */
	EI_DATA_TYPE_VECTOR,	/* eiVector */
	EI_DATA_TYPE_VECTOR2,	/* eiVector2 */
	EI_DATA_TYPE_VECTOR4,	/* eiVector4 */
	EI_DATA_TYPE_MATRIX,	/* eiMatrix */
	EI_DATA_TYPE_BOUND,		/* eiBound */
	EI_DATA_TYPE_RECT,		/* eiRect */
	EI_DATA_TYPE_RECT4I,	/* eiRect4i */
	EI_DATA_TYPE_INTARRAY,	/* eiInt array, will be automatically byte-swapped */
	/* compound types */
	EI_DATA_TYPE_JOB_TEST,	/* job only for internal testing. */
	EI_DATA_TYPE_ARRAY,		/* dynamic data array stored in database. */
	EI_DATA_TYPE_TABLE,		/* dynamic data table stored in database. */
	EI_DATA_TYPE_BLOCK,		/* data block of dynamic data table, for internal use only. */
	EI_DATA_TYPE_USER,		/* user data type starts from here. */
};

/** \brief The interface types of global objects */
enum {
	EI_INTERFACE_TYPE_NONE = 0,		/* none interface type, must be 0, should not be used. */
	EI_INTERFACE_TYPE_USER,			/* user interface type starts from here. */
};

/** \brief The TLS interface types */
enum {
	EI_TLS_TYPE_NONE = 0,		/* none TLS type, must be 0, should not be used. */
	EI_TLS_TYPE_USER,			/* user TLS type starts from here. */
};

/* exporting and importing */
/* eiCORE_EXPORTS should NOT be defined by users */
#ifdef _MSC_VER
#	if defined eiCORE_EXPORTS && !defined UNIT_TESTS
#		define eiCORE_API		__declspec(dllexport)
#		define eiCORE_EXTERN	extern __declspec(dllexport)
#	else
#		define eiCORE_API		__declspec(dllimport)
#		define eiCORE_EXTERN	extern __declspec(dllimport)
#	endif
#elif __GNUC__ >= 4
#	if defined eiCORE_EXPORTS && !defined UNIT_TESTS
#		define eiCORE_API		__attribute__((visibility("default")))
#		define eiCORE_EXTERN	extern
#	else
#		define eiCORE_API		
#		define eiCORE_EXTERN	extern
#	endif
#else
#	if defined eiCORE_EXPORTS && !defined UNIT_TESTS
#		define eiCORE_API		
#		define eiCORE_EXTERN	extern
#	else
#		define eiCORE_API		
#		define eiCORE_EXTERN	extern
#	endif
#endif

/** \brief Initialize and register the module.
 */
eiCORE_API void ei_core_init();

/** \brief Cleanup and exit the module.
 */
eiCORE_API void ei_core_exit();

/** \brief Calculate the count to reserve based on allocated size.
 */
eiCORE_API eiSizet ei_reserve_size(const eiSizet count);

#ifdef __cplusplus
}
#endif

#endif
