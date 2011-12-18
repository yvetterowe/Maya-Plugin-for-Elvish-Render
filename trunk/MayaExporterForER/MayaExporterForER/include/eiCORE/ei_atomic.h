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

#ifndef EI_ATOMIC_H
#define EI_ATOMIC_H

/** \brief This file contains the definitions of atomic data types.
 * \file ei_atomic.h
 * \author Bo Zhou
 */

#include <eiCORE/ei_core.h>
#include <xmmintrin.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EI_OS_WINDOWS

#include <windows.h>

typedef struct eiAtomic {
	volatile long counter;
} eiAtomic;

#ifdef EI_ARCH_X64

typedef struct eiAtomic64 {
	volatile long long counter;
} eiAtomic64;

#endif

#else

#include <asm/atomic.h>
#include <asm/system.h>

typedef atomic_t eiAtomic;

#ifdef EI_ARCH_X64

typedef atomic64_t eiAtomic64;

#endif

#endif

#ifdef EI_ARCH_X64

#define eiAtomicptr eiAtomic64

#else

#define eiAtomicptr eiAtomic

#endif

#ifdef __cplusplus
}
#endif

#endif
