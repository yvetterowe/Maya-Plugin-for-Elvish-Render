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

#ifndef EI_ASSERT_H
#define EI_ASSERT_H

/** \brief This file contains the macro for reporting assertion 
 * failures with file name and line number.
 * \file ei_assert.h
 */

#include <eiCORE/ei_verbose.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CRT: */
#ifdef _DEBUG
#	include <crtdbg.h>
#else
#	ifndef _ASSERT
#		define _ASSERT(expr) 
#	endif
#endif
#define eiASSERT(expr) if (!(expr)) { ei_error("Assertion failed: %s File: %s Line: %d\n", #expr, __FILE__, __LINE__); _ASSERT(0); }

/* debug assert can make sure redundant 
   checks don't get into release build. */
#ifdef _DEBUG
#define eiDBG_ASSERT(expr) eiASSERT(expr)
#else
#define eiDBG_ASSERT(expr) 
#endif

#ifdef __cplusplus
}
#endif

#endif
