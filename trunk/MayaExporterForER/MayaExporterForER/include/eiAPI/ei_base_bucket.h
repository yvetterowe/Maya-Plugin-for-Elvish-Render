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
 
#ifndef EI_BASE_BUCKET_H
#define EI_BASE_BUCKET_H

#include <eiAPI/ei_state.h>
#include <eiAPI/ei_raytracer.h>
#include <eiAPI/ei_nodesys.h>
#include <eiAPI/ei_options.h>
#include <eiAPI/ei_camera.h>
#include <eiCORE/ei_data_table.h>
#include <eiCORE/ei_random.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The bucket types */
enum {
	EI_BUCKET_TYPE_NONE = 0, 
	EI_BUCKET_FRAME, 
	EI_BUCKET_PHOTON_GI, 
	EI_BUCKET_PHOTON_CAUSTIC, 
	EI_BUCKET_TYPE_COUNT, 
};

/** \brief The base bucket class which contains the common 
 * working set of currently executing job.
 * for internal use only */
struct eiBaseBucket {
	/* the database we are working on */
	eiDatabase				*db;
	eiRayTracer				*rt;
	eiNodeSystem			*nodesys;
	eiOptions				*opt;
	eiCamera				*cam;
	eiDataTableIterator		light_insts_iter;
	eiRandomGen				randGen;
	eiInt					type;
};

/** \brief Build a geometry bucket for approximation. */
eiAPI void ei_build_approx_bucket(
	eiBaseBucket *bucket, 
	eiDatabase *db, 
	const eiInt random_offset);
/** \brief Initialize base bucket with some required 
 * parameters. */
eiAPI void ei_base_bucket_init(
	eiBaseBucket *bucket, 
	eiDatabase *db, 
	const eiTag opt_tag, 
	const eiTag cam_tag, 
	const eiTag lightInstances, 
	const eiInt random_offset);
/** \brief Cleanup base bucket */
eiAPI void ei_base_bucket_exit(
	eiBaseBucket *bucket, 
	const eiTag opt_tag, 
	const eiTag cam_tag);

#ifdef __cplusplus
}
#endif

#endif
