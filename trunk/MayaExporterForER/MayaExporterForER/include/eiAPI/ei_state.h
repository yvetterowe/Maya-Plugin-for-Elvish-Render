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
 
#ifndef EI_STATE_H
#define EI_STATE_H

/** \brief The rendering state of a ray bundle.
 * \file ei_state.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_bsp.h>
#include <eiAPI/ei_options.h>
#include <eiAPI/ei_camera.h>
#include <eiCORE/ei_vector.h>
#include <eiCORE/ei_vector2.h>
#include <eiCORE/ei_matrix.h>
#include <eiCORE/ei_slist.h>
#include <eiCORE/ei_table.h>

#ifdef __cplusplus
extern "C" {
#endif

/* absolute ray bias for avoiding self-intersections */
#define EI_RAY_BIAS					0.0025f
/* slope bias scale factor, it should be proportional 
   to the error of intersection point */
#define EI_RAY_BIAS_SCALE			0.02f
/* the maximum number of user scalars for describing the intersection */
#define EI_MAX_USER_DATA_SIZE		8

/** \brief The ray types */
enum {
	eiRAY_TYPE_NONE = 0, 
	eiRAY_EYE,					/* eye ray */
	eiRAY_TRANSPARENT,			/* transparency ray */
	eiRAY_REFLECT,				/* reflection ray */
	eiRAY_REFRACT,				/* refraction ray */
	eiRAY_LIGHT,				/* light ray */
	eiRAY_SHADOW,				/* shadow ray */
	eiRAY_ENVIRONMENT,			/* ray only into environment / volume */
	eiPHOTON_ABSORB,			/* photon is absorbed */
	eiPHOTON_LIGHT, 			/* photon emitted from a light source */
	eiPHOTON_REFLECT_SPECULAR,	/* specular reflection of a photon */
	eiPHOTON_REFLECT_GLOSSY,	/* glossy reflection of a photon */
	eiPHOTON_REFLECT_DIFFUSE,	/* diffuse reflection of a photon */
	eiPHOTON_TRANSMIT_SPECULAR,	/* specular transmission of a photon */
	eiPHOTON_TRANSMIT_GLOSSY,	/* glossy transmission of a photon */
	eiPHOTON_TRANSMIT_DIFFUSE,	/* diffuse transmission of a photon */
	eiRAY_DISPLACE, 			/* displacement during tessellation */
	eiRAY_OUTPUT,				/* output shader */
	eiPHOTON_SCATTER_VOLUME,	/* volume scattering of a photon */
	eiPHOTON_TRANSPARENT,		/* transparency photon */
	eiRAY_FINALGATHER,			/* final gather ray */
	eiPHOTON_EMIT_GI,			/* globillum photons (emitters only) */
	eiPHOTON_EMIT_CAUSTIC,		/* caustic photons (emitters only) */
	eiRAY_PROBE,				/* probe ray */
	eiRAY_CREATE_SHADER,		/* create shader */
	eiRAY_TYPE_COUNT, 
};

/* forward declarations */
typedef struct eiTLS					eiTLS;
typedef struct eiBaseBucket				eiBaseBucket;
typedef struct eiShaderInstParamTable	eiShaderInstParamTable;

/** \brief The sampled information for each sub-pixel. */
typedef struct eiSampleInfo {
	ei_slist_node	node;
	/* the location of referenced sub-pixel */
	eiInt			x;
	eiInt			y;
	eiScalar		weight;		/* additional weight */
	/* standard outputs: 
	   IMPORTANT: remember to change EI_SAMPLE_INFO_COLOR_OFFSET 
	   and EI_SAMPLE_INFO_OPACITY_OFFSET 
	   when data members are changed. */
	eiVector		color;		/* RGB floating-point color */
	eiVector		opacity;	/* opacity for each color channel */
} eiSampleInfo;

static eiSampleInfo s_sampleInfo;

#define EI_SAMPLE_INFO_COLOR_OFFSET		((eiSizet)(((eiByte *)&s_sampleInfo.color) - ((eiByte *)&s_sampleInfo)))
#define EI_SAMPLE_INFO_OPACITY_OFFSET	((eiSizet)(((eiByte *)&s_sampleInfo.opacity) - ((eiByte *)&s_sampleInfo)))

eiFORCEINLINE void ei_sample_info_init(eiSampleInfo *info, const eiUint data_size)
{
	initv(&info->color);
	initv(&info->opacity);
	memset(((eiByte *)info) + sizeof(eiSampleInfo), 0, data_size);
	/* initialize the next to null is 
	   required by ei_slist */
	ei_slist_node_init(&info->node);
}

eiFORCEINLINE void ei_sample_info_exit(eiSampleInfo *info)
{
	/* do nothing, because it's allocated from memory pool. */
	ei_slist_node_clear(&info->node);
}

/** \brief The description of shader cache. */
typedef struct eiShaderCache {
	/* the root node of the current executing shader graph */
	eiNode					*root;
	/* the parameter table of the root node */
	eiShaderInstParamTable	*root_param_table;
	/* the beginning address of temporary shader parameters */
	eiByte					*params;
	/* the size of the shader cache in bytes not including 
	   this description, NOT including the sizeof(eiShaderCache) */
	eiUint					size;
	/* is shader caching enabled for the current sub-graph */
	eiBool					enabled;
	/* the custom shader calling arguments */
	void					*arg;
} eiShaderCache;

/** \brief The structure holds the current state 
 * variables including ray information and 
 * differential geometry, etc.
 */
typedef struct eiState {
	/* TLS associated with this state */
	eiTLS						*tls;
	/* the ray type */
	eiInt						type;
	/* the ray origin in world space */
	eiVector					org;
	/* the ray direction in world space */
	eiVector					dir;
	/* inverse ray direction, 1 / dir in world space */
	eiVector					inv_dir;
	/* the raster coordinates in screen space */
	eiVector2					raster;
	/* the current sampling time */
	eiScalar					time;
	/* maximum hit parametric distance */
	eiScalar					max_t;
	/* near and far clips for current scene node */
	union {
		struct {
			eiScalar			t_near, t_far;
		};
		eiScalar				t_range[2];
	};
	/* the ray origin in object space, should be recomputed 
	   when current object instance is changed. */
	eiVector					obj_org;
	/* the ray direction in object space, should be recomputed 
	   when current object instance is changed. */
	eiVector					obj_dir;
	/* whether a hit has been found */
	eiBool						found_hit;
	/* dot product of normal and ray direction in world space */
	eiScalar					dot_nd;
	/* hit parametric distance */
	eiScalar					hit_t;
	/* the previous hit parametric distance, used for sort mode */
	eiScalar					prev_hit_t;
	/* the hit bsp-tree */
	eiTag						hit_bsp;
	/* the hit tessellation instance, for internal use only */
	eiIndex						hit_tessel_inst;
	/* the hit tessellation */
	eiTag						hit_tessel;
	/* ray-traceable instance tag of the hit */
	eiTag						hit_inst;
	/* source object tag of the hit */
	eiTag						hit_obj;
	/* material tag of the hit */
	eiTag						hit_mtl;
	/* tessellated triangle index of the hit */
	eiIndex						hit_tri;
	/* source primitive index of the hit */
	eiIndex						hit_prim;
	/* whether the ray passed through a subspace containing motion */
	eiBool						pass_motion;
	/* whether this ray hit something in motion */
	eiBool						hit_motion;
	/* barycentric coordinates of the hit in tessellated triangle */
	eiVector					bary;
	/* the ray bias for child rays */
	eiScalar					bias;
	/* the ray bias scale for child rays */
	eiScalar					bias_scale;
	/* the user data about the hit */
	eiScalar					user_data[ EI_MAX_USER_DATA_SIZE ];
	/* the hit surface position in camera space */
	eiVector					P;
	/* the surface normal interpolated smoothly 
	   from vertices in camera space */
	eiVector					N;
	/* the surface geometric normal, normal of the 
	   tessellated face in camera space */
	eiVector					Ng;
	/* the derivative of surface position along u */
	eiVector					dPdu;
	/* the derivative of surface position along v */
	eiVector					dPdv;
	/* surface parametric coordinate along u */
	eiScalar					u;
	/* surface parametric coordinate along v */
	eiScalar					v;
	/* the change in surface parametric coordinate along u */
	eiScalar					du;
	/* the change in surface parametric coordinate along v */
	eiScalar					dv;
	/* the amount of time covered by this shading sample */
	eiScalar					dtime;
	/* the derivative of surface position per unit time */
	eiVector					dPdtime;
	/* the sum length of ray segments */
	eiScalar					distance;
	/* the current reflected depth */
	eiInt						reflect_depth;
	/* the current refracted depth */
	eiInt						refract_depth;
	/* the current final gather diffuse bounce depth */
	eiInt						finalgather_diffuse_depth;
	eiInt						caustic_reflect_depth;
	eiInt						caustic_refract_depth;
	eiInt						globillum_reflect_depth;
	eiInt						globillum_refract_depth;
	/* for Quasi-Monte Carlo integration, 
	   the current instance of low discrepancy vector */
	eiUint						instance_number;
	/* for Quasi-Monte Carlo integration, 
	   the current integral dimension in ray tree */
	eiUint						dimension;
	/* temporal integral dimension for sample */
	eiUint						temp_dimension;
	/* sampling result contains user-defined output variables */
	eiSampleInfo				*result;
	/* the current sampling bucket */
	eiBaseBucket				*bucket;
	/* the current database we are working on */
	eiDatabase					*db;
	/* the current active options */
	eiOptions					*opt;
	/* the current active camera */
	eiCamera					*cam;
	/* the current shader cache pointer */
	eiByte						*shader_cache;
	/* the current calling shader instance */
	eiTag						shader;
	/* predefined shader variables */
	eiVector					L;
	eiVector					Cl;
	eiVector					Ol;
	/* ray origin in camera space */
	eiVector					E;
	/* ray direction in camera space */
	eiVector					I;
	/* Quasi-Monte Carlo points for area lights */
	eiScalar					u1, u2;
	eiInt						current_light_index;
	eiUint						current_area_sample;
	eiUint						num_area_samples;
	eiTag						current_light_list;
	eiVector					current_light_org;
	eiVector					current_light_dir;
	/* used for "illuminate" and "solar" statements */
	eiUint						current_surface;
	/* the number of current volume instances */
	eiUint						num_current_volumes;
	/* the tag list of current volume shaders */
	eiTag						*current_volumes;
} eiState;

/** \brief Initialize the state.
 */
eiAPI void ei_state_init(
	eiState *state, 
	const eiInt type, 
	eiBaseBucket *bucket);

/** \brief Cleanup the state.
 */
eiAPI void ei_state_exit(eiState *state);

eiFORCEINLINE void calc_inv_dir(eiVector * const inv_dir, const eiVector *dir)
{
	if (absf(dir->x) > eiSCALAR_EPS) {
		inv_dir->x = 1.0f / dir->x;
	} else {
		inv_dir->x = sgnf(dir->x) / eiSCALAR_EPS;
	}
	if (absf(dir->y) > eiSCALAR_EPS) {
		inv_dir->y = 1.0f / dir->y;
	} else {
		inv_dir->y = sgnf(dir->y) / eiSCALAR_EPS;
	}
	if (absf(dir->z) > eiSCALAR_EPS) {
		inv_dir->z = 1.0f / dir->z;
	} else {
		inv_dir->z = sgnf(dir->z) / eiSCALAR_EPS;
	}
}

/** \brief Precompute some quantities used frequently in ray-tracing.
 */
eiFORCEINLINE void ei_state_precompute(eiState *state)
{
	calc_inv_dir(&state->inv_dir, &state->dir);
}

/** \brief Refresh the state, the renderer may reuse a state as 
 * a new one.
 */
eiAPI void ei_flush_cache(eiState *state);

/** \brief Refresh the state, the renderer may reuse a state as 
 * a new one, this updates the state ID so it dirts the shading cache.
 */
eiAPI void ei_new_state(eiState *state);

/** \brief Set the initial volume, clear all existing volumes.
 */
void ei_state_init_volume(eiState *state, const eiTag volume);

/** \brief Inherit current volume list from parent, and append or 
 * remove current hit volume instance automatically.
 */
void ei_state_inherit_volume(eiState *state, eiState * const parent);

/** \brief Transform a ray using a matrix.
 */
eiFORCEINLINE void ei_transform_ray(
	eiVector *dst_org, 
	eiVector *dst_dir, 
	const eiVector *src_org, 
	const eiVector *src_dir, 
	const eiMatrix *transform)
{
	point_transform(dst_org, src_org, transform);
	vector_transform(dst_dir, src_dir, transform);
}

/* it's slow to use slerp */
/* #define USE_SLERP_FOR_MOTION */

/** \brief Transform a ray using a matrix, 
 * a motion matrix, and a time.
 */
eiFORCEINLINE void ei_motion_transform_ray(
	eiVector *dst_org, 
	eiVector *dst_dir, 
	const eiVector *src_org, 
	const eiVector *src_dir, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	const eiScalar time)
{
	eiVector	org1, org2, dir1, dir2;
#ifdef USE_SLERP_FOR_MOTION
	eiScalar	len1, len2, obj_len;
#endif

	point_transform(&org1, src_org, transform);
	point_transform(&org2, src_org, motion_transform);
	vector_transform(&dir1, src_dir, transform);
	vector_transform(&dir2, src_dir, motion_transform);

	lerp3(dst_org, &org1, &org2, time);

#ifdef USE_SLERP_FOR_MOTION

	len1 = len(&dir1);
	len2 = len(&dir2);
	obj_len = 1.0f;

	/* slerp requires unit vectors */
	mulvfi(&dir1, 1.0f / len1);
	mulvfi(&dir2, 1.0f / len2);
	slerp(dst_dir, &dir1, &dir2, time);
	/* interpolate and multiply the vector length back 
	   to make sure "state->hit_t" is uniform. */
	lerp(&obj_len, len1, len2, time);
	mulvfi(dst_dir, obj_len);

#else

	addi(&dir1, &org1);
	addi(&dir2, &org2);

	lerp3(dst_dir, &dir1, &dir2, time);
	subi(dst_dir, dst_org);

#endif
}

#ifdef __cplusplus
}
#endif

#endif
