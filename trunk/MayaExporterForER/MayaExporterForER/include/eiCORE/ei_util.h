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

#ifndef EI_UTIL_H
#define EI_UTIL_H

#include <eiCORE/ei_types.h>
#include <eiCORE/ei_platform.h>

#include <float.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define X_AXIS		0
#define Y_AXIS		1
#define Z_AXIS		2
#define W_AXIS		3

/* ensure isnan is available. */
#ifdef _MSC_VER
#	define isnan	_isnan
#endif

#define absf fabsf

#define INV_LOG_10_2	(1.0f / log10f(2.0f))
#define log2f(x)		(log10f((x)) * INV_LOG_10_2)

/* Quake 3 fast inverse square root algorithm */
eiFORCEINLINE eiScalar Fast_invsqrt(eiScalar x)
{
	eiScalar xhalf = 0.5f * x;
	eiInt i = *(eiInt *)&x;
	i = 0x5f3759df - (i >> 1);
	x = *(eiScalar *)&i;
	x = x * (1.5f - xhalf * x * x);
	return x;
}

/* Quake 3 fast square root algorithm */
eiFORCEINLINE eiScalar Fast_sqrt(const eiScalar x)
{
	union {
		eiInt intPart;
		eiScalar floatPart;
	} convertor;
	union {
		eiInt intPart;
		eiScalar floatPart;
	} convertor2;
	convertor.floatPart = x;
	convertor2.floatPart = x;
	convertor.intPart = 0x1fbcf800 + (convertor.intPart >> 1);
	convertor2.intPart = 0x5f3759df - (convertor2.intPart >> 1);
	return 0.5f * (convertor.floatPart + (x * convertor2.floatPart));
}

#define radians(x) ((x) * (eiPI / 180.0f))
#define degrees(x) ((x) * (180.0f / eiPI))

#define clampi(a, min, max) \
	if ((a) < (min)) {\
		(a) = (min);\
	} else if ((a) > (max)) {\
		(a) = (max);\
	}

#define lower_bound(a, min) \
	if ((a) < (min)) {\
		(a) = (min);\
	}

#define upper_bound(a, max) \
	if ((a) > (max)) {\
		(a) = (max);\
	}

#define mix(x, y, alpha) ((x) + ((y) - (x)) * (alpha))

#define invsqrtf(x) (1.0f / sqrtf(x))

eiFORCEINLINE eiScalar sgnf(const eiScalar a)
{
	if (a == 0.0f) {
		return 0.0f;
	} else if (a > 0.0f) {
		return 1.0f;
	} else {
		return -1.0f;
	}
}

eiFORCEINLINE eiScalar step(const eiScalar lo, const eiScalar value)
{
	if (value < lo) {
		return 0.0f;
	} else {
		return 1.0f;
	}
}

eiFORCEINLINE eiScalar smoothstep(const eiScalar lo, const eiScalar hi, const eiScalar value)
{
	if (value < lo)
		return 0.0f;
	else if (value >= hi)
		return 1.0f;
	else {
		eiScalar t = (value - lo) / (hi - lo);
		return mix(0.0f, 1.0f, t * t * (3.0f - 2.0f * t));
	}
}

#define sum(n) ((1 + (n)) * (n) / 2)

eiFORCEINLINE eiBool almost_zero(const eiScalar a, const eiScalar eps)
{
	if (absf(a) < eps) {
		return eiTRUE;
	} else {
		return eiFALSE;
	}
}

eiFORCEINLINE eiBool almost_equal(const eiScalar a, const eiScalar b, const eiScalar eps)
{
	return almost_zero(a - b, eps);
}

eiFORCEINLINE eiScalar curve(const eiScalar t)
{
	return t * t * (3.0f - 2.0f * t);
}

#ifdef _MSC_VER

eiFORCEINLINE eiInt truncf(const eiScalar a)
{
	return (eiInt)a;
}

eiFORCEINLINE eiInt roundf(const eiScalar a)
{
	eiInt d = truncf(a);
	if ((a - (eiScalar)d) < 0.5f) {
		return d;
	} else {
		return d + 1;
	}
}

#endif

eiFORCEINLINE eiInt lfloorf(const eiScalar a)
{
	eiInt	d = truncf(a);
	if (a != (eiScalar)d) {
		if (a > 0.0f) {
			return d;
		} else {
			return d - 1;
		}
	} else {
		return d;
	}
}

eiFORCEINLINE eiInt lceilf(const eiScalar a)
{
	eiInt	d = truncf(a);
	if (a != (eiScalar)d) {
		if (a > 0.0f) {
			return d + 1;
		} else {
			return d;
		}
	} else {
		return d;
	}
}

eiFORCEINLINE void lerp(eiScalar *r, const eiScalar a, const eiScalar b, const eiScalar t)
{
	*r = a + (b - a) * t;
}

eiFORCEINLINE void find_sorted_no_minmax(eiScalar *no_min, eiScalar *no_max,
										 const eiScalar min1, const eiScalar max1,
										 const eiScalar min2, const eiScalar max2)
{
	if (max1 < min2) {
		*no_min = max1;
		*no_max = min2;
	} else if (max2 < min1) {
		*no_min = max2;
		*no_max = min1;
	} else {
		if (min1 < min2) {
			*no_min = min2;
		} else {
			*no_min = min1;
		}
		if (max1 < max2) {
			*no_max = max1;
		} else {
			*no_max = max2;
		}
	}
}

eiFORCEINLINE eiBool is_hit_1d(const eiScalar amax, const eiScalar amin,
							   const eiScalar bmax, const eiScalar bmin,
							   eiScalar *hit_max, eiScalar *hit_min)
{
	if (amin > bmax || bmin > amax) {
		return eiFALSE;
	} else {
		find_sorted_no_minmax(hit_min, hit_max, amin, amax, bmin, bmax);
		return eiTRUE;
	}
}

eiFORCEINLINE void scan_chamber_bound(eiScalar *x0, eiScalar *x1,
									  const eiScalar y,
									  const eiScalar xmin, const eiScalar xmax,
									  const eiScalar ymin, const eiScalar ymax,
									  const eiScalar r)
{
	if (y < ymin)
	{
		eiScalar a, b;

		a = ymin - y;

		if (a > r)
		{
			*x0 = xmin;
			*x1 = xmax;
		}
		else
		{
			b = sqrtf(r * r - a * a);

			*x0 = xmin - b;
			*x1 = xmax + b;
		}
	}
	else if (y > ymax)
	{
		eiScalar a, b;

		a = y - ymax;

		if (a > r)
		{
			*x0 = xmin;
			*x1 = xmax;
		}
		else
		{
			b = sqrtf(r * r - a * a);

			*x0 = xmin - b;
			*x1 = xmax + b;
		}
	}
	else
	{
		*x0 = xmin - r;
		*x1 = xmax + r;
	}
}

eiFORCEINLINE eiScalar boxFilter(const eiScalar x, const eiScalar y,
								 const eiScalar xwidth, const eiScalar ywidth)
{
	return 1.0f;
}

eiFORCEINLINE eiScalar triangleFilter(const eiScalar x, const eiScalar y,
									  const eiScalar xwidth, const eiScalar ywidth)
{
	return ((1.0f - absf(x)) / (xwidth * 0.5f))
			* ((1.0f - absf(y)) / (ywidth * 0.5f));
}

eiFORCEINLINE eiScalar catmullRomFilter(const eiScalar x, const eiScalar y,
										const eiScalar xwidth, const eiScalar ywidth)
{
	eiScalar r2 = x * x + y * y;
	eiScalar r = sqrtf(r2);
	return (r >= 2.0f) ? 0.0f :
			(r < 1.0f) ? (3.0f * r * r2 - 5.0f * r2 + 2.0f) :
			(-r * r2 + 5.0f * r2 - 8.0f * r + 4.0f);
}

eiFORCEINLINE eiScalar gaussianFilter(eiScalar x, eiScalar y,
									  const eiScalar xwidth, const eiScalar ywidth)
{
	x *= 2.0f / xwidth;
	y *= 2.0f / ywidth;
	return expf(-2.0f * (x * x + y * y));
}

eiFORCEINLINE eiScalar sincFilter(const eiScalar x, const eiScalar y,
								  const eiScalar xwidth, const eiScalar ywidth)
{
	eiScalar s, t;
	if (x > -0.001f && x < 0.001f) {
		s = 1.0f;
	} else {
		s = sinf(x) / x;
	}
	if (y > -0.001f && y < 0.001f) {
		t = 1.0f;
	} else {
		t = sinf(y) / y;
	}
	return s * t;
}

eiFORCEINLINE void find_minmax(eiScalar *min, eiScalar *max,
							   const eiScalar a, const eiScalar b, const eiScalar c)
{
	if (a > b) {
		*max = a;
		*min = b;
	} else {
		*max = b;
		*min = a;
	}
	if (c > *max) {
		*max = c;
	} else if (c < *min) {
		*min = c;
	}
}

eiFORCEINLINE void find_sorted_minmax(eiScalar *min, eiScalar *max,
									  const eiScalar min1, const eiScalar max1,
									  const eiScalar min2, const eiScalar max2)
{
	if (max1 < min2) {
		*min = min1;
		*max = max2;
	} else if (max2 < min1) {
		*min = min2;
		*max = max1;
	} else {
		if (min1 < min2) {
			*min = min1;
		} else {
			*min = min2;
		}
		if (max1 < max2) {
			*max = max2;
		} else {
			*max = max1;
		}
	}
}

eiFORCEINLINE void find_minmax4(eiScalar *min, eiScalar *max,
							   const eiScalar a, const eiScalar b,
							   const eiScalar c, const eiScalar d)
{
	eiScalar max1, min1, max2, min2;
	if (a > b) {
		max1 = a;
		min1 = b;
	} else {
		max1 = b;
		min1 = a;
	}
	if (c > d) {
		max2 = c;
		min2 = d;
	} else {
		max2 = d;
		min2 = c;
	}
	find_sorted_minmax(min, max, min1, max1, min2, max2);
}

#ifdef __cplusplus
}
#endif

#endif
