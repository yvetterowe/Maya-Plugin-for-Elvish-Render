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

#ifndef EI_TYPES_H
#define EI_TYPES_H

/** \brief Definitions of basic types.
 * \file ei_types.h
 */

#include <eiCORE/ei_base.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define eiFALSE					0
#define eiTRUE					1

typedef signed char				eiInt8;
typedef unsigned char			eiUint8;

typedef signed short			eiInt16;
typedef unsigned short			eiUint16;

typedef signed int				eiInt32;
typedef unsigned int			eiUint32;

typedef signed long long		eiInt64;
typedef unsigned long long		eiUint64;

typedef float					eiScalar;
typedef double					eiGeoScalar;

typedef size_t					eiSizet;

typedef eiUint8					eiByte;
typedef eiInt16					eiShort;
typedef eiUint16				eiUshort;
typedef eiInt32					eiInt;
typedef eiUint32				eiUint;
typedef eiInt64					eiLong;
typedef eiUint64				eiUlong;

typedef union eiPointer {
	void		*p;
	eiUlong		padding;
} eiPointer;

#ifdef EI_ARCH_X86
typedef eiInt32					eiIntptr;
typedef eiUint32				eiUintptr;
#else
typedef eiInt64					eiIntptr;
typedef eiUint64				eiUintptr;
#endif

typedef eiUint					eiTag;
typedef eiUint					eiIndex;
typedef eiInt					eiBool;

typedef eiInt					eiHostID;
typedef eiInt					eiThreadID;

typedef void *					eiInterface;

#define eiSCALAR_EPS			(1.0e-5f)
#define eiGEOSCALAR_EPS			(1e-9)
#define eiBIG_SCALAR			(1e36f)
#define eiMAX_SCALAR			(3.402823466e+38f)
#define eiMAX_GEOSCALAR			(1.7976931348623158e+308)
#define eiMIN_SHORT				(-32767 - 1)
#define eiMAX_SHORT				(32767)
#define eiMIN_INT				(-2147483647 - 1)
#define eiMAX_INT				(2147483647)
#define eiMIN_LONG				(-9223372036854775807i64 - 1)
#define eiMAX_LONG				(9223372036854775807i64)
#define eiNULL_INDEX			(0xFFFFFFFF)
#define eiMAX_INDEX				(0xFFFFFFFE)
#define eiNULL_TAG				(0xFFFFFFFF)
#define eiMAX_SIZET				(eiSizet)(~((eiSizet)0))
#define eiINVALID_HOSTID		-1
#define eiINVALID_THREADID		-1

#define eiE				2.71828182845904523536 /* e */
#define eiLOG2E			1.44269504088896340736 /* log2(e) */
#define eiLOG10E		0.434294481903251827651 /* log10(e) */
#define eiLN2			0.693147180559945309417 /* ln(2) */
#define eiLN10			2.30258509299404568402 /* ln(10) */
#define eiPI			3.14159265358979323846 /* pi */
#define eiPI_2			1.57079632679489661923 /* pi/2 */
#define eiPI_4			0.785398163397448309616 /* pi/4 */
#define ei1_PI			0.318309886183790671538 /* 1/pi */
#define ei2_PI			0.636619772367581343076 /* 2/pi */
#define ei2_SQRTPI		1.12837916709551257390 /* 2/sqrt(pi) */
#define eiSQRT2			1.41421356237309504880 /* sqrt(2) */
#define eiSQRT1_2		0.707106781186547524401 /* 1/sqrt(2) */

#ifdef __cplusplus
}
#endif

#endif
