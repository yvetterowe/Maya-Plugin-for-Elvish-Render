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

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_raytracer.h>
#include <eiAPI/ei_rayhair.h>
#include <eiAPI/ei_buffer.h>
#include <eiAPI/ei_texture.h>
#include <eiAPI/ei_nodesys.h>
#include <eiAPI/ei_shadesys.h>
#include <eiAPI/ei_sampler.h>
#include <eiAPI/ei_output.h>
#include <eiAPI/ei_approx.h>
#include <eiAPI/ei_object.h>
#include <eiAPI/ei_light.h>
#include <eiAPI/ei_map.h>
#include <eiAPI/ei_finalgather.h>
#include <eiAPI/ei_photon.h>
#include <eiCORE/ei_data_gen.h>
#include <eiCORE/ei_assert.h>

void ei_api_init()
{
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_NODE ].byteswap = byteswap_bsp_node;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_NODE ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_NODE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_NODE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_NODE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_NODE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_NODE ].type_size = sizeof(eiBSPNode);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_TREE ].byteswap = byteswap_bsptree;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_TREE ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_TREE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_TREE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_TREE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_TREE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BSP_TREE ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBSCENE ].byteswap = byteswap_ray_subscene;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBSCENE ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBSCENE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBSCENE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBSCENE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBSCENE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBSCENE ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SCENE ].byteswap = byteswap_ray_scene;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SCENE ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SCENE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SCENE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SCENE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SCENE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SCENE ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT ].byteswap = byteswap_ray_object;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT_INST ].byteswap = byteswap_ray_object_inst;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT_INST ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT_INST ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT_INST ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT_INST ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT_INST ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_OBJECT_INST ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL ].byteswap = byteswap_ray_tessel;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL ].generate_data = generate_ray_tessel;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_ACCEL_TRIANGLES ].byteswap = byteswap_ray_accel_triangles;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_ACCEL_TRIANGLES ].generate_data = generate_ray_accel_triangles;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_ACCEL_TRIANGLES ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_ACCEL_TRIANGLES ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_ACCEL_TRIANGLES ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_ACCEL_TRIANGLES ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_ACCEL_TRIANGLES ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBTREE ].byteswap = byteswap_ray_subtree;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBTREE ].generate_data = generate_ray_subtree;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBTREE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBTREE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBTREE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBTREE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_SUBTREE ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL_INST ].byteswap = byteswap_ray_tessel_inst;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL_INST ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL_INST ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL_INST ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL_INST ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL_INST ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_TESSEL_INST ].type_size = sizeof(eiRayTesselInstance);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_HAIR_TREE ].byteswap = byteswap_ray_hair_tree;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_HAIR_TREE ].generate_data = generate_ray_hair_tree;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_HAIR_TREE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_HAIR_TREE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_HAIR_TREE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_HAIR_TREE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RAY_HAIR_TREE ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_BUFFER ].byteswap = byteswap_data_buffer;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BUFFER ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BUFFER ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BUFFER ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BUFFER ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BUFFER ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BUFFER ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER_TILE ].byteswap = byteswap_framebuffer_tile;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER_TILE ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER_TILE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER_TILE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER_TILE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER_TILE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER_TILE ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER ].byteswap = byteswap_framebuffer;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_FRAMEBUFFER ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_MAP ].byteswap = byteswap_texture_map;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_MAP ].generate_data = generate_texture_map;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_MAP ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_MAP ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_MAP ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_MAP ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_MAP ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_TILE ].byteswap = byteswap_texture_tile;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_TILE ].generate_data = generate_texture_tile;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_TILE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_TILE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_TILE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_TILE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TEXTURE_TILE ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_DESC ].byteswap = byteswap_node_desc;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_DESC ].generate_data = generate_node_desc;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_DESC ].clear_data = clear_node_desc;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_DESC ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_DESC ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_DESC ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_DESC ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM_DESC ].byteswap = byteswap_node_param_desc;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM_DESC ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM_DESC ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM_DESC ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM_DESC ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM_DESC ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM_DESC ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE ].byteswap = byteswap_node;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE ].generate_data = generate_node;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE ].clear_data = clear_node;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM ].byteswap = byteswap_node_param;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NODE_PARAM ].type_size = sizeof(eiNodeParam);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHADER_INST_PARAM_TABLE ].byteswap = byteswap_shader_inst_param_table;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHADER_INST_PARAM_TABLE ].generate_data = generate_shader_inst_param_table;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHADER_INST_PARAM_TABLE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHADER_INST_PARAM_TABLE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHADER_INST_PARAM_TABLE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHADER_INST_PARAM_TABLE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHADER_INST_PARAM_TABLE ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT_VARIABLE ].byteswap = byteswap_output_variable;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT_VARIABLE ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT_VARIABLE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT_VARIABLE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT_VARIABLE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT_VARIABLE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT_VARIABLE ].type_size = sizeof(eiOutputVariable);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT ].byteswap = byteswap_output;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_OUTPUT ].type_size = sizeof(eiOutput);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_APPROX ].byteswap = byteswap_approx;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_APPROX ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_APPROX ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_APPROX ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_APPROX ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_APPROX ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_APPROX ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TESSEL ].byteswap = byteswap_job_tessel;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TESSEL ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TESSEL ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TESSEL ].execute_job = execute_job_tessel;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TESSEL ].count_job = count_job_tessel;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TESSEL ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TESSEL ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_INST ].byteswap = byteswap_light_instance;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_INST ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_INST ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_INST ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_INST ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_INST ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_INST ].type_size = sizeof(eiLightInstance);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_MAP ].byteswap = byteswap_map;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_MAP ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_MAP ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_MAP ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_MAP ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_MAP ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_MAP ].type_size = sizeof(eiMap);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_IRRADIANCE ].byteswap = byteswap_irradiance;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_IRRADIANCE ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_IRRADIANCE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_IRRADIANCE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_IRRADIANCE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_IRRADIANCE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_IRRADIANCE ].type_size = sizeof(eiIrradiance);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_PHOTON ].byteswap = byteswap_photon;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_PHOTON ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_PHOTON ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_PHOTON ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_PHOTON ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_PHOTON ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_PHOTON ].type_size = sizeof(eiPhoton);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_FLUX ].byteswap = byteswap_light_flux;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_FLUX ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_FLUX ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_FLUX ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_FLUX ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_FLUX ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LIGHT_FLUX ].type_size = sizeof(eiLightFlux);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_PHOTON ].byteswap = byteswap_job_photon;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_PHOTON ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_PHOTON ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_PHOTON ].execute_job = execute_job_photon;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_PHOTON ].count_job = count_job_photon;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_PHOTON ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_PHOTON ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_BUCKET ].byteswap = byteswap_job_bucket;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_BUCKET ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_BUCKET ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_BUCKET ].execute_job = execute_job_bucket;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_BUCKET ].count_job = count_job_bucket;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_BUCKET ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_BUCKET ].type_size = 0;
}

void ei_api_exit()
{
}
