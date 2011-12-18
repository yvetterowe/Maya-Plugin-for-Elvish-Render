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

#ifndef EI_VECTOR4_H
#define EI_VECTOR4_H

#include <eiCORE/ei_util.h>
#include <eiCORE/ei_vector.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eiVector4 {
	union {
		struct {
			eiScalar	x;
			eiScalar	y;
			eiScalar	z;
			eiScalar	w;
		};
		struct {
			eiScalar	r;
			eiScalar	g;
			eiScalar	b;
			eiScalar	a;
		};
		struct {
			eiVector	xyz;
			eiScalar	__w;
		};
		eiScalar		comp[4];
	};
} eiVector4;

eiFORCEINLINE void ei_byteswap_vector4(eiVector4 *vec)
{
	ei_byteswap_scalar(&vec->x);
	ei_byteswap_scalar(&vec->y);
	ei_byteswap_scalar(&vec->z);
	ei_byteswap_scalar(&vec->w);
}

eiFORCEINLINE void initv4(eiVector4 *v)
{
	v->x = 0.0f;
	v->y = 0.0f;
	v->z = 0.0f;
	v->w = 0.0f;
}

eiFORCEINLINE void setv4(eiVector4 *v, 
								  const eiScalar vx, const eiScalar vy, 
								  const eiScalar vz, const eiScalar vw)
{
	v->x = vx;
	v->y = vy;
	v->z = vz;
	v->w = vw;
}

eiFORCEINLINE void movv4(eiVector4 *a, const eiVector4 *b)
{
	memcpy(a, b, sizeof(eiVector4));
}

eiFORCEINLINE eiInt cmpv4(const eiVector4 *a, const eiVector4 *b)
{
	return memcmp(a, b, sizeof(eiVector4));
}

eiFORCEINLINE void setv4f(eiVector4 *v, const eiScalar s)
{
	v->x = s;
	v->y = s;
	v->z = s;
	v->w = s;
}

eiFORCEINLINE void mulvf4i(eiVector4 *vec, const eiScalar f)
{
	vec->x *= f;
	vec->y *= f;
	vec->z *= f;
	vec->w *= f;
}

eiFORCEINLINE void mulvf4(eiVector4 *r, const eiVector4 *vec, const eiScalar f)
{
	movv4(r, vec);
	mulvf4i(r, f);
}

eiFORCEINLINE eiScalar dot4(const eiVector4 *a, const eiVector4 *b)
{
	return (a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w);
}

eiFORCEINLINE void mul4i(eiVector4 *a, const eiVector4 *b)
{
	a->x *= b->x;
	a->y *= b->y;
	a->z *= b->z;
	a->w *= b->w;
}

eiFORCEINLINE void mul4(eiVector4 *r, const eiVector4 *a, const eiVector4 *b)
{
	r->x = a->x * b->x;
	r->y = a->y * b->y;
	r->z = a->z * b->z;
	r->w = a->w * b->w;
}

eiFORCEINLINE void add4i(eiVector4 *a, const eiVector4 *b)
{
	a->x += b->x;
	a->y += b->y;
	a->z += b->z;
	a->w += b->w;
}

eiFORCEINLINE void add4(eiVector4 *r, const eiVector4 *a, const eiVector4 *b)
{
	r->x = a->x + b->x;
	r->y = a->y + b->y;
	r->z = a->z + b->z;
	r->w = a->w + b->w;
}

eiFORCEINLINE void sub4i(eiVector4 *a, const eiVector4 *b)
{
	a->x -= b->x;
	a->y -= b->y;
	a->z -= b->z;
	a->w -= b->w;
}

eiFORCEINLINE void sub4(eiVector4 *r, const eiVector4 *a, const eiVector4 *b)
{
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
	r->w = a->w - b->w;
}

eiFORCEINLINE void lerp4(eiVector4 *r, const eiVector4 *a, const eiVector4 *b, const eiScalar t)
{
	sub4(r, b, a);
	mulvf4i(r, t);
	add4i(r, a);
}

eiFORCEINLINE void get_normal3(const eiVector *v1, const eiVector *v2, 
							   const eiVector *v3, eiVector *normal)
{
	/* get normal vector of a triangle, remember that we are 
	   in right-handed coordinate system by default and clockwise 
	   vertices will form the front face. */
	eiVector t1, t2;

	sub(&t1, v2, v1);
	sub(&t2, v3, v1);
	cross(normal, &t1, &t2);
	normalizei(normal);
}

eiFORCEINLINE void get_normal(const eiVector *v1, const eiVector *v2, 
							  const eiVector *v3, eiVector4 *normal)
{
	/* get normal vector of a triangle, remember that we are 
	   in right-handed coordinate system by default and clockwise 
	   vertices will form the front face. */
	eiVector t1, t2;

	sub(&t1, v2, v1);
	sub(&t2, v3, v1);
	cross(&normal->xyz, &t1, &t2);
	normalizei(&normal->xyz);
	normal->w = - dot(v1, &normal->xyz);
}

#ifdef __cplusplus
}
#endif

#endif
