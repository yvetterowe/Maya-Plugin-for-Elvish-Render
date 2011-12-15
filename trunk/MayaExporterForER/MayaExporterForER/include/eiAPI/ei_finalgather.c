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

#include <eiAPI/ei_finalgather.h>
#include <eiAPI/ei_globillum.h>
#include <eiAPI/ei_shadesys.h>
#include <eiAPI/ei_material.h>
#include <eiAPI/ei_sampler.h>
#include <eiCORE/ei_data_table.h>
#include <eiCORE/ei_qmc.h>
#include <eiCORE/ei_assert.h>

#define BIG_NUM					1e+5f
#define HARMONIC_MEAN_COEFF		0.1f
#define MAX_TAN					999999.999999499999999999875f /* tan(89.9999) */

void ei_hemisphere_sample_zero_item(void *item, const eiInt item_size)
{
	eiHemisphereSample *sample = (eiHemisphereSample *)item;

	initv(&sample->color);
}

void ei_hemisphere_sample_add_item(void *item, void *value, const eiInt item_size)
{
	eiHemisphereSample *lhs = (eiHemisphereSample *)item;
	eiHemisphereSample *rhs = (eiHemisphereSample *)value;

	addi(&lhs->color, &rhs->color);
}

void ei_hemisphere_sample_mul_item(void *item, const eiScalar scale, const eiInt item_size)
{
	eiHemisphereSample *sample = (eiHemisphereSample *)item;

	mulvfi(&sample->color, scale);
}

void ei_irrad_grad_init(eiIrradianceGradient *grad)
{
	initv(&grad->r);
	initv(&grad->g);
	initv(&grad->b);
}

void ei_irrad_init(
	eiIrradiance *irrad, 
	const eiVector *Pi, 
	const eiVector *Ni, 
	const eiVector *Ei, 
	const eiIrradianceGradient *GradR_Ei, 
	const eiIrradianceGradient *GradT_Ei, 
	const eiScalar inv_Ri)
{
	movv(&irrad->base.pos, Pi);
	movv(&irrad->Ni, Ni);
	setRGBE(&irrad->Ei, Ei);
	setRGBE(&irrad->GradR_Ei[0], &GradR_Ei->r);
	setRGBE(&irrad->GradR_Ei[1], &GradR_Ei->g);
	setRGBE(&irrad->GradR_Ei[2], &GradR_Ei->b);
	setRGBE(&irrad->GradT_Ei[0], &GradT_Ei->r);
	setRGBE(&irrad->GradT_Ei[1], &GradT_Ei->g);
	setRGBE(&irrad->GradT_Ei[2], &GradT_Ei->b);
	irrad->inv_Ri = inv_Ri;
}

void ei_irrad_copy(
	eiIrradiance *irrad, 
	const eiIrradiance *value)
{
	movv(&irrad->base.pos, &value->base.pos);
	movv(&irrad->Ni, &value->Ni);
	irrad->Ei = value->Ei;
	irrad->GradR_Ei[0] = value->GradR_Ei[0];
	irrad->GradR_Ei[1] = value->GradR_Ei[1];
	irrad->GradR_Ei[2] = value->GradR_Ei[2];
	irrad->GradT_Ei[0] = value->GradT_Ei[0];
	irrad->GradT_Ei[1] = value->GradT_Ei[1];
	irrad->GradT_Ei[2] = value->GradT_Ei[2];
	irrad->inv_Ri = value->inv_Ri;
}

void byteswap_irradiance(eiDatabase *db, void *ptr, const eiUint size)
{
	eiIrradiance	*irrad;

	irrad = (eiIrradiance *)ptr;

	ei_byteswap_map_node(&irrad->base);
	ei_byteswap_vector(&irrad->Ni);
	ei_byteswap_scalar(&irrad->inv_Ri);
}

eiBool ei_irrad_cond_proc(
	const eiMapNode *node, 
	const eiScalar R2, 
	void *param)
{
	eiIrradianceCondition	*cond;
	eiIrradiance			*irrad;
	eiScalar				inv_A;
	eiScalar				inv_Rmin;
	eiScalar				inv_Rmax;
	eiScalar				inv_Ri;
	eiScalar				wi;

	cond = (eiIrradianceCondition *)param;
	irrad = (eiIrradiance *)node;

	inv_A = 1.0f / cond->A;
	inv_Rmin = inv_A * (1.0f / 10.0f);
	inv_Rmax = inv_A * (1.0f / 1.5f);
	inv_Ri = irrad->inv_Ri;

	clampi(inv_Ri, inv_Rmin, inv_Rmax);

	wi = Fast_sqrt(R2) * inv_Ri + Fast_sqrt(MAX(0.0f, 1.0f - dot(&cond->N, &irrad->Ni)));

	if ((1.0f - EI_FG_ERROR_COEFF * wi) < 0.0f)
	{
		return eiFALSE;
	}
	else
	{
		return eiTRUE;
	}
}

eiBool ei_irrad_force_cond_proc(
	const eiMapNode *node, 
	const eiScalar R2, 
	void *param)
{
	eiIrradianceForceCondition	*cond;
	eiIrradiance				*irrad;

	cond = (eiIrradianceForceCondition *)param;
	irrad = (eiIrradiance *)node;

	if (dot(&cond->N, &irrad->Ni) > (eiSQRT2 * 0.5f))
	{
		return eiTRUE;
	}
	else
	{
		return eiFALSE;
	}
}

static eiFORCEINLINE void interp_irradiances(
	eiDatabase *db, 
	const eiTag tag, 
	eiVector *L, 
	const eiVector *P, 
	const eiVector *N, 
	const eiScalar A, 
	eiScalar * const w, 
	eiUint * const numAvailablePoints, 
	eiMapLookup *np)
{
	eiMap					*map;
	eiDataTableIterator		iter;
	eiIrradianceGradient	GradR_Ei;
	eiIrradianceGradient	GradT_Ei;
	eiInt					i;

	map = (eiMap *)ei_db_access(db, tag);
	ei_data_table_begin(db, map->points, &iter);

	ei_irrad_grad_init(&GradR_Ei);
	ei_irrad_grad_init(&GradT_Ei);

	for (i = 1; i <= np->found; ++i)
	{
		eiIrradiance	*irrad;
		eiVector		Li, dn, dp;
		eiScalar		wi;
		eiVector		u_axis, v_axis, local_dn, local_dp;
		
		irrad = (eiIrradiance *)ei_data_table_read(&iter, np->index[i]);

		sub(&dp, P, &irrad->base.pos);

		wi = Fast_sqrt(np->dist2[i]) * irrad->inv_Ri 
			+ Fast_sqrt(MAX(0.0f, 1.0f - dot(N, &irrad->Ni)));

		++ (*numAvailablePoints);

		if (almost_zero(wi, eiSCALAR_EPS))
		{
			wi = BIG_NUM;
		}
		else
		{
			wi = 1.0f / wi;
		}
		(*w) += wi;

		cross(&dn, &irrad->Ni, N);

		ortho_basis(&irrad->Ni, &u_axis, &v_axis);

		local_dn.x = dot(&dn, &u_axis);
		local_dn.y = dot(&dn, &v_axis);
		local_dn.z = dot(&dn, &irrad->Ni);
		local_dp.x = dot(&dp, &u_axis);
		local_dp.y = dot(&dp, &v_axis);
		local_dp.z = dot(&dp, &irrad->Ni);

		getRGBE(&Li, &irrad->Ei);

		getRGBE(&GradR_Ei.r, &irrad->GradR_Ei[0]);
		getRGBE(&GradR_Ei.g, &irrad->GradR_Ei[1]);
		getRGBE(&GradR_Ei.b, &irrad->GradR_Ei[2]);

		getRGBE(&GradT_Ei.r, &irrad->GradT_Ei[0]);
		getRGBE(&GradT_Ei.g, &irrad->GradT_Ei[1]);
		getRGBE(&GradT_Ei.b, &irrad->GradT_Ei[2]);

		Li.r += dot(&local_dn, &GradR_Ei.r);
		Li.g += dot(&local_dn, &GradR_Ei.g);
		Li.b += dot(&local_dn, &GradR_Ei.b);

		Li.r += dot(&local_dp, &GradT_Ei.r);
		Li.g += dot(&local_dp, &GradT_Ei.g);
		Li.b += dot(&local_dp, &GradT_Ei.b);

		mulvfi(&Li, wi * (1.0f / (eiScalar)eiPI));

		addi(L, &Li);
	}

	ei_data_table_end(&iter);
	ei_db_end(db, tag);
}

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
	eiUint * const numAvailablePoints)
{
	eiTLS					*tls;
	eiGlobillumTLS					*pTls;
	eiMapLookup				np;
	eiIrradianceCondition	cond;

	tls = ei_db_get_tls(db);
	pTls = (eiGlobillumTLS *)ei_tls_get_interface(tls, EI_TLS_TYPE_GLOBILLUM);

	/* we can resize it based on the fact that memory will not be de-allocated 
	   for array, it just increases the capacity. */
	ei_array_resize(&pTls->map_lookup_dist2, gather_points + 1);
	ei_array_resize(&pTls->map_lookup_index, gather_points + 1);

	np.dist2 = (eiScalar *)ei_array_data(&pTls->map_lookup_dist2);
	np.index = (eiInt *)ei_array_data(&pTls->map_lookup_index);

	movv(&np.pos, P);
	np.max = gather_points;
	np.found = 0;
	np.got_heap = 0;
	np.dist2[0] = max_dist * max_dist;

	movv(&cond.P, P);
	movv(&cond.N, N);
	cond.A = A;

	ei_map_locate_points(
		db, 
		tag, 
		&np, 
		1, 
		ei_irrad_cond_proc, 
		(void *)(&cond));

	interp_irradiances(
		db, 
		tag, 
		L, 
		P, 
		N, 
		A, 
		w, 
		numAvailablePoints, 
		&np);
}

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
	eiUint * const numAvailablePoints)
{
	eiTLS							*tls;
	eiGlobillumTLS					*pTls;
	eiMapLookup						np;
	eiIrradianceForceCondition		cond;

	tls = ei_db_get_tls(db);
	pTls = (eiGlobillumTLS *)ei_tls_get_interface(tls, EI_TLS_TYPE_GLOBILLUM);

	/* we can resize it based on the fact that memory will not be de-allocated 
	   for array, it just increases the capacity. */
	ei_array_resize(&pTls->map_lookup_dist2, gather_points + 1);
	ei_array_resize(&pTls->map_lookup_index, gather_points + 1);

	np.dist2 = (eiScalar *)ei_array_data(&pTls->map_lookup_dist2);
	np.index = (eiInt *)ei_array_data(&pTls->map_lookup_index);

	movv(&np.pos, P);
	np.max = gather_points;
	np.found = 0;
	np.got_heap = 0;
	np.dist2[0] = max_dist * max_dist;

	movv(&cond.P, P);
	movv(&cond.N, N);
	cond.proj_pixel_area = A;

	ei_map_locate_points(
		db, 
		tag, 
		&np, 
		1, 
		ei_irrad_force_cond_proc, 
		(void *)(&cond));

	interp_irradiances(
		db, 
		tag, 
		L, 
		P, 
		N, 
		A, 
		w, 
		numAvailablePoints, 
		&np);
}

static eiBool ei_finalgather_ray_hit_proc(eiState *state)
{
	return eiTRUE;
}

eiBool ei_rt_trace_finalgather(
	eiRayTracer *rt, 
	eiNodeSystem *nodesys, 
	eiState *state)
{
	if (ei_rt_trace(rt, state, ei_finalgather_ray_hit_proc, eiFALSE))
	{
		eiVector4	result;
		eiBucket	*bucket;
		eiScalar	dist;

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

		/* linearly fade off */
		bucket = (eiBucket *)state->bucket;
		dist = state->hit_t;

		if (bucket->par.finalgather_falloff && 
			dist > bucket->par.finalgather_falloff_start)
		{
			eiVector	fore_color, back_color;
			eiVector	result;

			fore_color = state->result->color;

			/* shade by background */
			initv(&result);

			ei_trace_environment(&result, state, &state->I);

			back_color = state->result->color;

			lerp3(&state->result->color, &fore_color, &back_color, 
				(dist - bucket->par.finalgather_falloff_start) / 
				(bucket->par.finalgather_falloff_stop - bucket->par.finalgather_falloff_start));
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

void ei_sample_finalgather(
	eiVector *color, 
	eiState *state)
{
	eiRayTracer				*rt;
	eiNodeSystem			*nodesys;
	eiTLS					*tls;
	eiGlobillumTLS			*pTls;
	eiVector				biased_P;
	eiBucket				*bucket = (eiBucket *)state->bucket;
	eiScalar				Ri = bucket->par.finalgather_radius;
	eiScalar				hamRi = 0.0f;
	eiIrradianceGradient	gradR;
	eiIrradianceGradient	gradT;
	eiBuffer				*fg_buffer;
	eiScalar				sin_theta, cos_theta, phi;
	eiUint					M, N;
	eiScalar				inv_M, inv_N;
	eiScalar				MN;
	eiScalar				inv_MN;
	eiVector				u_axis, v_axis;
	eiScalar				offset_x, offset_y;
	eiUint					instance_offset = 0;
	eiUint					j, k;
	eiScalar				inv_Ri = BIG_NUM;
	eiScalar				inv_hamRi = BIG_NUM;
	eiIrradiance			irrad;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);

	tls = ei_db_get_tls(state->db);
	pTls = (eiGlobillumTLS *)ei_tls_get_interface(tls, EI_TLS_TYPE_GLOBILLUM);

	initv(color);
	ei_irrad_grad_init(&gradR);
	ei_irrad_grad_init(&gradT);

	/* move the position a little backward from the hit point 
	   to eliminate numerous problem */
	movv(&biased_P, &state->I);
	mulvfi(&biased_P, - calc_bias(&state->Ng, &state->I, state->bias, state->bias_scale));
	addi(&biased_P, &state->P);

	/* get hemisphere sample buffer from thread local storage */
	fg_buffer = ei_globillum_tls_acquire_finalgather_buffer(pTls, state->opt->finalgather_rays);

	/* stratified sampling */
	M = ei_buffer_get_width(fg_buffer);
	N = ei_buffer_get_height(fg_buffer);
	inv_M = 1.0f / (eiScalar)M;
	inv_N = 1.0f / (eiScalar)N;
	MN = (eiScalar)(M * N);
	inv_MN = 1.0f / MN;

	ortho_basis(&state->N, &u_axis, &v_axis);

	/* get an offset point from the global sequence for current dimension */
	offset_x = (eiScalar)ei_sigma(state->dimension, state->instance_number);
	offset_y = (eiScalar)ei_sigma(state->dimension + 1, state->instance_number);

	for (k = 0; k < N; ++k)
	{
		for (j = 0; j < M; ++j)
		{
			eiScalar			randVar[2];
			eiState				fg_ray;
			eiVector			prev_color;
			eiVector			prev_opacity;
			eiVector			dir;
			eiVector			local_u_axis, local_v_axis, local_w_axis;
			eiHemisphereSample	hsample;

			/* a Hammersley point set is used here for better discrepancy */
			randVar[0] = fmodf(offset_x + (eiScalar)instance_offset * inv_MN, 1.0f);
			randVar[1] = fmodf(offset_y + (eiScalar)ei_sigma(0, instance_offset), 1.0f);

			ei_state_init(&fg_ray, eiRAY_FINALGATHER, state->bucket);

			/* backup color and opacity */
			movv(&prev_color, &state->result->color);
			movv(&prev_opacity, &state->result->opacity);
			fg_ray.result = state->result;

			initv(&fg_ray.result->color);
			initv(&fg_ray.result->opacity);

			fg_ray.instance_number = state->instance_number + instance_offset;

			sin_theta = sqrtf(((eiScalar)j + randVar[0]) * inv_M);
			cos_theta = sqrtf(1.0f - sin_theta * sin_theta);
			phi = 2.0f * (eiScalar)eiPI * ((eiScalar)k + randVar[1]) * inv_N;
			dir.x = sin_theta * cosf(phi);
			dir.y = sin_theta * sinf(phi);
			dir.z = cos_theta;

			local_u_axis = u_axis;
			mulvfi(&local_u_axis, dir.x);
			local_v_axis = v_axis;
			mulvfi(&local_v_axis, dir.y);
			local_w_axis = state->N;
			mulvfi(&local_w_axis, dir.z);
			add(&dir, &local_u_axis, &local_v_axis);
			addi(&dir, &local_w_axis);

			fg_ray.org = biased_P;
			fg_ray.dir = dir;

			if (bucket->par.finalgather_falloff)
			{
				fg_ray.max_t = bucket->par.finalgather_falloff_stop;
			}

			fg_ray.dimension = state->dimension + 2;
			fg_ray.time = state->time;
			fg_ray.dtime = state->dtime;
			fg_ray.raster = state->raster;

			ei_state_inherit_volume(&fg_ray, state);

			ei_rt_trace_finalgather(rt, nodesys, &fg_ray);

			/* save the sampling result to the buffer */
			hsample.color = fg_ray.result->color;
			if (almost_zero(cos_theta, eiSCALAR_EPS))
			{
				hsample.tan_theta = MAX_TAN;
			}
			else
			{
				hsample.tan_theta = sin_theta / cos_theta;
			}
			hsample.R = fg_ray.hit_t;

			ei_buffer_set(fg_buffer, j, k, &hsample);

			/* compute the harmonic mean */
			Ri = MIN(Ri, hsample.R);
			hamRi += (1.0f / hsample.R);

			++ instance_offset;

			state->pass_motion |= fg_ray.pass_motion;

			ei_state_exit(&fg_ray);

			/* restore color and opacity */
			movv(&state->result->color, &prev_color);
			movv(&state->result->opacity, &prev_opacity);
		}
	}

	/* compute the harmonic mean */
	hamRi = MN / hamRi;

	/* filter the hemisphere samples */
	/* don't filter the result if users don't want to interpolate anything */
	if (state->opt->finalgather_samples > 0)
	{
		ei_buffer_filter(fg_buffer, truncf(state->opt->finalgather_filter_size));
	}

	for (k = 0; k < N; ++k)
	{
		eiVector				uk, vk, vk_minor;
		eiScalar				phik, phik_minor;
		eiIrradianceGradient	Sumpart1;
		eiVector				SumtL, Sumpart2;
		eiVector				vk_SumtL;
		eiVector				vk_Sumpart2;

		phik = 2.0f * (eiScalar)eiPI * ((eiScalar)k + 0.5f) * inv_N;
		phik_minor = 2.0f * (eiScalar)eiPI * (eiScalar)k * inv_N;

		setv(&uk, 
			cosf(phik), 
			sinf(phik), 
			0.0f);

		setv(&vk, 
			cosf(phik + (eiScalar)eiPI_2), 
			sinf(phik + (eiScalar)eiPI_2), 
			0.0f);

		setv(&vk_minor, 
			cosf(phik_minor + (eiScalar)eiPI_2), 
			sinf(phik_minor + (eiScalar)eiPI_2), 
			0.0f);

		ei_irrad_grad_init(&Sumpart1);
		initv(&SumtL);
		initv(&Sumpart2);

		for (j = 0; j < M; ++j)
		{
			eiScalar	sin_thetaj_minor, cos_thetaj_minor, sin_thetaj_plus;
			eiVector	tL;
			eiVector	part1, part2;
			eiVector	uk_Sumpart1;

			sin_thetaj_minor = sqrtf((eiScalar)j * inv_M);
			sin_thetaj_plus = sqrtf((eiScalar)(j + 1) * inv_M);
			cos_thetaj_minor = sqrtf(1.0f - sin_thetaj_minor * sin_thetaj_minor);

			addi(color, &(((eiHemisphereSample *)ei_buffer_getptr(fg_buffer, j, k))->color));

			/* rotational gradient */
			tL = ((eiHemisphereSample *)ei_buffer_getptr(fg_buffer, j, k))->color;
			mulvfi(&tL, - ((eiHemisphereSample *)ei_buffer_getptr(fg_buffer, j, k))->tan_theta);
			addi(&SumtL, &tL);

			/* translational gradient */
			sub(&part1, 
				&(((eiHemisphereSample *)ei_buffer_getptr(fg_buffer, j, k))->color), 
				&(((eiHemisphereSample *)ei_buffer_getptr(fg_buffer, j - 1, k))->color));

			mulvfi(&part1, 
				sin_thetaj_minor * cos_thetaj_minor * cos_thetaj_minor / 
				MIN(((eiHemisphereSample *)ei_buffer_getptr(fg_buffer, j, k))->R, 
				((eiHemisphereSample *)ei_buffer_getptr(fg_buffer, j - 1, k))->R));

			uk_Sumpart1 = uk;
			mulvfi(&uk_Sumpart1, part1.r);
			addi(&Sumpart1.r, &uk_Sumpart1);

			uk_Sumpart1 = uk;
			mulvfi(&uk_Sumpart1, part1.g);
			addi(&Sumpart1.g, &uk_Sumpart1);

			uk_Sumpart1 = uk;
			mulvfi(&uk_Sumpart1, part1.b);
			addi(&Sumpart1.b, &uk_Sumpart1);

			sub(&part2, 
				&(((eiHemisphereSample *)ei_buffer_getptr(fg_buffer, j, k))->color), 
				&(((eiHemisphereSample *)ei_buffer_getptr(fg_buffer, j, k - 1))->color));

			mulvfi(&part2, 
				(sin_thetaj_plus - sin_thetaj_minor) / 
				MIN(((eiHemisphereSample *)ei_buffer_getptr(fg_buffer, j, k))->R, 
				((eiHemisphereSample *)ei_buffer_getptr(fg_buffer, j, k - 1))->R));

			addi(&Sumpart2, &part2);
		}

		/* rotational gradient */
		vk_SumtL = vk;
		mulvfi(&vk_SumtL, SumtL.r);
		addi(&gradR.r, &vk_SumtL);

		vk_SumtL = vk;
		mulvfi(&vk_SumtL, SumtL.g);
		addi(&gradR.g, &vk_SumtL);
		
		vk_SumtL = vk;
		mulvfi(&vk_SumtL, SumtL.b);
		addi(&gradR.b, &vk_SumtL);

		/* translational gradient */
		mulvfi(&Sumpart1.r, 2.0f * (eiScalar)eiPI * inv_N);
		mulvfi(&Sumpart1.g, 2.0f * (eiScalar)eiPI * inv_N);
		mulvfi(&Sumpart1.b, 2.0f * (eiScalar)eiPI * inv_N);

		addi(&gradT.r, &Sumpart1.r);
		addi(&gradT.g, &Sumpart1.g);
		addi(&gradT.b, &Sumpart1.b);

		vk_Sumpart2 = vk_minor;
		mulvfi(&vk_Sumpart2, Sumpart2.r);
		addi(&gradT.r, &vk_Sumpart2);

		vk_Sumpart2 = vk_minor;
		mulvfi(&vk_Sumpart2, Sumpart2.g);
		addi(&gradT.g, &vk_Sumpart2);

		vk_Sumpart2 = vk_minor;
		mulvfi(&vk_Sumpart2, Sumpart2.b);
		addi(&gradT.b, &vk_Sumpart2);
	}

	/* rotational gradient */
	mulvfi(&gradR.r, (eiScalar)eiPI * inv_MN);
	mulvfi(&gradR.g, (eiScalar)eiPI * inv_MN);
	mulvfi(&gradR.b, (eiScalar)eiPI * inv_MN);

	mulvfi(color, (eiScalar)eiPI * inv_MN);

	/* cache the irradiance */
	if (!almost_zero(Ri, eiSCALAR_EPS))
	{
		inv_Ri = 1.0f / Ri;
	}

	if (!almost_zero(hamRi, eiSCALAR_EPS))
	{
		inv_hamRi = 1.0f / hamRi;
	}

	ei_irrad_init(
		&irrad, 
		&state->P, 
		&state->N, 
		color, 
		&gradR, 
		&gradT, 
		HARMONIC_MEAN_COEFF * inv_hamRi + (1.0f - HARMONIC_MEAN_COEFF) * inv_Ri);

	ei_add_irradiance(
		bucket, 
		&irrad);
}
