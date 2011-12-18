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

#include <eiAPI/ei_photon.h>
#include <eiAPI/ei_globillum.h>
#include <eiAPI/ei_light.h>
#include <eiAPI/ei_material.h>
#include <eiAPI/ei_shadesys.h>
#include <eiAPI/ei.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_data_table.h>

/* if photon map cannot be fully filled, we will retry this 
   maximum number of times */
#define MAX_NUM_RETRIES					10

void byteswap_light_flux(eiDatabase *db, void *ptr, const eiUint size)
{
	eiLightFlux		*flux;

	flux = (eiLightFlux *)ptr;

	ei_byteswap_scalar(&flux->energy);
	ei_byteswap_int(&flux->light_index);
}

void byteswap_photon(eiDatabase *db, void *ptr, const eiUint size)
{
	eiPhoton	*p;

	p = (eiPhoton *)ptr;

	ei_byteswap_map_node(&p->base);
}

eiBool ei_photon_fast_cond_proc(
	const eiMapNode *node, 
	const eiScalar R2, 
	void *param)
{
	return eiTRUE;
}

eiBool ei_photon_cond_proc(
	const eiMapNode *node, 
	const eiScalar R2, 
	void *param)
{
	eiPhotonCondition	*cond;
	eiPhoton			*p;
	eiVector			pdir;

	cond = (eiPhotonCondition *)param;
	p = (eiPhoton *)node;

	ei_photon_dir(&pdir, p);

	if (dot(&pdir, &cond->N) < 0.0f)
		return eiTRUE;
	else
		return eiFALSE;
}

void ei_photon_map_precompute_irrad(
	eiDatabase *db, 
	const eiTag tag, 
	const eiScalar max_dist, 
	const eiUint gather_points, 
	const eiInt filter, 
	const eiScalar coneKernel)
{
	eiMap					*map;
	eiDataTableIterator		iter;
	eiInt					i;

	map = (eiMap *)ei_db_access(db, tag);
	ei_data_table_begin(db, map->points, &iter);

	for (i = 1; i <= map->stored_points; ++i)
	{
		eiPhoton	*p1;
		eiVector	N;
		eiVector	irrad;
			
		p1 = (eiPhoton *)ei_data_table_write(&iter, i);

		ei_photon_dir(&N, p1);

		ei_photon_map_lookup_irrad(
			db, 
			tag, 
			&irrad, 
			&p1->base.pos, 
			&N, 
			max_dist, 
			gather_points, 
			filter, 
			coneKernel);

		setRGBE(&p1->power, &irrad);
	}

	ei_data_table_end(&iter);
	ei_db_end(db, tag);
}

void ei_photon_map_fast_lookup_irrad(
	eiDatabase *db, 
	const eiTag tag, 
	eiVector * const L, 
	const eiVector *P, 
	const eiVector *N, 
	const eiScalar max_dist, 
	const eiUint gather_points)
{
	eiTLS					*tls;
	eiGlobillumTLS			*pTls;
	eiMapLookup				np;
	eiPhotonFastCondition	cond;
	eiMap					*map;
	eiDataTableIterator		iter;
	eiPhoton				*p1;

	initv(L);

	tls = ei_db_get_tls(db);
	pTls = (eiGlobillumTLS *)ei_tls_get_interface(tls, EI_TLS_TYPE_GLOBILLUM);

	/* we can resize it based on the fact that memory will not be de-allocated 
	   for array, it just increases the capacity. */
	ei_array_resize(&pTls->map_lookup_dist2, gather_points + 1);
	ei_array_resize(&pTls->map_lookup_index, gather_points + 1);

	np.dist2 = (eiScalar *)ei_array_data(&pTls->map_lookup_dist2);
	np.index = (eiInt *)ei_array_data(&pTls->map_lookup_index);

	movv(&np.pos, P);
	np.max = 1;
	np.found = 0;
	np.got_heap = 0;
	np.dist2[0] = max_dist * max_dist;

	movv(&cond.P, P);
	movv(&cond.N, N);

	ei_map_locate_points(
		db, 
		tag, 
		&np, 
		1, 
		ei_photon_fast_cond_proc, 
		(void *)(&cond));

	if (np.found != 1)
	{
		return;
	}

	map = (eiMap *)ei_db_access(db, tag);
	ei_data_table_begin(db, map->points, &iter);
	
	p1 = (eiPhoton *)ei_data_table_read(&iter, np.index[1]);

	getRGBE(L, &p1->power);

	ei_data_table_end(&iter);
	ei_db_end(db, tag);
}

static eiFORCEINLINE eiScalar coneFilter(const eiScalar dp, const eiScalar inv_k, const eiScalar inv_r)
{
	return 1.0f - dp * inv_k * inv_r;
}

static eiFORCEINLINE eiScalar gaussFilter(const eiScalar dp2, const eiScalar inv_r2)
{
	return 0.918f - (0.918f / (1.0f - expf(-1.953f))) * (1.0f - expf((-1.953f * 0.5f) * dp2 * inv_r2));
}

void ei_photon_map_lookup_irrad(
	eiDatabase *db, 
	const eiTag tag, 
	eiVector * const L, 
	const eiVector *P, 
	const eiVector *N, 
	const eiScalar max_dist, 
	const eiUint gather_points, 
	const eiInt filter, 
	const eiScalar coneKernel)
{
	eiTLS					*tls;
	eiGlobillumTLS			*pTls;
	eiMapLookup				np;
	eiScalar				max_dist2;
	eiPhotonCondition		cond;
	eiMap					*map;
	eiDataTableIterator		iter;
	eiScalar				r2;
	eiPhoton				*p;
	eiVector				pdir;
	eiVector				power;
	eiInt					i;

	initv(L);

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
	max_dist2 = max_dist * max_dist;
	np.dist2[0] = max_dist2;

	movv(&cond.P, P);
	movv(&cond.N, N);

	ei_map_locate_points(
		db, 
		tag, 
		&np, 
		1, 
		ei_photon_cond_proc, 
		(void *)(&cond));

	if (np.found < 1)
	{
		return;
	}

	map = (eiMap *)ei_db_access(db, tag);
	ei_data_table_begin(db, map->points, &iter);

	r2 = eiSCALAR_EPS;

	for (i = 1; i <= np.found; ++i)
	{
		r2 = MAX(r2, np.dist2[i]);
	}

	switch (filter)
	{
	case EI_CAUSTIC_FILTER_BOX:
		{
			for (i = 1; i <= np.found; ++i)
			{
				eiScalar	fr;

				p = (eiPhoton *)ei_data_table_read(&iter, np.index[i]);

				ei_photon_dir(&pdir, p);

				fr = dot(&pdir, N);

				if (fr < 0.0f)
				{
					getRGBE(&power, &p->power);
					mulvfi(&power, -fr);
					addi(L, &power);
				}
			}

			mulvfi(L, (1.0f / (2.0f * (eiScalar)eiPI)) / r2);
		}
		break;

	case EI_CAUSTIC_FILTER_CONE:
		{
			eiScalar	r;
			eiScalar	inv_r;
			eiScalar	inv_coneKernel = 1.0f / coneKernel;

			r = Fast_sqrt(r2);
			inv_r = 1.0f / r;
			
			for (i = 1; i <= np.found; ++i)
			{
				eiScalar	fr;

				p = (eiPhoton *)ei_data_table_read(&iter, np.index[i]);

				ei_photon_dir(&pdir, p);

				fr = dot(&pdir, N);

				if (fr < 0.0f)
				{
					getRGBE(&power, &p->power);
					mulvfi(&power, -fr * coneFilter(Fast_sqrt(np.dist2[i]), inv_coneKernel, inv_r));
					addi(L, &power);
				}
			}

			mulvfi(L, (1.0f / (eiScalar)eiPI) / ((1.0f - 2.0f / (3.0f * coneKernel)) * r2));
		}
		break;

	case EI_CAUSTIC_FILTER_GAUSSIAN:
		{
			eiScalar	inv_r2;

			inv_r2 = 1.0f / r2;

			for (i = 1; i <= np.found; ++i)
			{
				eiScalar	fr;

				p = (eiPhoton *)ei_data_table_read(&iter, np.index[i]);

				ei_photon_dir(&pdir, p);

				fr = dot(&pdir, N);

				if (fr < 0.0f)
				{
					getRGBE(&power, &p->power);
					mulvfi(&power, -fr * gaussFilter(np.dist2[i], inv_r2));
					addi(L, &power);
				}
			}

			mulvfi(L, (1.0f / (2.0f * (eiScalar)eiPI)) / r2);
		}
		break;

	default:
		{
			/* error */
		}
		break;
	}

	ei_data_table_end(&iter);
	ei_db_end(db, tag);
}

void ei_photon_map_scale_photons(
	eiDatabase *db, 
	const eiTag tag, 
	const eiInt prev_scale, 
	const eiScalar factor)
{
	eiMap					*map;
	eiDataTableIterator		iter;
	eiInt					i;

	map = (eiMap *)ei_db_access(db, tag);
	ei_data_table_begin(db, map->points, &iter);

	for (i = prev_scale + 1; i <= map->stored_points; ++i)
	{
		eiPhoton	*p;
		eiVector	power;
			
		p = (eiPhoton *)ei_data_table_write(&iter, i);

		getRGBE(&power, &p->power);
		mulvfi(&power, factor);
		setRGBE(&p->power, &power);
	}

	ei_data_table_end(&iter);
	ei_db_end(db, tag);
}

static void ei_photon_bucket_init(
	eiPhotonBucket *bucket, 
	eiPhotonJob *job, 
	eiDatabase *db)
{
	ei_base_bucket_init(
		&bucket->base, 
		db, 
		job->opt, 
		job->cam, 
		job->lightInstances, 
		job->halton_num);

	if (job->photon_type == eiPHOTON_EMIT_CAUSTIC)
	{
		bucket->base.type = EI_BUCKET_PHOTON_CAUSTIC;
	}
	else
	{
		bucket->base.type = EI_BUCKET_PHOTON_GI;
	}
	
	bucket->job = job;
}

static eiBool ei_photon_bucket_exit(
	eiPhotonBucket *bucket)
{
	if (bucket->job->caustic_photons != eiNULL_TAG)
	{
		ei_data_array_flush(bucket->base.db, bucket->job->caustic_photons);
	}
	if (bucket->job->globillum_photons != eiNULL_TAG)
	{
		ei_data_array_flush(bucket->base.db, bucket->job->globillum_photons);
	}
	if (bucket->job->count != eiNULL_TAG)
	{
		ei_db_flush(bucket->base.db, bucket->job->count);
	}

	ei_base_bucket_exit(
		&bucket->base, 
		bucket->job->opt, 
		bucket->job->cam);

	return eiTRUE;
}

static eiBool ei_photon_bucket_run(
	eiPhotonBucket *bucket, 
	eiPhotonJob *job, 
	eiDatabase *db, 
	eiBaseWorker *pWorker)
{
	eiTag		photon_storage = eiNULL_TAG;
	eiInt		num_light_fluxes;
	eiInt		count = 0;
	eiInt		*pCount;

	ei_photon_bucket_init(bucket, job, db);

	if (job->num_target_photons == 0)
	{
		return ei_photon_bucket_exit(bucket);
	}

	/* TODO: we don't need to run this job if photon map has already been full */
	if (job->photon_type == eiPHOTON_EMIT_CAUSTIC)
	{
		photon_storage = job->caustic_photons;
	}
	else
	{
		photon_storage = job->globillum_photons;
	}

	/* shoot photons from light sources with more photons 
	   coming from brighter sources */
	num_light_fluxes = ei_data_array_size(bucket->base.db, job->light_flux_histogram);

	while (ei_base_worker_is_running(pWorker) && 
		ei_data_array_size(bucket->base.db, photon_storage) < job->num_target_photons && 
		count < (MAX_NUM_RETRIES * job->num_target_photons))
	{
		eiInt		light_index = -1;
		eiScalar	e = (eiScalar)ei_random(&bucket->base.randGen) * job->acc;
		eiInt		i;

		for (i = 0; i < num_light_fluxes; ++i)
		{
			eiLightFlux		*flux;

			flux = ei_data_array_read(bucket->base.db, job->light_flux_histogram, i);

			if (e <= flux->energy)
			{
				light_index = flux->light_index;
				break;
			}

			ei_data_array_end(bucket->base.db, job->light_flux_histogram, i);
		}

		/* shoot a random photon from the chosen light */
		if (light_index >= 0)
		{
			eiLightInstance		*light_inst;

			light_inst = (eiLightInstance *)ei_data_table_read(&bucket->base.light_insts_iter, light_index);
			
			ei_light_instance_shoot_photon(
				light_inst, 
				job->photon_type, 
				&job->halton_num, 
				&bucket->base);
			
			ei_base_worker_step_progress(pWorker, 1);
		}

		++ count;
	}

	/* write the shot photon count */
	pCount = (eiInt *)ei_db_access(bucket->base.db, bucket->job->count);
	(*pCount) = count;
	ei_db_end(bucket->base.db, bucket->job->count);

	return ei_photon_bucket_exit(bucket);
}

void ei_store_photon(eiState *state)
{
	eiPhotonBucket			*bucket;
	eiTag					caustic_photons;
	eiTag					globillum_photons;
	eiScalar				atten;
	eiVector				energy;

	/* we don't store the photons for direct illumination */
	if (state->hit_inst == eiNULL_TAG || 
		state->type == eiPHOTON_EMIT_GI || 
		state->type == eiPHOTON_EMIT_CAUSTIC)
	{
		return;
	}

	bucket = (eiPhotonBucket *)state->bucket;
	caustic_photons = bucket->job->caustic_photons;
	globillum_photons = bucket->job->globillum_photons;

	/* compute photon decay */
	atten = MIN(1.0f, powf(MAX(eiSCALAR_EPS, state->distance), - state->opt->photon_decay));

	movv(&energy, &state->result->color);
	mulvfi(&energy, atten);

	/* store LS+DE paths to caustic photon map, store 
	   other paths to global illumination photon map */
	if (caustic_photons != eiNULL_TAG && 
		state->globillum_reflect_depth == 0 && 
		state->globillum_refract_depth == 0)
	{
		eiPhoton	p;

		ei_photon_init(&p, &energy, &state->P, &state->I);

		ei_data_array_push_back(state->db, caustic_photons, &p);
	}
	else if (globillum_photons != eiNULL_TAG)
	{
		eiPhoton	p;

		ei_photon_init(&p, &energy, &state->P, &state->I);

		ei_data_array_push_back(state->db, globillum_photons, &p);
	}
}

eiInt ei_choose_simple_scatter_type(
	eiState *state, 
	const eiScalar transp, 
	eiVector *diffuse, 
	eiVector *specular)
{
	eiRayObjectInstance		*inst;
	eiScalar				d = 0.0f;
	eiScalar				r = 0.0f;
	eiScalar				t = 0.0f;
	eiScalar				e;

	if (state->hit_inst == eiNULL_TAG)
	{
		return eiRAY_TYPE_NONE;
	}

	inst = (eiRayObjectInstance *)ei_db_access(state->db, state->hit_inst);

	/* ignore diffuse reflections when shooting caustic photons */
	if (state->opt->globillum && 
		state->bucket->type != EI_BUCKET_PHOTON_CAUSTIC && 
		ei_attr_get_flag(&inst->attr, EI_ATTR_CAST_GLOBILLUM))
	{
		d = average(diffuse);
	}
	
	if (ei_attr_get_flag(&inst->attr, EI_ATTR_CAST_CAUSTIC))
	{
		eiScalar	spec;

		spec = average(specular);
		r = spec * (1.0f - transp);
		t = spec * transp;
	}

	ei_db_end(state->db, state->hit_inst);
	
	e = (eiScalar)ei_random(&state->bucket->randGen);

	if (e < d)
	{
		mulvfi(diffuse, 1.0f / d);

		return eiPHOTON_REFLECT_DIFFUSE;
	}
	else if (e < d + r)
	{
		mulvfi(specular, (1.0f / r) * (1.0f - transp));

		return eiPHOTON_REFLECT_SPECULAR;
	}
	else if (e < d + r + t)
	{
		mulvfi(specular, (1.0f / t) * transp);

		return eiPHOTON_TRANSMIT_SPECULAR;
	}
	else
	{
		/* absoption case */
		return eiRAY_TYPE_NONE;
	}
}

static eiFORCEINLINE eiBool ei_photon_depth_control(
	eiState *state, 
	eiOptions *opt)
{
	if (state->globillum_reflect_depth > opt->photon_reflect_depth || 
		state->globillum_refract_depth > opt->photon_refract_depth || 
		(state->globillum_reflect_depth + state->globillum_refract_depth) > opt->photon_sum_depth || 
		state->caustic_reflect_depth > opt->photon_reflect_depth || 
		state->caustic_refract_depth > opt->photon_refract_depth || 
		(state->caustic_reflect_depth + state->caustic_refract_depth ) > opt->photon_sum_depth)
	{
		return eiFALSE;
	}

	return eiTRUE;
}

static eiBool ei_photon_ray_hit_proc(eiState *state)
{
	return eiTRUE;
}

eiBool ei_rt_trace_photon(
	eiRayTracer *rt, 
	eiNodeSystem *nodesys, 
	eiState *state)
{
	if (ei_rt_trace(rt, state, ei_photon_ray_hit_proc, eiFALSE))
	{
		eiVector4	result;

		/* sum the distance */
		state->distance += state->hit_t;

		initv4(&result);

		ei_rt_compute_hit_details(rt, state);

		/* shade by calling the material at the intersection */
		if (state->hit_mtl != eiNULL_TAG)
		{
			eiMaterial		*mtl;

			mtl = (eiMaterial *)ei_db_access(rt->db, state->hit_mtl);
			ei_call_shader_instance_list(nodesys, &result, state, mtl->photon_list, NULL);
			ei_db_end(rt->db, state->hit_mtl);

			/* TODO: maybe we should call photon volume shaders here */
			ei_call_current_volume_list(nodesys, &result, state, NULL);
		}

		return eiTRUE;
	}
	else
	{
		return eiFALSE;
	}
}

eiBool ei_photon_reflection_specular(
	const eiVector *color, 
	eiState *state, 
	const eiVector *dir)
{
	eiRayTracer				*rt;
	eiNodeSystem			*nodesys;
	eiRayObjectInstance		*inst;
	eiVector				src;
	eiSampleInfo			sample_info;
	eiState					photon_ray;
	eiBool					result;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);
	eiDBG_ASSERT(rt != NULL);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	if (state->hit_inst == eiNULL_TAG)
	{
		return eiFALSE;
	}

	inst = (eiRayObjectInstance *)ei_db_access(state->db, state->hit_inst);

	if (!ei_attr_get_flag(&inst->attr, EI_ATTR_CAST_CAUSTIC))
	{
		ei_db_end(state->db, state->hit_inst);

		return eiFALSE;
	}

	ei_db_end(state->db, state->hit_inst);

	/* a fixed size sample info is considered sufficient for 
	   photon emission without arbitrary output variable support */
	ei_sample_info_init(&sample_info, 0);

	ei_state_init(&photon_ray, eiPHOTON_REFLECT_SPECULAR, state->bucket);

	movv(&src, dir);
	mulvfi(&src, calc_bias(&state->Ng, dir, state->bias, state->bias_scale));
	addi(&src, &state->P);

	photon_ray.dimension = state->dimension;
	photon_ray.instance_number = state->instance_number;

	movv(&photon_ray.org, &src);
	movv(&photon_ray.dir, dir);

	photon_ray.time = state->time;
	photon_ray.dtime = state->dtime;
	photon_ray.distance = state->distance;
	photon_ray.result = &sample_info;
	movv(&photon_ray.result->color, color);

	photon_ray.caustic_reflect_depth = state->caustic_reflect_depth + 1;
	photon_ray.caustic_refract_depth = state->caustic_refract_depth;
	photon_ray.globillum_reflect_depth = state->globillum_reflect_depth;
	photon_ray.globillum_refract_depth = state->globillum_refract_depth;

	if (!ei_photon_depth_control(&photon_ray, state->opt))
	{
		ei_state_exit(&photon_ray);

		ei_sample_info_exit(&sample_info);
		
		return eiFALSE;
	}

	result = ei_rt_trace_photon(rt, nodesys, &photon_ray);

	ei_state_exit(&photon_ray);

	ei_sample_info_exit(&sample_info);

	return result;
}

eiBool ei_photon_reflection_diffuse(
	const eiVector *color, 
	eiState *state, 
	const eiVector *dir)
{
	eiRayTracer				*rt;
	eiNodeSystem			*nodesys;
	eiRayObjectInstance		*inst;
	eiVector				src;
	eiSampleInfo			sample_info;
	eiState					photon_ray;
	eiBool					result;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);
	eiDBG_ASSERT(rt != NULL);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	if (state->hit_inst == eiNULL_TAG)
	{
		return eiFALSE;
	}

	inst = (eiRayObjectInstance *)ei_db_access(state->db, state->hit_inst);

	if (!ei_attr_get_flag(&inst->attr, EI_ATTR_CAST_GLOBILLUM))
	{
		ei_db_end(state->db, state->hit_inst);

		return eiFALSE;
	}

	ei_db_end(state->db, state->hit_inst);

	/* a fixed size sample info is considered sufficient for 
	   photon emission without arbitrary output variable support */
	ei_sample_info_init(&sample_info, 0);

	ei_state_init(&photon_ray, eiPHOTON_REFLECT_DIFFUSE, state->bucket);

	movv(&src, dir);
	mulvfi(&src, calc_bias(&state->Ng, dir, state->bias, state->bias_scale));
	addi(&src, &state->P);

	photon_ray.dimension = state->dimension;
	photon_ray.instance_number = state->instance_number;

	movv(&photon_ray.org, &src);
	movv(&photon_ray.dir, dir);

	photon_ray.time = state->time;
	photon_ray.dtime = state->dtime;
	photon_ray.distance = state->distance;
	photon_ray.result = &sample_info;
	movv(&photon_ray.result->color, color);

	photon_ray.caustic_reflect_depth = state->caustic_reflect_depth;
	photon_ray.caustic_refract_depth = state->caustic_refract_depth;
	photon_ray.globillum_reflect_depth = state->globillum_reflect_depth + 1;
	photon_ray.globillum_refract_depth = state->globillum_refract_depth;

	if (!ei_photon_depth_control(&photon_ray, state->opt))
	{
		ei_state_exit(&photon_ray);

		ei_sample_info_exit(&sample_info);

		return eiFALSE;
	}

	result = ei_rt_trace_photon(rt, nodesys, &photon_ray);

	ei_state_exit(&photon_ray);

	ei_sample_info_exit(&sample_info);

	return result;
}

eiBool ei_photon_transparent(
	const eiVector *color, 
	eiState *state)
{
	eiRayTracer				*rt;
	eiNodeSystem			*nodesys;
	eiRayObjectInstance		*inst;
	eiVector				src;
	eiSampleInfo			sample_info;
	eiState					photon_ray;
	eiBool					result;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);
	eiDBG_ASSERT(rt != NULL);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	if (state->hit_inst == eiNULL_TAG)
	{
		return eiFALSE;
	}

	inst = (eiRayObjectInstance *)ei_db_access(state->db, state->hit_inst);

	if (!ei_attr_get_flag(&inst->attr, EI_ATTR_CAST_CAUSTIC))
	{
		ei_db_end(state->db, state->hit_inst);

		return eiFALSE;
	}

	ei_db_end(state->db, state->hit_inst);

	/* a fixed size sample info is considered sufficient for 
	   photon emission without arbitrary output variable support */
	ei_sample_info_init(&sample_info, 0);

	ei_state_init(&photon_ray, eiPHOTON_TRANSPARENT, state->bucket);

	movv(&src, &state->I);
	mulvfi(&src, calc_bias(&state->Ng, &state->I, state->bias, state->bias_scale));
	addi(&src, &state->P);

	photon_ray.dimension = state->dimension;
	photon_ray.instance_number = state->instance_number;

	movv(&photon_ray.org, &src);
	movv(&photon_ray.dir, &state->I);

	photon_ray.time = state->time;
	photon_ray.dtime = state->dtime;
	photon_ray.distance = state->distance;
	photon_ray.result = &sample_info;
	movv(&photon_ray.result->color, color);

	photon_ray.caustic_reflect_depth = state->caustic_reflect_depth;
	photon_ray.caustic_refract_depth = state->caustic_refract_depth + 1;
	photon_ray.globillum_reflect_depth = state->globillum_reflect_depth;
	photon_ray.globillum_refract_depth = state->globillum_refract_depth;

	if (!ei_photon_depth_control(&photon_ray, state->opt))
	{
		ei_state_exit(&photon_ray);

		ei_sample_info_exit(&sample_info);

		return eiFALSE;
	}

	result = ei_rt_trace_photon(rt, nodesys, &photon_ray);

	ei_state_exit(&photon_ray);

	ei_sample_info_exit(&sample_info);

	return result;
}

eiBool ei_photon_transmission_specular(
	const eiVector *color, 
	eiState *state, 
	const eiVector *dir)
{
	eiRayTracer				*rt;
	eiNodeSystem			*nodesys;
	eiRayObjectInstance		*inst;
	eiVector				src;
	eiSampleInfo			sample_info;
	eiState					photon_ray;
	eiBool					result;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);
	eiDBG_ASSERT(rt != NULL);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	if (state->hit_inst == eiNULL_TAG)
	{
		return eiFALSE;
	}

	inst = (eiRayObjectInstance *)ei_db_access(state->db, state->hit_inst);

	if (!ei_attr_get_flag(&inst->attr, EI_ATTR_CAST_CAUSTIC))
	{
		ei_db_end(state->db, state->hit_inst);

		return eiFALSE;
	}

	ei_db_end(state->db, state->hit_inst);

	/* a fixed size sample info is considered sufficient for 
	   photon emission without arbitrary output variable support */
	ei_sample_info_init(&sample_info, 0);

	ei_state_init(&photon_ray, eiPHOTON_TRANSMIT_SPECULAR, state->bucket);

	movv(&src, dir);
	mulvfi(&src, calc_bias(&state->Ng, dir, state->bias, state->bias_scale));
	addi(&src, &state->P);

	photon_ray.dimension = state->dimension;
	photon_ray.instance_number = state->instance_number;

	movv(&photon_ray.org, &src);
	movv(&photon_ray.dir, dir);

	photon_ray.time = state->time;
	photon_ray.dtime = state->dtime;
	photon_ray.distance = state->distance;
	photon_ray.result = &sample_info;
	movv(&photon_ray.result->color, color);

	photon_ray.caustic_reflect_depth = state->caustic_reflect_depth;
	photon_ray.caustic_refract_depth = state->caustic_refract_depth + 1;
	photon_ray.globillum_reflect_depth = state->globillum_reflect_depth;
	photon_ray.globillum_refract_depth = state->globillum_refract_depth;

	if (!ei_photon_depth_control(&photon_ray, state->opt))
	{
		ei_state_exit(&photon_ray);

		ei_sample_info_exit(&sample_info);

		return eiFALSE;
	}

	result = ei_rt_trace_photon(rt, nodesys, &photon_ray);

	ei_state_exit(&photon_ray);

	ei_sample_info_exit(&sample_info);

	return result;
}

void byteswap_job_photon(eiDatabase *db, void *job, const eiUint size)
{
	eiPhotonJob *pJob = (eiPhotonJob *)job;

	ei_byteswap_int(&pJob->light_flux_histogram);
	ei_byteswap_scalar(&pJob->acc);
	ei_byteswap_int(&pJob->num_target_photons);
	ei_byteswap_int(&pJob->photon_type);
	ei_byteswap_int(&pJob->caustic_photons);
	ei_byteswap_int(&pJob->globillum_photons);
	ei_byteswap_int(&pJob->count);
	ei_byteswap_int(&pJob->halton_num);
	ei_byteswap_int(&pJob->lightInstances);
}

eiBool execute_job_photon(eiDatabase *db, eiBaseWorker *pWorker, void *job, void *param)
{
	eiPhotonJob		*pJob;
	eiPhotonBucket		bucket;
	
	pJob = (eiPhotonJob *)job;

	return ei_photon_bucket_run(&bucket, pJob, db, pWorker);
}

eiUint count_job_photon(eiDatabase *db, void *job)
{
	eiPhotonJob *pJob = (eiPhotonJob *)job;

	return pJob->num_target_photons;
}
