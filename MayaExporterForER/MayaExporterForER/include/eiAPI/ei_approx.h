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
 
#ifndef EI_APPROX_H
#define EI_APPROX_H

/** \brief The surface approximation settings for tessellations 
 * and subdivisions.
 * \file ei_approx.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiCORE/ei_dataflow.h>

#ifdef __cplusplus
extern "C" {
#endif

/* approximation methods */
enum {
	EI_APPROX_METHOD_NONE = 0, 
	EI_APPROX_METHOD_REGULAR, 
	EI_APPROX_METHOD_LENGTH, 
	EI_APPROX_METHOD_TYPE_COUNT, 
};

#define EI_APPROX_U				0
#define EI_APPROX_V				1
#define EI_APPROX_LENGTH		0

/** \brief The surface approximation settings */
typedef struct eiApprox {
	eiShort		method;			/* approximation method */
	eiByte		any;			/* stop if any criterion is met */
	eiByte		view_dep;		/* view dependent */
	eiScalar	args[4];		/* approximation arguments depends on "method" */
	eiScalar	sharp;			/* 0.0 = smooth normals, 1.0 = faceted */
	eiUint		min_subdiv;		/* minimum subdivision level */
	eiUint		max_subdiv;		/* maximum subdivision level */
	eiUint		max_grid_size;	/* maximum number of generated triangles */
	eiScalar	motion_factor;	/* reduce the approximation quality when motion blur is on */
} eiApprox;

eiAPI void ei_approx_set_defaults(eiApprox *approx);

eiAPI void ei_approx_copy(eiApprox *dst, const eiApprox *src);

eiAPI eiBool ei_approx_is_enabled(const eiApprox *approx);
/** \brief Users must always use this function to determine 
 * whether an approximation statement is really view-dependent, 
 * it will consider the actual method being used. */
eiBool ei_approx_is_view_dep(const eiApprox *approx);

/* for internal use only */
void byteswap_approx(eiDatabase *db, void *data, const eiUint size);

#ifdef __cplusplus
}
#endif

#endif
