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
 
#ifndef EI_OPTIONS_H
#define EI_OPTIONS_H

/** \brief The global options of the renderer.
 * \file ei_options.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_element.h>
#include <eiAPI/ei_approx.h>
#include <eiCORE/ei_vector4.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Final gather progress modes */
enum {
	EI_FINALGATHER_PROGRESS_POINT = 0, 
	EI_FINALGATHER_PROGRESS_PAINT, 
};

/** \brief The diagnostic mode */
enum {
	EI_DIAGNOSTIC_MODE_NONE = 0, 
	/* visualize sampling rate */
	EI_DIAGNOSTIC_MODE_SAMPLING_RATE, 
	/* visualize final gather points */
	EI_DIAGNOSTIC_MODE_FINALGATHER_POINTS, 
	EI_DIAGNOSTIC_MODE_COUNT, 
};

/** \brief The class encapsulates all global options 
 * of the renderer. */
#pragma pack(push, 1)
typedef struct eiOptions {
	eiNode					node;
	eiVector4				contrast;
	eiInt					min_samples;
	eiInt					max_samples;
	eiInt					bucket_size;
	eiInt					filter;
	eiScalar				filter_size;
	eiTag					approx;
	eiScalar				max_displace;
	eiScalar				shutter_open;
	eiScalar				shutter_close;
	eiBool					motion;
	eiInt					motion_segments;
	eiInt					trace_reflect_depth;
	eiInt					trace_refract_depth;
	eiInt					trace_sum_depth;
	eiBool					shadow;
	eiInt					acceleration;
	eiInt					bsp_size;
	eiInt					bsp_depth;
	eiBool					lens;
	eiBool					volume;
	eiBool					geometry;
	eiBool					displace;
	eiBool					imager;
	eiBool					caustic;
	eiInt					caustic_photons;
	eiInt					caustic_samples;
	eiScalar				caustic_radius;
	eiVector				caustic_scale;
	eiInt					caustic_filter;
	eiScalar				caustic_filter_const;
	eiInt					photon_reflect_depth;
	eiInt					photon_refract_depth;
	eiInt					photon_sum_depth;
	eiScalar				photon_decay;
	eiBool					globillum;
	eiInt					globillum_photons;
	eiInt					globillum_samples;
	eiScalar				globillum_radius;
	eiVector				globillum_scale;
	eiInt					photonvol_samples;
	eiScalar				photonvol_radius;
	eiBool					finalgather;
	eiInt					finalgather_progress;
	eiInt					finalgather_rays;
	eiInt					finalgather_samples;
	eiScalar				finalgather_density;
	eiScalar				finalgather_radius;
	eiBool					finalgather_falloff;
	eiScalar				finalgather_falloff_start;
	eiScalar				finalgather_falloff_stop;
	eiScalar				finalgather_filter_size;
	eiInt					finalgather_reflect_depth;
	eiInt					finalgather_refract_depth;
	eiInt					finalgather_sum_depth;
	eiInt					finalgather_diffuse_bounces;
	eiVector				finalgather_scale;
	eiScalar				exposure_gain;
	eiScalar				exposure_gamma;
	eiScalar				quantize_one;
	eiScalar				quantize_min;
	eiScalar				quantize_max;
	eiScalar				quantize_dither_amplitude;
	eiInt					face;
	eiInt					diagnostic_mode;
} eiOptions;
#pragma pack(pop)

void ei_options_init(eiNodeSystem *nodesys, eiNode *node);
void ei_options_exit(eiNodeSystem *nodesys, eiNode *node);

/** \brief Instance this element into global scene database or update 
 * the existing element */
void ei_options_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer);

/** \brief Set all options to defaults. */
void ei_options_set_defaults(eiOptions *opt, eiDatabase *db);

eiNodeObject *ei_create_options_node_object(void *param);
/** \brief Install the node into node system */
void ei_install_options_node(eiNodeSystem *nodesys);

#ifdef __cplusplus
}
#endif

#endif
