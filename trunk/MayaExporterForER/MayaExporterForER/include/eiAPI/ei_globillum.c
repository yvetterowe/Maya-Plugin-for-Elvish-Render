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

#include <eiAPI/ei_globillum.h>
#include <eiAPI/ei_photon.h>
#include <eiAPI/ei_finalgather.h>
#include <eiAPI/ei_shader.h>
#include <eiAPI/ei_sampler.h>
#include <eiAPI/ei.h>
#include <eiCORE/ei_qmc.h>
#include <eiCORE/ei_assert.h>

void ei_globillum_tls_init(eiGlobillumTLS *pTls)
{
	ei_array_init(&pTls->map_lookup_dist2, sizeof(eiScalar));
	ei_array_init(&pTls->map_lookup_index, sizeof(eiInt));

	ei_buffer_init(
		&pTls->finalgather_buffer, 
		sizeof(eiHemisphereSample), 
		NULL, 
		NULL, 
		NULL, 
		ei_hemisphere_sample_zero_item, 
		ei_hemisphere_sample_add_item, 
		ei_hemisphere_sample_mul_item);
}

void ei_globillum_tls_exit(eiGlobillumTLS *pTls)
{
	ei_buffer_clear(&pTls->finalgather_buffer);
	ei_array_clear(&pTls->map_lookup_index);
	ei_array_clear(&pTls->map_lookup_dist2);
}

eiBuffer *ei_globillum_tls_acquire_finalgather_buffer(
	eiGlobillumTLS *pTls, 
	const eiUint finalgather_rays)
{
	eiUint	M, N;

	M = (eiUint)sqrtf(finalgather_rays * (1.0f / (eiScalar)eiPI));
	N = (eiUint)(eiPI * (eiScalar)M);

	if (M < 1)
	{
		M = 1;
	}
	if (N < 1)
	{
		N = 1;
	}

	if (M == ei_buffer_get_width(&pTls->finalgather_buffer) && 
		N == ei_buffer_get_height(&pTls->finalgather_buffer))
	{
		return &pTls->finalgather_buffer;
	}

	ei_buffer_clear(&pTls->finalgather_buffer);
	ei_buffer_allocate(&pTls->finalgather_buffer, M, N);

	return &pTls->finalgather_buffer;
}

static eiFORCEINLINE void intersect_with_plane(
	eiVector *hit, 
	const eiVector *src, 
	const eiVector *dir, 
	const eiVector4 *plane)
{
	eiScalar	dot_nd, t;

	dot_nd = plane->x * dir->x + plane->y * dir->y + plane->z * dir->z;
	
	t = - (src->x * plane->x + src->y * plane->y + src->z * plane->z + plane->w);

	t /= dot_nd;

	hit->x = src->x + dir->x * t;
	hit->y = src->y + dir->y * t;
	hit->z = src->z + dir->z * t;
}

static eiFORCEINLINE eiBool ei_finalgather_depth_control(
	eiState *state, 
	eiOptions *opt)
{
	if (state->finalgather_diffuse_depth > opt->finalgather_diffuse_bounces)
	{
		/* shade by background */
		eiVector	result;

		initv(&result);

		ei_trace_environment(&result, state, &state->I);

		return eiFALSE;
	}

	return eiTRUE;
}

void ei_compute_irradiance(eiVector * const irrad, eiState * const state)
{
	eiRayTracer			*rt;
	eiNodeSystem		*nodesys;
	eiBucket			*bucket;
	eiVector			color;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);

	bucket = (eiBucket *)state->bucket;

	initv(irrad);
	initv(&color);

	if (bucket->job->causticMap != eiNULL_TAG)
	{
		ei_photon_map_lookup_irrad(
			state->db, 
			bucket->job->causticMap, 
			irrad, 
			&state->P, 
			&state->N, 
			bucket->par.caustic_radius, 
			state->opt->caustic_samples, 
			state->opt->caustic_filter, 
			state->opt->caustic_filter_const);
	}

	if (state->opt->finalgather)
	{
		/* we don't really compute irradiance with hemisphere sampling or 
		   interpolation if reflections or refractions have gone through 
		   too many depths, we just trace a diffuse final gather ray to 
		   approximate the indirect illumination if the diffuse bounce 
		   depth didn't reach the limit. */
		if (state->type == eiRAY_FINALGATHER || 
			state->reflect_depth > state->opt->finalgather_reflect_depth || 
			state->refract_depth > state->opt->finalgather_refract_depth)
		{
			/* lookup the photon map for indirect illumination instead if 
			   we have it, rather than tracing a diffuse final gather ray */
			if (bucket->job->globillumMap != eiNULL_TAG)
			{
				/* NOTICE: the cone kernel is hard-coded to 1.0f here */
				ei_photon_map_lookup_irrad(
					state->db, 
					bucket->job->globillumMap, 
					&color, 
					&state->P, 
					&state->N, 
					bucket->par.globillum_radius, 
					state->opt->globillum_samples, 
					EI_CAUSTIC_FILTER_BOX, 
					1.0f);
			}
			else
			{
				/* trace a diffuse final gather ray */
				eiVector	u_axis, v_axis;
				eiScalar	u1, u2;
				eiVector	dir;
				eiVector	local_u_axis, local_v_axis, local_w_axis;
				eiVector	ray_src;
				eiState		fg_ray;
				eiVector	prev_color;
				eiVector	prev_opacity;

				ortho_basis(&state->N, &u_axis, &v_axis);

				u1 = (eiScalar)ei_sigma(state->dimension, state->instance_number);
				u2 = (eiScalar)ei_sigma(state->dimension + 1, state->instance_number);
				state->dimension += 2;

				ei_cosine_sample_hemisphere(&dir, u1, u2);

				local_u_axis = u_axis;
				mulvfi(&local_u_axis, dir.x);
				local_v_axis = v_axis;
				mulvfi(&local_v_axis, dir.y);
				local_w_axis = state->N;
				mulvfi(&local_w_axis, dir.z);
				add(&dir, &local_u_axis, &local_v_axis);
				addi(&dir, &local_w_axis);

				ray_src = dir;
				mulvfi(&ray_src, calc_bias(&state->Ng, &state->I, state->bias, state->bias_scale));
				addi(&ray_src, &state->P);

				ei_state_init(&fg_ray, eiRAY_FINALGATHER, state->bucket);

				/* backup color and opacity */
				movv(&prev_color, &state->result->color);
				movv(&prev_opacity, &state->result->opacity);
				fg_ray.result = state->result;

				initv(&fg_ray.result->color);
				initv(&fg_ray.result->opacity);

				fg_ray.instance_number = state->instance_number;

				fg_ray.org = ray_src;
				fg_ray.dir = dir;

				if (bucket->par.finalgather_falloff)
				{
					fg_ray.max_t = bucket->par.finalgather_falloff_stop;
				}

				fg_ray.dimension = state->dimension + 2;
				fg_ray.time = state->time;
				fg_ray.dtime = state->dtime;
				fg_ray.raster = state->raster;
				fg_ray.finalgather_diffuse_depth = state->finalgather_diffuse_depth + 1;

				if (ei_finalgather_depth_control(&fg_ray, state->opt))
				{
					ei_state_inherit_volume(&fg_ray, state);

					ei_rt_trace_finalgather(rt, nodesys, &fg_ray);

					addi(&color, &fg_ray.result->color);

					state->pass_motion |= fg_ray.pass_motion;
				}

				ei_state_exit(&fg_ray);

				/* restore color and opacity */
				movv(&state->result->color, &prev_color);
				movv(&state->result->opacity, &prev_opacity);
			}
		}
		else
		{
			if (bucket->job->pass_mode == EI_PASS_FINALGATHER_INITIAL || 
				state->opt->finalgather_samples <= 0)
			{
				/* force to sample final gather for initial pass */
				/* force to sample if users don't want interpolation, 
				   they should set final gather samples to <= 0 */
				ei_sample_finalgather(&color, state);

				mulvfi(&color, 1.0f / (eiScalar)eiPI);
			}
			else
			{
				eiScalar	proj_pixel_area = 1.0f;
				/* proj_pixel_area is the square root of: 
				   at each pixel we compute the area of that pixel projected onto 
				   the plane passing through P with normal N using the camera projection. 
				   these value can easily be derived using the solid angle of the pixel 
				   through which P is seen */
				eiCamera	*cam = state->cam;
				eiVector	src1, src2, src3, src4, 
							dir1, dir2, dir3, dir4;
				eiVector4	surface_plane;
				eiVector	p1, p2, p3, p4;

				/* perspective projection */
				if (cam->focal != eiMAX_SCALAR)
				{
					initv(&src1);
					initv(&src2);
					initv(&src3);
					initv(&src4);
					ei_camera_get_ray_dir(cam, &dir1, state->raster.x - 0.5f, state->raster.y - 0.5f);
					ei_camera_get_ray_dir(cam, &dir2, state->raster.x + 0.5f, state->raster.y - 0.5f);
					ei_camera_get_ray_dir(cam, &dir3, state->raster.x + 0.5f, state->raster.y + 0.5f);
					ei_camera_get_ray_dir(cam, &dir4, state->raster.x - 0.5f, state->raster.y + 0.5f);
					normalizei(&dir1);
					normalizei(&dir2);
					normalizei(&dir3);
					normalizei(&dir4);
				}
				else
				{
					ei_camera_get_ray_pos(cam, &src1, state->raster.x - 0.5f, state->raster.y - 0.5f);
					ei_camera_get_ray_pos(cam, &src2, state->raster.x + 0.5f, state->raster.y - 0.5f);
					ei_camera_get_ray_pos(cam, &src3, state->raster.x + 0.5f, state->raster.y + 0.5f);
					ei_camera_get_ray_pos(cam, &src4, state->raster.x - 0.5f, state->raster.y + 0.5f);
					setv(&dir1, 0.0f, 0.0f, -1.0f);
					setv(&dir2, 0.0f, 0.0f, -1.0f);
					setv(&dir3, 0.0f, 0.0f, -1.0f);
					setv(&dir4, 0.0f, 0.0f, -1.0f);
				}

				surface_plane.x = state->N.x;
				surface_plane.y = state->N.y;
				surface_plane.z = state->N.z;
				surface_plane.w = - dot(&state->P, &state->N);

				intersect_with_plane(&p1, &src1, &dir1, &surface_plane);
				intersect_with_plane(&p2, &src2, &dir2, &surface_plane);
				intersect_with_plane(&p3, &src3, &dir3, &surface_plane);
				intersect_with_plane(&p4, &src4, &dir4, &surface_plane);

				proj_pixel_area = sqrtf((absf(tri_area(&p1, &p2, &p3)) + absf(tri_area(&p1, &p3, &p4))) * 0.5f) * 0.5f;

				/* try to interpolate all max points, if there is at least 1 record, 
				   otherwise add new record */
				if (!ei_lookup_irradiance(
					bucket, 
					&color, 
					&state->P, 
					&state->N, 
					proj_pixel_area, 
					bucket->par.finalgather_radius, 
					state->opt->finalgather_samples, 
					(bucket->job->pass_mode == EI_PASS_FRAME)))
				{
					ei_sample_finalgather(&color, state);

					mulvfi(&color, 1.0f / (eiScalar)eiPI);
				}
			}
		}
	}
	else if (bucket->job->globillumMap != eiNULL_TAG)
	{
		/* NOTICE: the cone kernel is hard-coded to 1.0f here */
		ei_photon_map_lookup_irrad(
			state->db, 
			bucket->job->globillumMap, 
			&color, 
			&state->P, 
			&state->N, 
			bucket->par.globillum_radius, 
			state->opt->globillum_samples, 
			EI_CAUSTIC_FILTER_BOX, 
			1.0f);
	}

	addi(irrad, &color);
}
