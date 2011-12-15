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

#include <eiAPI/ei_options.h>
#include <eiAPI/ei_raytracer.h>
#include <eiAPI/ei_sampler.h>
#include <eiAPI/ei.h>
#include <eiCORE/ei_dataflow.h>
#include <eiCORE/ei_assert.h>

#define EI_DEFAULT_BSP_SIZE				10
#define EI_DEFAULT_BSP_DEPTH			30
#define EI_DEFAULT_BUCKET_SIZE			48

void ei_options_init(eiNodeSystem *nodesys, eiNode *node)
{
	eiOptions	*opt;

	opt = (eiOptions *)node;

	opt->node.type = EI_ELEMENT_OPTIONS;
	ei_db_create(
		nodesys->m_db, 
		&opt->approx, 
		EI_DATA_TYPE_APPROX, 
		sizeof(eiApprox), 
		EI_DB_FLUSHABLE);
	ei_db_end(nodesys->m_db, opt->approx);

	ei_options_set_defaults(opt, nodesys->m_db);
}

void ei_options_exit(eiNodeSystem *nodesys, eiNode *node)
{
	eiOptions	*opt;

	opt = (eiOptions *)node;

	if (opt->approx != eiNULL_TAG)
	{
		ei_db_delete(nodesys->m_db, opt->approx);
		opt->approx = eiNULL_TAG;
	}
}

void ei_options_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer)
{
}

void ei_options_set_defaults(eiOptions *opt, eiDatabase *db)
{
	eiApprox	*opt_approx;

	opt->acceleration = EI_ACCEL_BSP;
	opt->motion = eiTRUE;
	opt->motion_segments = 5;
	opt->trace_reflect_depth = 6;
	opt->trace_refract_depth = 6;
	opt->trace_sum_depth = 6;
	opt->exposure_gain = 1.0f;
	opt->exposure_gamma = 1.0f;
	opt->filter = EI_FILTER_BOX;
	opt->filter_size = 1.0f;
	opt->face = EI_FACE_FRONT;
	opt_approx = (eiApprox *)ei_db_access(db, opt->approx);
	ei_approx_set_defaults(opt_approx);
	ei_db_end(db, opt->approx);
	opt->max_displace = 0.0f;
	opt->shutter_open = 0.0f;
	opt->shutter_close = 1.0f;
	opt->min_samples = 0;
	opt->max_samples = 2;
	opt->bsp_size = EI_DEFAULT_BSP_SIZE;
	opt->bsp_depth = EI_DEFAULT_BSP_DEPTH;
	opt->bucket_size = EI_DEFAULT_BUCKET_SIZE;
	opt->quantize_one = 255.0f;
	opt->quantize_min = 0.0f;
	opt->quantize_max = 255.0f;
	opt->quantize_dither_amplitude = 0.5f;
	opt->lens = eiTRUE;
	opt->volume = eiTRUE;
	opt->geometry = eiTRUE;
	opt->displace = eiTRUE;
	opt->imager = eiTRUE;
	opt->caustic = eiFALSE;
	opt->caustic_photons = 100000;
	opt->caustic_samples = 100;
	opt->caustic_radius = 0.0f;
	setvf(&opt->caustic_scale, 1.0f);
	opt->caustic_filter = EI_CAUSTIC_FILTER_CONE;
	opt->caustic_filter_const = 1.1f;
	opt->photon_reflect_depth = 5;
	opt->photon_refract_depth = 5;
	opt->photon_sum_depth = 5;
	opt->photon_decay = 2.0f;
	opt->globillum = eiFALSE;
	opt->globillum_photons = 10000;
	opt->globillum_samples = 100;
	opt->globillum_radius = 0.0f;
	setvf(&opt->globillum_scale, 1.0f);
	opt->finalgather = eiFALSE;
	opt->finalgather_progress = EI_FINALGATHER_PROGRESS_PAINT;
	opt->finalgather_rays = 500;
	opt->finalgather_samples = 30;
	opt->finalgather_density = 1.0f;
	opt->finalgather_radius = 0.0f;
	opt->finalgather_falloff = eiFALSE;
	opt->finalgather_falloff_start = 0.0f;
	opt->finalgather_falloff_stop = 0.0f;
	opt->finalgather_filter_size = 4.0f;
	opt->diagnostic_mode = EI_DIAGNOSTIC_MODE_NONE;
}

eiNodeObject *ei_create_options_node_object(void *param)
{
	eiElement	*element;

	element = ei_create_element();

	element->base.base.deletethis = ei_element_deletethis;
	element->base.init_node = ei_options_init;
	element->base.exit_node = ei_options_exit;
	element->base.node_changed = NULL;
	element->update_instance = ei_options_update_instance;

	return ((eiNodeObject *)element);
}

void ei_install_options_node(eiNodeSystem *nodesys)
{
	eiTag		desc_tag;
	eiNodeDesc	*desc;
	eiTag		default_tag;
	eiInt		default_int;
	eiBool		default_bool;
	eiScalar	default_scalar;
	eiVector	default_vec;
	eiVector4	default_vec4;

	desc = ei_nodesys_node_desc(nodesys, &desc_tag, "options");
	if (desc == NULL)
	{
		return;
	}

	default_tag = eiNULL_TAG;
	default_int = 0;
	default_bool = eiFALSE;
	default_scalar = 0.0f;
	initv(&default_vec);
	initv4(&default_vec4);

	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_VECTOR4, 
		"contrast", 
		&default_vec4);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"min_samples", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"max_samples", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"bucket_size", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"filter", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"filter_size", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"approx", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"max_displace", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"shutter_open", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"shutter_close", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOOL, 
		"motion", 
		&default_bool);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"motion_segments", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"trace_reflect_depth", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"trace_refract_depth", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"trace_sum_depth", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOOL, 
		"shadow", 
		&default_bool);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"acceleration", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"bsp_size", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"bsp_depth", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOOL, 
		"lens", 
		&default_bool);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOOL, 
		"volume", 
		&default_bool);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOOL, 
		"geometry", 
		&default_bool);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOOL, 
		"displace", 
		&default_bool);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOOL, 
		"imager", 
		&default_bool);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOOL, 
		"caustic", 
		&default_bool);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"caustic_photons", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"caustic_samples", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"caustic_radius", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_VECTOR, 
		"caustic_scale", 
		&default_vec);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"caustic_filter", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"caustic_filter_const", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"photon_reflect_depth", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"photon_refract_depth", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"photon_sum_depth", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"photon_decay", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOOL, 
		"globillum", 
		&default_bool);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"globillum_photons", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"globillum_samples", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"globillum_radius", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_VECTOR, 
		"globillum_scale", 
		&default_vec);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"photonvol_samples", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"photonvol_radius", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOOL, 
		"finalgather", 
		&default_bool);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"finalgather_progress", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"finalgather_rays", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"finalgather_samples", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"finalgather_density", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"finalgather_radius", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOOL, 
		"finalgather_falloff", 
		&default_bool);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"finalgather_falloff_start", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"finalgather_falloff_stop", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"finalgather_filter_size", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"finalgather_reflect_depth", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"finalgather_refract_depth", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"finalgather_sum_depth", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"finalgather_diffuse_bounces", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_VECTOR, 
		"finalgather_scale", 
		&default_vec);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"exposure_gain", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"exposure_gamma", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"quantize_one", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"quantize_min", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"quantize_max", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"quantize_dither_amplitude", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"face", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"diagnostic_mode", 
		&default_int);

	ei_nodesys_end_node_desc(nodesys, desc, desc_tag);
}
