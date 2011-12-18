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
 
#ifndef EI_SHADESYS_H
#define EI_SHADESYS_H

/** \brief The shading system for the shading language interface.
 * \file ei_shadesys.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_shader.h>
#include <eiAPI/ei_state.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Begin editing a shader instance. create the shader 
 * instance if it's not created. */
eiAPI eiNode *ei_shader_instance(
	eiNodeSystem *nodesys, 
	const char *inst_name);
/** \brief End editing a shader instance. */
eiAPI void ei_end_shader_instance(
	eiNodeSystem *nodesys, 
	eiNode *inst);
/** \brief Cleanup a shader instance. */
void ei_shader_instance_exit(
	eiNodeSystem *nodesys, 
	eiNode *node);

/** \brief Call a shader instance by tag. */
eiAPI eiBool ei_call_shader_instance(
	eiNodeSystem *nodesys, 
	eiVector4 * const result, 
	eiState * const state, 
	const eiTag shader, 
	void *arg);

/** \brief Execute all shader instances in sequence with the same state pointer. */
eiAPI eiBool ei_call_shader_instance_list(
	eiNodeSystem *nodesys, 
	eiVector4 * const result, 
	eiState * const state, 
	const eiTag shader_list, 
	void *arg);

/** \brief The implementation of shader parameter evaluation. */
void *ei_eval_imp(
	eiState *state, 
	eiIndex param_index, 
	eiBool shader_cache_enabled, 
	eiByte * const dXdu, 
	eiByte * const dXdv);

/** \brief Generate parameter table of shader instance, 
 * build parameter table and sort connected shader nodes. 
 * for internal use only. */
void generate_shader_inst_param_table(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls);

/* for internal use only */
void byteswap_shader_inst_param_table(eiDatabase *db, void *data, const eiUint size);

#ifdef __cplusplus
}
#endif

#endif
