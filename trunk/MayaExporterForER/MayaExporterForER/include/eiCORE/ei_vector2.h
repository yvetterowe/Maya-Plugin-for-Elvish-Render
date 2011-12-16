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

#ifndef EI_VECTOR2_H
#define EI_VECTOR2_H

#include <eiCORE/ei_util.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eiVector2 {
	union {
		struct {
			eiScalar	x;
			eiScalar	y;
		};
		eiScalar		comp[2];
	};
} eiVector2;

eiFORCEINLINE void ei_byteswap_vector2(eiVector2 *vec)
{
	ei_byteswap_scalar(&vec->x);
	ei_byteswap_scalar(&vec->y);
}

eiFORCEINLINE void initv2(eiVector2 *v)
{
	v->x = 0.0f;
	v->y = 0.0f;
}

eiFORCEINLINE void setv2(eiVector2 *v, 
								  const eiScalar vx, const eiScalar vy)
{
	v->x = vx;
	v->y = vy;
}

eiFORCEINLINE void movv2(eiVector2 *a, const eiVector2 *b)
{
	memcpy(a, b, sizeof(eiVector2));
}

eiFORCEINLINE eiInt cmpv2(const eiVector2 *a, const eiVector2 *b)
{
	return memcmp(a, b, sizeof(eiVector2));
}

eiFORCEINLINE void setv2f(eiVector2 *v, const eiScalar s)
{
	v->x = s;
	v->y = s;
}

eiFORCEINLINE void mulvf2i(eiVector2 *vec, const eiScalar f)
{
	vec->x *= f;
	vec->y *= f;
}

eiFORCEINLINE void mulvf2(eiVector2 *r, const eiVector2 *vec, const eiScalar f)
{
	movv2(r, vec);
	mulvf2i(r, f);
}

eiFORCEINLINE eiScalar dot2(const eiVector2 *a, const eiVector2 *b)
{
	return (a->x * b->x + a->y * b->y);
}

eiFORCEINLINE void mul2i(eiVector2 *a, const eiVector2 *b)
{
	a->x *= b->x;
	a->y *= b->y;
}

eiFORCEINLINE void mul2(eiVector2 *r, const eiVector2 *a, const eiVector2 *b)
{
	r->x = a->x * b->x;
	r->y = a->y * b->y;
}

eiFORCEINLINE void add2i(eiVector2 *a, const eiVector2 *b)
{
	a->x += b->x;
	a->y += b->y;
}

eiFORCEINLINE void add2(eiVector2 *r, const eiVector2 *a, const eiVector2 *b)
{
	r->x = a->x + b->x;
	r->y = a->y + b->y;
}

eiFORCEINLINE void sub2i(eiVector2 *a, const eiVector2 *b)
{
	a->x -= b->x;
	a->y -= b->y;
}

eiFORCEINLINE void sub2(eiVector2 *r, const eiVector2 *a, const eiVector2 *b)
{
	r->x = a->x - b->x;
	r->y = a->y - b->y;
}

eiFORCEINLINE eiScalar len2(const eiVector2 *vec)
{
	return sqrtf(dot2(vec, vec));
}

eiFORCEINLINE eiScalar dist2(const eiVector2 *a, const eiVector2 *b)
{
	eiVector2 c;
	sub2(&c, b, a);
	return len2(&c);
}

eiFORCEINLINE void normalize2i(eiVector2 *vec)
{
	eiScalar l = invsqrtf(dot2(vec, vec));
	mulvf2i(vec, l);
}

eiFORCEINLINE eiScalar normalize_with_len2(eiVector2 *vec)
{
	eiScalar l = MAX(len2(vec), eiSCALAR_EPS);
	mulvf2i(vec, 1.0f / l);
	return l;
}

eiFORCEINLINE void Fast_normalize2(eiVector2 *vec)
{
	eiScalar l = Fast_invsqrt(dot2(vec, vec));
	mulvf2i(vec, l);
}

eiFORCEINLINE void lerp2(eiVector2 *r, const eiVector2 *a, const eiVector2 *b, const eiScalar t)
{
	r->x = a->x + (b->x - a->x) * t;
	r->y = a->y + (b->y - a->y) * t;
}

#ifdef __cplusplus
}
#endif

#endif
