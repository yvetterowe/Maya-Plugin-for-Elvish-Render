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

#ifndef EI_VECTOR_H
#define EI_VECTOR_H

#include <eiCORE/ei_util.h>
#include <eiCORE/ei_vector2.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eiVector {
	union {
		struct {
			eiScalar	x;
			eiScalar	y;
			eiScalar	z;
		};
		struct {
			eiScalar	r;
			eiScalar	g;
			eiScalar	b;
		};
		struct {
			eiVector2	xy;
			eiScalar	_z;
		};
		eiScalar		comp[3];
	};
} eiVector;

eiFORCEINLINE void ei_byteswap_vector(eiVector *vec)
{
	ei_byteswap_scalar(&vec->x);
	ei_byteswap_scalar(&vec->y);
	ei_byteswap_scalar(&vec->z);
}

eiFORCEINLINE void initv(eiVector *v)
{
	v->x = 0.0f;
	v->y = 0.0f;
	v->z = 0.0f;
}

eiFORCEINLINE void setv(eiVector *v, 
								 const eiScalar vx, const eiScalar vy, const eiScalar vz)
{
	v->x = vx;
	v->y = vy;
	v->z = vz;
}

eiFORCEINLINE void movv(eiVector *a, const eiVector *b)
{
	memcpy(a, b, sizeof(eiVector));
}

eiFORCEINLINE eiInt cmpv(const eiVector *a, const eiVector *b)
{
	return memcmp(a, b, sizeof(eiVector));
}

eiFORCEINLINE eiBool almost_zerov(const eiVector *a, const eiScalar zero)
{
	return (almost_zero(a->x, zero) && almost_zero(a->y, zero) && almost_zero(a->z, zero));
}

eiFORCEINLINE eiBool almost_equalv(const eiVector *a, const eiVector *b, const eiScalar zero)
{
	return (almost_equal(a->x, b->x, zero) && almost_equal(a->y, b->y, zero) && almost_equal(a->z, b->z, zero));
}

eiFORCEINLINE void setvf(eiVector *v, const eiScalar s)
{
	v->x = s;
	v->y = s;
	v->z = s;
}

eiFORCEINLINE eiScalar varea(const eiVector *v)
{
	return ((v->x * v->y + v->y * v->z + v->z * v->x) * 2.0f);
}

eiFORCEINLINE eiInt vmax_axis(const eiVector *v)
{
	eiInt axis = Z_AXIS;
	if (v->x > v->y && v->x > v->z) {
		axis = X_AXIS;
	} else if (v->y > v->z) {
		axis = Y_AXIS;
	}
	return axis;
}

eiFORCEINLINE void mulvfi(eiVector *vec, const eiScalar f)
{
	vec->x *= f;
	vec->y *= f;
	vec->z *= f;
}

eiFORCEINLINE void mulvf(eiVector *r, const eiVector *vec, const eiScalar f)
{
	movv(r, vec);
	mulvfi(r, f);
}

eiFORCEINLINE void cross(eiVector *rv, const eiVector *a, const eiVector *b)
{
	rv->x = a->y * b->z - a->z * b->y;
    rv->y = a->z * b->x - a->x * b->z;
    rv->z = a->x * b->y - a->y * b->x;
}

eiFORCEINLINE eiScalar dot(const eiVector *a, const eiVector *b)
{
	return (a->x * b->x + a->y * b->y + a->z * b->z);
}

eiFORCEINLINE void muli(eiVector *a, const eiVector *b)
{
	a->x *= b->x;
	a->y *= b->y;
	a->z *= b->z;
}

eiFORCEINLINE void mul(eiVector *r, const eiVector *a, const eiVector *b)
{
	r->x = a->x * b->x;
	r->y = a->y * b->y;
	r->z = a->z * b->z;
}

eiFORCEINLINE void addi(eiVector *a, const eiVector *b)
{
	a->x += b->x;
	a->y += b->y;
	a->z += b->z;
}

eiFORCEINLINE void add(eiVector *r, const eiVector *a, const eiVector *b)
{
	r->x = a->x + b->x;
	r->y = a->y + b->y;
	r->z = a->z + b->z;
}

eiFORCEINLINE void subi(eiVector *a, const eiVector *b)
{
	a->x -= b->x;
	a->y -= b->y;
	a->z -= b->z;
}

eiFORCEINLINE void sub(eiVector *r, const eiVector *a, const eiVector *b)
{
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
}

eiFORCEINLINE void neg(eiVector *r, const eiVector *v)
{
	r->x = -v->x;
	r->y = -v->y;
	r->z = -v->z;
}

eiFORCEINLINE eiScalar len(const eiVector *vec)
{
	return sqrtf(dot(vec, vec));
}

eiFORCEINLINE eiScalar dist(const eiVector *a, const eiVector *b)
{
	eiVector c;
	sub(&c, b, a);
	return len(&c);
}

eiFORCEINLINE eiScalar distsq(const eiVector *a, const eiVector *b)
{
	eiScalar dst1, dst2;
	dst1 = a->x - b->x;
	dst2 = dst1 * dst1;
	dst1 = a->y - b->y;
	dst2 += dst1 * dst1;
	dst1 = a->z - b->z;
	dst2 += dst1 * dst1;
	return dst2;
}

eiFORCEINLINE void normalizei(eiVector *vec)
{
	eiScalar l = invsqrtf(MAX(dot(vec, vec), eiSCALAR_EPS));
	mulvfi(vec, l);
}

eiFORCEINLINE eiScalar normalize_with_len(eiVector *vec)
{
	eiScalar l = MAX(len(vec), eiSCALAR_EPS);
	mulvfi(vec, 1.0f / l);
	return l;
}

eiFORCEINLINE void Fast_normalize(eiVector *vec)
{
	eiScalar l = Fast_invsqrt(MAX(dot(vec, vec), eiSCALAR_EPS));
	mulvfi(vec, l);
}

eiFORCEINLINE void lerp3(eiVector *r, const eiVector *a, const eiVector *b, const eiScalar t)
{
	r->x = a->x + (b->x - a->x) * t;
	r->y = a->y + (b->y - a->y) * t;
	r->z = a->z + (b->z - a->z) * t;
}

#define INTERP_FAST(r, a, d, t)		(r) = (a) + (d) * (t);
#define INTERP_FAST3(r, a, d, t)	(r)->x = (a)->x + (d)->x * (t);\
									(r)->y = (a)->y + (d)->y * (t);\
									(r)->z = (a)->z + (d)->z * (t);

eiFORCEINLINE void slerp(eiVector *r, const eiVector *a, const eiVector *b, const eiScalar t)
{
	/* assume a, b are both unit vectors... */
	eiScalar cos_theta = dot(a, b);

	if (absf(cos_theta) > 0.9995f)
	{
		/* if the inputs are too close for comfort, linearly interpolate
           and normalize the result. */
		lerp3(r, a, b, t);
		normalizei(r);
	}
	else
	{
		eiScalar theta;
		eiVector p0, p1;

		/* stay within domain of acos() */
		clampi(cos_theta, -1.0f, 1.0f);

		theta = acosf(cos_theta);

		movv(&p0, a);
		movv(&p1, b);
		mulvfi(&p0, sinf((1.0f - t) * theta));
		mulvfi(&p1, sinf(t * theta));
		add(r, &p0, &p1);
		mulvfi(r, 1.0f / sinf(theta));
	}
}

eiFORCEINLINE eiScalar average(const eiVector *a)
{
	return (a->x + a->y + a->z) * (1.0f / 3.0f);
}

eiFORCEINLINE void ortho_basis(const eiVector *N, eiVector *X, eiVector *Y)
{
	eiScalar min = eiMAX_SCALAR;
	eiInt min_index = -1;

	/* find the minor axis of the ray */
	STMT(
		eiScalar tmp = absf(N->comp[0]);
		if (tmp < min) {
			min = tmp;
			min_index = 0;
		}
	)
	STMT(
		eiScalar tmp = absf(N->comp[1]);
		if (tmp < min) {
			min = tmp;
			min_index = 2;
		}
	)
	STMT(
		eiScalar tmp = absf(N->comp[2]);
		if (tmp < min) {
			min = tmp;
			min_index = 2;
		}
	)

	/* it's very IMPORTANT to keep the result identical when 
	   different axis is chosen! */
	switch (min_index) {
	case 0:
		setv(Y, 0.0f, -N->comp[2], N->comp[1]);
		Fast_normalize(Y);
		cross(X, Y, N);
		Fast_normalize(X);
		break;
	case 1:
		setv(X, -N->comp[2], 0.0f, N->comp[0]);
		Fast_normalize(X);
		cross(Y, X, N);
		Fast_normalize(Y);
		break;
	case 2:
		setv(X, -N->comp[1], N->comp[0], 0.0f);
		Fast_normalize(X);
		cross(Y, X, N);
		Fast_normalize(Y);
		break;
	}
}

eiFORCEINLINE eiScalar det3x3(const eiVector *a, const eiVector *b, const eiVector *c)
{
   return (a->x * (b->y * c->z - b->z * c->y)
			- b->x * (a->y * c->z - a->z * c->y)
			+ c->x * (a->y * b->z - a->z * b->y));
}

eiFORCEINLINE eiScalar tri_area(const eiVector *a, const eiVector *b, const eiVector *c)
{
	return det3x3(a, b, c);
}

eiFORCEINLINE eiScalar calc_tri_area(const eiVector *pos1, const eiVector *pos2, const eiVector *pos3)
{
	eiVector	fpos1, fpos2, fpos3;

	movv(&fpos1, pos1);
	movv(&fpos2, pos2);
	movv(&fpos3, pos3);

	if (almost_zero(fpos1.x, eiSCALAR_EPS) && 
		almost_zero(fpos2.x, eiSCALAR_EPS) && 
		almost_zero(fpos3.x, eiSCALAR_EPS))
	{
		fpos1.x = 1.0f;
		fpos2.x = 1.0f;
		fpos3.x = 1.0f;
	}

	if (almost_zero(fpos1.y, eiSCALAR_EPS) && 
		almost_zero(fpos2.y, eiSCALAR_EPS) && 
		almost_zero(fpos3.y, eiSCALAR_EPS))
	{
		fpos1.y = 1.0f;
		fpos2.y = 1.0f;
		fpos3.y = 1.0f;
	}

	if (almost_zero(fpos1.z, eiSCALAR_EPS) && 
		almost_zero(fpos2.z, eiSCALAR_EPS) && 
		almost_zero(fpos3.z, eiSCALAR_EPS))
	{
		fpos1.z = 1.0f;
		fpos2.z = 1.0f;
		fpos3.z = 1.0f;
	}

	return tri_area(&fpos1, &fpos2, &fpos3) * 0.5f;
}

#define TRI_AREA_2D(a, b, c) (((c)->x - (a)->x) * ((b)->y - (a)->y) - ((b)->x - (a)->x) * ((c)->y - (a)->y))

eiFORCEINLINE void dir_to_angle(const eiVector *dir, eiScalar *theta, eiScalar *phi)
{
	*theta = acosf(dir->z);						/* return value in [0...Pi] */
	*phi = atan2f(dir->y, dir->x);				/* return value in [-Pi...Pi] */
	if (*phi < 0.0f) {
		*phi = *phi + 2.0f * (eiScalar)eiPI;	/* make value in [0...2Pi] */
	}
}

eiFORCEINLINE void angle_to_dir(const eiScalar theta, const eiScalar phi, eiVector *dir)
{
	eiScalar sin_theta = sinf(theta);
	dir->x = sin_theta * cosf(phi);
	dir->y = sin_theta * sinf(phi);
	dir->z = cosf(theta);
}

eiFORCEINLINE void get_unnormalized_normal(const eiVector *v1, const eiVector *v2, 
										   const eiVector *v3, eiVector *normal)
{
	/* get normal vector of a triangle, remember that we are 
	   in right-handed coordinate system by default and clockwise 
	   vertices will form the front face.
	   unnormalized version is faster. */
	eiVector t1, t2;
	sub(&t1, v2, v1);
	sub(&t2, v3, v1);
	cross(normal, &t1, &t2);
}

eiFORCEINLINE void interp_normal(eiVector *N, 
								const eiVector *normal1, 
								const eiVector *normal2, 
								const eiVector *normal3, 
								const eiVector *bary)
{
	eiVector N1, N2;

	movv(&N1, normal1);
	mulvfi(&N1, bary->x);

	movv(&N2, normal2);
	mulvfi(&N2, bary->y);

	addi(&N1, &N2);

	movv(&N2, normal3);
	mulvfi(&N2, bary->z);

	addi(&N1, &N2);
	normalizei(&N1);

	movv(N, &N1);
}

/* unnormalized version */
eiFORCEINLINE void interp_point(eiVector *N, 
								const eiVector *normal1, 
								const eiVector *normal2, 
								const eiVector *normal3, 
								const eiVector *bary)
{
	eiVector N1, N2;

	movv(&N1, normal1);
	mulvfi(&N1, bary->x);

	movv(&N2, normal2);
	mulvfi(&N2, bary->y);

	addi(&N1, &N2);

	movv(&N2, normal3);
	mulvfi(&N2, bary->z);

	addi(&N1, &N2);

	movv(N, &N1);
}

eiFORCEINLINE eiScalar point_plane_dist(const eiVector *P, const eiVector *N, const eiVector *V)
{
	eiVector w;
	sub(&w, P, V);
	return dot(N, &w);
}

eiFORCEINLINE void point_on_plane(eiVector *result, const eiVector *P, const eiVector *N, const eiVector *V)
{
	movv(result, N);
	mulvfi(result, - point_plane_dist(P, N, V));
	addi(result, P);
}

eiFORCEINLINE void interp_scalar(eiScalar *p, 
								 const eiScalar a, 
								 const eiScalar b, 
								 const eiScalar c, 
								 const eiVector *bary)
{
	*p = bary->x * a + bary->y * b + bary->z * c;
}

eiFORCEINLINE void interp_int(eiInt *p, 
							  const eiInt a, 
							  const eiInt b, 
							  const eiInt c, 
							  const eiVector *bary)
{
	*p = roundf(bary->x * (eiScalar)a + bary->y * (eiScalar)b + bary->z * (eiScalar)c);
}

eiFORCEINLINE void interp_bool(eiBool *p, 
								const eiBool a, 
								const eiBool b, 
								const eiBool c, 
								const eiVector *bary)
{
	*p = (roundf(bary->x * (eiScalar)a + bary->y * (eiScalar)b + bary->z * (eiScalar)c) != 0);
}

/** \brief Interpolate 3 points of any type by barycentric coordinates. */
eiFORCEINLINE void interp_any(
	eiByte *p, 
	const eiByte *a, const eiByte *b, const eiByte *c, 
	const eiVector *bary, 
	const eiInt type)
{
	switch (type)
	{
	case EI_DATA_TYPE_INT:
		interp_int((eiInt *)p, *((eiInt *)a), *((eiInt *)b), *((eiInt *)c), bary);
		break;
	case EI_DATA_TYPE_BOOL:
		interp_bool((eiBool *)p, *((eiBool *)a), *((eiBool *)b), *((eiBool *)c), bary);
		break;
	case EI_DATA_TYPE_SCALAR:
		interp_scalar((eiScalar *)p, *((eiScalar *)a), *((eiScalar *)b), *((eiScalar *)c), bary);
		break;
	case EI_DATA_TYPE_VECTOR:
		interp_point((eiVector *)p, (eiVector *)a, (eiVector *)b, (eiVector *)c, bary);
		break;
	default:
		/* not supported */
		break;
	}
}

eiFORCEINLINE void lerp_int(eiInt *p, 
							const eiInt a, 
							const eiInt b, 
							const eiScalar t)
{
	*p = roundf((eiScalar)a + ((eiScalar)b - (eiScalar)a) * t);
}

eiFORCEINLINE void lerp_bool(eiBool *p, 
							 const eiBool a, 
							 const eiBool b, 
							 const eiScalar t)
{
	*p = (roundf((eiScalar)a + ((eiScalar)b - (eiScalar)a) * t) != 0);
}

/** \brief Linearly interpolate 2 points of any type. */
eiFORCEINLINE void lerp_any(
	eiByte *p, 
	const eiByte *a, const eiByte *b, 
	const eiScalar t, 
	const eiInt type)
{
	switch (type)
	{
	case EI_DATA_TYPE_INT:
		lerp_int((eiInt *)p, *((eiInt *)a), *((eiInt *)b), t);
		break;
	case EI_DATA_TYPE_BOOL:
		lerp_bool((eiBool *)p, *((eiBool *)a), *((eiBool *)b), t);
		break;
	case EI_DATA_TYPE_SCALAR:
		lerp((eiScalar *)p, *((eiScalar *)a), *((eiScalar *)b), t);
		break;
	case EI_DATA_TYPE_VECTOR:
		lerp3((eiVector *)p, (eiVector *)a, (eiVector *)b, t);
		break;
	default:
		/* not supported */
		break;
	}
}

/** \brief Substract 2 points of any type. */
eiFORCEINLINE void sub_any(
	eiByte *p, 
	const eiByte *a, const eiByte *b, 
	const eiInt type)
{
	switch (type)
	{
	case EI_DATA_TYPE_INT:
		*((eiInt *)p) = *((eiInt *)a) - *((eiInt *)b);
		break;
	case EI_DATA_TYPE_BOOL:
		*((eiBool *)p) = *((eiBool *)a) - *((eiBool *)b);
		break;
	case EI_DATA_TYPE_SCALAR:
		*((eiScalar *)p) = *((eiScalar *)a) - *((eiScalar *)b);
		break;
	case EI_DATA_TYPE_VECTOR:
		sub((eiVector *)p, (eiVector *)a, (eiVector *)b);
		break;
	default:
		/* not supported */
		break;
	}
}

#ifdef __cplusplus
}
#endif

#endif
