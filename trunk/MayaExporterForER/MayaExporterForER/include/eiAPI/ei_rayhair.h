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
 
#ifndef EI_RAYHAIR_H
#define EI_RAYHAIR_H

/** \brief The hair ray-tracing engine.
 * \file ei_rayhair.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_raytracer.h>
#include <eiAPI/ei_object.h>
#include <xmmintrin.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The ray-traceable hair BSP-tree */
typedef struct eiRayHairTree {
	/* source hair object tag */
	eiTag			obj_tag;		/* 4 */
	/* the number of BSP-nodes */
	eiUint			num_nodes;		/* 4 */
	/* the number of padding bytes before GPIT data. */
	eiUint			gpit_padding;	/* 4 */
	/* the bounding box of this hair object in object space */
	eiBound			box;			/* 4 */
} eiRayHairTree;

/** \brief The curve vertex */
typedef struct eiCurveVertex {
	union {
		struct {
			eiScalar	w;
			eiScalar	x;
			eiScalar	y;
			eiScalar	z;
		};
		struct {
			eiScalar	__w;
			eiVector	xyz;
		};
		__m128			m;
	};
} eiCurveVertex;

/** \brief The linear curve with degree 1 */
typedef struct eiCurve1 {
	/* the w component is radius */
	eiCurveVertex	p[2];
	/* the maximum radius */
	eiScalar		max_radius;
} eiCurve1;

/** \brief The quadratic curve with degree 2 */
typedef struct eiCurve2 {
	/* the w component is radius */
	eiCurveVertex	p[3];
	/* the maximum width */
	eiScalar		max_radius;
} eiCurve2;

/** \brief The cubic curve with degree 3 */
typedef struct eiCurve3 {
	/* the w component is radius */
	eiCurveVertex	p[4];
	/* the maximum radius */
	eiScalar		max_radius;
} eiCurve3;

eiAPI void ei_curve1_eval_exported(const eiCurve1 *c, eiVector4 *p, const eiScalar t);
eiAPI void ei_curve2_eval_exported(const eiCurve2 *c, eiVector4 *p, const eiScalar t);
eiAPI void ei_curve3_eval_exported(const eiCurve3 *c, eiVector4 *p, const eiScalar t);

eiAPI void ei_curve1_evalT_exported(const eiCurve1 *c, eiVector *p, const eiScalar t);
eiAPI void ei_curve2_evalT_exported(const eiCurve2 *c, eiVector *p, const eiScalar t);
eiAPI void ei_curve3_evalT_exported(const eiCurve3 *c, eiVector *p, const eiScalar t);

eiAPI void ei_curve1_eval_scalars(const eiInt dim, const eiScalar *p0, const eiScalar *p1, eiScalar * const p, const eiScalar t);
eiAPI void ei_curve2_eval_scalars(const eiInt dim, const eiScalar *p0, const eiScalar *p1, const eiScalar *p2, eiScalar * const p, const eiScalar t);
eiAPI void ei_curve3_eval_scalars(const eiInt dim, const eiScalar *p0, const eiScalar *p1, const eiScalar *p2, const eiScalar *p3, eiScalar * const p, const eiScalar t);

eiAPI void ei_curve1_evalT_scalars(const eiInt dim, const eiScalar *p0, const eiScalar *p1, eiScalar * const p, const eiScalar t);
eiAPI void ei_curve2_evalT_scalars(const eiInt dim, const eiScalar *p0, const eiScalar *p1, const eiScalar *p2, eiScalar * const p, const eiScalar t);
eiAPI void ei_curve3_evalT_scalars(const eiInt dim, const eiScalar *p0, const eiScalar *p1, const eiScalar *p2, const eiScalar *p3, eiScalar * const p, const eiScalar t);

eiAPI void ei_curve1_precompute_exported(eiCurve1 *c);
eiAPI void ei_curve2_precompute_exported(eiCurve2 *c);
eiAPI void ei_curve3_precompute_exported(eiCurve3 *c);

eiAPI eiBool ei_curve1_converge_exported(
	const eiCurve1 *c, 
	eiInt depth, 
	const eiScalar v0, 
	const eiScalar vn, 
	eiScalar *t, 
	eiScalar *z);
eiAPI eiBool ei_curve2_converge_exported(
	const eiCurve2 *c, 
	eiInt depth, 
	const eiScalar v0, 
	const eiScalar vn, 
	eiScalar *t, 
	eiScalar *z);
eiAPI eiBool ei_curve3_converge_exported(
	const eiCurve3 *c, 
	eiInt depth, 
	const eiScalar v0, 
	const eiScalar vn, 
	eiScalar *t, 
	eiScalar *z);

/** \brief Generate hair BSP-tree, for internal use only. */
void generate_ray_hair_tree(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls);

/** \brief Perform intersection test directly with this object. in nearest 
 * mode, the intersection test should return the nearest intersection between 
 * t_near and t_far; in sort mode, it should return all intersections between 
 * t_near and t_far. */
void ei_hair_object_intersect(
	eiObject *obj, 
	const eiTag tessel_tag, 
	const eiIndex tessel_instance_index, 
	const eiIndex parent_bsptree, 
	eiState *state, 
	ei_array *hit_info_array, 
	const eiBool sort_by_distance);

/* for internal use only */
void byteswap_ray_hair_tree(eiDatabase *db, void *data, const eiUint size);

#ifdef __cplusplus
}
#endif

#endif
