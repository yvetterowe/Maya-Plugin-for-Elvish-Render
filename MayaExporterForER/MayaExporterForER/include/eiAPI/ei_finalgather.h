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
 
#ifndef EI_FINALGATHER_H
#define EI_FINALGATHER_H

/** \brief Progressive final gathering.
 * \file ei_finalgather.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_map.h>
#include <eiAPI/ei_state.h>
#include <eiAPI/ei_raytracer.h>
#include <eiAPI/ei_nodesys.h>
#include <eiCORE/ei_rgbe.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EI_FG_ERROR_COEFF			2.0f
#define EI_FG_MIN_INTERP_POINTS		5

/** \brief Hemisphere sample for filtering */
typedef struct eiHemisphereSample {
	eiVector		color;
    eiScalar		tan_theta;
    eiScalar		R;
} eiHemisphereSample;

void ei_hemisphere_sample_zero_item(void *item, const eiInt item_size);
void ei_hemisphere_sample_add_item(void *item, void *value, const eiInt item_size);
void ei_hemisphere_sample_mul_item(void *item, const eiScalar scale, const eiInt item_size);

/** \brief Irradiance gradient */
typedef struct eiIrradianceGradient {
	eiVector		r;
	eiVector		g;
	eiVector		b;
} eiIrradianceGradient;

void ei_irrad_grad_init(eiIrradianceGradient *grad);

/** \brief Irradiance value */
typedef struct eiIrradiance {
	eiMapNode				base;
	eiVector				Ni;				/* 12 */
	eiRGBE					Ei;				/*  4 */
	eiRGBE					GradR_Ei[3];	/* 12 */
	eiRGBE					GradT_Ei[3];	/* 12 */
	eiScalar				inv_Ri;			/*  4 */
} eiIrradiance;

/* for internal use only */
void byteswap_irradiance(eiDatabase *db, void *ptr, const eiUint size);

void ei_irrad_init(
	eiIrradiance *irrad, 
	const eiVector *Pi, 
	const eiVector *Ni, 
	const eiVector *Ei, 
	const eiIrradianceGradient *GradR_Ei, 
	const eiIrradianceGradient *GradT_Ei, 
	const eiScalar inv_Ri);
void ei_irrad_copy(
	eiIrradiance *irrad, 
	const eiIrradiance *value);

/** \brief Irradiance lookup condition */
typedef struct eiIrradianceCondition {
	eiVector	P;
	eiVector	N;
	eiScalar	A;
} eiIrradianceCondition;

eiBool ei_irrad_cond_proc(
	const eiMapNode *node, 
	const eiScalar R2, 
	void *param);

/** \brief Irradiance force lookup condition */
typedef struct eiIrradianceForceCondition {
	eiVector	P;
	eiVector	N;
	eiScalar	proj_pixel_area;
} eiIrradianceForceCondition;

eiBool ei_irrad_force_cond_proc(
	const eiMapNode *node, 
	const eiScalar R2, 
	void *param);

void ei_irrad_cache_find(
	eiDatabase *db, 
	const eiTag tag, 
	eiVector *L, 
	const eiVector *P, 
	const eiVector *N, 
	const eiScalar A, 
	const eiScalar max_dist, 
	const eiUint gather_points, 
	eiScalar * const w, 
	eiUint * const numAvailablePoints);
void ei_irrad_cache_force_interp(
	eiDatabase *db, 
	const eiTag tag, 
	eiVector *L, 
	const eiVector *P, 
	const eiVector *N, 
	const eiScalar A, 
	const eiScalar max_dist, 
	const eiUint gather_points, 
	eiScalar * const w, 
	eiUint * const numAvailablePoints);

/** \brief Shoot a final gather ray to do intersection test against the scene. */
eiBool ei_rt_trace_finalgather(
	eiRayTracer *rt, 
	eiNodeSystem *nodesys, 
	eiState *state);
/** \brief Sample a hemisphere for a final gather point. */
void ei_sample_finalgather(
	eiVector *color, 
	eiState *state);

#ifdef __cplusplus
}
#endif

#endif
