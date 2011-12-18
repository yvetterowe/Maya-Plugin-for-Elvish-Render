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

#ifndef EI_PHOTON_H
#define EI_PHOTON_H

#include <eiAPI/ei_map.h>
#include <eiAPI/ei_raytracer.h>
#include <eiAPI/ei_base_bucket.h>
#include <eiCORE/ei_rgbe.h>
#include <eiCORE/ei_sincos.h>
#include <eiCORE/ei_random.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eiLightFlux {
	eiScalar	energy;
	eiInt		light_index;
} eiLightFlux;

/* for internal use only */
void byteswap_light_flux(eiDatabase *db, void *ptr, const eiUint size);

typedef struct eiPhoton {
	eiMapNode	base;
	eiByte		theta;	/* 1 */
	eiByte		phi;	/* 1 */
	eiRGBE		power;	/* 4 */
} eiPhoton;

/* for internal use only */
void byteswap_photon(eiDatabase *db, void *ptr, const eiUint size);

eiFORCEINLINE void ei_photon_init(
	eiPhoton *p, 
	const eiVector *energy, 
	const eiVector *pos, 
	const eiVector *dir)
{
	eiInt	iTheta, iPhi;

	movv(&p->base.pos, pos);
	setRGBE(&p->power, energy);

	iTheta = (eiInt)(acos(dir->z) * (256.0f / (eiScalar)eiPI));
	if (iTheta > 255) {
		p->theta = 255;
	} else {
		p->theta = (eiByte)iTheta;
	}

	iPhi = (eiInt)(atan2(dir->y, dir->x) * (256.0f / (2.0f * (eiScalar)eiPI)));
	if (iPhi > 255) {
		p->phi = 255;
	} else if (iPhi < 0) {
		p->phi = (eiByte)(iPhi + 256);
	} else {
		p->phi = (eiByte)iPhi;
	}
}

eiFORCEINLINE void ei_photon_dir(eiVector *dir, const eiPhoton *p)
{
	dir->x = sintheta[p->theta] * cosphi[p->phi];
	dir->y = sintheta[p->theta] * sinphi[p->phi];
	dir->z = costheta[p->theta];
}

typedef struct eiPhotonFastCondition {
	eiVector	P;
	eiVector	N;
} eiPhotonFastCondition;

eiBool ei_photon_fast_cond_proc(
	const eiMapNode *node, 
	const eiScalar R2, 
	void *param);

typedef struct eiPhotonCondition {
	eiVector	P;
	eiVector	N;
} eiPhotonCondition;

eiBool ei_photon_cond_proc(
	const eiMapNode *node, 
	const eiScalar R2, 
	void *param);

void ei_photon_map_precompute_irrad(
	eiDatabase *db, 
	const eiTag tag, 
	const eiScalar max_dist, 
	const eiUint gather_points, 
	const eiInt filter, 
	const eiScalar coneKernel);

void ei_photon_map_fast_lookup_irrad(
	eiDatabase *db, 
	const eiTag tag, 
	eiVector * const L, 
	const eiVector *P, 
	const eiVector *N, 
	const eiScalar max_dist, 
	const eiUint gather_points);

void ei_photon_map_lookup_irrad(
	eiDatabase *db, 
	const eiTag tag, 
	eiVector * const L, 
	const eiVector *P, 
	const eiVector *N, 
	const eiScalar max_dist, 
	const eiUint gather_points, 
	const eiInt filter, 
	const eiScalar coneKernel);

void ei_photon_map_scale_photons(
	eiDatabase *db, 
	const eiTag tag, 
	const eiInt prev_scale, 
	const eiScalar factor);

typedef struct eiPhotonJob {
	eiTag			opt;
	eiTag			cam;
	eiTag			light_flux_histogram;
	eiScalar		acc;
	eiInt			num_target_photons;
	eiInt			photon_type;
	/* local photon cache */
	eiTag			caustic_photons;
	eiTag			globillum_photons;
	eiTag			count;
	/* instance number of Halton sequence */
	eiInt			halton_num;
	eiTag			lightInstances;
} eiPhotonJob;

typedef struct eiPhotonBucket {
	eiBaseBucket	base;
	eiPhotonJob		*job;
} eiPhotonBucket;

/** \brief Shoot a photon ray to do intersection test against the scene. */
eiBool ei_rt_trace_photon(
	eiRayTracer *rt, 
	eiNodeSystem *nodesys, 
	eiState *state);

/* for internal use only */
void byteswap_job_photon(eiDatabase *db, void *job, const eiUint size);
eiBool execute_job_photon(eiDatabase *db, eiBaseWorker *pWorker, void *job, void *param);
eiUint count_job_photon(eiDatabase *db, void *job);

#ifdef __cplusplus
}
#endif

#endif
