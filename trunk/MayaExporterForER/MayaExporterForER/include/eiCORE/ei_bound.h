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

#ifndef EI_BOUND_H
#define EI_BOUND_H

#include <eiCORE/ei_util.h>
#include <eiCORE/ei_vector.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eiBound {
	union {
		struct {
			eiScalar	xmin;
			eiScalar	ymin;
			eiScalar	zmin;
			eiScalar	xmax;
			eiScalar	ymax;
			eiScalar	zmax;
		};
		struct {
			eiVector	min;
			eiVector	max;
		};
	};
} eiBound;

eiFORCEINLINE void ei_byteswap_bound(eiBound *bound)
{
	ei_byteswap_vector(&bound->min);
	ei_byteswap_vector(&bound->max);
}

eiFORCEINLINE void initb(eiBound *r)
{
	r->min.x = eiMAX_SCALAR;
	r->min.y = eiMAX_SCALAR;
	r->min.z = eiMAX_SCALAR;
	r->max.x = - eiMAX_SCALAR;
	r->max.y = - eiMAX_SCALAR;
	r->max.z = - eiMAX_SCALAR;
}

eiFORCEINLINE void setb(eiBound *r, 
								const eiScalar xmin, const eiScalar xmax, 
								const eiScalar ymin, const eiScalar ymax, 
								const eiScalar zmin, const eiScalar zmax)
{
	r->min.x = xmin;
	r->min.y = ymin;
	r->min.z = zmin;
	r->max.x = xmax;
	r->max.y = ymax;
	r->max.z = zmax;
}

eiFORCEINLINE void movb(eiBound *a, const eiBound *b)
{
	memcpy(a, b, sizeof(eiBound));
}

eiFORCEINLINE eiInt cmpb(const eiBound *a, const eiBound *b)
{
	return memcmp(a, b, sizeof(eiBound));
}

eiFORCEINLINE void addb(eiBound *r, const eiBound *b)
{
	if (r->min.x > b->min.x) {
		r->min.x = b->min.x;
	}
	if (r->max.x < b->max.x) {
		r->max.x = b->max.x;
	}
	if (r->min.y > b->min.y) {
		r->min.y = b->min.y;
	}
	if (r->max.y < b->max.y) {
		r->max.y = b->max.y;
	}
	if (r->min.z > b->min.z) {
		r->min.z = b->min.z;
	}
	if (r->max.z < b->max.z) {
		r->max.z = b->max.z;
	}
}

eiFORCEINLINE void addbv(eiBound *r, const eiVector *v)
{
	if (r->min.x > v->x) {
		r->min.x = v->x;
	}
	if (r->max.x < v->x) {
		r->max.x = v->x;
	}
	if (r->min.y > v->y) {
		r->min.y = v->y;
	}
	if (r->max.y < v->y) {
		r->max.y = v->y;
	}
	if (r->min.z > v->z) {
		r->min.z = v->z;
	}
	if (r->max.z < v->z) {
		r->max.z = v->z;
	}
}

eiFORCEINLINE void addbf(eiBound *r, const eiScalar v)
{
	r->min.x -= v;
	r->max.x += v;
	r->min.y -= v;
	r->max.y += v;
	r->min.z -= v;
	r->max.z += v;
}

eiFORCEINLINE void setbv(eiBound *r, const eiVector *v)
{
	r->min.x = r->max.x = v->x;
	r->min.y = r->max.y = v->y;
	r->min.z = r->max.z = v->z;
}

eiFORCEINLINE eiBool bcover(const eiBound *b, const eiVector *v)
{
	return (v->x >= b->min.x && v->x <= b->max.x && 
		v->y >= b->min.y && v->y <= b->max.y && 
		v->z >= b->min.z && v->z <= b->max.z);
}

eiFORCEINLINE void bdiag(eiVector *r, const eiBound *a)
{
	r->x = a->max.x - a->min.x;
	r->y = a->max.y - a->min.y;
	r->z = a->max.z - a->min.z;
}

eiFORCEINLINE eiScalar bdiag2(const eiBound *a)
{
	eiVector d;

	bdiag(&d, a);

	return dot(&d, &d);
}

eiFORCEINLINE void bcenter(eiVector *r, const eiBound *a)
{
	r->x = (a->min.x + a->max.x) * 0.5f;
	r->y = (a->min.y + a->max.y) * 0.5f;
	r->z = (a->min.z + a->max.z) * 0.5f;
}

eiFORCEINLINE eiScalar barea(const eiBound *a)
{
	eiVector d;

	bdiag(&d, a);

	return varea(&d);
}

eiFORCEINLINE eiScalar bvol(const eiBound *a)
{
	eiVector d;

	bdiag(&d, a);

	return (d.x * d.y * d.z);
}

eiFORCEINLINE eiInt bmax_axis(const eiBound *a)
{
	eiVector d;

	bdiag(&d, a);

	return vmax_axis(&d);
}

eiFORCEINLINE eiBool is_hit_3d(const eiBound *a, const eiBound *b, eiBound *hit)
{
	if (is_hit_1d(a->xmax, a->xmin, b->xmax, b->xmin, &hit->xmax, &hit->xmin) && 
		is_hit_1d(a->ymax, a->ymin, b->ymax, b->ymin, &hit->ymax, &hit->ymin) && 
		is_hit_1d(a->zmax, a->zmin, b->zmax, b->zmin, &hit->zmax, &hit->zmin)) {
		return eiTRUE;
	} else {
		return eiFALSE;
	}
}

eiFORCEINLINE eiBool test_hit_3d(const eiBound *a, const eiBound *b)
{
	if (a->xmax < b->xmin || a->xmin > b->xmax || 
		a->ymax < b->ymin || a->ymin > b->ymax || 
		a->zmax < b->zmin || a->zmin > b->zmax) {
		return eiFALSE;
	} else {
		return eiTRUE;
	}
}

eiFORCEINLINE eiBool intersect_box(const eiVector *pos, const eiVector *inv_dir, 
								   const eiBound *box, 
								   eiScalar *tmin, eiScalar *tmax)
{
	eiScalar tymin, tymax, tzmin, tzmax;

	if (inv_dir->x < 0.0f) {
		*tmin = (box->xmax - pos->x) * inv_dir->x;
		*tmax = (box->xmin - pos->x) * inv_dir->x;
	} else {
		*tmin = (box->xmin - pos->x) * inv_dir->x;
		*tmax = (box->xmax - pos->x) * inv_dir->x;
	}

	if (inv_dir->y < 0.0f) {
		tymin = (box->ymax - pos->y) * inv_dir->y;
		tymax = (box->ymin - pos->y) * inv_dir->y;
	} else {
		tymin = (box->ymin - pos->y) * inv_dir->y;
		tymax = (box->ymax - pos->y) * inv_dir->y;
	}

	if (tymin > *tmin) {
		*tmin = tymin;
	}
	if (tymax < *tmax) {
		*tmax = tymax;
	}

	if (inv_dir->z < 0.0f) {
		tzmin = (box->zmax - pos->z) * inv_dir->z;
		tzmax = (box->zmin - pos->z) * inv_dir->z;
	} else {
		tzmin = (box->zmin - pos->z) * inv_dir->z;
		tzmax = (box->zmax - pos->z) * inv_dir->z;
	}

	if (tzmin > *tmin) {
		*tmin = tzmin;
	}
	if (tzmax < *tmax) {
		*tmax = tzmax;
	}

	return (*tmax >= *tmin);
}

#define BELONG_MIDDLE			0
#define BELONG_LEFT				1
#define BELONG_RIGHT			2
#define BELONG_BOTH				3
#define TEST_LINE_PLANE_EPS		eiSCALAR_EPS

eiFORCEINLINE eiInt testLinePlane(const eiVector *a, const eiVector *b, 
								  const eiInt axis, const eiScalar plane, 
								  eiBound *left_box, eiBound *right_box)
{
	if (absf(a->comp[axis] - plane) < TEST_LINE_PLANE_EPS && 
		absf(b->comp[axis] - plane) < TEST_LINE_PLANE_EPS)
	{
		addbv(left_box, a);
		addbv(left_box, b);
		addbv(right_box, a);
		addbv(right_box, b);

		return BELONG_MIDDLE;
	}
	else
	{
		if (a->comp[axis] <= plane)
		{
			if (b->comp[axis] <= plane)
			{
				addbv(left_box, a);
				addbv(left_box, b);

				return BELONG_LEFT;
			}
			else
			{
				if (absf(a->comp[axis] - b->comp[axis]) > TEST_LINE_PLANE_EPS)
				{
					eiVector	dir;
					eiScalar	t;

					addbv(left_box, a);
					addbv(right_box, b);

					sub(&dir, b, a);
					t = (plane - a->comp[axis] ) / dir.comp[axis];
					mulvfi(&dir, t);
					addi(&dir, a);
					addbv(left_box, &dir);
					addbv(right_box, &dir);
				}
				else
				{
					addbv(left_box, a);
					addbv(left_box, b);
					addbv(right_box, a);
					addbv(right_box, b);
				}

				return BELONG_BOTH;
			}
		}
		else
		{
			if (b->comp[axis] <= plane)
			{
				if (absf(a->comp[axis] - b->comp[axis]) > TEST_LINE_PLANE_EPS)
				{
					eiVector	dir;
					eiScalar	t;

					addbv(left_box, b);
					addbv(right_box, a);

					sub(&dir, a, b);
					t = (plane - b->comp[axis]) / dir.comp[axis];
					mulvfi(&dir, t);
					addi(&dir, b);
					addbv(left_box, &dir);
					addbv(right_box, &dir);
				}
				else
				{
					addbv(left_box, a);
					addbv(left_box, b);
					addbv(right_box, a);
					addbv(right_box, b);
				}

				return BELONG_BOTH;
			}
			else
			{
				addbv(right_box, a);
				addbv(right_box, b);

				return BELONG_RIGHT;
			}
		}
	}
}

eiFORCEINLINE void get_sphere_from_bound(eiVector *center, eiScalar *radius, const eiBound *box)
{
	eiScalar lx, ly, lz;

	center->x = (box->xmin + box->xmax) * 0.5f;
	center->y = (box->ymin + box->ymax) * 0.5f;
	center->z = (box->zmin + box->zmax) * 0.5f;

	lx = box->xmax - center->x;
	ly = box->ymax - center->y;
	lz = box->zmax - center->z;

	*radius = sqrtf(lx * lx + ly * ly + lz * lz);
}

#ifdef __cplusplus
}
#endif

#endif
