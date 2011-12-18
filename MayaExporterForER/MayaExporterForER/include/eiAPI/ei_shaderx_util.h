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
 
#ifndef EI_SHADERX_UTIL_H
#define EI_SHADERX_UTIL_H

/** \brief The utility functions for shading language interface.
 * \file ei_shaderx_util.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_shaderx_types.h>

inline scalar inversesqrt(scalar a)
{
	return 1.0f / sqrt(MAX(eiSCALAR_EPS, a));
}

inline scalar sign(const scalar a)
{
	if (a == 0.0f) {
		return 0.0f;
	} else if (a > 0.0f) {
		return 1.0f;
	} else {
		return -1.0f;
	}
}

template < typename T >
inline T min(const T & a, const T & b)
{
	return ((a < b) ? a : b);
}

template < typename T >
inline T max(const T & a, const T & b)
{
	return ((a > b) ? a : b);
}

template < typename T >
inline T clamp(const T & a, const T & lo, const T & hi)
{
	if (a < lo) {
		return lo;
	} else if (a > hi) {
		return hi;
	} else {
		return a;
	}
}

template <>
inline color clamp<color>(const color & a, const color & lo, const color & hi)
{
	return color(clamp(a.r, lo.r, hi.r), 
				clamp(a.g, lo.g, hi.g), 
				clamp(a.b, lo.b, hi.b));
}

template <>
inline point clamp<point>(const point & a, const point & lo, const point & hi)
{
	return point(clamp(a.x, lo.x, hi.x), 
				clamp(a.y, lo.y, hi.y), 
				clamp(a.z, lo.z, hi.z));
}

template <>
inline vector clamp<vector>(const vector & a, const vector & lo, const vector & hi)
{
	return vector(clamp( a.x, lo.x, hi.x), 
				clamp( a.y, lo.y, hi.y), 
				clamp( a.z, lo.z, hi.z));
}

template <>
inline normal clamp<normal>(const normal & a, const normal & lo, const normal & hi)
{
	return normal(clamp(a.x, lo.x, hi.x), 
				clamp(a.y, lo.y, hi.y), 
				clamp(a.z, lo.z, hi.z));
}

inline bool is_black(const color & col)
{
	return (col.r == 0.0f && col.g == 0.0f && col.b == 0.0f);
}

inline scalar dot(const vector & a, const vector & b)
{
	return dot(&a, &b);
}

inline point cross(const vector & a, const vector & b)
{
	vector result;
	cross(&result, &a, &b);
	return result;
}

template < typename T >
inline scalar length(const T & V)
{
	return sqrt(MAX(eiSCALAR_EPS, V % V));
}

inline scalar distance(const point & P1, const point & P2)
{
	return length(P1 - P2);
}

inline scalar ptlined(const point & A, const point & B, const point & P)
{
	vector	vtmp, vtmp2, vtmp3;
	scalar	l;

	vtmp = P - B;
	vtmp2 = A - B;
	if ((vtmp % vtmp2) <= 0.0f) {
		l = sqrt(MAX(eiSCALAR_EPS, vtmp % vtmp));
	} else {
		vtmp2 = - vtmp2;
		vtmp = P - A;
		if ((vtmp % vtmp2) <= 0.0f) {
			l = sqrt(MAX(eiSCALAR_EPS, vtmp % vtmp));
		} else {
			vtmp = B - A;
			vtmp2 = B - P;
			vtmp3 = vtmp ^ vtmp2;
			l = sqrt(MAX(eiSCALAR_EPS, vtmp3 % vtmp3)) / sqrt(MAX(eiSCALAR_EPS, vtmp % vtmp));
		}
	}

	return l;
}

template < typename T >
inline T normalize(const T & V)
{
	return V * inversesqrt(V % V);
}

inline vector faceforward(const vector & vN, const vector & vI, const vector & Nref)
{
	return sign(-vI % Nref) * vN;
}

inline matrix translate(const matrix & m, const vector & t)
{
	return m * translate(t.x, t.y, t.z);
}

inline matrix rotate(const matrix & m, scalar angle, const vector & axis)
{
	return m * rotate(angle, axis);
}

inline matrix scale(const matrix & m, const point & s)
{
	return m * scale(s.x, s.y, s.z);
}

#endif
