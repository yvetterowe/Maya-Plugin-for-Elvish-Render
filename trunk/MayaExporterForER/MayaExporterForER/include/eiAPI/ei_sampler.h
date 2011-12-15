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
 
#ifndef EI_SAMPLER_H
#define EI_SAMPLER_H

/** \brief The adaptive sampler for rendering the final image.
 * \file ei_sampler.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiCORE/ei_vector.h>
#include <eiCORE/ei_vector2.h>
#include <eiCORE/ei_vector4.h>
#include <eiCORE/ei_rect.h>
#include <eiCORE/ei_slist.h>
#include <eiCORE/ei_array.h>
#include <eiCORE/ei_fixed_pool.h>
#include <eiAPI/ei_buffer.h>
#include <eiAPI/ei_nodesys.h>
#include <eiAPI/ei_base_bucket.h>
#include <eiAPI/ei_finalgather.h>

#ifdef __cplusplus
extern "C" {
#endif

/* forward declarations */
typedef struct eiDatabase			eiDatabase;
typedef struct eiBaseWorker			eiBaseWorker;
typedef struct eiOptions			eiOptions;
typedef struct eiCamera				eiCamera;
typedef struct eiRayTracer			eiRayTracer;
typedef struct eiConnection			eiConnection;

/** \brief A precomputed filter table for reconstructing 
 * the image. encapsulate a filter table which predetermines 
 * the locations of samples in a pixel for all kinds of 
 * hiding algorithms. */
typedef struct eiFilterTable {
	/* the buffer of eiVector2 */
	eiBuffer		table;
	/* the buffer of eiScalar */
	eiBuffer		weights;
	eiInt			filter_radius;
	eiInt			num_subpixels;
} eiFilterTable;

/** \brief Precomputed parameters so that we don't 
 * recompute them during rendering. */
typedef struct eiRenderParams {
	/* precomputed filter table */
	eiFilterTable				filterTable;
	/* sampling parameters */
	eiScalar					filter_radius;
	eiScalar					inv_num_subpixels;
	eiScalar					num_spans;
	eiScalar					inv_num_spans;
	eiScalar					subpixel_to_pixel;
	eiScalar					inv_subpixel_to_pixel;
	eiInt						min_depth;
	eiInt						max_depth;
	eiInt						bound_max_depth;
	eiInt						num_subpixels;
	eiInt						sample_filter_radius;
	/* photon mapping parameters */
	eiScalar					caustic_radius;
	eiScalar					globillum_radius;
	/* final gather parameters */
	eiScalar					finalgather_radius;
	eiBool						finalgather_falloff;
	eiScalar					finalgather_falloff_start;
	eiScalar					finalgather_falloff_stop;
} eiRenderParams;

/** \brief The pass mode of bucket rendering */
enum {
	EI_PASS_NONE = 0,				/* unknown pass mode */
	EI_PASS_FRAME,					/* frame rendering */
	EI_PASS_FINALGATHER_INITIAL,	/* initial final gather point generation */
	EI_PASS_FINALGATHER_REFINE,		/* final gather point refinement */
};

/** \brief A bucket is an image tile, the rendered image 
 * is produced one tile by one tile, this is the job for 
 * rendering a bucket. */
typedef struct eiBucketJob {
	eiInt			pos_i;
	eiInt			pos_j;
	eiRect4i		rect;
	eiUint			user_output_size;
	eiTag			opt;
	eiTag			cam;
	eiTag			colorFrameBuffer;
	eiTag			opacityFrameBuffer;
	/* the data array of frame buffer tags */
	eiTag			frameBuffers;
	/* the data table of light instances */
	eiTag			lightInstances;
	/* the global photon maps */
	eiTag			causticMap;
	eiTag			globillumMap;
	/* the global irradiance cache */
	eiTag			irradCache;
	/* the pass mode we want to run */
	eiInt			pass_mode;
	/* the point spacing of final gather */
	eiScalar		point_spacing;
	/* the irradiance buffer for current pass */
	eiTag			passIrradBuffer;
	/* the bucket identifier */
	eiInt			bucket_id;
} eiBucketJob;

/** \brief The working memory of bucket rendering. */
typedef struct eiBucket {
	eiBaseBucket			base;
	eiInt					rect_width;
	eiInt					rect_height;
	eiInt					l_rasterPos_x;
	eiInt					l_rasterPos_y;
	eiVector2				rasterPos;
	eiScalar				local_to_screen_x;
	eiScalar				local_to_screen_y;
	/* the buffer of eiSampleInfo pointer */
	eiBuffer				sampleBuffer;
	/* the buffer of eiPixelInfo */
	eiBuffer				pixelBuffer;
	eiFrameBufferCache		colorFrameBufferCache;
	eiFrameBufferCache		opacityFrameBufferCache;
	/* the array of eiFrameBufferCache */
	ei_array				frameBufferCaches;
	eiBucketJob				*job;
	eiRenderParams			par;
	ei_fixed_pool			samplePool;
} eiBucket;

eiSampleInfo *create_sample_info(eiBucket *bucket);
void delete_sample_info(eiBucket *bucket, eiSampleInfo * const info);
void reset_sample_info(eiBucket *bucket, eiSampleInfo * const info);
void ei_sample_info_add(eiBucket *bucket, eiSampleInfo * const c1, eiSampleInfo * const c2);
void ei_sample_info_mul(eiBucket *bucket, eiSampleInfo * const c, const eiScalar a);

/** \brief Add a new irradiance value. */
void ei_add_irradiance(eiBucket *bucket, const eiIrradiance *irrad);
/** \brief Lookup irradiance values within certain radius. */
eiBool ei_lookup_irradiance(
	eiBucket *bucket, 
	eiVector * const L, 
	const eiVector *P, 
	const eiVector *N, 
	const eiScalar A, 
	const eiScalar R, 
	const eiUint numInterpPoints, 
	const eiBool forceInterp);

/* for internal use only */
void byteswap_job_bucket(eiDatabase *db, void *job, const eiUint size);
eiBool execute_job_bucket(eiDatabase *db, eiBaseWorker *pWorker, void *job, void *param);
eiUint count_job_bucket(eiDatabase *db, void *job);

#ifdef __cplusplus
}
#endif

#endif
