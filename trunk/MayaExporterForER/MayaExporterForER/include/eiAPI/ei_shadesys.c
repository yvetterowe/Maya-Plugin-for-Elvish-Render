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

#include <eiAPI/ei_shadesys.h>
#include <eiAPI/ei_buffer.h>
#include <eiAPI/ei_object.h>
#include <eiAPI/ei_sampler.h>
#include <eiCORE/ei_algorithm.h>
#include <eiCORE/ei_btree.h>
#include <eiCORE/ei_array.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_assert.h>

/** \brief The internal parameter indices of shader instance. */
enum {
	eiShaderInstance_param_table = 0, 
	eiShaderInstance_internal_parameter_count, 
};

/** \brief The connected shader instance node. */
typedef struct eiShaderNode {
	/* the tag of the corresponding shader instance */
	eiTag				tag;
	/* the offset of this shader instance in thread local 
	   shader cache in bytes, including the sizeof(eiShaderCache) */
	eiIndex				shader_cache_offset;
} eiShaderNode;

/** \brief The unique shader node in shader tree. */
typedef struct eiShaderTreeNode {
	ei_btree_node		node;
	/* the tag of shader instance */
	eiTag				tag;
} eiShaderTreeNode;

/** \brief The parameter table of shader instance. */
struct eiShaderInstParamTable {
	/* the tag of the corresponding shader instance */
	eiTag				inst;
	/* the total size of shader cache in bytes, NOT including 
	   the sizeof(eiShaderCache) */
	eiUint				shader_cache_size;
	/* the number of connected nodes in this sub-shader-graph. 
	   the table of connected nodes will be sorted by tag. */
	eiInt				num_sorted_nodes;
};

/** \brief The thread local storage of a shader parameter 
 * only for current shader node, NOT including connected 
 * shader nodes in shade graph. */
typedef struct eiShaderLocalParamTLS {
	/* has the shader parameter been cached for current 
	   shader node, if not, re-evaluation is needed. */
	eiBool				cached;
} eiShaderLocalParamTLS;

/** \brief The thread local storage of a shader parameter. */
typedef struct eiShaderParamTLS {
	/* has the shader parameter been cached for current 
	   shader graph, if not, re-evaluation is needed. */
	eiBool				cached;
} eiShaderParamTLS;

/** \brief The thread local storage of a shader instance. */
typedef struct eiShaderInstanceTLS {
	/* has the shader instance been called for current shader 
	   graph, if not, re-evaluation is needed. */
	eiBool				called;
	/* the cached returned value */
	eiBool				ret_val;
	/* the cached result */
	eiVector4			result;
} eiShaderInstanceTLS;

static eiInt ei_shader_node_compare(void *lhs, void *rhs)
{
	/* must cast to signed types to minus correctly */
	return ((eiInt)(((eiShaderNode *)lhs)->tag) - (eiInt)(((eiShaderNode *)rhs)->tag));
}

static void ei_shader_tree_node_init(eiShaderTreeNode *node, const eiTag tag)
{
	ei_btree_node_init(&node->node);
	node->tag = tag;
}

static void ei_shader_tree_node_exit(eiShaderTreeNode *node)
{
	ei_btree_node_clear(&node->node);
}

static eiIntptr ei_shader_tree_node_compare(void *lhs, void *rhs, void *param)
{
	/* must cast to signed types to minus correctly */
	return ((eiIntptr)(((eiShaderTreeNode *)lhs)->tag) - (eiIntptr)(((eiShaderTreeNode *)rhs)->tag));
}

static eiShaderTreeNode *ei_create_shader_tree_node(const eiTag tag)
{
	eiShaderTreeNode	*node;

	node = (eiShaderTreeNode *)ei_allocate(sizeof(eiShaderTreeNode));
	ei_shader_tree_node_init(node, tag);

	return node;
}

static void ei_shader_tree_node_delete(ei_btree_node *node, void *param)
{
	if (node == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_shader_tree_node_exit((eiShaderTreeNode *)node);

	eiCHECK_FREE(node);
}

static eiTag ei_create_shader_inst_param_table(
	eiDatabase *db, 
	const eiTag inst_tag)
{
	eiShaderInstParamTable	*tab;
	eiTag					param_table;

	param_table = eiNULL_TAG;

	tab = (eiShaderInstParamTable *)ei_db_create(
		db, 
		&param_table, 
		EI_DATA_TYPE_SHADER_INST_PARAM_TABLE, 
		sizeof(eiShaderInstParamTable), 
		EI_DB_FLUSHABLE);

	tab->inst = inst_tag;
	tab->shader_cache_size = 0;
	tab->num_sorted_nodes = 0;

	ei_db_end(db, param_table);

	return param_table;
}

void ei_shader_instance_exit(
	eiNodeSystem *nodesys, 
	eiNode *node)
{
	eiTag		param_table;

	ei_nodesys_get_parameter_at(
		nodesys, 
		node, 
		eiShaderInstance_param_table, 
		&param_table);
	eiASSERT(param_table != eiNULL_TAG);

	/* cleanup derived class */
	ei_db_delete(nodesys->m_db, param_table);
}

eiNode *ei_shader_instance(
	eiNodeSystem *nodesys, 
	const char *inst_name)
{
	eiNode			*inst;
	eiBool			need_init;
	eiTag			default_tag;

	/* create base class */
	inst = ei_nodesys_node(nodesys, inst_name, &need_init);

	if (need_init)
	{
		/* declare node parameters */
		default_tag = eiNULL_TAG;
		ei_nodesys_declare_parameter(nodesys, 
			&((eiNode *)inst), eiCONSTANT, 
			EI_DATA_TYPE_TAG, "param_table", &default_tag);

		/* initialize derived class */
		ei_nodesys_set_tag(
			nodesys, 
			&inst, 
			"param_table", 
			ei_create_shader_inst_param_table(nodesys->m_db, inst->tag));
	}

	return inst;
}

void ei_end_shader_instance(
	eiNodeSystem *nodesys, 
	eiNode *inst)
{
	ei_nodesys_end_node(nodesys, inst);
}

static eiFORCEINLINE eiBool ei_call_shader_imp(
	const eiTag shader, 
	eiShader *pShader, 
	eiNode *pShaderInst, 
	eiShaderInstanceTLS *pShaderInstTls, 
	eiShaderCache *shader_cache, 
	eiVector4 * const result, 
	eiState * const state, 
	eiByte **params)
{
	eiByte		*prev_params;
	eiByte		*prev_shader_cache;
	eiTag		prev_shader;
	eiBool		ret_val;

	/* if the shader has been called, just return the 
	   cached returned value and result. */
	if (shader_cache->enabled && pShaderInstTls->called)
	{
		*result = pShaderInstTls->result;
		*params = ((eiByte *)(pShaderInstTls + 1)) + pShaderInst->param_table_size;
		return pShaderInstTls->ret_val;
	}

	/* push the temporary shader parameters */
	prev_params = shader_cache->params;
	shader_cache->params = *params;

	/* push the current shader cache */
	prev_shader_cache = state->shader_cache;
	state->shader_cache = (eiByte *)shader_cache;

	/* push the current calling shader instance */
	prev_shader = state->shader;
	state->shader = shader;

	/* call the main function of the shader */
	ret_val = pShader->main(pShader, result, state, shader_cache->arg);

	/* pop the current calling shader instance */
	state->shader = prev_shader;

	/* pop the current shader cache */
	state->shader_cache = prev_shader_cache;

	/* pop the temporary shader parameters */
	shader_cache->params = prev_params;

	if (shader_cache->enabled)
	{
		/* set this shader instance being called */
		pShaderInstTls->called = eiTRUE;
		/* cache the returned value and result */
		pShaderInstTls->ret_val = ret_val;
		pShaderInstTls->result = *result;
	}

	return ret_val;
}

eiBool ei_call_shader_instance(
	eiNodeSystem *nodesys, 
	eiVector4 * const result, 
	eiState * const state, 
	const eiTag shader, 
	void *arg)
{
	eiNode					*pShaderInst;
	eiTag					pShaderInstParamTableTag;
	eiShaderInstParamTable	*pShaderInstParamTable;
	/* the shader cache is the thread local cache 
	   of all connected shader instances and their 
	   parameters */
	eiShaderCache			*shader_cache;
	eiShaderNode			*sorted_nodes;
	eiShaderNode			shader_node_key;
	eiShaderNode			*pShaderNode;
	eiShaderInstanceTLS		*pShaderInstTls;
	eiUint					params_size;
	eiByte					*params;
	eiBool					ret_val;

	if (shader == eiNULL_TAG)
	{
		return eiFALSE;
	}

	pShaderInst = (eiNode *)ei_db_access(nodesys->m_db, shader);

	ei_nodesys_get_parameter_at(
		nodesys, 
		pShaderInst, 
		eiShaderInstance_param_table, 
		&pShaderInstParamTableTag);

	pShaderInstParamTable = (eiShaderInstParamTable *)ei_db_access(
		nodesys->m_db, pShaderInstParamTableTag);

	/* allocate shader cache from stack memory */
	shader_cache = (eiShaderCache *)_alloca(sizeof(eiShaderCache) + pShaderInstParamTable->shader_cache_size);
	shader_cache->root = pShaderInst;
	shader_cache->root_param_table = pShaderInstParamTable;
	shader_cache->size = pShaderInstParamTable->shader_cache_size;
	/* enable shader cache by default for each shader call */
	shader_cache->enabled = eiTRUE;
	shader_cache->arg = arg;

	/* flush the shader cache */
	if (shader_cache->size != 0)
	{
		memset(shader_cache + 1, 0, shader_cache->size);
	}

	/* index thread local shader parameters by shader instance */
	sorted_nodes = (eiShaderNode *)(pShaderInstParamTable + 1);
	shader_node_key.tag = shader;
	
	pShaderNode = sorted_nodes + ei_binsearch(
		sorted_nodes, 
		pShaderInstParamTable->num_sorted_nodes, 
		&shader_node_key, 
		sizeof(eiShaderNode), 
		ei_shader_node_compare);
	
	pShaderInstTls = (eiShaderInstanceTLS *)(((eiByte *)shader_cache) + pShaderNode->shader_cache_offset);

	/* allocate un-cached temporary shader parameters from stack memory */
	params_size = pShaderInst->param_table_size * 2 + ((eiShader *)pShaderInst->object)->size;
	params = NULL;
	if (params_size != 0)
	{
		params = (eiByte *)_alloca(params_size);
		if (pShaderInst->param_table_size != 0)
		{
			memset(params, 0, pShaderInst->param_table_size * 2);
		}
		/* copy the shader class object into thread local storage */
		memcpy(params + pShaderInst->param_table_size * 2, pShaderInst->object, ((eiShader *)pShaderInst->object)->size);
	}

	ret_val = ei_call_shader_imp(
		shader, 
		(eiShader *)(params + pShaderInst->param_table_size * 2), 
		pShaderInst, 
		pShaderInstTls, 
		shader_cache, 
		result, 
		state, 
		&params);

	/* bind all output variables with shader output parameters that match the names */
	if (state->result != NULL && 
		state->bucket != NULL && 
		state->bucket->type == EI_BUCKET_FRAME)
	{
		eiBucket	*bucket;
		eiIntptr	numFrameBuffers;
		eiIntptr	i;

		bucket = (eiBucket *)state->bucket;
		numFrameBuffers = ei_array_size(&bucket->frameBufferCaches);

		for (i = 0; i < numFrameBuffers; ++i)
		{
			eiIndex				output_shader_param_index;
			eiFrameBufferCache	*fb_cache;

			fb_cache = (eiFrameBufferCache *)ei_array_get(&bucket->frameBufferCaches, i);

			/* find the output variable name in shader parameters */
			output_shader_param_index = ei_nodesys_lookup_parameter(
				nodesys, 
				(eiNode *)pShaderInst, 
				ei_framebuffer_cache_get_name(fb_cache));

			if (output_shader_param_index != eiNULL_INDEX)
			{
				eiNodeParam		*output_shader_param;

				/* get the output shader parameter */
				output_shader_param = ei_nodesys_read_parameter(nodesys, 
					(eiNode *)pShaderInst, output_shader_param_index);

				/* copy the output parameter value to the sample info */
				ei_db_cast(
					state->db, 
					((eiByte *)state->result) + ei_framebuffer_cache_get_data_offset(fb_cache), 
					ei_framebuffer_cache_get_type(fb_cache), 
					params + output_shader_param->offset, 
					output_shader_param->type);
			}
		}
	}

	ei_db_end(nodesys->m_db, pShaderInstParamTableTag);
	ei_db_end(nodesys->m_db, shader);

	return ret_val;
}

eiBool ei_call_shader_instance_list(
	eiNodeSystem *nodesys, 
	eiVector4 * const result, 
	eiState * const state, 
	const eiTag shader_list, 
	void *arg)
{
	eiBool			status;
	eiDataArray		*shader_instances;
	eiInt			i;

	if (shader_list == eiNULL_TAG)
	{
		return eiFALSE;
	}

	status = eiFALSE;
	shader_instances = (eiDataArray *)ei_db_access(nodesys->m_db, shader_list);

	for (i = 0; i < shader_instances->size; ++i)
	{
		eiTag	shader;

		shader = *((eiTag *)ei_data_array_get(nodesys->m_db, shader_instances, i));

		if (shader != eiNULL_TAG)
		{
			/* all shader instances in the list share the same result pointer */
			status |= ei_call_shader_instance(nodesys, result, state, shader, arg);
		}
	}

	ei_db_end(nodesys->m_db, shader_list);

	return status;
}

void *ei_eval_imp(eiState *state, eiIndex param_index, const eiBool shader_cache_enabled)
{
	eiNodeSystem			*nodesys;
	eiNode					*pRootShaderInst;
	eiShaderInstParamTable	*pRootShaderInstParamTable;
	eiShaderNode			*root_sorted_nodes;
	eiNode					*pParentShaderInst;
	eiTag					pParentShaderInstParamTableTag;
	eiShaderInstParamTable	*pParentShaderInstParamTable;
	eiShaderNode			parent_shader_node_key;
	eiShaderNode			*pParentShaderNode;
	eiShaderInstanceTLS		*pParentShaderInstTls;
	eiByte					*parentParams;
	eiNodeParam				*shader_param;
	eiSizet					param_offset;
	eiByte					*param;
	eiShaderParamTLS		*paramTls;
	eiShaderLocalParamTLS	*localParamTls;

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		state->db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	/* get the root shader instance of the current shader graph */
	pRootShaderInst = ((eiShaderCache *)state->shader_cache)->root;
	pRootShaderInstParamTable = ((eiShaderCache *)state->shader_cache)->root_param_table;
	/* get sorted nodes from the root */
	root_sorted_nodes = (eiShaderNode *)(pRootShaderInstParamTable + 1);

	/* get the parent shader instance that issues this evaluation */
	pParentShaderInst = (eiNode *)ei_db_access(state->db, state->shader);

	ei_nodesys_get_parameter_at(
		nodesys, 
		pParentShaderInst, 
		eiShaderInstance_param_table, 
		&pParentShaderInstParamTableTag);

	pParentShaderInstParamTable = (eiShaderInstParamTable *)ei_db_access(
		state->db, pParentShaderInstParamTableTag);

	/* index thread local shader parameters by shader instance */
	parent_shader_node_key.tag = state->shader;

	pParentShaderNode = root_sorted_nodes + ei_binsearch(
		root_sorted_nodes, 
		pRootShaderInstParamTable->num_sorted_nodes, 
		&parent_shader_node_key, 
		sizeof(eiShaderNode), 
		ei_shader_node_compare);
	
	pParentShaderInstTls = (eiShaderInstanceTLS *)(((eiByte *)state->shader_cache) 
		+ pParentShaderNode->shader_cache_offset);

	parentParams = ((eiByte *)(pParentShaderInstTls + 1)) + pParentShaderInst->param_table_size;

	/* must offset the parameter index by the number of internal node 
	   parameters of shader instance which users cannot see */
	param_index += eiShaderInstance_internal_parameter_count;
	shader_param = ei_nodesys_read_parameter(nodesys, pParentShaderInst, param_index);
	param_offset = shader_param->offset;

	param = ((eiShaderCache *)state->shader_cache)->params + param_offset;

	/* if the shader cache is enabled, and the cached value is valid, 
	   just copy the cached value to evaluating parameter */
	paramTls = (eiShaderParamTLS *)(((eiByte *)(pParentShaderInstTls + 1)) + param_offset);

	/* get the local parameter cache for the current executing shader */
	localParamTls = (eiShaderLocalParamTLS *)(((eiByte *)param) + pParentShaderInst->param_table_size);

	if (shader_cache_enabled && paramTls->cached)
	{
		/* the parameter has been cached (evaluated for the first 
		   time for local scope), users may have made changes to 
		   the parameter, don't read it from shader cache, just return */
		if (localParamTls->cached)
		{
			ei_db_end(state->db, pParentShaderInstParamTableTag);
			ei_db_end(state->db, state->shader);

			return param;
		}
		
		memcpy(param, parentParams + param_offset, shader_param->size);

		ei_db_end(state->db, pParentShaderInstParamTableTag);
		ei_db_end(state->db, state->shader);

		return param;
	}

	/* call the connected shader if available */
	if (shader_param->inst != eiNULL_TAG)
	{
		eiNode					*pShaderInst;
		eiVector4				result;
		eiShaderNode			shader_node_key;
		eiShaderNode			*pShaderNode;
		eiShaderInstanceTLS		*pShaderInstTls;
		eiUint					params_size;
		eiByte					*params;
		eiShaderCache			*shader_cache;
		eiBool					prev_shader_cache_enabled;
		eiNodeParam				*output_shader_param;

		/* the connected shader instance */
		pShaderInst = (eiNode *)ei_db_access(state->db, shader_param->inst);

		/* index thread local shader parameters by shader instance */
		shader_node_key.tag = shader_param->inst;
		
		pShaderNode = root_sorted_nodes + ei_binsearch(
			root_sorted_nodes, 
			pRootShaderInstParamTable->num_sorted_nodes, 
			&shader_node_key, 
			sizeof(eiShaderNode), 
			ei_shader_node_compare);
		
		pShaderInstTls = (eiShaderInstanceTLS *)(((eiByte *)state->shader_cache) 
			+ pShaderNode->shader_cache_offset);

		/* push shader cache enabled state */
		shader_cache = (eiShaderCache *)state->shader_cache;
		prev_shader_cache_enabled = shader_cache->enabled;
		shader_cache->enabled = shader_cache_enabled;

		/* allocate un-cached temporary shader parameters from stack memory */
		params_size = pShaderInst->param_table_size * 2 + ((eiShader *)pShaderInst->object)->size;
		params = NULL;
		if (params_size != 0)
		{
			params = (eiByte *)_alloca(params_size);
			if (pShaderInst->param_table_size != 0)
			{
				memset(params, 0, pShaderInst->param_table_size * 2);
			}
			/* copy the shader class object into thread local storage */
			memcpy(params + pShaderInst->param_table_size * 2, pShaderInst->object, ((eiShader *)pShaderInst->object)->size);
		}

		ei_call_shader_imp(
			shader_param->inst, 
			(eiShader *)(params + pShaderInst->param_table_size * 2), 
			pShaderInst, 
			pShaderInstTls, 
			shader_cache, 
			&result, 
			state, 
			&params);

		/* pop shader cache enabled state */
		shader_cache->enabled = prev_shader_cache_enabled;

		if (shader_param->param != eiNULL_INDEX)
		{
			/* get the output shader parameter */
			output_shader_param = ei_nodesys_read_parameter(nodesys, 
				(eiNode *)pShaderInst, shader_param->param);

			/* copy the output parameter value to the evaluating parameter */
			ei_db_cast(state->db, 
				(eiByte *)param, shader_param->type, 
				params + output_shader_param->offset, output_shader_param->type);
		}
		else
		{
			/* assume what are asking for the standard result 
			   if parameter tag is eiNULL_INDEX */
			ei_db_cast(state->db, 
				(eiByte *)param, shader_param->type, 
				(eiByte *)(&result), EI_DATA_TYPE_VECTOR4);
		}

		ei_db_end(state->db, shader_param->inst);
	}
	else
	{
		/* bind primitive variable to this shader parameter if their names match */
		if (!ei_get_prim_var(
			state, 
			shader_param->name, 
			shader_param->type, 
			param, 
			NULL, 
			NULL))
		{
			/* no primitive variable available, copy the constant parameter value 
			   to the evaluating parameter */
			ei_nodesys_get_parameter_value(nodesys, pParentShaderInst, shader_param, param);
		}
	}

	/* copy the evaluating parameter to parameter cache 
	   if shader cache is enabled */
	if (shader_cache_enabled)
	{
		memcpy(parentParams + param_offset, param, shader_param->size);

		/* flag the parameter cached */
		paramTls->cached = eiTRUE;
		/* the parameter has been evaluated for local scope 
		   (the current executing shader) */
		localParamTls->cached = eiTRUE;
	}

	ei_db_end(state->db, pParentShaderInstParamTableTag);
	ei_db_end(state->db, state->shader);

	return param;
}

/* push the node to the array and make sure it's unique */
static void ei_add_shader_node_unique(
	ei_btree *shader_nodes, 
	const eiTag shader_inst_tag)
{
	eiShaderTreeNode	key;
	eiShaderTreeNode	*shader_node;

	key.tag = shader_inst_tag;

	if (ei_btree_lookup(shader_nodes, &key.node, NULL) == NULL)
	{
		shader_node = ei_create_shader_tree_node(shader_inst_tag);
		ei_btree_insert(shader_nodes, &shader_node->node, NULL);
	}
}

static void ei_collect_shader_nodes(
	const eiTag shader_inst_tag, 
	eiNode *inst, 
	ei_btree *shader_nodes, 
	eiNodeSystem *nodesys)
{
	eiInt				num_params;
	eiInt				i, j;
	
	/* traverse all dependencies/inputs */
	num_params = ei_nodesys_get_parameter_count(nodesys, inst);
	
	for (i = 0; i < num_params; ++i)
	{
		eiNodeParam				*param;
		eiTag					sub_inst_tag;
		eiNode					*sub_inst;
		eiTag					sub_inst_param_table_tag;
		eiShaderInstParamTable	*sub_inst_param_table;
		eiShaderNode			*sorted_nodes;

		param = ei_nodesys_read_parameter(nodesys, inst, i);
		sub_inst_tag = param->inst;
		
		if (sub_inst_tag != eiNULL_TAG)
		{
			/* the sub-nodes will be automatically collected by data generator */
			sub_inst = (eiNode *)ei_db_access(nodesys->m_db, sub_inst_tag);

			ei_nodesys_get_parameter_at(
				nodesys, 
				sub_inst, 
				eiShaderInstance_param_table, 
				&sub_inst_param_table_tag);

			sub_inst_param_table = (eiShaderInstParamTable *)ei_db_access(
				nodesys->m_db, sub_inst_param_table_tag);

			sorted_nodes = (eiShaderNode *)(sub_inst_param_table + 1);

			for (j = 0; j < sub_inst_param_table->num_sorted_nodes; ++j)
			{
				ei_add_shader_node_unique(shader_nodes, sorted_nodes[j].tag);
			}
			
			ei_db_end(nodesys->m_db, sub_inst_param_table_tag);
			ei_db_end(nodesys->m_db, sub_inst_tag);
		}
	}
	
	ei_add_shader_node_unique(shader_nodes, shader_inst_tag);
}

static eiInt add_shader_node_proc(ei_btree_node *node, void *param)
{
	eiShaderTreeNode	*shader_tree_node;
	ei_array			*shader_nodes;
	eiShaderNode		shader_node;

	shader_tree_node = (eiShaderTreeNode *)node;
	shader_nodes = (ei_array *)param;

	shader_node.tag = shader_tree_node->tag;
	shader_node.shader_cache_offset = 0;

	ei_array_push_back(shader_nodes, &shader_node);

	return eiTRUE;
}

void generate_shader_inst_param_table(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls)
{
	eiNodeSystem			*nodesys;
	eiShaderInstParamTable	*tab;
	eiNode					*inst;
	ei_btree				shader_tree;
	ei_array				shader_nodes;
	eiInt					i;

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	eiDBG_ASSERT(pData != NULL && pData->ptr != NULL);
	tab = (eiShaderInstParamTable *)pData->ptr;

	inst = (eiNode *)ei_db_access(db, tab->inst);

	/* find all connected shader nodes */
	ei_btree_init(&shader_tree, ei_shader_tree_node_compare, ei_shader_tree_node_delete, NULL);
	
	ei_collect_shader_nodes(tab->inst, inst, &shader_tree, nodesys);

	ei_array_init(&shader_nodes, sizeof(eiShaderNode));

	ei_btree_traverse(&shader_tree, add_shader_node_proc, &shader_nodes);

	ei_btree_clear(&shader_tree);
	
	/* sort connected shader nodes by tag */
	ei_heapsort(
		ei_array_data(&shader_nodes), 
		ei_array_size(&shader_nodes), 
		sizeof(eiShaderNode), 
		ei_shader_node_compare);
	
	tab->num_sorted_nodes = ei_array_size(&shader_nodes);
	
	/* resize the data */
	tab = (eiShaderInstParamTable *)ei_db_resize(
		db, 
		data_tag, 
		sizeof(eiShaderInstParamTable) 
		+ sizeof(eiShaderNode) * tab->num_sorted_nodes);

	/* fill shader cache offset for nodes */
	tab->shader_cache_size = 0;

	for (i = 0; i < tab->num_sorted_nodes; ++i)
	{
		eiShaderNode		*pShaderNode;
		eiNode				*pShaderInst;

		pShaderNode = (eiShaderNode *)ei_array_get(&shader_nodes, i);
		pShaderNode->shader_cache_offset = sizeof(eiShaderCache) + tab->shader_cache_size;

		pShaderInst = (eiNode *)ei_db_access(db, pShaderNode->tag);

		/* the size of parameters should have been calculated for 
		   all dependencies due to the nature of depth first traversal */
		tab->shader_cache_size += (sizeof(eiShaderInstanceTLS) 
			+ pShaderInst->param_table_size 
			+ pShaderInst->param_table_size);

		ei_db_end(db, pShaderNode->tag);
	}
		
	/* copy shader nodes into the parameter table */
	memcpy(tab + 1, 
		ei_array_data(&shader_nodes), 
		sizeof(eiShaderNode) * tab->num_sorted_nodes);
	
	ei_array_clear(&shader_nodes);

	ei_db_end(db, tab->inst);
}

void byteswap_shader_inst_param_table(eiDatabase *db, void *data, const eiUint size)
{
	eiShaderInstParamTable *tab = (eiShaderInstParamTable *)data;

	ei_byteswap_int(&tab->inst);
	ei_byteswap_int(&tab->shader_cache_size);
	ei_byteswap_int(&tab->num_sorted_nodes);
}
