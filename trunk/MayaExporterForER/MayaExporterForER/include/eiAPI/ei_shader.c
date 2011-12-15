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

#include <eiAPI/ei_shader.h>
#include <eiAPI/ei_light.h>
#include <eiAPI/ei_shadesys.h>
#include <eiAPI/ei_raytracer.h>
#include <eiAPI/ei_object.h>
#include <eiAPI/ei_material.h>
#include <eiAPI/ei_texture.h>
#include <eiAPI/ei_sampler.h>
#include <eiCORE/ei_qmc.h>
#include <eiCORE/ei_random.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_data_table.h>
#include <eiCORE/ei_assert.h>

void ei_shader_init(eiShader *shader)
{
	ei_node_object_init(&shader->base);

	shader->base.exit_node = ei_shader_instance_exit;
	shader->main = NULL;
	shader->size = 0;
	shader->state = NULL;
}

void ei_shader_exit(eiShader *shader)
{
	ei_node_object_exit(&shader->base);
}

eiBool ei_call_shader(
	eiVector4 * const result, 
	eiState * const state, 
	const eiTag shader, 
	void *arg)
{
	eiNodeSystem	*nodesys;

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	if (ei_db_type(state->db, shader) == EI_DATA_TYPE_ARRAY)
	{
		/* call the shader list */
		return ei_call_shader_instance_list(
			nodesys, 
			result, 
			state, 
			shader, 
			arg);
	}
	else
	{
		/* call the shader */
		return ei_call_shader_instance(
			nodesys, 
			result, 
			state, 
			shader, 
			arg);
	}
}

void *ei_eval(eiState *state, const eiIndex param_index)
{
	return ei_eval_imp(state, param_index, eiTRUE);
}

void *ei_call(eiState *state, const eiIndex param_index)
{
	return ei_eval_imp(state, param_index, eiFALSE);
}

eiBool ei_sample(
	eiState * const state, 
	eiGeoScalar *samples, 
	eiUint *instance_number, 
	const eiUint dimension, 
	const eiUint *n)
{
	eiUint	dim;

	if (samples == NULL || dimension < 1)
	{
		/* error */
		return eiFALSE;
	}

	if (n == NULL)
	{
		if (instance_number == NULL)
		{
			/* single sample */
			for (dim = 0; dim < dimension; ++dim)
			{
				samples[dim] = ei_sigma(state->dimension + dim, state->instance_number);
			}
			state->dimension += dimension;

			/* return FALSE to terminate sampling */
			return eiFALSE;
		}
		else
		{
			/* adaptive sampling */
			/* initialize sampling */
			if (*instance_number == 0)
			{
				/* backup current dimension */
				state->temp_dimension = state->dimension;
				/* advance current dimension */
				state->dimension += dimension;
			}

			/* compute the offset vector */
			for (dim = 0; dim < dimension; ++dim)
			{
				samples[dim] = ei_sigma(state->temp_dimension + dim, state->instance_number);
			}

			/* a Halton point set is used here for adaptive sampling */
			for (dim = 0; dim < dimension; ++dim)
			{
				samples[dim] = fmodf((eiScalar)samples[dim] + (eiScalar)ei_sigma(dim, *instance_number), 1.0f);
			}

			++ *instance_number;

			/* always return TRUE */
			return eiTRUE;
		}
	}
	else
	{
		if (instance_number == NULL)
		{
			/* error */
			return eiFALSE;
		}
		else
		{
			/* fixed sampling */
			/* initialize sampling */
			if (*instance_number == 0)
			{
				/* backup current dimension */
				state->temp_dimension = state->dimension;
				/* advance current dimension */
				state->dimension += dimension;
			}
			else if (*instance_number == *n)
			{
				/* enough samples taken, terminate sampling */
				return eiFALSE;
			}

			/* compute the offset vector */
			for (dim = 0; dim < dimension; ++dim)
			{
				samples[dim] = ei_sigma(state->temp_dimension + dim, state->instance_number);
			}

			/* a Hammersley point set is used here for better discrepancy */
			samples[0] = fmodf((eiScalar)samples[0] + (eiScalar)(*instance_number) / (eiScalar)(*n), 1.0f);
			for (dim = 1; dim < dimension; ++dim)
			{
				samples[dim] = fmodf((eiScalar)samples[dim] + (eiScalar)ei_sigma(dim - 1, *instance_number), 1.0f);
			}

			++ *instance_number;

			/* continue sampling */
			return eiTRUE;
		}
	}
}

eiSampleInfo *ei_new_sample(eiState * const state)
{
	return create_sample_info((eiBucket *)state->bucket);
}

void ei_reset_sample(eiState * const state, eiSampleInfo * const sample)
{
	reset_sample_info((eiBucket *)state->bucket, sample);
}

void ei_add_sample(eiState * const state, eiSampleInfo * const c1, eiSampleInfo * const c2)
{
	ei_sample_info_add((eiBucket *)state->bucket, c1, c2);
}

void ei_mul_sample(eiState * const state, eiSampleInfo * const c, const eiScalar a)
{
	ei_sample_info_mul((eiBucket *)state->bucket, c, a);
}

void ei_delete_sample(eiState * const state, eiSampleInfo * const sample)
{
	delete_sample_info((eiBucket *)state->bucket, sample);
}

eiBool ei_illuminance(
	eiState * const state, 
	const eiVector *position, 
	const eiVector *axis, 
	const eiScalar angle, 
	const char *category)
{
	eiNodeSystem			*nodesys;
	eiBaseBucket			*bucket;
	eiBool					found_light;
	eiInt					num_light_insts;
	eiDataTableIterator		*light_insts_iter;

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	bucket = state->bucket;
	
	found_light = eiFALSE;

	++ state->current_light_index;

	light_insts_iter = &bucket->light_insts_iter;
	num_light_insts = light_insts_iter->tab->item_count;

	/* loop until we found a light or run out of lights */
	while (!found_light && state->current_light_index < num_light_insts)
	{
		eiLightInstance		*light_inst;
		eiLight				*light;
		eiTag				light_list;
		eiInt				num_area_samples;
		eiVector			light_org;
		eiVector			light_dir;
		eiVector			cone_axis;
		eiBool				accept_light;

		light_inst = (eiLightInstance *)ei_data_table_read(light_insts_iter, state->current_light_index);
		light = (eiLight *)ei_db_access(state->db, light_inst->light);

		accept_light = eiTRUE;

		if (category != NULL && strlen(category) > 0)
		{
			eiBool		inclusive;
			const char	*key;

			inclusive = eiTRUE;
			key = category;

			if (category[0] == '-')
			{
				inclusive = eiFALSE;
				key = category + 1;
			}

			/* in inclusive mode, we accept the light if we found the key */
			accept_light = (ei_nodesys_lookup_parameter(nodesys, (eiNode *)light, key) != eiNULL_INDEX);

			/* flip the result if exclusive mode is used */
			if (!inclusive)
			{
				accept_light = !accept_light;
			}
		}

		light_list = light->light_list;

		if (state->type == eiRAY_FINALGATHER)
		{
			/* this optimization reduces area samples for final gathering */
			num_area_samples = 1;
		}
		else
		{
			/* use low sampling settings if trace sum depth is big enough */
			if ((state->reflect_depth + state->refract_depth) > light->low_level)
			{
				num_area_samples = light->low_u_samples * light->low_v_samples;
			}
			else
			{
				num_area_samples = light->u_samples * light->v_samples;
			}
		}

		movv(&light_org, &light_inst->origin);

		ei_db_end(state->db, light_inst->light);

		if (accept_light)
		{
			sub(&light_dir, &light_org, &state->P);
			normalizei(&light_dir);

			movv(&cone_axis, axis);
			normalizei(&cone_axis);

			if (dot(&light_dir, &cone_axis) > cosf(angle))
			{
				state->current_light_org = light_org;
				state->current_light_dir = light_dir;
				state->current_light_list = light_list;
				state->num_area_samples = num_area_samples;

				found_light = eiTRUE;
				break;
			}
		}

		++ state->current_light_index;
	}

	if (!found_light)
	{
		state->current_light_index = -1;
		state->current_area_sample = 0;
		return eiFALSE;
	}
	else
	{
		state->current_area_sample = 0;
		return eiTRUE;
	}
}

eiBool ei_sample_light(eiState * const state)
{
	eiNodeSystem	*nodesys;
	eiScalar		offset_x, offset_y;
	eiScalar		u1, u2;
	eiState			light_ray;
	eiRayTLS		*pRayTls;
	eiVector4		result;

	if (state->current_area_sample >= state->num_area_samples)
	{
		return eiFALSE;
	}

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);

	/* get an offset point from the global sequence for current dimension */
	offset_x = (eiScalar)ei_sigma(state->dimension, state->instance_number);
	offset_y = (eiScalar)ei_sigma(state->dimension + 1, state->instance_number);

	/* here we use a scrambled Halton sequence for adaptive sampling */
	u1 = fmodf(offset_x + (eiScalar)ei_sigma(0, state->current_area_sample), 1.0f);
	u2 = fmodf(offset_y + (eiScalar)ei_sigma(1, state->current_area_sample), 1.0f);

	ei_state_init(&light_ray, eiRAY_LIGHT, state->bucket);

	light_ray.org = state->P;
	light_ray.dir = state->current_light_dir;
	light_ray.E = light_ray.org;
	light_ray.I = light_ray.dir;
	light_ray.P = state->current_light_org;
	light_ray.N = state->N;
	light_ray.Ng = state->Ng;
	light_ray.dimension = state->dimension + 2;
	light_ray.instance_number = state->instance_number + state->current_area_sample;
	light_ray.u1 = u1;
	light_ray.u2 = u2;
	light_ray.time = state->time;
	light_ray.dtime = state->dtime;
	light_ray.raster = state->raster;
	light_ray.current_light_index = state->current_light_index;

	pRayTls = (eiRayTLS *)ei_tls_get_interface(state->tls, EI_TLS_TYPE_RAYTRACER);

	/* add statistics */
	++ pRayTls->num_rays[ light_ray.type ];

	/* call light shader attached to this instance to calculate Cl, Ol, L */
	initv4(&result);

	ei_call_shader_instance_list(nodesys, &result, &light_ray, state->current_light_list, NULL);

	state->Cl = light_ray.Cl;
	state->Ol = light_ray.Ol;
	neg(&state->L, &light_ray.L);
	state->pass_motion |= light_ray.pass_motion;

	ei_state_exit(&light_ray);

	++ state->current_area_sample;
	return eiTRUE;
}

eiBool ei_converged(eiState * const state, const eiVector *c1, const eiVector *c2)
{
	static const eiScalar eps = 1.0e-12f;

	if (absf(c1->r - c2->r) < eps && 
		absf(c1->g - c2->g) < eps && 
		absf(c1->b - c2->b) < eps)
	{
		return eiTRUE;
	}
	else
	{
		return eiFALSE;
	}
}

eiBool ei_illuminate(
	eiState * const state, 
	const eiVector *position, 
	const eiVector *axis, 
	const eiScalar angle)
{
	eiVector	light_dir, dir1, dir2;

	/* "position" is actually light origin, 
	   and "org" is the surface point to be illuminated. */
	sub(&light_dir, &state->org, position);
	movv(&dir1, &light_dir);
	normalizei(&dir1);
	movv(&dir2, axis);
	normalizei(&dir2);

	if (dot(&dir1, &dir2) < cosf(angle))
	{
		/* shading point is out of range, set light intensity to black. */
		initv(&state->Cl);
		return eiFALSE;
	}

	if (state->current_surface >= 1)
	{
		state->current_surface = 0;
		return eiFALSE;
	}
	else
	{
		movv(&state->L, &light_dir);
		++ state->current_surface;
		return eiTRUE;
	}
}

eiBool ei_solar(
	eiState * const state, 
	const eiVector *axis, 
	const eiScalar angle)
{
	eiVector	light_dir;

	if (angle == 0.0f)
	{
		neg(&light_dir, axis);
		normalizei(&light_dir);
	}
	else
	{
		eiVector	u_axis, v_axis, w_axis;

		ei_uniform_sample_cone(&light_dir, state->u1, state->u2, cosf(angle));
		movv(&w_axis, axis);
		normalizei(&w_axis);
		ortho_basis(&w_axis, &u_axis, &v_axis);

		mulvfi(&u_axis, light_dir.x);
		mulvfi(&v_axis, light_dir.y);
		mulvfi(&w_axis, light_dir.z);

		add(&light_dir, &u_axis, &v_axis);
		addi(&light_dir, &w_axis);
		normalizei(&light_dir);
	}

	if (state->current_surface >= 1)
	{
		state->current_surface = 0;
		return eiFALSE;
	}
	else
	{
		/* to make L outward from the light, we use negative "light_dir" here, 
		   so we can keep identical with illuminate statement. */
		neg(&state->L, &light_dir);
		++ state->current_surface;
		return eiTRUE;
	}
}

eiScalar calc_bias(
	const eiVector *N, 
	const eiVector *dir, 
	const eiScalar bias, 
	const eiScalar bias_scale)
{
	return ((absf(N->x) + absf(N->y) + absf(N->z)) * bias_scale) / 
		MAX(eiSCALAR_EPS, absf(dot(N, dir))) + bias;
}

void ei_call_current_volume_list(
	eiNodeSystem *nodesys, 
	eiVector4 * const result, 
	eiState * const state, 
	void *arg)
{
	eiVector	prev_I;
	eiUint		i;

	if (!state->opt->volume)
	{
		return;
	}

	/* backup the ray direction because we may re-use this state */
	movv(&prev_I, &state->I);
	mulvfi(&state->I, state->hit_t);

	for (i = 0; i < state->num_current_volumes; ++i)
	{
		ei_call_shader_instance_list(nodesys, result, state, state->current_volumes[i], arg);
	}

	/* restore the ray direction */
	movv(&state->I, &prev_I);
}

static eiBool ei_eye_ray_hit_proc(eiState *state)
{
	/* TODO: maybe we can handle transparency here */
	return eiTRUE;
}

/** \brief Shoot a ray to do intersection test against the scene, and shade the 
 * intersection point for eye. */
static eiFORCEINLINE eiBool ei_rt_trace_eye(
	eiRayTracer *rt, 
	eiNodeSystem *nodesys, 
	eiState *state)
{
	if (ei_rt_trace(rt, state, ei_eye_ray_hit_proc, eiFALSE))
	{
		eiVector4	result;

		initv4(&result);

		ei_rt_compute_hit_details(rt, state);

		/* shade by calling the material at the intersection */
		if (state->hit_mtl != eiNULL_TAG)
		{
			eiMaterial		*mtl;

			mtl = (eiMaterial *)ei_db_access(rt->db, state->hit_mtl);
			ei_call_shader_instance_list(nodesys, &result, state, mtl->surface_list, NULL);
			ei_db_end(rt->db, state->hit_mtl);

			ei_call_current_volume_list(nodesys, &result, state, NULL);
		}

		return eiTRUE;
	}
	else
	{
		eiVector	result;

		initv(&result);

		ei_trace_environment(&result, state, &state->I);

		return eiFALSE;
	}
}

void ei_trace_eye(
	eiSampleInfo * const color, 
	const eiVector *src, 
	const eiVector *dir, 
	eiState * const state)
{
	eiRayTracer		*rt;
	eiNodeSystem	*nodesys;
	eiState			eye_ray;
	eiVector		ray_dir;
	eiScalar		inv_dir_z;
	eiScalar		t_near, t_far;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	
	ei_state_init(&eye_ray, eiRAY_EYE, state->bucket);

	eye_ray.result = color;
	eye_ray.dimension = state->dimension;
	eye_ray.instance_number = state->instance_number;

	movv(&ray_dir, dir);
	normalizei(&ray_dir);

	movv(&eye_ray.org, src);
	movv(&eye_ray.dir, &ray_dir);

	eye_ray.max_t = state->max_t;
	eye_ray.time = state->time;
	eye_ray.dtime = state->dtime;
	eye_ray.raster = state->raster;

	/* limit near and far by clipping planes */
	if (absf(eye_ray.dir.z) > eiSCALAR_EPS) {
		inv_dir_z = 1.0f / eye_ray.dir.z;
	} else {
		inv_dir_z = sgnf(eye_ray.dir.z) / eiSCALAR_EPS;
	}
	/* be aware our z-axis is outwards */
	t_near = (-state->cam->clip_hither - eye_ray.org.z) * inv_dir_z;
	t_far = (-state->cam->clip_yon - eye_ray.org.z) * inv_dir_z;
	eye_ray.t_near = MAX(t_near, eye_ray.t_near);
	eye_ray.t_far = MIN(t_far, eye_ray.t_far);

	/* append camera volume shader as the initial volume */
	if (state->opt->volume && 
		state->cam->volume_list != eiNULL_TAG && 
		!ei_data_array_empty(state->db, state->cam->volume_list))
	{
		ei_state_init_volume(&eye_ray, state->cam->volume_list);
	}

	ei_rt_trace_eye(rt, nodesys, &eye_ray);

	state->pass_motion |= eye_ray.pass_motion;

	ei_state_exit(&eye_ray);
}

static eiBool ei_shadow_ray_hit_proc(eiState *state)
{
	eiRayTracer		*rt;
	eiNodeSystem	*nodesys;
	eiVector4		result;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);

	initv4(&result);

	ei_rt_compute_hit_details(rt, state);

	/* shade by calling the material at the intersection */
	if (state->hit_mtl != eiNULL_TAG)
	{
		eiMaterial		*mtl;
		eiState			child;
		static const eiVector one = {1.0f, 1.0f, 1.0f};

		mtl = (eiMaterial *)ei_db_access(rt->db, state->hit_mtl);
		ei_call_shader_instance_list(nodesys, &result, state, mtl->shadow_list, NULL);
		ei_db_end(rt->db, state->hit_mtl);

		state->hit_t -= state->prev_hit_t;
		ei_call_current_volume_list(nodesys, &result, state, NULL);
		state->hit_t += state->prev_hit_t;
		state->prev_hit_t = state->hit_t;

		/* append the current hit into the volume list of shadow ray */
		child.tls = state->tls;
		child.db = state->db;
		child.num_current_volumes = 0;
		child.current_volumes = NULL;

		ei_state_inherit_volume(&child, state);
		
		state->num_current_volumes = child.num_current_volumes;
		if (state->current_volumes != NULL)
		{
			ei_tls_free(state->tls, state->current_volumes);
			state->current_volumes = NULL;
		}
		state->current_volumes = child.current_volumes;

		/* if we hit an opaque object, return true to terminate tracing */
		if (almost_equalv(&state->Ol, &one, eiSCALAR_EPS))
		{
			return eiTRUE;
		}
	}

	return eiFALSE;
}

void ei_trace_shadow(
	eiVector * const color, 
	const eiVector *src, 
	const eiVector *dir, 
	eiState * const state)
{
	eiRayTracer		*rt;
	eiNodeSystem	*nodesys;
	eiState			shadow_ray;
	eiScalar		ray_len;
	eiVector		ray_dir;

	if (state->hit_inst != eiNULL_TAG)
	{
		eiRayObjectInstance		*obj_inst;

		obj_inst = (eiRayObjectInstance *)ei_db_access(state->db, state->hit_inst);
		if (!ei_attr_get_flag(&obj_inst->attr, EI_ATTR_RECV_SHADOW))
		{
			ei_db_end(state->db, state->hit_inst);
			return;
		}
		ei_db_end(state->db, state->hit_inst);
	}

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	
	ei_state_init(&shadow_ray, eiRAY_SHADOW, state->bucket);

	movv(&shadow_ray.Cl, color);
	setv(&shadow_ray.Ol, 0.0f, 0.0f, 0.0f);
	shadow_ray.dimension = state->dimension;
	shadow_ray.instance_number = state->instance_number;

	movv(&ray_dir, dir);
	ray_len = normalize_with_len(&ray_dir);

	movv(&shadow_ray.org, src);
	movv(&shadow_ray.dir, &ray_dir);

	/* bias a little */
	shadow_ray.max_t = ray_len - calc_bias(&state->Ng, &ray_dir, state->bias, state->bias_scale);

	shadow_ray.time = state->time;
	shadow_ray.dtime = state->dtime;
	shadow_ray.raster = state->raster;

	ei_rt_trace(rt, &shadow_ray, ei_shadow_ray_hit_proc, eiTRUE);

	if (shadow_ray.found_hit)
	{
		eiVector		I;
		eiVector4		result;

		/* call volume shader for the last segment */
		shadow_ray.hit_t = shadow_ray.max_t - shadow_ray.hit_t;
		mulvf(&I, &shadow_ray.I, shadow_ray.hit_t);
		addi(&shadow_ray.P, &I);

		initv4(&result);

		ei_call_current_volume_list(nodesys, &result, &shadow_ray, NULL);
	}

	movv(color, &shadow_ray.Cl);
	state->pass_motion |= shadow_ray.pass_motion;

	ei_state_exit(&shadow_ray);
}

/** \brief Control the depth of secondary ray before actual tracing it. */
static eiFORCEINLINE eiBool ei_secondary_depth_control(
	eiNodeSystem *nodesys, 
	eiState *state, 
	eiState *parent, 
	eiOptions *opt)
{
	eiInt	trace_reflect_depth;
	eiInt	trace_refract_depth;
	eiInt	trace_sum_depth;

	trace_reflect_depth = opt->trace_reflect_depth;
	trace_refract_depth = opt->trace_refract_depth;
	trace_sum_depth = opt->trace_sum_depth;

	if (state->reflect_depth > trace_reflect_depth || 
		state->refract_depth > trace_refract_depth || 
		(state->reflect_depth + state->refract_depth) > trace_sum_depth)
	{
		eiVector	result;

		initv(&result);

		ei_trace_environment(&result, state, &state->I);

		return eiFALSE;
	}

	return eiTRUE;
}

static eiBool ei_secondary_ray_hit_proc(eiState *state)
{
	return eiTRUE;
}

/** \brief Shoot a secondary ray to do intersection test against the scene, 
 * and shade the intersection point. */
static eiFORCEINLINE eiBool ei_rt_trace_secondary(
	eiRayTracer *rt, 
	eiNodeSystem *nodesys, 
	eiState *state)
{
	if (ei_rt_trace(rt, state, ei_secondary_ray_hit_proc, eiFALSE))
	{
		eiVector4	result;

		initv4(&result);

		/* sum the distance */
		state->distance += state->hit_t;

		ei_rt_compute_hit_details(rt, state);

		/* shade by calling the material at the intersection */
		if (state->hit_mtl != eiNULL_TAG)
		{
			eiMaterial		*mtl;

			mtl = (eiMaterial *)ei_db_access(rt->db, state->hit_mtl);
			ei_call_shader_instance_list(nodesys, &result, state, mtl->surface_list, NULL);
			ei_db_end(rt->db, state->hit_mtl);

			ei_call_current_volume_list(nodesys, &result, state, NULL);
		}

		return eiTRUE;
	}
	else
	{
		eiVector	result;

		initv(&result);

		ei_trace_environment(&result, state, &state->I);

		return eiFALSE;
	}
}

void ei_trace_reflection(
	eiVector * const color, 
	const eiVector *dir, 
	eiState * const state)
{
	eiRayTracer		*rt;
	eiNodeSystem	*nodesys;
	eiState			refl_ray;
	eiVector		prev_color;
	eiVector		prev_opacity;
	eiVector		ray_src, ray_dir;

	if (state->hit_inst != eiNULL_TAG)
	{
		eiRayObjectInstance		*obj_inst;

		obj_inst = (eiRayObjectInstance *)ei_db_access(state->db, state->hit_inst);
		if (!ei_attr_get_flag(&obj_inst->attr, EI_ATTR_RECV_REFLECTION))
		{
			ei_db_end(state->db, state->hit_inst);
			return;
		}
		ei_db_end(state->db, state->hit_inst);
	}

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	
	ei_state_init(&refl_ray, eiRAY_REFLECT, state->bucket);

	/* backup color and opacity */
	movv(&prev_color, &state->result->color);
	movv(&prev_opacity, &state->result->opacity);
	refl_ray.result = state->result;

	initv(&refl_ray.result->color);
	initv(&refl_ray.result->opacity);
	refl_ray.dimension = state->dimension;
	refl_ray.instance_number = state->instance_number;

	movv(&ray_dir, dir);
	normalizei(&ray_dir);

	movv(&ray_src, &ray_dir);
	mulvfi(&ray_src, calc_bias(&state->Ng, &ray_dir, state->bias, state->bias_scale));
	addi(&ray_src, &state->P);

	movv(&refl_ray.org, &ray_src);
	movv(&refl_ray.dir, &ray_dir);

	refl_ray.time = state->time;
	refl_ray.dtime = state->dtime;
	refl_ray.raster = state->raster;
	refl_ray.reflect_depth = state->reflect_depth + 1;
	refl_ray.refract_depth = state->refract_depth;

	if (!ei_secondary_depth_control(nodesys, &refl_ray, state, state->opt))
	{
		ei_state_exit(&refl_ray);

		return;
	}

	ei_state_inherit_volume(&refl_ray, state);

	ei_rt_trace_secondary(rt, nodesys, &refl_ray);

	movv(color, &refl_ray.result->color);
	state->pass_motion |= refl_ray.pass_motion;

	ei_state_exit(&refl_ray);

	/* restore color and opacity */
	movv(&state->result->color, &prev_color);
	movv(&state->result->opacity, &prev_opacity);
}

void ei_trace_transmission(
	eiVector * const color, 
	const eiVector *dir, 
	eiState * const state)
{
	eiRayTracer		*rt;
	eiNodeSystem	*nodesys;
	eiState			refr_ray;
	eiVector		prev_color;
	eiVector		prev_opacity;
	eiVector		ray_src, ray_dir;

	if (state->hit_inst != eiNULL_TAG)
	{
		eiRayObjectInstance		*obj_inst;

		obj_inst = (eiRayObjectInstance *)ei_db_access(state->db, state->hit_inst);
		if (!ei_attr_get_flag(&obj_inst->attr, EI_ATTR_RECV_REFRACTION))
		{
			ei_db_end(state->db, state->hit_inst);
			return;
		}
		ei_db_end(state->db, state->hit_inst);
	}

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	
	ei_state_init(&refr_ray, eiRAY_REFRACT, state->bucket);

	/* backup color and opacity */
	movv(&prev_color, &state->result->color);
	movv(&prev_opacity, &state->result->opacity);
	refr_ray.result = state->result;

	initv(&refr_ray.result->color);
	initv(&refr_ray.result->opacity);
	refr_ray.dimension = state->dimension;
	refr_ray.instance_number = state->instance_number;

	movv(&ray_dir, dir);
	normalizei(&ray_dir);

	movv(&ray_src, &ray_dir);
	mulvfi(&ray_src, calc_bias(&state->Ng, &ray_dir, state->bias, state->bias_scale));
	addi(&ray_src, &state->P);

	movv(&refr_ray.org, &ray_src);
	movv(&refr_ray.dir, &ray_dir);

	refr_ray.time = state->time;
	refr_ray.dtime = state->dtime;
	refr_ray.raster = state->raster;
	refr_ray.reflect_depth = state->reflect_depth;
	refr_ray.refract_depth = state->refract_depth + 1;

	if (!ei_secondary_depth_control(nodesys, &refr_ray, state, state->opt))
	{
		ei_state_exit(&refr_ray);

		return;
	}

	ei_state_inherit_volume(&refr_ray, state);

	ei_rt_trace_secondary(rt, nodesys, &refr_ray);

	movv(color, &refr_ray.result->color);
	state->pass_motion |= refr_ray.pass_motion;

	ei_state_exit(&refr_ray);

	/* restore color and opacity */
	movv(&state->result->color, &prev_color);
	movv(&state->result->opacity, &prev_opacity);
}

void ei_trace_transparent(
	eiVector * const color, 
	eiState * const state)
{
	eiRayTracer		*rt;
	eiNodeSystem	*nodesys;
	eiState			trans_ray;
	eiVector		prev_color;
	eiVector		prev_opacity;
	eiVector		ray_src, ray_dir;

	if (state->hit_inst != eiNULL_TAG)
	{
		eiRayObjectInstance		*obj_inst;

		obj_inst = (eiRayObjectInstance *)ei_db_access(state->db, state->hit_inst);
		if (!ei_attr_get_flag(&obj_inst->attr, EI_ATTR_RECV_TRANSPARENCY))
		{
			ei_db_end(state->db, state->hit_inst);
			return;
		}
		ei_db_end(state->db, state->hit_inst);
	}

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	
	ei_state_init(&trans_ray, eiRAY_TRANSPARENT, state->bucket);

	/* backup color and opacity */
	movv(&prev_color, &state->result->color);
	movv(&prev_opacity, &state->result->opacity);
	trans_ray.result = state->result;

	initv(&trans_ray.result->color);
	initv(&trans_ray.result->opacity);
	trans_ray.dimension = state->dimension;
	trans_ray.instance_number = state->instance_number;

	movv(&ray_dir, &state->dir);
	normalizei(&ray_dir);

	movv(&ray_src, &ray_dir);
	mulvfi(&ray_src, calc_bias(&state->Ng, &ray_dir, state->bias, state->bias_scale));
	addi(&ray_src, &state->P);

	movv(&trans_ray.org, &ray_src);
	movv(&trans_ray.dir, &ray_dir);

	trans_ray.time = state->time;
	trans_ray.dtime = state->dtime;
	trans_ray.raster = state->raster;
	trans_ray.reflect_depth = state->reflect_depth;
	trans_ray.refract_depth = state->refract_depth + 1;

	if (!ei_secondary_depth_control(nodesys, &trans_ray, state, state->opt))
	{
		ei_state_exit(&trans_ray);

		return;
	}

	ei_state_inherit_volume(&trans_ray, state);

	ei_rt_trace_secondary(rt, nodesys, &trans_ray);

	movv(color, &trans_ray.result->color);
	state->pass_motion |= trans_ray.pass_motion;

	ei_state_exit(&trans_ray);

	/* restore color and opacity */
	movv(&state->result->color, &prev_color);
	movv(&state->result->opacity, &prev_opacity);
}

void ei_trace_environment(
	eiVector * const color, 
	eiState * const state, 
	const eiVector *dir)
{
	eiRayTracer		*rt;
	eiNodeSystem	*nodesys;
	eiState			env_ray;
	eiVector		ray_dir;
	eiRayTLS		*pRayTls;
	eiTag			env_list;
	eiVector4		result;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	
	ei_state_init(&env_ray, eiRAY_ENVIRONMENT, state->bucket);

	env_ray.result = state->result;

	initv(&env_ray.result->color);
	initv(&env_ray.result->opacity);
	env_ray.dimension = state->dimension;
	env_ray.instance_number = state->instance_number;

	movv(&ray_dir, dir);
	normalizei(&ray_dir);

	movv(&env_ray.E, &state->E);
	movv(&env_ray.I, &ray_dir);

	/* transform vectors from camera space to world space */
	point_transform(&env_ray.org, &env_ray.E, &state->cam->camera_to_world);
	vector_transform(&env_ray.dir, &env_ray.I, &state->cam->camera_to_world);
	normalizei(&env_ray.dir);

	env_ray.time = state->time;
	env_ray.dtime = state->dtime;
	env_ray.raster = state->raster;
	env_ray.reflect_depth = state->reflect_depth;
	env_ray.refract_depth = state->refract_depth;

	pRayTls = (eiRayTLS *)ei_tls_get_interface(state->tls, EI_TLS_TYPE_RAYTRACER);

	/* add statistics */
	++ pRayTls->num_rays[ env_ray.type ];

	/* shade by background */
	env_list = state->cam->env_list;

	/* use the shader in hit material to override if available */
	if (state->found_hit && state->hit_mtl != eiNULL_TAG)
	{
		eiMaterial		*mtl;

		mtl = (eiMaterial *)ei_db_access(rt->db, state->hit_mtl);

		if (mtl->env_list != eiNULL_TAG && 
			!ei_data_array_empty(state->db, mtl->env_list))
		{
			env_list = mtl->env_list;
		}

		ei_db_end(rt->db, state->hit_mtl);
	}

	initv4(&result);

	ei_call_shader_instance_list(nodesys, &result, &env_ray, env_list, NULL);

	/* call volume shader with existing state */
	if (state->opt->volume && 
		state->cam->volume_list != eiNULL_TAG && 
		!ei_data_array_empty(state->db, state->cam->volume_list))
	{
		/* extend the environment ray into very far away */
		movv(&env_ray.P, &env_ray.E);
		mulvfi(&env_ray.I, eiBIG_SCALAR);

		ei_call_shader_instance_list(nodesys, &result, &env_ray, state->cam->volume_list, NULL);
	}

	movv(color, &env_ray.result->color);
	state->pass_motion |= env_ray.pass_motion;

	ei_state_exit(&env_ray);
}

static eiBool ei_probe_ray_hit_proc(eiState *state)
{
	return eiTRUE;
}

eiBool ei_trace_probe(
	const eiVector *src, 
	const eiVector *dir, 
	const eiScalar max_dist, 
	eiState * const state)
{
	eiRayTracer		*rt;
	eiState			probe_ray;
	eiVector		ray_src, ray_dir;
	eiBool			result;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);
	
	ei_state_init(&probe_ray, eiRAY_PROBE, state->bucket);

	probe_ray.dimension = state->dimension;
	probe_ray.instance_number = state->instance_number;

	movv(&ray_dir, dir);
	normalizei(&ray_dir);

	movv(&ray_src, &ray_dir);
	mulvfi(&ray_src, calc_bias(&state->Ng, &ray_dir, state->bias, state->bias_scale));
	addi(&ray_src, src);

	movv(&probe_ray.org, &ray_src);
	movv(&probe_ray.dir, &ray_dir);
	probe_ray.max_t = max_dist;

	probe_ray.time = state->time;
	probe_ray.dtime = state->dtime;
	probe_ray.raster = state->raster;

	result = ei_rt_trace(rt, &probe_ray, ei_probe_ray_hit_proc, eiFALSE);

	state->pass_motion |= probe_ray.pass_motion;

	ei_state_exit(&probe_ray);

	return result;
}

/* V and N must start from the same origin */
static eiFORCEINLINE void ReflectSpecular(
	eiVector * const dir, 
	const eiVector *V, 
	const eiVector *N)
{
	/* R = 2 * VN * N - V */
	movv(dir, N);
	mulvfi(dir, 2.0f * dot(V, N));
	subi(dir, V);
}

static eiFORCEINLINE eiScalar CosTheta(const eiVector *w)
{
	return w->z;
}

static eiFORCEINLINE eiScalar AbsCosTheta(const eiVector *w)
{
	return fabsf(w->z);
}

static eiFORCEINLINE eiScalar SinTheta2(const eiVector *w)
{
    return MAX(0.0f, 1.0f - CosTheta(w) * CosTheta(w));
}

static eiFORCEINLINE eiScalar SinTheta(const eiVector *w)
{
    return sqrtf(SinTheta2(w));
}

static eiFORCEINLINE eiScalar CosPhi(const eiVector *w)
{
	eiScalar	sintheta, result;

    sintheta = SinTheta(w);
	if (sintheta == 0.f) {
		return 1.0f;
	}
	result = w->x / sintheta;
	clampi(result, -1.0f, 1.0f);
    return result;
}

static eiFORCEINLINE eiScalar SinPhi(const eiVector *w)
{
	eiScalar	sintheta, result;

    sintheta = SinTheta(w);
	if (sintheta == 0.f) {
		return 0.0f;
	}
	result = w->y / sintheta;
	clampi(result, -1.0f, 1.0f);
    return result;
}

static eiFORCEINLINE eiBool SameHemisphere(const eiVector *w, const eiVector *wp)
{
    return w->z * wp->z > 0.0f;
}

static eiFORCEINLINE eiScalar FrDielScalar(
	const eiScalar cosi, 
	const eiScalar cost, 
	const eiScalar etai, 
	const eiScalar etat)
{
    eiScalar Rparl = ((etat * cosi) - (etai * cost)) /
                     ((etat * cosi) + (etai * cost));
    eiScalar Rperp = ((etai * cosi) - (etat * cost)) /
                     ((etai * cosi) + (etat * cost));
    return (Rparl*Rparl + Rperp*Rperp) * 0.5f;
}

static eiFORCEINLINE eiScalar FrCondScalar(
	const eiScalar cosi, 
	const eiScalar eta, 
	const eiScalar k)
{
	eiScalar tmp = (eta*eta + k*k) * cosi*cosi;
	eiScalar Rparl2 = (tmp - (2.0f * eta * cosi) + 1.0f) /
                      (tmp + (2.0f * eta * cosi) + 1.0f);
	eiScalar tmp_f = eta*eta + k*k;
	eiScalar Rperp2 =
        (tmp_f - (2.0f * eta * cosi) + cosi*cosi) /
        (tmp_f + (2.0f * eta * cosi) + cosi*cosi);
	return (Rparl2 + Rperp2) * 0.5f;
}

static eiFORCEINLINE void FrCond(
	eiVector * const result, 
	const eiScalar cosi, 
	const eiVector *eta, 
	const eiVector *k)
{
	result->r = FrCondScalar(cosi, eta->r, k->r);
	result->g = FrCondScalar(cosi, eta->g, k->g);
	result->b = FrCondScalar(cosi, eta->b, k->b);
}

static eiFORCEINLINE eiScalar Fdr(const eiScalar eta)
{
    if (eta >= 1.0f)
        return -1.4399f / (eta*eta) + 0.7099f / eta + 0.6681f +
            0.0636f * eta;
    else
        return -0.4399f + .7099f / eta - .3319f / (eta * eta) +
            0.0636f / (eta*eta*eta);
}

eiScalar ei_halfangle_cosine(
	const eiVector *wo, 
	const eiVector *wi)
{
	eiScalar	cosThetaO, cosThetaI;
	eiVector	wh;

    cosThetaO = AbsCosTheta(wo);
    cosThetaI = AbsCosTheta(wi);
	if (cosThetaI == 0.f || cosThetaO == 0.f) {
		return 0.0f;
	}
	add(&wh, wi, wo);
	if (wh.x == 0.0f && wh.y == 0.0f && wh.z == 0.0f) {
		return 0.0f;
	}
    normalizei(&wh);
	return dot(wi, &wh);
}

void ei_fresnel_conductor(
	eiVector * const result, 
	eiScalar cosi, 
	const eiVector *eta, 
	const eiVector *k)
{
	FrCond(result, fabsf(cosi), eta, k);
}

void ei_fresnel_dielectric(
	eiVector * const result, 
	eiScalar cosi, 
	const eiScalar eta_i, 
	const eiScalar eta_t)
{
	eiBool		entering;
	eiScalar	ei, et;
	eiScalar	sint, cost;

    /* compute Fresnel reflectance for dielectric */
    clampi(cosi, -1.0f, 1.0f);

    /* compute indices of refraction for dielectric */
    entering = cosi > 0.0f;
    ei = eta_i;
	et = eta_t;
	if (!entering) {
		eiScalar	tmp;
		/* swap ei, et */
		tmp = ei;
		ei = et;
		et = tmp;
	}

    /* compute sint using Snell's law */
	sint = ei / et * sqrtf(MAX(0.0f, 1.0f - cosi * cosi));
    if (sint >= 1.0f) {
        /* handle total internal reflection */
        setvf(result, 1.0f);
    } else {
        cost = sqrtf(MAX(0.0f, 1.0f - sint * sint));
        setvf(result, FrDielScalar(fabsf(cosi), cost, ei, et));
    }
}

static eiFORCEINLINE void SchlickFresnel(
	eiVector * const result, 
	const eiScalar costheta, 
	const eiVector *Rs)
{
	eiScalar k = powf(1.0f - costheta, 5.0f);
	result->r = Rs->r + k * (1.0f - Rs->r);
	result->g = Rs->g + k * (1.0f - Rs->g);
	result->b = Rs->b + k * (1.0f - Rs->b);
}

void ei_fresnelblend_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi, 
	const eiVector *Rd, 
	const eiVector *Rs, 
	eiMicrofacet_bsdf dist, 
	void *dist_param)
{
	eiVector	wh;
	eiScalar	Kd, Ks, D;
	eiVector	diffuse, specular, F;

	add(&wh, wi, wo);
	if (wh.x == 0.0f && wh.y == 0.0f && wh.z == 0.0f) {
		setvf(f, 0.0f);
		return;
	}
	normalizei(&wh);
	Kd = (28.0f / (23.0f * (eiScalar)eiPI)) * 
		(1.0f - powf(1.0f - 0.5f * AbsCosTheta(wi), 5.0f)) * 
		(1.0f - powf(1.0f - 0.5f * AbsCosTheta(wo), 5.0f));
	diffuse.r = Kd * Rd->r * (1.0f - Rs->r);
	diffuse.g = Kd * Rd->g * (1.0f - Rs->g);
	diffuse.b = Kd * Rd->b * (1.0f - Rs->b);
	D = dist(&wh, dist_param);
	SchlickFresnel(&F, dot(wi, &wh), Rs);
	Ks = 1.0f / (4.0f * absf(dot(wi, &wh)) * MAX(AbsCosTheta(wi), AbsCosTheta(wo)));
	specular.r = Ks * D * F.r;
	specular.g = Ks * D * F.g;
	specular.b = Ks * D * F.b;
	add(f, &diffuse, &specular);
}

eiBool ei_fresnelblend_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	eiMicrofacet_sample_bsdf dist, 
	void *dist_param, 
	eiScalar u1, eiScalar u2)
{
	if (u1 < 0.5f) {
		u1 = 2.0f * u1;
		ei_cosine_sample_hemisphere(wi, u1, u2);
		if (wo->z < 0.0f) {
			wi->z *= -1.f;
		}
	} else {
		u1 = 2.0f * (u1 - 0.5f);
		dist(wo, wi, dist_param, u1, u2);
		if (!SameHemisphere(wo, wi)) {
			return eiFALSE;
		}
	}
	return eiTRUE;
}

eiScalar ei_fresnelblend_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	eiMicrofacet_pdf dist, 
	void *dist_param)
{
	return 0.5f * (AbsCosTheta(wi) * (1.0f / (eiScalar)eiPI) + dist(wo, wi, dist_param));
}

void ei_lambert_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi)
{
	setvf(f, 1.0f / (eiScalar)eiPI);
}

eiBool ei_lambert_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	eiScalar u1, eiScalar u2)
{
	ei_cosine_sample_hemisphere(wi, u1, u2);
	if (wo->z < 0.0f) {
		wi->z *= -1.0f;
	}
	return eiTRUE;
}

eiScalar ei_lambert_pdf(
	const eiVector *wo, 
	const eiVector *wi)
{
	return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * (1.0f / (eiScalar)eiPI) : 0.0f;
}

static eiFORCEINLINE void OrenNayarPrecompute(
	eiScalar * const A, 
	eiScalar * const B, 
	const eiScalar sigma)
{
	eiScalar sigma2 = sigma * sigma;
	*A = 1.0f - (sigma2 / (2.0f * (sigma2 + 0.33f)));
	*B = 0.45f * sigma2 / (sigma2 + 0.09f);
}

void ei_orennayar_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar sig)
{
	eiScalar	A, B;
	eiScalar	sinthetai, sinthetao;
	eiScalar	maxcos;
	eiScalar	sinalpha, tanbeta;

	OrenNayarPrecompute(&A, &B, sig);

	sinthetai = SinTheta(wi);
    sinthetao = SinTheta(wo);
    /* compute cosine term of Oren-Nayar model */
    maxcos = 0.0f;
    if (sinthetai > 1e-4f && sinthetao > 1e-4f) {
		eiScalar	sinphii, cosphii, sinphio, cosphio;
		eiScalar	dcos;
        sinphii = SinPhi(wi);
		cosphii = CosPhi(wi);
		sinphio = SinPhi(wo);
		cosphio = CosPhi(wo);
        dcos = cosphii * cosphio + sinphii * sinphio;
        maxcos = MAX(0.0f, dcos);
    }

    /* compute sine and tangent terms of Oren-Nayar model */
    if (AbsCosTheta(wi) > AbsCosTheta(wo)) {
        sinalpha = sinthetao;
        tanbeta = sinthetai / AbsCosTheta(wi);
    }
    else {
        sinalpha = sinthetai;
        tanbeta = sinthetao / AbsCosTheta(wo);
    }
   setvf(f, (1.0f / (eiScalar)eiPI) * (A + B * maxcos * sinalpha * tanbeta));
}

eiBool ei_orennayar_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	const eiScalar sig, 
	eiScalar u1, eiScalar u2)
{
	ei_cosine_sample_hemisphere(wi, u1, u2);
	if (wo->z < 0.0f) {
		wi->z *= -1.0f;
	}
	return eiTRUE;
}

eiScalar ei_orennayar_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar sig)
{
	return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * (1.0f / (eiScalar)eiPI) : 0.0f;
}

void ei_specularreflection_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi)
{
	setvf(f, 1.0f / MAX(eiSCALAR_EPS, AbsCosTheta(wi)));
}

eiBool ei_specularreflection_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi)
{
	/* compute perfect specular reflection direction */
    wi->x = -wo->x;
	wi->y = -wo->y;
	wi->z = wo->z;

	return eiTRUE;
}

eiScalar ei_specularreflection_pdf(
	const eiVector *wo, 
	const eiVector *wi)
{
	return 1.0f;
}

void ei_speculartransmission_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar etai, 
	const eiScalar etat)
{
	setvf(f, 1.0f / MAX(eiSCALAR_EPS, AbsCosTheta(wi)));
}

eiBool ei_speculartransmission_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	const eiScalar etai, 
	const eiScalar etat)
{
	eiBool		entering;
	eiScalar	ei, et;
	eiScalar	sini2, eta, sint2;
	eiScalar	cost, sintOverSini;

	/* figure out which eta is incident and which is transmitted */
    entering = CosTheta(wo) > 0.0f;
    ei = etai;
	et = etat;
	if (!entering) {
		eiScalar	tmp;
		/* swap ei, et */
		tmp = ei;
		ei = et;
		et = tmp;
	}

    /* compute transmitted ray direction */
	sini2 = SinTheta2(wo);
	eta = ei / et;
	sint2 = eta * eta * sini2;

    /* handle total internal reflection for transmission */
	if (sint2 >= 1.0f) {
		return eiFALSE;
	}
    cost = sqrtf(MAX(0.f, 1.0f - sint2));
	if (entering) {
		cost = -cost;
	}
    sintOverSini = eta;
    wi->x = sintOverSini * -wo->x;
	wi->y = sintOverSini * -wo->y;
	wi->z = cost;

	return eiTRUE;
}

eiScalar ei_speculartransmission_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar etai, 
	const eiScalar etat)
{
	return 1.0f;
}

static eiFORCEINLINE eiScalar G(
	const eiVector *wo, 
	const eiVector *wi, 
	const eiVector *wh)
{
	eiScalar NdotWh = AbsCosTheta(wh);
	eiScalar NdotWo = AbsCosTheta(wo);
	eiScalar NdotWi = AbsCosTheta(wi);
	eiScalar WOdotWh = fabsf(dot(wo, wh));
	return MIN(1.0f, MIN((2.0f * NdotWh * NdotWo / WOdotWh),
		(2.0f * NdotWh * NdotWi / WOdotWh)));
}

void ei_microfacet_term(
	eiVector * const result, 
	const eiVector *wo, 
	const eiVector *wi)
{
	eiScalar	cosThetaO, cosThetaI;
	eiVector	wh;

    cosThetaO = AbsCosTheta(wo);
    cosThetaI = AbsCosTheta(wi);
	if (cosThetaI == 0.f || cosThetaO == 0.f) {
		setvf(result, 0.0f);
		return;
	}
	add(&wh, wi, wo);
	if (wh.x == 0.0f && wh.y == 0.0f && wh.z == 0.0f) {
		setvf(result, 0.0f);
		return;
	}
    normalizei(&wh);
    setvf(result, G(wo, wi, &wh) / (4.0f * cosThetaI * cosThetaO));
}

eiScalar ei_blinn_microfacet_bsdf(
	const eiVector *wh, 
	void *param)
{
	eiBlinnParams	*p;
	eiScalar		costhetah;

	p = (eiBlinnParams *)param;

	costhetah = AbsCosTheta(wh);
	return (p->exponent + 2.0f) * (1.0f / (2.0f * (eiScalar)eiPI)) * powf(costhetah, p->exponent);
}

void ei_blinn_microfacet_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	void *param, 
	eiScalar u1, eiScalar u2)
{
	eiBlinnParams	*p;
	eiScalar		costheta, sintheta, phi;
	eiVector		wh;

	p = (eiBlinnParams *)param;

	/* compute sampled half-angle vector wh for Blinn distribution */
    costheta = powf(u1, 1.0f / (p->exponent + 1.0f));
    sintheta = sqrtf(MAX(0.0f, 1.0f - costheta * costheta));
    phi = u2 * 2.0f * (eiScalar)eiPI;
    setv(&wh, sintheta * cosf(phi), sintheta * sinf(phi), costheta);
	if (!SameHemisphere(wo, &wh)) {
		neg(&wh, &wh);
	}

    /* generate wi by reflecting about wh */
    ReflectSpecular(wi, wo, &wh);
}

eiScalar ei_blinn_microfacet_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	void *param)
{
	eiBlinnParams	*p;
	eiVector		wh;
	eiScalar		costheta, pdf;

	p = (eiBlinnParams *)param;

	add(&wh, wo, wi);
	normalizei(&wh);

	costheta = AbsCosTheta(&wh);
	pdf = 0.0f;
	if (dot(wo, &wh) > 0.0f) {
		pdf = ((p->exponent + 1.0f) * powf(costheta, p->exponent)) / 
			(2.0f * (eiScalar)eiPI * 4.0f * dot(wo, &wh));
	}
    return pdf;
}

void ei_blinn_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar exponent)
{
	eiBlinnParams	param;
	eiVector		wh;

	param.exponent = exponent;
	
	add(&wh, wo, wi);
	normalizei(&wh);

	setvf(f, ei_blinn_microfacet_bsdf(
		&wh, 
		&param));
}

eiBool ei_blinn_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	const eiScalar exponent, 
	eiScalar u1, eiScalar u2)
{
	eiBlinnParams	param;

	param.exponent = exponent;

	ei_blinn_microfacet_sample_bsdf(
		wo, 
		wi, 
		&param, 
		u1, u2);

	return SameHemisphere(wo, wi);
}

eiScalar ei_blinn_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar exponent)
{
	eiBlinnParams	param;

	param.exponent;

	return ei_blinn_microfacet_pdf(
		wo, 
		wi, 
		&param);
}

eiScalar ei_ward_microfacet_bsdf(
	const eiVector *wh, 
	void *param)
{
	eiWardParams	*p;
	eiScalar		costhetah, d, e;

	p = (eiWardParams *)param;

	costhetah = AbsCosTheta(wh);
	d = 1.0f - costhetah * costhetah;
	if (d == 0.0f) {
		return 0.0f;
	}
	e = (p->shiny_u * wh->x * wh->x + p->shiny_v * wh->y * wh->y) / d;
	return sqrtf((p->shiny_u + 2.0f) * (p->shiny_v + 2.0f)) * (1.0f / (2.0f * (eiScalar)eiPI)) * powf(costhetah, e);
}

static eiFORCEINLINE void ei_ward_sample_quadrant(
	const eiScalar u1, const eiScalar u2, 
	const eiScalar shiny_u, const eiScalar shiny_v, 
	eiScalar *phi, eiScalar *costheta)
{
	eiScalar	cosphi, sinphi;
	if (shiny_u == shiny_v)
	{
		*phi = (eiScalar)eiPI * u1 * 0.5f;
	}
	else
	{
		*phi = atanf(sqrtf((shiny_u + 1.0f) / (shiny_v + 1.0f)) * tanf((eiScalar)eiPI * u1 * 0.5f));
	}
	cosphi = cosf(*phi);
	sinphi = sinf(*phi);
	*costheta = powf(u2, 1.0f / (shiny_u * cosphi * cosphi + shiny_v * sinphi * sinphi + 1.0f));
}

void ei_ward_microfacet_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	void *param, 
	eiScalar u1, eiScalar u2)
{
	eiWardParams	*p;
	/* sample from first quadrant and remap to hemisphere */
	eiScalar		phi, costheta, sintheta;
	eiVector		wh;

	p = (eiWardParams *)param;

	if (u1 < 0.25f) {
		ei_ward_sample_quadrant(4.0f * u1, u2, p->shiny_u, p->shiny_v, &phi, &costheta);
	} else if (u1 < 0.5f) {
		u1 = 4.0f * (0.5f - u1);
		ei_ward_sample_quadrant(u1, u2, p->shiny_u, p->shiny_v, &phi, &costheta);
		phi = (eiScalar)eiPI - phi;
	} else if (u1 < 0.75f) {
		u1 = 4.0f * (u1 - 0.5f);
		ei_ward_sample_quadrant(u1, u2, p->shiny_u, p->shiny_v, &phi, &costheta);
		phi += (eiScalar)eiPI;
	} else {
		u1 = 4.0f * (1.0f - u1);
		ei_ward_sample_quadrant(u1, u2, p->shiny_u, p->shiny_v, &phi, &costheta);
		phi = 2.0f * (eiScalar)eiPI - phi;
	}
	sintheta = sqrtf(MAX(0.0f, 1.0f - costheta * costheta));
	setv(&wh, sintheta * cosf(phi), sintheta * sinf(phi), costheta);
	if (!SameHemisphere(wo, &wh)) {
		neg(&wh, &wh);
	}

	/* generate wi by reflecting about wh */
	ReflectSpecular(wi, wo, &wh);
}

eiScalar ei_ward_microfacet_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	void *param)
{
	eiWardParams	*p;
	eiVector		wh;
	eiScalar		costhetah, ds, pdf;

	p = (eiWardParams *)param;

	add(&wh, wo, wi);
	normalizei(&wh);

    costhetah = AbsCosTheta(&wh);
    ds = 1.f - costhetah * costhetah;
    pdf = 0.0f;
    if (ds > 0.0f && dot(wo, &wh) > 0.0f) {
		eiScalar	e, d;
        e = (p->shiny_u * wh.x * wh.x + p->shiny_v * wh.y * wh.y) / ds;
        d = sqrtf((p->shiny_u + 1.0f) * (p->shiny_v + 1.0f)) * (1.0f / (2.0f * (eiScalar)eiPI)) * 
			powf(costhetah, e);
        pdf = d / (4.0f * dot(wo, &wh));
    }
    return pdf;
}

void ei_ward_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar shiny_u, const eiScalar shiny_v)
{
	eiWardParams	param;
	eiVector		wh;

	param.shiny_u = shiny_u;
	param.shiny_v = shiny_v;

	add(&wh, wo, wi);
	normalizei(&wh);

	setvf(f, ei_ward_microfacet_bsdf(
		&wh, 
		&param));
}

eiBool ei_ward_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	const eiScalar shiny_u, const eiScalar shiny_v, 
	eiScalar u1, eiScalar u2)
{
	eiWardParams	param;

	param.shiny_u = shiny_u;
	param.shiny_v = shiny_v;

	ei_ward_microfacet_sample_bsdf(
		wo, 
		wi, 
		&param, 
		u1, u2);

	return SameHemisphere(wo, wi);
}

eiScalar ei_ward_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar shiny_u, const eiScalar shiny_v)
{
	eiWardParams	param;

	param.shiny_u = shiny_u;
	param.shiny_v = shiny_v;

	return ei_ward_microfacet_pdf(
		wo, 
		wi, 
		&param);
}

void ei_scalar_texture(
	eiState * const state, 
	eiScalar *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s, const eiScalar t)
{
	ei_lookup_scalar_texture(
		state->db, 
		value, 
		tag, 
		channel, 
		s, t);
}

void ei_vector_texture(
	eiState * const state, 
	eiVector *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s, const eiScalar t)
{
	ei_lookup_vector_texture(
		state->db, 
		value, 
		tag, 
		channel, 
		s, t);
}

void ei_scalar_texture_filtered(
	eiState * const state, 
	eiScalar *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s1, const eiScalar t1, 
	const eiScalar s2, const eiScalar t2, 
	const eiScalar s3, const eiScalar t3, 
	const eiScalar s4, const eiScalar t4)
{
	ei_lookup_scalar_texture_filtered(
		state->db, 
		value, 
		tag, 
		channel, 
		s1, t1, 
		s2, t2, 
		s3, t3, 
		s4, t4);
}

void ei_vector_texture_filtered(
	eiState * const state, 
	eiVector *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s1, const eiScalar t1, 
	const eiScalar s2, const eiScalar t2, 
	const eiScalar s3, const eiScalar t3, 
	const eiScalar s4, const eiScalar t4)
{
	ei_lookup_vector_texture_filtered(
		state->db, 
		value, 
		tag, 
		channel, 
		s1, t1, 
		s2, t2, 
		s3, t3, 
		s4, t4);
}

eiGeoScalar ei_state_random(eiState * const state)
{
	return ei_random(&state->bucket->randGen);
}

void ei_to_camera(eiMatrix *mat, eiState * const state)
{
	initm(mat, 1.0f);
}

void ei_from_camera(eiMatrix *mat, eiState * const state)
{
	initm(mat, 1.0f);
}

void ei_to_object(eiMatrix *mat, eiState * const state)
{
	eiRayObjectInstance *inst;

	if (state->hit_inst == eiNULL_TAG)
	{
		initm(mat, 1.0f);
		return;
	}

	inst = (eiRayObjectInstance *)ei_db_access(state->db, state->hit_inst);

	mulmm(mat, &state->cam->camera_to_world, &inst->world_to_object);
	
	ei_db_end(state->db, state->hit_inst);
}

void ei_from_object(eiMatrix *mat, eiState * const state)
{
	eiRayObjectInstance *inst;

	if (state->hit_inst == eiNULL_TAG)
	{
		initm(mat, 1.0f);
		return;
	}

	inst = (eiRayObjectInstance *)ei_db_access(state->db, state->hit_inst);

	mulmm(mat, &inst->object_to_world, &state->cam->world_to_camera);
	
	ei_db_end(state->db, state->hit_inst);
}

void ei_to_light(eiMatrix *mat, eiState * const state)
{
	eiBaseBucket			*bucket;
	eiDataTableIterator		*light_insts_iter;
	eiLightInstance			*light_inst;

	if (state->current_light_index == -1)
	{
		initm(mat, 1.0f);
		return;
	}

	bucket = state->bucket;
	light_insts_iter = &bucket->light_insts_iter;
	light_inst = (eiLightInstance *)ei_data_table_read(light_insts_iter, state->current_light_index);

	mulmm(mat, &state->cam->camera_to_world, &light_inst->world_to_light);
}

void ei_from_light(eiMatrix *mat, eiState * const state)
{
	eiBaseBucket			*bucket;
	eiDataTableIterator		*light_insts_iter;
	eiLightInstance			*light_inst;

	if (state->current_light_index == -1)
	{
		initm(mat, 1.0f);
		return;
	}

	bucket = state->bucket;
	light_insts_iter = &bucket->light_insts_iter;
	light_inst = (eiLightInstance *)ei_data_table_read(light_insts_iter, state->current_light_index);

	mulmm(mat, &light_inst->light_to_world, &state->cam->world_to_camera);
}

void ei_to_world(eiMatrix *mat, eiState * const state)
{
	movm(mat, &state->cam->camera_to_world);
}

void ei_from_world(eiMatrix *mat, eiState * const state)
{
	movm(mat, &state->cam->world_to_camera);
}

void ei_to_local(eiVector * const r, const eiVector *v, eiState * const state)
{
	eiVector	u_axis, v_axis;

	ortho_basis(&state->N, &u_axis, &v_axis);

	r->x = dot(v, &u_axis);
	r->y = dot(v, &v_axis);
	r->z = dot(v, &state->N);
}

void ei_from_local(eiVector * const r, const eiVector *v, eiState * const state)
{
	eiVector	u_axis, v_axis;

	ortho_basis(&state->N, &u_axis, &v_axis);

	r->x = u_axis.x * v->x + v_axis.x * v->y + state->N.x * v->z;
	r->y = u_axis.y * v->x + v_axis.y * v->y + state->N.y * v->z;
	r->z = u_axis.z * v->x + v_axis.z * v->y + state->N.z * v->z;
}

eiScalar ei_brdf_cook_torrance(
	const eiVector *in, 
	const eiVector *out, 
	const eiVector *normal, 
	eiScalar r)
{
	eiVector half;
	eiScalar dotNH, dotNV, dotNL, dotVH, shadowing, masking, G;
	eiScalar cosAlpha, sinAlpha, tanAlpha, tanAlphaOverR, D;
	eiScalar result = 0;

	// H
	add(&half, in, out);
	normalizei(&half);

	// G
	dotNH = dot(normal, &half);
	if (dotNH > 0)
	{
		dotNV = dot(normal, out);
		dotNL = dot(normal, in);
		dotVH = dot(in, &half);
		shadowing = dotNH * dotNV / dotVH;
		masking = dotNH * dotNL / dotVH;
		G = MIN(1.0f, 2.0f * MIN(shadowing, masking));

		// D
		cosAlpha = MIN(dotNH, 1.0f);
		sinAlpha = sqrtf(1.0f - cosAlpha * cosAlpha);
		tanAlpha = sinAlpha / cosAlpha;
		tanAlphaOverR = MAX(0, tanAlpha) / r;
		D = expf(- powf(tanAlphaOverR, 2)) / (powf(r, 2) * powf(cosAlpha, 4));

		result = D * G / (dotNL * dotNV * (eiScalar)eiPI);
	}

	eiASSERT(!isnan(result));
	return result;
}
