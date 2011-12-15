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

#include <eiAPI/ei_sampler.h>
#include <eiCORE/ei_dataflow.h>
#include <eiCORE/ei_assert.h>
#include <eiAPI/ei_options.h>
#include <eiAPI/ei_camera.h>
#include <eiAPI/ei_material.h>
#include <eiAPI/ei_object.h>
#include <eiAPI/ei_raytracer.h>
#include <eiAPI/ei_shadesys.h>
#include <eiAPI/ei.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_random.h>
#include <eiCORE/ei_qmc.h>

#define EI_SAMPLES_PER_BANK		4096

/** \brief Precompute the weights of samples by the filter, 
 * despite of stochastic sampling, the weights will always 
 * be calculated at regular sampling points. */
static void ei_filter_table_init(
	eiFilterTable *tab, 
	const eiInt num_subpixels, const eiScalar filter_rad, const eiInt filter)
{
	eiInt		i, j;
	eiVector2	val;
	eiScalar	x, y;
	eiScalar	inv_filter_radius;
	eiScalar	w;

	tab->num_subpixels = num_subpixels;

	ei_buffer_init(&tab->table, sizeof(eiVector2), 
		NULL, NULL, NULL, NULL, NULL, NULL);
	ei_buffer_allocate(&tab->table, tab->num_subpixels + 1, tab->num_subpixels + 1);

	for (i = 0; i <= tab->num_subpixels; ++i)
	{
		for (j = 0; j <= tab->num_subpixels; ++j)
		{
			val.x = i / (eiScalar)tab->num_subpixels;
			val.y = j / (eiScalar)tab->num_subpixels;

			ei_buffer_set_clamped(&tab->table, i, j, &val);
		}
	}
	
	tab->filter_radius = lceilf((eiScalar)tab->num_subpixels * filter_rad) + 1;

	inv_filter_radius = 1.0f / (eiScalar)tab->filter_radius;
	
	ei_buffer_init(&tab->weights, sizeof(eiScalar), 
		NULL, NULL, NULL, NULL, NULL, NULL);
	ei_buffer_allocate(&tab->weights, tab->filter_radius, tab->filter_radius);
	
	for (i = 0; i < tab->filter_radius; ++i)
	{
		for (j = 0; j < tab->filter_radius; ++j)
		{
			x = (eiScalar)i * inv_filter_radius;
			y = (eiScalar)j * inv_filter_radius;

			w = 0.0f;

			switch (filter)
			{
			case EI_FILTER_BOX:
				w = boxFilter(x, y, 1.0f, 1.0f);
				break;

			case EI_FILTER_TRIANGLE:
				w = triangleFilter(x, y, 1.0f, 1.0f);
				break;

			case EI_FILTER_CATMULLROM:
				w = catmullRomFilter(x, y, 1.0f, 1.0f);
				break;

			case EI_FILTER_GAUSSIAN:
				w = gaussianFilter(x, y, 1.0f, 1.0f);
				break;

			case EI_FILTER_SINC:
				w = sincFilter(x, y, 1.0f, 1.0f);
				break;

			default:
				/* error */
				break;
			}

			ei_buffer_set_clamped(&tab->weights, i, j, &w);
		}
	}
}

static void ei_filter_table_exit(eiFilterTable *tab)
{
	ei_buffer_clear(&tab->table);
	ei_buffer_clear(&tab->weights);
}

static eiFORCEINLINE void ei_filter_table_reg(
	eiFilterTable *tab, 
	const eiInt x, const eiInt y, eiScalar *sx, eiScalar *sy)
{
	eiVector2	*val;

	val = (eiVector2 *)ei_buffer_getptr(&tab->table, x, y);

	*sx = val->x;
	*sy = val->y;
}

static eiFORCEINLINE eiScalar ei_filter_table_get_weight(
	eiFilterTable *tab, 
	const eiInt x, const eiInt y)
{
	eiScalar	w;

	ei_buffer_get(&tab->weights, x, y, &w);

	return w;
}

static eiFORCEINLINE eiInt ei_filter_table_get_filter_radius(eiFilterTable *tab)
{
	return tab->filter_radius;
}

/** \brief The sampled information for each pixel, 
 * containing a list of sub-pixels within this pixel. */
typedef struct eiPixelInfo {
	ei_slist		sample_list;
} eiPixelInfo;

static void ei_pixel_info_init(void *item)
{
	eiPixelInfo *info = (eiPixelInfo *)item;

	ei_slist_init(&info->sample_list, NULL);
}

static void ei_pixel_info_exit(void *item)
{
	eiPixelInfo *info = (eiPixelInfo *)item;

	/* we don't need to delete pointers in sample_list, 
	   because they were allocated from memory pool, 
	   and will be automatically deleted by memory pool 
	   finally. */
	ei_slist_clear(&info->sample_list);
}

/** \brief Must call this functon to copy pixel info. */
static void ei_pixel_info_copy(void *dst_item, const void *src_item, const eiInt item_size)
{
	eiPixelInfo		*dst;
	eiPixelInfo		*src;

	dst = (eiPixelInfo *)dst_item;
	src = (eiPixelInfo *)src_item;

	/* no sample list should be copy! */
}

static void ei_render_params_init(
	eiRenderParams *par, 
	eiOptions *opt, 
	const eiScalar scene_diag)
{
	par->filter_radius = opt->filter_size * 0.5f;
	par->min_depth = opt->min_samples;
	par->max_depth = opt->max_samples;
	/* make sure the parameter is valid */
	if (par->max_depth < par->min_depth ) {
		par->max_depth = par->min_depth;
	}
	par->bound_max_depth = MAX(0, par->max_depth);
	par->num_subpixels = (1 << ( par->bound_max_depth - par->min_depth));
	if (par->min_depth < 0) {
		par->num_spans = (eiScalar)(1 << (-par->min_depth));
	} else {
		par->num_spans = 1.0f / (eiScalar)(1 << par->min_depth);
	}
	par->inv_num_subpixels = 1.0f / (eiScalar)par->num_subpixels;
	par->inv_num_spans = 1.0f / par->num_spans;
	par->sample_filter_radius = lceilf(par->filter_radius * par->inv_num_spans);
	par->subpixel_to_pixel = par->num_spans * par->inv_num_subpixels;
	par->inv_subpixel_to_pixel = 1.0f / par->subpixel_to_pixel;

	/* initialize filter table */
	ei_filter_table_init(&par->filterTable, 
		par->num_subpixels, par->filter_radius * par->inv_num_spans, opt->filter);

	/* precompute for photon mapping */
	par->globillum_radius = opt->globillum_radius;
	par->caustic_radius = opt->caustic_radius;
	if (par->globillum_radius == 0.0f)
	{
		par->globillum_radius = scene_diag * 0.1f;
	}
	if (par->caustic_radius == 0.0f)
	{
		par->caustic_radius = scene_diag * 0.01f;
	}

	/* precompute for final gathering */
	par->finalgather_radius = opt->finalgather_radius;
	if (par->finalgather_radius == 0.0f)
	{
		par->finalgather_radius = scene_diag * 0.1f;
	}

	par->finalgather_falloff = opt->finalgather_falloff;
	par->finalgather_falloff_start = opt->finalgather_falloff_start;
	par->finalgather_falloff_stop = opt->finalgather_falloff_stop;
	if (par->finalgather_falloff_start == 0.0f && par->finalgather_falloff_stop == 0.0f)
	{
		par->finalgather_falloff = eiFALSE;
	}
	else
	{
		if (par->finalgather_falloff_stop < par->finalgather_falloff_start)
		{
			par->finalgather_falloff_stop = par->finalgather_falloff_start;
		}
	}
}

static void ei_render_params_exit(eiRenderParams *par)
{
	ei_filter_table_exit(&par->filterTable);
}

static void ei_bucket_init(
	eiBucket *bucket, eiBucketJob *job, eiDatabase *db)
{
	eiIntptr	i;
	eiIntptr	numFrameBuffers;

	ei_base_bucket_init(
		&bucket->base, 
		db, 
		job->opt, 
		job->cam, 
		job->lightInstances, 
		job->bucket_id);

	bucket->base.type = EI_BUCKET_FRAME;
	bucket->job = job;

	ei_buffer_init(&bucket->sampleBuffer, sizeof(eiSampleInfo *), 
		NULL, NULL, NULL, NULL, NULL, NULL);
	ei_buffer_init(&bucket->pixelBuffer, sizeof(eiPixelInfo), 
		ei_pixel_info_init, ei_pixel_info_exit, ei_pixel_info_copy, NULL, NULL, NULL);

	ei_render_params_init(&bucket->par, bucket->base.opt, ei_rt_scene_diag(bucket->base.rt));

	bucket->rect_width = job->rect.right - job->rect.left + 1;
	bucket->rect_height = job->rect.bottom - job->rect.top + 1;

	/* build frame buffer caches */
	ei_framebuffer_cache_init(
		&bucket->colorFrameBufferCache, 
		db, 
		job->colorFrameBuffer, 
		bucket->rect_width, 
		bucket->rect_height, 
		job->pos_i, 
		job->pos_j);
	ei_framebuffer_cache_init(
		&bucket->opacityFrameBufferCache, 
		db, 
		job->opacityFrameBuffer, 
		bucket->rect_width, 
		bucket->rect_height, 
		job->pos_i, 
		job->pos_j);

	numFrameBuffers = ei_data_array_size(db, job->frameBuffers);

	ei_array_init(&bucket->frameBufferCaches, sizeof(eiFrameBufferCache));
	ei_array_resize(&bucket->frameBufferCaches, numFrameBuffers);

	for (i = 0; i < numFrameBuffers; ++i)
	{
		eiTag	userFrameBuffer;

		userFrameBuffer = *((eiTag *)ei_data_array_read(db, job->frameBuffers, i));
		ei_data_array_end(db, job->frameBuffers, i);

		ei_framebuffer_cache_init(
			(eiFrameBufferCache *)ei_array_get(&bucket->frameBufferCaches, i), 
			db, 
			userFrameBuffer, 
			bucket->rect_width, 
			bucket->rect_height, 
			job->pos_i, 
			job->pos_j);
	}

	/* create sample pool */
	ei_fixed_pool_init(&bucket->samplePool, 
		sizeof(eiSampleInfo) + job->user_output_size, EI_SAMPLES_PER_BANK);
}

static eiBool ei_bucket_exit(eiBucket *bucket)
{
	eiIntptr	i;

	ei_buffer_clear(&bucket->pixelBuffer);
	ei_buffer_clear(&bucket->sampleBuffer);
	ei_fixed_pool_clear(&bucket->samplePool);

	for (i = 0; i < ei_array_size(&bucket->frameBufferCaches); ++i)
	{
		eiFrameBufferCache	*fbCache;

		fbCache = (eiFrameBufferCache *)ei_array_get(&bucket->frameBufferCaches, i);

		ei_framebuffer_cache_flush(fbCache);
		ei_framebuffer_cache_exit(fbCache);
	}
	ei_array_clear(&bucket->frameBufferCaches);

	ei_framebuffer_cache_flush(&bucket->opacityFrameBufferCache);
	ei_framebuffer_cache_exit(&bucket->opacityFrameBufferCache);
	ei_framebuffer_cache_flush(&bucket->colorFrameBufferCache);
	ei_framebuffer_cache_exit(&bucket->colorFrameBufferCache);

	if (bucket->job->passIrradBuffer != eiNULL_TAG)
	{
		ei_data_array_flush(bucket->base.db, bucket->job->passIrradBuffer);
	}

	ei_render_params_exit(&bucket->par);

	ei_base_bucket_exit(
		&bucket->base, 
		bucket->job->opt, 
		bucket->job->cam);

	return eiTRUE;
}

eiSampleInfo *create_sample_info(eiBucket *bucket)
{
	eiSampleInfo	*info;

	info = (eiSampleInfo *)ei_fixed_pool_allocate(&bucket->samplePool);
	ei_sample_info_init(info, bucket->job->user_output_size);

	return info;
}

void delete_sample_info(eiBucket *bucket, eiSampleInfo * const info)
{
	ei_sample_info_exit(info);
	ei_fixed_pool_free(&bucket->samplePool, info);
}

/* reset the sample info and reuse the allocated memory */
void reset_sample_info(eiBucket *bucket, eiSampleInfo * const info)
{
	ei_sample_info_exit(info);
	ei_sample_info_init(info, bucket->job->user_output_size);
}

/* write a sample into frame buffer. */
static eiFORCEINLINE void write_sample(
	eiBucket *bucket, const eiInt x, const eiInt y, eiSampleInfo *color)
{
	eiIntptr	i;

	ei_framebuffer_cache_set(&bucket->colorFrameBufferCache, x, y, &color->color);
	ei_framebuffer_cache_set(&bucket->opacityFrameBufferCache, x, y, &color->opacity);
	
	for (i = 0; i < ei_array_size(&bucket->frameBufferCaches); ++i)
	{
		eiFrameBufferCache	*fb_cache;
		eiInt				fb_data_offset;

		fb_cache = (eiFrameBufferCache *)ei_array_get(&bucket->frameBufferCaches, i);

		fb_data_offset = ei_framebuffer_cache_get_data_offset(fb_cache);

		ei_framebuffer_cache_set(fb_cache, x, y, ((eiByte *)color) + fb_data_offset);
	}
}

/* paint a sample into frame buffer. */
static eiFORCEINLINE void paint_sample(
	eiBucket *bucket, const eiInt x, const eiInt y, const eiInt r, eiSampleInfo *color)
{
	eiIntptr	i;

	ei_framebuffer_cache_paint(&bucket->colorFrameBufferCache, x, y, r, &color->color);
	ei_framebuffer_cache_paint(&bucket->opacityFrameBufferCache, x, y, r, &color->opacity);
	
	for (i = 0; i < ei_array_size(&bucket->frameBufferCaches); ++i)
	{
		eiFrameBufferCache	*fb_cache;
		eiInt				fb_data_offset;

		fb_cache = (eiFrameBufferCache *)ei_array_get(&bucket->frameBufferCaches, i);

		fb_data_offset = ei_framebuffer_cache_get_data_offset(fb_cache);

		ei_framebuffer_cache_paint(fb_cache, x, y, r, ((eiByte *)color) + fb_data_offset);
	}
}

/* add two sample info, defined as adding each component. */
void ei_sample_info_add(eiBucket *bucket, eiSampleInfo * const c1, eiSampleInfo * const c2)
{
	eiIntptr	numFrameBuffers;
	eiIntptr	i;

	addi(&c1->color, &c2->color);
	addi(&c1->opacity, &c2->opacity);

	numFrameBuffers = ei_array_size(&bucket->frameBufferCaches);
	
	for (i = 0; i < numFrameBuffers; ++i)
	{
		eiFrameBufferCache	*fb_cache;
		eiInt				fb_type;
		eiInt				fb_data_offset;

		fb_cache = (eiFrameBufferCache *)ei_array_get(&bucket->frameBufferCaches, i);

		fb_type = ei_framebuffer_cache_get_type(fb_cache);
		fb_data_offset = ei_framebuffer_cache_get_data_offset(fb_cache);

		switch (fb_type)
		{
		case EI_DATA_TYPE_INT:
			*((eiInt *)((eiByte *)c1 + fb_data_offset)) += *((eiInt *)((eiByte *)c2 + fb_data_offset));
			break;
		case EI_DATA_TYPE_SCALAR:
			*((eiScalar *)((eiByte *)c1 + fb_data_offset)) += *((eiScalar *)((eiByte *)c2 + fb_data_offset));
			break;
		case EI_DATA_TYPE_VECTOR:
			addi((eiVector *)((eiByte *)c1 + fb_data_offset), (eiVector *)((eiByte *)c2 + fb_data_offset));
			break;
		default:
			/* error */
			break;
		}
	}
}

/* scale a sample info, defined as scaling each component. */
void ei_sample_info_mul(eiBucket *bucket, eiSampleInfo * const c, const eiScalar a)
{
	eiIntptr	numFrameBuffers;
	eiIntptr	i;

	mulvfi(&c->color, a);
	mulvfi(&c->opacity, a);

	numFrameBuffers = ei_array_size(&bucket->frameBufferCaches);
	
	for (i = 0; i < numFrameBuffers; ++i)
	{
		eiFrameBufferCache	*fb_cache;
		eiInt				fb_type;
		eiInt				fb_data_offset;

		fb_cache = (eiFrameBufferCache *)ei_array_get(&bucket->frameBufferCaches, i);

		fb_type = ei_framebuffer_cache_get_type(fb_cache);
		fb_data_offset = ei_framebuffer_cache_get_data_offset(fb_cache);

		switch (fb_type)
		{
		case EI_DATA_TYPE_INT:
			*((eiInt *)((eiByte *)c + fb_data_offset)) = roundf((eiScalar)(*((eiInt *)((eiByte *)c + fb_data_offset))) * a);
			break;
		case EI_DATA_TYPE_SCALAR:
			*((eiScalar *)((eiByte *)c + fb_data_offset)) *= a;
			break;
		case EI_DATA_TYPE_VECTOR:
			mulvfi((eiVector *)((eiByte *)c + fb_data_offset), a);
			break;
		default:
			/* error */
			break;
		}
	}
}

void ei_add_irradiance(eiBucket *bucket, const eiIrradiance *irrad)
{
	if (bucket->job->passIrradBuffer != eiNULL_TAG)
	{
		ei_data_array_push_back(bucket->base.db, bucket->job->passIrradBuffer, irrad);
	}
}

eiBool ei_lookup_irradiance(
	eiBucket *bucket, 
	eiVector * const L, 
	const eiVector *P, 
	const eiVector *N, 
	const eiScalar A, 
	const eiScalar R, 
	const eiUint numInterpPoints, 
	const eiBool forceInterp)
{
	eiScalar	w = 0.0f;
	eiUint		numAvailablePoints = 0;

	if (forceInterp)
	{
		ei_irrad_cache_force_interp(
			bucket->base.db, 
			bucket->job->irradCache, 
			L, 
			P, 
			N, 
			A, 
			R, 
			bucket->base.opt->finalgather_samples, 
			&w, 
			&numAvailablePoints);
	}
	else
	{
		ei_irrad_cache_find(
			bucket->base.db, 
			bucket->job->irradCache, 
			L, 
			P, 
			N, 
			A, 
			R, 
			bucket->base.opt->finalgather_samples, 
			&w, 
			&numAvailablePoints);
	}

	if (numAvailablePoints < MIN(EI_FG_MIN_INTERP_POINTS, numInterpPoints))
	{
		return eiFALSE;
	}

	mulvfi(L, 1.0f / w);

	return eiTRUE;
}

static eiFORCEINLINE eiScalar calc_contrast(
	const eiScalar x1, 
	const eiScalar x2, 
	const eiScalar x3, 
	const eiScalar x4)
{
	eiScalar	xb, d1, d2, d3, d4, s;

	xb = (x1 + x2 + x3 + x4) * 0.25f;
	d1 = x1 - xb;
	d2 = x2 - xb;
	d3 = x3 - xb;
	d4 = x4 - xb;
	s = d1 * d1 + d2 * d2 + d3 * d3 + d4 * d4;
	s = sqrtf((1.0f / 3.0f) * MAX(s, 0.0f));

	return s;
}

/* compare whether four samples diff too much. */
static eiFORCEINLINE eiBool ei_sample_info_diff(
	eiBucket *bucket, 
	const eiSampleInfo *c1, const eiSampleInfo *c2, 
	const eiSampleInfo *c3, const eiSampleInfo *c4, 
	const eiInt depth)
{
	eiVector4	*contrast;
	eiScalar	scale;

	contrast = &bucket->base.opt->contrast;

	scale = 3.0f / (eiScalar)(1 << MAX(depth, 0));

	if (calc_contrast(c1->color.r, c2->color.r, c3->color.r, c4->color.r) * scale > contrast->r || 
		calc_contrast(c1->color.g, c2->color.g, c3->color.g, c4->color.g) * scale > contrast->g || 
		calc_contrast(c1->color.b, c2->color.b, c3->color.b, c4->color.b) * scale > contrast->b)
	{
		return eiTRUE;
	}
	else
	{
		return eiFALSE;
	}
}

/* average two samples. */
static eiFORCEINLINE void ei_sample_info_average2(
	eiBucket *bucket, 
	eiSampleInfo *avg, 
	eiSampleInfo *c1, eiSampleInfo *c2)
{
	memcpy(avg, c1, sizeof(eiSampleInfo) + bucket->job->user_output_size);
	ei_sample_info_add(bucket, avg, c2);
	ei_sample_info_mul(bucket, avg, 0.5f);
}

/* average four samples. */
static eiFORCEINLINE void ei_sample_info_average4(
	eiBucket *bucket, 
	eiSampleInfo *avg, 
	eiSampleInfo *c1, eiSampleInfo *c2, 
	eiSampleInfo *c3, eiSampleInfo *c4)
{
	memcpy(avg, c1, sizeof(eiSampleInfo) + bucket->job->user_output_size);
	ei_sample_info_add(bucket, avg, c2);
	ei_sample_info_add(bucket, avg, c3);
	ei_sample_info_add(bucket, avg, c4);
	ei_sample_info_mul(bucket, avg, 0.25f);
}

/* set the raster position in subpixel unit.
   l_rasterPos means the local raster position in this bucket which will 
   be used frequently. */
static eiFORCEINLINE void set_raster_pos(eiBucket *bucket, const eiInt x, const eiInt y)
{
	eiRenderParams	*par = &bucket->par;

	bucket->l_rasterPos_x = x;
	bucket->l_rasterPos_y = y;
	bucket->rasterPos.x = (eiScalar)x * par->subpixel_to_pixel + bucket->local_to_screen_x;
	bucket->rasterPos.y = (eiScalar)y * par->subpixel_to_pixel + bucket->local_to_screen_y;
}

/** \brief Sample the lens, either by calling 
 * lens shader or tracing a ray */
static void sample_lens(
	eiBucket *bucket, 
	eiSampleInfo *color, 
	const eiScalar time, 
	const eiVector2 *rpos, 
	eiBool * const pass_motion, 
	const eiUint ray_instance_number)
{
	eiOptions	*opt;
	eiCamera	*cam;
	eiState		eye_ray;

	opt = bucket->base.opt;
	cam = bucket->base.cam;
	ei_state_init(&eye_ray, eiRAY_EYE, &bucket->base);

	eye_ray.instance_number = ray_instance_number;

	/* perspective projection */
	if (cam->focal != eiMAX_SCALAR)
	{
		eiVector	ray_dir;

		initv(&eye_ray.org);
		ei_camera_get_ray_dir(cam, &ray_dir, rpos->x, rpos->y);
		eye_ray.dir = ray_dir;
		normalizei(&eye_ray.dir);
		/* get the unnormalized one */
		movv(&eye_ray.I, &ray_dir);
	}
	else
	{
		ei_camera_get_ray_pos(cam, &eye_ray.org, rpos->x, rpos->y);
		setv(&eye_ray.dir, 0.0f, 0.0f, -1.0f);
		movv(&eye_ray.I, &eye_ray.dir);
	}

	/* dependent trajectory splitting
	   used dimensions: 
	       antialiasing:   0
		   motion blur:    1
	   so here we set current available dimension to be 2 */
	eye_ray.dimension = 2;
	eye_ray.time = time;
	eye_ray.dtime = opt->shutter_close - opt->shutter_open;
	eye_ray.raster = *rpos;
	eye_ray.result = color;

	/* call lens shader to modify the ray if available, 
	   the lens shader is responsible for shooting rays */
	if (opt->lens && 
		cam->lens_list != eiNULL_TAG && 
		!ei_data_array_empty(bucket->base.db, cam->lens_list))
	{
		eiVector4	result;

		initv4(&result);

		ei_call_shader_instance_list(
			bucket->base.nodesys, 
			&result, 
			&eye_ray, 
			cam->lens_list, 
			NULL);
	}
	else
	{
		/* no lens shader available, trace the eye ray 
		   directly by ourselves */
		ei_trace_eye(color, &eye_ray.org, &eye_ray.dir, &eye_ray);
	}

	/* call imager shader to modify the final sub-pixel color */
	if (opt->imager && 
		cam->imager_list != eiNULL_TAG && 
		!ei_data_array_empty(bucket->base.db, cam->imager_list))
	{
		eiVector4	result;

		initv4(&result);

		ei_call_shader_instance_list(
			bucket->base.nodesys, 
			&result, 
			&eye_ray, 
			cam->imager_list, 
			NULL);
	}

	/* if the lens shader shoot sub-rays, it should update 
	   this flag of the parent ray too */
	*pass_motion |= eye_ray.pass_motion;

	ei_state_exit(&eye_ray);
}

/* put a sub-pixel sample into buffers. */
static eiFORCEINLINE void put_sample(
	eiBucket *bucket, 
	eiSampleInfo *info, 
	const eiInt x, const eiInt y, const eiInt depth, 
	eiSampleInfo *s1, eiSampleInfo *s2, eiSampleInfo *s3, eiSampleInfo *s4, 
	const eiScalar sf)
{
	eiRenderParams	*par;
	eiInt			lx, ly;
	eiInt			ix2, iy2;
	ei_slist		*sample_list;
	eiScalar		dw;

	par = &bucket->par;

	lx = bucket->l_rasterPos_x + x;
	ly = bucket->l_rasterPos_y + y;

	info->x = lx;
	info->y = ly;
	info->weight = (eiScalar)(par->num_subpixels >> (depth - par->min_depth));
	info->weight *= info->weight;

	/* add to sample list for fast reconstruction */
	ix2 = (eiInt)((eiScalar)lx * par->inv_num_subpixels + 0.5f);
	iy2 = (eiInt)((eiScalar)ly * par->inv_num_subpixels + 0.5f);

	sample_list = &(((eiPixelInfo *)ei_buffer_getptr(&bucket->pixelBuffer, ix2, iy2))->sample_list);

	ei_slist_push_back(sample_list, &info->node);
	ei_buffer_set(&bucket->sampleBuffer, lx, ly, &info);

	/* substract the weight for higher-level pixels */
	dw = info->weight * sf;

	if (s1 != NULL)
	{
		s1->weight -= dw;
	}

	if (s2 != NULL)
	{
		s2->weight -= dw;
	}

	if (s3 != NULL)
	{
		s3->weight -= dw;
	}

	if (s4 != NULL)
	{
		s4->weight -= dw;
	}
}

/* sample a sub-pixel location. set_raster_pos should be called properly 
   before calling this function. */
static eiSampleInfo *sample(
	eiBucket *bucket, 
	const eiInt x, const eiInt y, const eiInt depth, 
	eiSampleInfo *s1, eiSampleInfo *s2, eiSampleInfo *s3, eiSampleInfo *s4, 
	const eiScalar sf)
{
	eiRenderParams	*par;
	eiOptions		*opt;
	eiCamera		*cam;
	eiSampleInfo	*c;
	eiScalar		sx, sy;
	eiVector2		rpos;
	eiUint			ray_instance_number;
	eiGeoScalar		jx, jy;
	eiScalar		time0, time;
	eiBool			pass_motion;
	
	par = &bucket->par;
	opt = bucket->base.opt;
	cam = bucket->base.cam;
	c = create_sample_info(bucket);

	/* compute pixel location */
	ei_filter_table_reg(&par->filterTable, x, y, &sx, &sy);

	sx = sx * (eiScalar)par->num_spans + bucket->rasterPos.x;
	sy = sy * (eiScalar)par->num_spans + bucket->rasterPos.y;

	ray_instance_number = 0;
	jx = 0.0;
	jy = 0.0;
	ei_sample_subpixel(&ray_instance_number, &jx, &jy, 
		(eiInt)(sx * par->inv_subpixel_to_pixel), 
		(eiInt)(sy * par->inv_subpixel_to_pixel));

	sx += (eiScalar)((jx - 0.5) * par->subpixel_to_pixel);
	sy += (eiScalar)((jy - 0.5) * par->subpixel_to_pixel);

	setv2(&rpos, sx, sy);
	time0 = (eiScalar)ei_sigma(1, ray_instance_number);
	pass_motion = eiFALSE;

	if (opt->motion && opt->motion_segments > 0)
	{
		eiSampleInfo	*sub_c;
		eiScalar		interval;
		eiInt			actualNumTemporalSamples;
		eiInt			i;

		sub_c = create_sample_info(bucket);
		interval = 1.0f / (eiScalar)opt->motion_segments;
		actualNumTemporalSamples = 0;

		for (i = 0; i <= opt->motion_segments; ++i)
		{
			/* trajectory splitting */
			time = fmodf((time0 + (eiScalar)i * interval), 1.0f);

			reset_sample_info(bucket, sub_c);

			/* decorrelating */
			sample_lens(bucket, sub_c, time, &rpos, &pass_motion, ray_instance_number + i);

			ei_sample_info_add(bucket, c, sub_c);
			++ actualNumTemporalSamples;

			/* if this ray didn't pass any subspace containing motion, 
			   don't trace any further */
			if (!pass_motion)
			{
				break;
			}
		}

		ei_sample_info_mul(bucket, c, 1.0f / (eiScalar)actualNumTemporalSamples);

		delete_sample_info(bucket, sub_c);
	}
	else
	{
		sample_lens(bucket, c, time0, &rpos, &pass_motion, ray_instance_number);
	}

	put_sample(
		bucket, 
		c, 
		x, y, depth, 
		s1, s2, s3, s4, 
		sf);

	return c;
}

/* compute a final gather sample */
void fg_sample(
	eiBucket *bucket, 
	const eiScalar sx, const eiScalar sy, 
	const eiScalar fixed_sx, const eiScalar fixed_sy)
{
	eiBucketJob		*job;
	eiRenderParams	*par;
	eiOptions		*opt;
	eiCamera		*cam;
	eiSampleInfo	*c;
	eiVector2		rpos;
	eiUint			ray_instance_number;
	eiGeoScalar		jx, jy;
	eiScalar		time0;
	eiBool			pass_motion;
	
	job = bucket->job;
	par = &bucket->par;
	opt = bucket->base.opt;
	cam = bucket->base.cam;
	c = create_sample_info(bucket);

	ray_instance_number = 0;
	jx = 0.0;
	jy = 0.0;
	ei_sample_subpixel(&ray_instance_number, &jx, &jy, 
		(eiInt)(sx * par->inv_subpixel_to_pixel), 
		(eiInt)(sy * par->inv_subpixel_to_pixel));

	setv2(&rpos, sx, sy);
	time0 = (eiScalar)ei_sigma(1, ray_instance_number);
	pass_motion = eiFALSE;

	sample_lens(bucket, c, time0, &rpos, &pass_motion, ray_instance_number);

	switch (opt->finalgather_progress)
	{
	case EI_FINALGATHER_PROGRESS_POINT:
		{
			write_sample(bucket, (eiInt)sx - job->rect.left, (eiInt)sy - job->rect.top, c);
		}
		break;

	case EI_FINALGATHER_PROGRESS_PAINT:
		{
			paint_sample(bucket, (eiInt)fixed_sx - job->rect.left, (eiInt)fixed_sy - job->rect.top, (eiInt)job->point_spacing, c);
		}
		break;

	default:
		{
			/* error */
		}
		break;
	}

	delete_sample_info(bucket, c);
}

/* do not really sample, just fill in the sample info. */
static eiFORCEINLINE void fake_sample(
	eiBucket *bucket, 
	eiSampleInfo *info, 
	const eiInt x, const eiInt y, const eiInt depth, 
	eiSampleInfo *s1, eiSampleInfo *s2, eiSampleInfo *s3, eiSampleInfo *s4, 
	const eiScalar sf)
{
	put_sample(
		bucket, 
		info, 
		x, y, depth, 
		s1, s2, s3, s4, 
		sf);
}

/* super sample a pixel. */
static void supersample(eiBucket *bucket, eiInt left, eiInt top)
{
	eiRenderParams	*par;
	eiInt			my_step, my_step2;
	eiSampleInfo	*s1, *s2, *s3, *s4;
	eiInt			depth;

	par = &bucket->par;

	my_step = par->num_subpixels;
	left = left * par->num_subpixels;
	top = top * par->num_subpixels;

	for (depth = par->min_depth + 1; depth <= par->bound_max_depth; ++ depth)
	{
		eiBool	needSupersampling;
		eiInt	i, j;

		my_step2 = my_step / 2;
		needSupersampling = eiFALSE;

		for (j = top; j < (top + par->num_subpixels); j += my_step)
		{
			for (i = left; i < (left + par->num_subpixels); i += my_step)
			{
				set_raster_pos(bucket, i, j);

				s1 = *((eiSampleInfo **)ei_buffer_getptr(&bucket->sampleBuffer, i, j));
				s2 = *((eiSampleInfo **)ei_buffer_getptr(&bucket->sampleBuffer, i + my_step, j));
				s3 = *((eiSampleInfo **)ei_buffer_getptr(&bucket->sampleBuffer, i, j + my_step));
				s4 = *((eiSampleInfo **)ei_buffer_getptr(&bucket->sampleBuffer, i + my_step, j + my_step));

				if (s1 != NULL && s2 != NULL && s3 != NULL && s4 != NULL)
				{
					if (depth <= par->max_depth && ei_sample_info_diff(bucket, s1, s2, s3, s4, depth))
					{
						eiSampleInfo	*ss0, *ss1, *ss2, *ss3, *ss4;

						ss0 = sample(bucket, my_step2, my_step2, depth, s1, s2, s3, s4, 0.25f);

						ss1 = *((eiSampleInfo **)ei_buffer_getptr(&bucket->sampleBuffer, i + my_step2, j + 0));
						if (ss1 == NULL)
						{
							ss1 = sample(bucket, my_step2, 0, depth, s1, s2, NULL, NULL, 0.5f);
						}

						ss2 = *((eiSampleInfo **)ei_buffer_getptr(&bucket->sampleBuffer, i + 0, j + my_step2));
						if (ss2 == NULL)
						{
							ss2 = sample(bucket, 0, my_step2, depth, s1, s3, NULL, NULL, 0.5f);
						}

						ss3 = sample(bucket, my_step, my_step2, depth, s2, s4, NULL, NULL, 0.5f);
						ss4 = sample(bucket, my_step2, my_step, depth, s3, s4, NULL, NULL, 0.5f);

						needSupersampling = eiTRUE;
					}
					else if (depth <= 0)
					{
						eiSampleInfo	*avg;

						avg = create_sample_info(bucket);
						ei_sample_info_average4(bucket, avg, s1, s2, s3, s4);
						fake_sample(bucket, avg, my_step2, my_step2, depth, s1, s2, s3, s4, 0.25f);

						if (*((eiSampleInfo **)ei_buffer_getptr(&bucket->sampleBuffer, i + my_step2, j + 0)) == NULL)
						{
							avg = create_sample_info(bucket);
							ei_sample_info_average2(bucket, avg, s1, s2);
							fake_sample(bucket, avg, my_step2, 0, depth, s1, s2, NULL, NULL, 0.5f);
						}

						if (*((eiSampleInfo **)ei_buffer_getptr(&bucket->sampleBuffer, i + 0, j + my_step2)) == NULL)
						{
							avg = create_sample_info(bucket);
							ei_sample_info_average2(bucket, avg, s1, s3);
							fake_sample(bucket, avg, 0, my_step2, depth, s1, s3, NULL, NULL, 0.5f);
						}

						avg = create_sample_info(bucket);
						ei_sample_info_average2(bucket, avg, s2, s4);
						fake_sample(bucket, avg, my_step, my_step2, depth, s2, s4, NULL, NULL, 0.5f);

						avg = create_sample_info(bucket);
						ei_sample_info_average2(bucket, avg, s3, s4);
						fake_sample(bucket, avg, my_step2, my_step, depth, s3, s4, NULL, NULL, 0.5f);
					}
				}
			}
		}

		if (depth > 0 && !needSupersampling)
		{
			break;
		}

		my_step = my_step2;
	}
}

/* reconstruct a specified pixel and write to frame buffer. */
static void contribute(eiBucket *bucket, const eiInt px, const eiInt py)
{
	eiRenderParams	*par;
	eiSampleInfo	*factor, *total;
	ei_slist		*sample_list;
	eiScalar		x, y, filter_radius;
	eiInt			xmin, xmax, ymin, ymax;
	eiInt			lx, ly;
	eiScalar		sum_weight, weight;
	eiInt			i, j;

	par = &bucket->par;

	factor = create_sample_info(bucket);
	total = create_sample_info(bucket);

	x = ((eiScalar)px + 0.5f) * par->inv_num_spans + (eiScalar)par->sample_filter_radius;
	y = ((eiScalar)py + 0.5f) * par->inv_num_spans + (eiScalar)par->sample_filter_radius;

	filter_radius = bucket->base.opt->filter_size * 0.5f * par->inv_num_spans;

	xmin = lfloorf(x - filter_radius);
	xmax = lceilf(x + filter_radius);
	ymin = lfloorf(y - filter_radius);
	ymax = lceilf(y + filter_radius);

	lx = (eiInt)(x * (eiScalar)par->num_subpixels);
	ly = (eiInt)(y * (eiScalar)par->num_subpixels);

	sum_weight = 0.0f;

	for (j = ymin; j <= ymax; ++j)
	{
		for (i = xmin; i <= xmax; ++i)
		{
			eiSampleInfo	*info;

			sample_list = &(((eiPixelInfo *)ei_buffer_getptr(&bucket->pixelBuffer, i, j))->sample_list);

			for (info = (eiSampleInfo *)ei_slist_begin(sample_list); 
				info != (eiSampleInfo *)ei_slist_end(sample_list); 
				info = (eiSampleInfo *)info->node.next)
			{
				memcpy(factor, info, sizeof(eiSampleInfo) + bucket->job->user_output_size);

				weight = ei_filter_table_get_weight(&par->filterTable, abs(info->x - lx), abs(info->y - ly));
				weight *= MAX(0.0f, info->weight);

				ei_sample_info_mul(bucket, factor, weight);
				sum_weight += weight;

				ei_sample_info_add(bucket, total, factor);
			}
		}
	}

	ei_sample_info_mul(bucket, total, 1.0f / sum_weight);	/* cost of a division. */

	/* compute sampling rate for diagnostics */
	if (bucket->base.opt->diagnostic_mode == EI_DIAGNOSTIC_MODE_SAMPLING_RATE)
	{
		eiScalar	sampling_rate;

		sample_list = &(((eiPixelInfo *)ei_buffer_getptr(&bucket->pixelBuffer, (eiInt)x, (eiInt)y))->sample_list);
		sampling_rate = (eiScalar)(ei_slist_size(sample_list) - 1) * par->inv_num_subpixels;
		clampi(sampling_rate, 0.0f, 1.0f);
		setvf(&total->color, sampling_rate);
	}

	write_sample(bucket, px, py, total);

	delete_sample_info(bucket, total);
	delete_sample_info(bucket, factor);
}

static void ei_bucket_run_frame(
	eiBucket *bucket, eiBucketJob *job, eiDatabase *db, eiBaseWorker *pWorker)
{
	eiRenderParams	*par;
	eiInt			recon_width;
	eiInt			recon_height;
	eiInt			sample_width;
	eiInt			sample_height;
	eiInt			pb_width;
	eiInt			pb_height;
	eiInt			i, j;

	/* initialize some parameters */
	par = &bucket->par;
	recon_width = lceilf((eiScalar)bucket->rect_width  * par->inv_num_spans);
	recon_height = lceilf((eiScalar)bucket->rect_height * par->inv_num_spans);
	sample_width = recon_width + 2 * par->sample_filter_radius;
	sample_height = recon_height + 2 * par->sample_filter_radius;
	pb_width = sample_width  * par->num_subpixels;
	pb_height = sample_height * par->num_subpixels;

	bucket->local_to_screen_x = (eiScalar)job->rect.left - (eiScalar)par->sample_filter_radius * par->num_spans;
	bucket->local_to_screen_y = (eiScalar)job->rect.top - (eiScalar)par->sample_filter_radius * par->num_spans;

	/* sampleBuffer is used to prevent from sampling repeatedly at the same point */
	ei_buffer_allocate(&bucket->sampleBuffer, pb_width + 1, pb_height + 1);
	ei_buffer_zero_memory(&bucket->sampleBuffer);

	ei_buffer_allocate(&bucket->pixelBuffer, sample_width + 1, sample_height + 1);

	/* do basic sampling */
	for (j = 0; j <= pb_height; j += par->num_subpixels)
	{
		if (!ei_base_worker_is_running(pWorker))
		{
			return;
		}

		for (i = 0; i <= pb_width; i += par->num_subpixels)
		{
			set_raster_pos(bucket, i, j);

			sample(bucket, 0, 0, par->min_depth, NULL, NULL, NULL, NULL, 0.0f);
		}

		ei_base_worker_step_progress(pWorker, bucket->rect_width / 3);
	}

	/* do super-sampling for anti-aliasing */
	for (j = 0; j < sample_height; ++j)
	{
		if (!ei_base_worker_is_running(pWorker))
		{
			return;
		}

		for (i = 0; i < sample_width; ++i)
		{
			supersample(bucket, i, j);
		}

		ei_base_worker_step_progress(pWorker, bucket->rect_width / 3);
	}

	/* create a block of image from the buffer with filtering */
	for (j = 0; j < bucket->rect_height; ++j)
	{
		if (!ei_base_worker_is_running(pWorker))
		{
			return;
		}

		for (i = 0; i < bucket->rect_width; ++i)
		{
			contribute(bucket, i, j);
		}

		ei_base_worker_step_progress(pWorker, bucket->rect_width / 3);
	}
}

static void ei_bucket_run_finalgather(
	eiBucket *bucket, eiBucketJob *job, eiDatabase *db, eiBaseWorker *pWorker)
{
	eiScalar		offset_x, offset_y;
	eiInt			num_spacings_x, num_spacings_y;
	eiInt			num_fg_points_x, num_fg_points_y;
	eiInt			random_instance_id;
	eiInt			i, j;

	offset_x = (eiScalar)job->rect.left / job->point_spacing;
	num_spacings_x = lceilf(offset_x);
	offset_x = (eiScalar)num_spacings_x * job->point_spacing;

	offset_y = (eiScalar)job->rect.top / job->point_spacing;
	num_spacings_y = lceilf(offset_y);
	offset_y = (eiScalar)num_spacings_y * job->point_spacing;

	num_fg_points_x = lceilf(((eiScalar)(job->rect.right + 1) - offset_x) / job->point_spacing);
	num_fg_points_y = lceilf(((eiScalar)(job->rect.bottom + 1) - offset_y) / job->point_spacing);
	random_instance_id = 0;

	for (j = 0; j < num_fg_points_y; ++j)
	{
		if (!ei_base_worker_is_running(pWorker))
		{
			return;
		}

		for (i = 0; i < num_fg_points_x; ++i)
		{
			eiScalar	sx, sy;
			eiScalar	rv[2];
			eiScalar	fixed_sx, fixed_sy;

			/* prevent generating generated points in last pass */
			if (job->pass_mode == EI_PASS_FINALGATHER_REFINE && 
				(i % 2) == 0 && (j % 2) == 0)
			{
				continue;
			}

			rv[0] = (eiScalar)ei_sigma(0, random_instance_id);
			rv[1] = (eiScalar)ei_sigma(1, random_instance_id);
			++ random_instance_id;

			sx = ((eiScalar)i + (rv[0] * 0.25f - 0.125f)) * job->point_spacing;
			sy = ((eiScalar)j + (rv[1] * 0.25f - 0.125f)) * job->point_spacing;

			sx = sx + offset_x + (eiScalar)((num_spacings_y + j) % 2) * job->point_spacing * 0.5f;
			sy = sy + offset_y;

			fixed_sx = ((eiScalar)i) * job->point_spacing;
			fixed_sy = ((eiScalar)j) * job->point_spacing;

			fixed_sx = fixed_sx + offset_x + (eiScalar)((num_spacings_y + j) % 2) * job->point_spacing * 0.5f;
			fixed_sy = fixed_sy + offset_y;

			fg_sample(bucket, sx, sy, fixed_sx, fixed_sy);
		}

		ei_base_worker_step_progress(pWorker, bucket->rect_width);
	}
}

static eiBool ei_bucket_run(
	eiBucket *bucket, eiBucketJob *job, eiDatabase *db, eiBaseWorker *pWorker)
{
	ei_bucket_init(bucket, job, db);

	switch (bucket->job->pass_mode)
	{
	case EI_PASS_FRAME:
		{
			ei_bucket_run_frame(
				bucket, 
				job, 
				db, 
				pWorker);
		}
		break;

	case EI_PASS_FINALGATHER_INITIAL:
	case EI_PASS_FINALGATHER_REFINE:
		{
			ei_bucket_run_finalgather(
				bucket, 
				job, 
				db, 
				pWorker);
		}
		break;

	default:
		{
			ei_error("Invalid pass mode.\n");
		}
		break;
	}

	return ei_bucket_exit(bucket);
}

void byteswap_job_bucket(eiDatabase *db, void *job, const eiUint size)
{
	eiBucketJob *pJob = (eiBucketJob *)job;

	ei_byteswap_int(&pJob->pos_i);
	ei_byteswap_int(&pJob->pos_j);
	ei_byteswap_rect4i(&pJob->rect);
	ei_byteswap_int(&pJob->user_output_size);
	ei_byteswap_int(&pJob->opt);
	ei_byteswap_int(&pJob->cam);
	ei_byteswap_int(&pJob->colorFrameBuffer);
	ei_byteswap_int(&pJob->opacityFrameBuffer);
	ei_byteswap_int(&pJob->frameBuffers);
	ei_byteswap_int(&pJob->lightInstances);
	ei_byteswap_int(&pJob->causticMap);
	ei_byteswap_int(&pJob->globillumMap);
	ei_byteswap_int(&pJob->irradCache);
	ei_byteswap_int(&pJob->pass_mode);
	ei_byteswap_scalar(&pJob->point_spacing);
	ei_byteswap_int(&pJob->passIrradBuffer);
	ei_byteswap_int(&pJob->bucket_id);
}

eiBool execute_job_bucket(eiDatabase *db, eiBaseWorker *pWorker, void *job, void *param)
{
	eiBucketJob		*pJob;
	eiBucket		bucket;
	
	pJob = (eiBucketJob *)job;

	return ei_bucket_run(&bucket, pJob, db, pWorker);
}

eiUint count_job_bucket(eiDatabase *db, void *job)
{
	eiBucketJob *pJob = (eiBucketJob *)job;

	return ((pJob->rect.right - pJob->rect.left + 1) * (pJob->rect.bottom - pJob->rect.top + 1));
}
