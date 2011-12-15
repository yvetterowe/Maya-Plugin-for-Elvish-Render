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

#include <eiAPI/ei_light.h>
#include <eiAPI/ei_element.h>
#include <eiAPI/ei_attributes.h>
#include <eiAPI/ei_shadesys.h>
#include <eiAPI/ei_raytracer.h>
#include <eiAPI/ei_photon.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_data_table.h>
#include <eiCORE/ei_assert.h>

void ei_light_init(eiNodeSystem *nodesys, eiNode *node)
{
	eiLight		*lgt;

	lgt = (eiLight *)node;

	lgt->node.type = EI_ELEMENT_LIGHT;

	lgt->light_list = ei_create_data_array(nodesys->m_db, EI_DATA_TYPE_TAG);
	lgt->emitter_list = ei_create_data_array(nodesys->m_db, EI_DATA_TYPE_TAG);

	initv(&lgt->origin);
	setvf(&lgt->energy, 1.0f);
	lgt->u_samples = 1;
	lgt->v_samples = 1;
	lgt->low_level = eiMAX_INT;
	lgt->low_u_samples = 1;
	lgt->low_v_samples = 1;
}

void ei_light_exit(eiNodeSystem *nodesys, eiNode *node)
{
	eiLight		*lgt;

	lgt = (eiLight *)node;

	ei_delete_data_array(nodesys->m_db, lgt->emitter_list);
	ei_delete_data_array(nodesys->m_db, lgt->light_list);
}

void ei_light_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer)
{
	eiLight				*light;
	eiLightInstance		inst;
	eiDataTable			*light_insts;

	/* don't create renderable representation for this leaf if it's invisible */
	if (!ei_attr_get_flag(attr, EI_ATTR_VISIBLE))
	{
		return;
	}

	light = (eiLight *)node;

	ei_light_instance_init(&inst, node->tag, transform, motion_transform, &light->origin);

	light_insts = (eiDataTable *)ei_db_access(nodesys->m_db, cache->light_instances);
	ei_data_table_push_back(nodesys->m_db, &light_insts, &inst);
	ei_db_end(nodesys->m_db, cache->light_instances);
}

void ei_light_add_light(eiLight *lgt, eiDatabase *db, const eiTag shader)
{
	ei_data_array_push_back(db, lgt->light_list, &shader);
}

void ei_light_add_emitter(eiLight *lgt, eiDatabase *db, const eiTag shader)
{
	ei_data_array_push_back(db, lgt->emitter_list, &shader);
}

void ei_light_instance_init(
	eiLightInstance *inst, 
	const eiTag ref_light, 
	const eiMatrix *light_to_world, 
	const eiMatrix *motion_light_to_world, 
	const eiVector *origin)
{
	inst->light = ref_light;
	movm(&inst->light_to_world, light_to_world);
	movm(&inst->motion_light_to_world, motion_light_to_world);
	inv(&inst->world_to_light, &inst->light_to_world);
	/* cache the origin in world space, we will transform it to 
	   camera space later */
	point_transform(&inst->origin, origin, &inst->light_to_world);
}

void ei_light_instance_exit(eiLightInstance *inst)
{
}

void ei_light_instance_transform(
	eiLightInstance *inst, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform)
{
	eiVector	origin;

	movv(&origin, &inst->origin);
	point_transform(&inst->origin, &origin, transform);
}

eiScalar ei_light_instance_get_max_flux(
	eiLightInstance *inst, 
	eiDatabase *db)
{
	eiLight		*light;
	eiScalar	max_flux;

	light = (eiLight *)ei_db_access(db, inst->light);
	max_flux = average(&light->energy);
	ei_db_end(db, inst->light);

	return max_flux;
}

static eiBool generate_photon_ray(
	eiVector *org, 
	eiVector *dir, 
	eiDatabase *db, 
	eiLight *light, 
	const eiInt photon_type, 
	eiInt *halton_num, 
	eiUint *dimension, 
	eiBaseBucket *bucket)
{
	eiNodeSystem	*nodesys;
	eiState			state;
	eiVector4		result;

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	if (light->emitter_list == eiNULL_TAG || 
		ei_data_array_empty(db, light->emitter_list))
	{
		return eiFALSE;
	}

	ei_state_init(&state, photon_type, bucket);

	++ (*halton_num);
	state.instance_number = *halton_num;
	state.dimension = 0;

	/* for photon emitter, we use the light origin in light space */
	movv(&state.P, &light->origin);

	initv4(&result);

	ei_call_shader_instance_list(
		nodesys, 
		&result, 
		&state, 
		light->emitter_list, 
		NULL);

	(*dimension) = state.dimension;
	movv(org, &state.E);
	movv(dir, &state.I);

	ei_state_exit(&state);

	return eiTRUE;
}

void ei_light_instance_shoot_photon(
	eiLightInstance *light_inst, 
	const eiInt photon_type, 
	eiInt *halton_num, 
	eiBaseBucket *bucket)
{
	eiDatabase		*db;
	eiOptions		*opt;
	eiCamera		*cam;
	eiRayTracer		*rt;
	eiNodeSystem	*nodesys;
	eiLight			*light;
	eiVector		ray_src, ray_dir;
	eiVector		temp_src, temp_dir;
	eiUint			dimension = 2;
	eiSampleInfo	sample_info;
	eiState			photon_ray;

	db = bucket->db;
	opt = bucket->opt;
	cam = bucket->cam;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_RAYTRACER);
	eiDBG_ASSERT(rt != NULL);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	if (light_inst->light == eiNULL_TAG)
	{
		return;
	}

	light = (eiLight *)ei_db_access(db, light_inst->light);

	if (!generate_photon_ray(
		&ray_src, 
		&ray_dir, 
		db, 
		light, 
		photon_type, 
		halton_num, 
		&dimension, 
		bucket))
	{
		ei_db_end(db, light_inst->light);
		return;
	}

	/* transform from light space to camera space */
	point_transform(&temp_src, &ray_src, &light_inst->light_to_world);
	point_transform(&ray_src, &temp_src, &cam->world_to_camera);
	vector_transform(&temp_dir, &ray_dir, &light_inst->light_to_world);
	vector_transform(&ray_dir, &temp_dir, &cam->world_to_camera);
	normalizei(&ray_dir);

	/* a fixed size sample info is considered sufficient for 
	   photon emission without arbitrary output variable support */
	ei_sample_info_init(&sample_info, 0);

	ei_state_init(&photon_ray, photon_type, bucket);

	/* dimension 0, 1 have been used for selecting a direction 
	   to shoot this photon */
	photon_ray.dimension = dimension;
	photon_ray.instance_number = (*halton_num);
	
	movv(&photon_ray.org, &ray_src);
	movv(&photon_ray.dir, &ray_dir);

	photon_ray.time = 0.0f;
	photon_ray.dtime = opt->shutter_close - opt->shutter_open;
	photon_ray.result = &sample_info;
	photon_ray.result->color = light->energy;
	mulvfi(&photon_ray.result->color, 1.0f / ei_light_instance_get_max_flux(light_inst, db));

	ei_rt_trace_photon(
		rt, 
		nodesys, 
		&photon_ray);

	ei_state_exit(&photon_ray);

	ei_sample_info_exit(&sample_info);

	ei_db_end(db, light_inst->light);
}

void byteswap_light_instance(eiDatabase *db, void *data, const eiUint size)
{
	eiLightInstance *inst = (eiLightInstance *)data;

	ei_byteswap_int(&inst->light);
	ei_byteswap_matrix(&inst->light_to_world);
	ei_byteswap_matrix(&inst->motion_light_to_world);
	ei_byteswap_matrix(&inst->world_to_light);
	ei_byteswap_vector(&inst->origin);
}

eiNodeObject *ei_create_light_node_object(void *param)
{
	eiElement	*element;

	element = ei_create_element();

	element->base.base.deletethis = ei_element_deletethis;
	element->base.init_node = ei_light_init;
	element->base.exit_node = ei_light_exit;
	element->base.node_changed = NULL;
	element->update_instance = ei_light_update_instance;

	return ((eiNodeObject *)element);
}

void ei_install_light_node(eiNodeSystem *nodesys)
{
	eiTag		desc_tag;
	eiNodeDesc	*desc;
	eiTag		default_tag;
	eiInt		default_int;
	eiVector	default_vec;

	desc = ei_nodesys_node_desc(nodesys, &desc_tag, "light");
	if (desc == NULL)
	{
		return;
	}

	default_tag = eiNULL_TAG;
	default_int = 0;
	initv(&default_vec);

	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"light_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"emitter_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_VECTOR, 
		"origin", 
		&default_vec);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_VECTOR, 
		"energy", 
		&default_vec);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"u_samples", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"v_samples", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"low_level", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"low_u_samples", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"low_v_samples", 
		&default_int);

	ei_nodesys_end_node_desc(nodesys, desc, desc_tag);
}
