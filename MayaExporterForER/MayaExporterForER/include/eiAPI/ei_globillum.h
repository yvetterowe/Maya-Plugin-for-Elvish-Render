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
 
#ifndef EI_GLOBILLUM_H
#define EI_GLOBILLUM_H

/** \brief The common APIs for global illumination.
 * \file ei_globillum.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_buffer.h>
#include <eiCORE/ei_array.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The thread local storage for global illumination */
typedef struct eiGlobillumTLS {
	ei_array		map_lookup_dist2;
	ei_array		map_lookup_index;
	eiBuffer		finalgather_buffer;
} eiGlobillumTLS;

/** \brief Initialize thread local storage. for internal use only. */
eiAPI void ei_globillum_tls_init(eiGlobillumTLS *pTls);

/** \brief Cleanup thread local storage. for internal use only. */
eiAPI void ei_globillum_tls_exit(eiGlobillumTLS *pTls);

/** \brief Acquire final gather buffer from thread local storage 
 * for a specific number of final gather rays. for internal use 
 * only. */
eiAPI eiBuffer *ei_globillum_tls_acquire_finalgather_buffer(
	eiGlobillumTLS *pTls, 
	const eiUint finalgather_rays);

#ifdef __cplusplus
}
#endif

#endif
