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

#ifndef EI_BASE_H
#define EI_BASE_H

/** \brief This file contains definitions for multiple platforms
 * which will be used across the entire projects.
 * \file ei_base.h
 * \author Bo Zhou
 * \todo Currently we only support x86 and x64(AMD64 or EM64T, NOT IA64)
 * for Windows and Linux now, we will support OSX in future.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* architectures */
#ifdef _MSC_VER
	#if defined _M_X64
		#define EI_ARCH_X64
	#elif defined _M_IX86
		#define EI_ARCH_X86
	#endif
#elif defined __GNUC__
	#if defined __x86_64 || defined __amd64
		#define EI_ARCH_X64
	#elif defined __i386
		#define EI_ARCH_X86
	#endif
#endif

/* operating systems and endiannesses */
#ifdef _WIN32
	#define EI_OS_WINDOWS
	#define EI_OS_LITTLE_ENDIAN
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0400 /* Windows 2000 minimum required */
	#endif
#elif defined linux || defined __linux__
	#define EI_OS_POSIX
	#define EI_OS_LINUX
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		#define EI_OS_LITTLE_ENDIAN
	#else
		#define EI_OS_BIG_ENDIAN
	#endif
#endif

/* inlining */
#ifdef _MSC_VER
	#define eiINLINE		__inline
	#define eiFORCEINLINE	__forceinline
#elif defined __GNUC__
	#define eiINLINE		inline
	#define eiFORCEINLINE	inline __attribute__((always_inline))
#endif

/* alignments */
#define EI_DEFAULT_ALIGNMENT	16
#ifdef _MSC_VER
	#define eiALIGN		__declspec(align(EI_DEFAULT_ALIGNMENT))
	#define eiALIGN16	__declspec(align(16))
	#define eiALIGN32	__declspec(align(32))
#elif defined __GNUC__
	#define eiALIGN		__attribute__(aligned(EI_DEFAULT_ALIGNMENT))
	#define eiALIGN16	__attribute__(aligned(16))
	#define eiALIGN32	__attribute__(aligned(32))
#endif

/* calling conventions */
#ifdef _MSC_VER
	#define eiCDECL		__cdecl
	#define eiSTDCALL	__stdcall
#else
	#define eiCDECL
	#define eiSTDCALL
#endif

/* CRT debugging */
#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
#endif

/* statements */
#define STMT(stmt)			do { stmt } while (0);

#ifdef _DEBUG
	#define DEBUG_STMT(stmt)	do { stmt } while (0);
#else
	#define DEBUG_STMT(stmt)
#endif

#ifdef __cplusplus
}
#endif

#endif
