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

#include <eiAPI/ei_nodesys.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_vector.h>
#include <eiCORE/ei_vector4.h>
#include <eiCORE/ei_assert.h>

/** \brief The description of a node parameter, 
 * including name, type, and default value. */
typedef struct eiNodeParamDesc {
	char				name[ EI_MAX_PARAM_NAME_LEN ];
	/* the storage class of the node parameter */
	eiInt				storage_class;
	/* the data type of the node parameter */
	eiInt				type;
	/* the size of this parameter in bytes */
	eiUint				size;
} eiNodeParamDesc;

/** \brief The class description of a node. */
struct eiNodeDesc {
	char				name[ EI_MAX_NODE_NAME_LEN ];
	/* a data array of tags, each tag points to a eiNodeParamDesc 
	   plus the varying-sized default value. */
	eiTag				param_descs;
	/* this node plugin object must be re-created for 
	   each local host. */
	eiNodeObject		*object;
};

void ei_node_object_init(eiNodeObject *object)
{
	object->init_node = NULL;
	object->exit_node = NULL;
	object->node_changed = NULL;
}

void ei_node_object_exit(eiNodeObject *object)
{
}

const char *ei_node_desc_name(eiNodeDesc *desc)
{
	return desc->name;
}

static void ei_node_param_desc_init(
	eiNodeParamDesc *desc, 
	const char *name, 
	const eiInt storage_class, 
	const eiInt type, 
	const eiUint size, 
	const void *value)
{
	strncpy(desc->name, name, EI_MAX_PARAM_NAME_LEN - 1);
	desc->storage_class = storage_class;
	desc->type = type;
	desc->size = size;
	/* none is used to pad dummy node parameter */
	if (type != EI_DATA_TYPE_NONE)
	{
		memcpy(desc + 1, value, size);
	}
	else
	{
		/* the padding size must be at least larger than eiUint */
		eiASSERT(size >= sizeof(eiUint));
		*((eiUint *)(desc + 1)) = (eiUint)value;
	}
}

static void ei_node_param_desc_exit(
	eiNodeParamDesc *desc, 
	eiDatabase *db)
{
	/* TODO: delete parameter value by type if needed */
}

static void ei_node_param_init(
	eiNodeParam *param, 
	const eiInt storage_class, 
	const eiInt type, 
	const char *name, 
	const eiUint size)
{
	strncpy(param->name, name, EI_MAX_PARAM_NAME_LEN - 1);
	param->storage_class = storage_class;
	param->type = type;
	param->size = size;
	param->inst = eiNULL_TAG;
	param->param = eiNULL_TAG;
	param->channel_offset = 0;
	param->channel_dim = 0;
}

static void ei_node_param_exit(eiNodeParam *param)
{
	/* TODO: delete parameter value by type if needed */
}

static void ei_node_desc_init(
	eiNodeDesc *desc, 
	eiNodeSystem *nodesys, 
	const char *name)
{
	strncpy(desc->name, name, EI_MAX_NODE_NAME_LEN - 1);
	desc->param_descs = ei_create_data_array(nodesys->m_db, EI_DATA_TYPE_TAG);
	/* set the node plugin object to NULL, it will be re-created 
	   for each local host in data generator. */
	desc->object = NULL;
}

static void ei_node_desc_exit(
	eiNodeDesc *desc, 
	eiNodeSystem *nodesys)
{
	eiInt	num_param_descs;
	eiInt	i;

	num_param_descs = ei_data_array_size(nodesys->m_db, desc->param_descs);

	for (i = 0; i < num_param_descs; ++i)
	{
		eiTag			param_desc_tag;
		eiNodeParamDesc	*param_desc;

		param_desc_tag = *((eiTag *)ei_data_array_read(nodesys->m_db, desc->param_descs, i));
		ei_data_array_end(nodesys->m_db, desc->param_descs, i);

		param_desc = (eiNodeParamDesc *)ei_db_access(nodesys->m_db, param_desc_tag);
		ei_node_param_desc_exit(param_desc, nodesys->m_db);
		ei_db_end(nodesys->m_db, param_desc_tag);

		ei_db_delete(nodesys->m_db, param_desc_tag);
	}

	ei_delete_data_array(nodesys->m_db, desc->param_descs);
}

static eiFORCEINLINE eiNodeParam *get_node_param_by_index(
	eiNode *node, 
	const eiIndex param_index)
{
	eiNodeParam		*params;

	params = (eiNodeParam *)(((eiByte *)(node + 1)) + node->param_table_size);

	return (params + param_index);
}

static void ei_node_init(
	eiNode *node, 
	eiDatabase *db, 
	const eiTag tag, 
	const char *name)
{
	strncpy(node->name, name, EI_MAX_NODE_NAME_LEN - 1);
	node->type = 0;
	node->tag = tag;
	node->desc = eiNULL_TAG;
	node->num_params = 0;
	node->param_table_size = 0;
	node->object = NULL;
	node->time = 0;
}

static void ei_node_exit(
	eiNode *node, 
	eiDatabase *db)
{
	eiInt	i;

	for (i = 0; i < node->num_params; ++i)
	{
		eiNodeParam		*param;

		param = get_node_param_by_index(node, i);
		ei_node_param_exit(param);
	}
}

void ei_nodesys_init(
	eiNodeSystem *nodesys, 
	eiDatabase *db)
{
	nodesys->m_db = db;
	eiASSERT(db != NULL);

	ei_plugsys_init(&nodesys->m_plugsys);
	
	ei_symbol_table_init(&nodesys->m_desc_table, (void *)eiNULL_TAG);
	ei_symbol_table_init(&nodesys->m_node_table, (void *)eiNULL_TAG);
	ei_symbol_table_init(&nodesys->m_creator_table, NULL);
}

void ei_nodesys_exit(eiNodeSystem *nodesys)
{
	ei_symbol_table_exit(&nodesys->m_creator_table);
	ei_symbol_table_exit(&nodesys->m_node_table);
	ei_symbol_table_exit(&nodesys->m_desc_table);

	ei_plugsys_exit(&nodesys->m_plugsys);
}

eiPluginSystem *ei_nodesys_plugin_system(eiNodeSystem *nodesys)
{
	return &nodesys->m_plugsys;
}

ei_create_node_object_func ei_nodesys_find_creator(
	eiNodeSystem *nodesys, 
	const char *name)
{
	return (ei_create_node_object_func)ei_symbol_table_find(&nodesys->m_creator_table, name);
}

void ei_nodesys_register_creator(
	eiNodeSystem *nodesys, 
	const char *name, 
	ei_create_node_object_func creator)
{
	/* lookup the creator to prevent from duplicated registrations */
	if (ei_symbol_table_find(&nodesys->m_creator_table, name) != NULL)
	{
		ei_warning("Duplicated registration for creator %s\n", name);
		return;
	}

	/* add the creator to symbol map */
	ei_symbol_table_add(&nodesys->m_creator_table, name, (void *)creator);
}

eiTag ei_nodesys_find_node_desc(
	eiNodeSystem *nodesys, 
	const char *desc_name)
{
	return (eiTag)ei_symbol_table_find(&nodesys->m_desc_table, desc_name);
}

eiNodeDesc *ei_nodesys_node_desc(
	eiNodeSystem *nodesys, 
	eiTag * const tag, 
	const char *desc_name)
{
	eiNodeDesc		*desc;

	/* lookup the node desc and return existing one, thus we 
	   allow incremental changes to the scene graph. */
	*tag = (eiTag)ei_symbol_table_find(&nodesys->m_desc_table, desc_name);

	if (*tag != eiNULL_TAG)
	{
		/* we don't allow incremental updates to node desc, 
		   because nodes cannot get notifications when node 
		   desc changes to re-build their parameters from 
		   the desc, we only allow static node desc. */
		ei_error("Duplicated node desc %s\n", desc_name);

		return NULL;
	}

	/* create the node desc if it's not created before */
	desc = (eiNodeDesc *)ei_db_create(
		nodesys->m_db, 
		tag, 
		EI_DATA_TYPE_NODE_DESC, 
		sizeof(eiNodeDesc), 
		EI_DB_FLUSHABLE | EI_DB_GEN_LOCAL);

	ei_node_desc_init(desc, nodesys, desc_name);

	/* add the node desc to symbol map */
	ei_symbol_table_add(&nodesys->m_desc_table, desc->name, (void *)(*tag));

	return desc;
}

void ei_nodesys_add_parameter(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const eiInt param_type, 
	const char *param_name, 
	const void *default_value)
{
	eiNodeParamDesc		*param_desc;
	eiTag				param_desc_tag;
	eiUint				value_size;

	if (desc == NULL)
	{
		ei_error("Invalid node desc.\n");
		return;
	}

	if (param_name == NULL)
	{
		ei_error("Invalid parameter name.\n");
		return;
	}

	if (default_value == NULL)
	{
		ei_error("Invalid default parameter value.\n");
		return;
	}

	/* none is used to pad dummy node parameter */
	if (param_type != EI_DATA_TYPE_NONE)
	{
		value_size = ei_db_type_size(nodesys->m_db, param_type);
	}
	else
	{
		value_size = (eiUint)default_value;
	}

	param_desc = (eiNodeParamDesc *)ei_db_create(
		nodesys->m_db, 
		&param_desc_tag, 
		EI_DATA_TYPE_NODE_PARAM_DESC, 
		sizeof(eiNodeParamDesc) + value_size, 
		EI_DB_FLUSHABLE);

	ei_node_param_desc_init(param_desc, param_name, param_storage_class, param_type, value_size, default_value);

	ei_db_end(nodesys->m_db, param_desc_tag);

	ei_data_array_push_back(nodesys->m_db, desc->param_descs, &param_desc_tag);
}

eiNodeObject *ei_node_desc_get_object(eiNodeDesc *desc)
{
	return desc->object;
}

void ei_nodesys_end_node_desc(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiTag tag)
{
	if (desc == NULL)
	{
		ei_error("Invalid node desc.\n");
		return;
	}

	ei_db_end(nodesys->m_db, tag);
}

void ei_nodesys_delete_node_desc(
	eiNodeSystem *nodesys, 
	const eiTag tag)
{
	eiNodeDesc		*desc;

	desc = (eiNodeDesc *)ei_db_access(nodesys->m_db, tag);

	/* remove the node desc from symbol map */
	ei_symbol_table_remove(&nodesys->m_desc_table, desc->name);

	ei_node_desc_exit(desc, nodesys);

	ei_db_end(nodesys->m_db, tag);

	ei_db_delete(nodesys->m_db, tag);
}

eiTag ei_nodesys_find_node(
	eiNodeSystem *nodesys, 
	const char *node_name)
{
	return (eiTag)ei_symbol_table_find(&nodesys->m_node_table, node_name);
}

eiIndex ei_nodesys_find_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name)
{
	eiInt	i;

	for (i = 0; i < node->num_params; ++i)
	{
		eiNodeParam		*param;

		param = get_node_param_by_index(node, i);
		
		/* do a case-sensitive comparsion */
		if (strcmp(param->name, param_name) == 0)
		{
			return i;
		}
	}

	return eiNULL_INDEX;
}

eiIndex ei_nodesys_lookup_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name)
{
	/* lookup node parameter from symbol table */
	return (eiIndex)ei_symbol_table_find(&node->param_table, param_name);
}

eiNode *ei_nodesys_node(
	eiNodeSystem *nodesys, 
	const char *node_name, 
	eiBool *need_init)
{
	eiNode		*node;
	eiTag		tag;
	
	/* lookup the node and return existing one, thus we 
	   allow incremental changes to the scene graph. */
	tag = ei_nodesys_find_node(nodesys, node_name);

	if (tag != eiNULL_TAG)
	{
		/* already created before, do not need initialization */
		if (need_init != NULL)
		{
			*need_init = eiFALSE;
		}

		return (eiNode *)ei_db_access(nodesys->m_db, tag);
	}

	/* create the node if it's not created before */
	node = (eiNode *)ei_db_create(
		nodesys->m_db, 
		&tag, 
		EI_DATA_TYPE_NODE, 
		sizeof(eiNode), 
		EI_DB_FLUSHABLE | EI_DB_GEN_LOCAL);

	/* construct base node */
	ei_node_init(node, nodesys->m_db, tag, node_name);

	/* add the node to symbol map */
	ei_symbol_table_add(&nodesys->m_node_table, node->name, (void *)tag);

	/* just created, need initialization */
	if (need_init != NULL)
	{
		*need_init = eiTRUE;
	}

	return node;
}

eiIndex ei_nodesys_declare_parameter(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const eiInt param_storage_class, 
	const eiInt param_type, 
	const char *param_name, 
	const void *default_value)
{
	eiUint			value_size;
	eiNodeParam		param;
	eiSizet			params_size;
	eiNodeParam		*params;
	eiByte			*param_table;
	eiIndex			param_index;

	if (node == NULL || *node == NULL)
	{
		ei_error("Invalid node.\n");
		return eiNULL_INDEX;
	}

	if (param_name == NULL)
	{
		ei_error("Invalid parameter name.\n");
		return eiNULL_INDEX;
	}

	if (default_value == NULL)
	{
		ei_error("Invalid default parameter value.\n");
		return eiNULL_INDEX;
	}
	
	/* none is used to pad dummy node parameter */
	if (param_type != EI_DATA_TYPE_NONE)
	{
		value_size = ei_db_type_size(nodesys->m_db, param_type);
	}
	else
	{
		value_size = *((eiUint *)default_value);
	}
	
	ei_node_param_init(
		&param, 
		param_storage_class, 
		param_type, 
		param_name, 
		value_size);

	/* backup the node parameters before resizing */
	params_size = (*node)->num_params * sizeof(eiNodeParam);
	params = NULL;
	if (params_size > 0)
	{
		params = (eiNodeParam *)ei_allocate(params_size);
		memcpy(params, get_node_param_by_index(*node, 0), params_size);
	}
		
	/* set the parameter offset in the parameter table */
	param.offset = (*node)->param_table_size;
	(*node)->param_table_size += param.size;
	
	/* resize the parameter table */
	*node = (eiNode *)ei_db_resize(nodesys->m_db, (*node)->tag, 
		sizeof(eiNode) + 
		(*node)->param_table_size + 
		((*node)->num_params + 1) * sizeof(eiNodeParam));
	param_table = (eiByte *)(*node + 1);

	/* restore the node parameters (move backwards) before 
	   being overwritten by parameter values */
	if (params_size > 0)
	{
		memcpy(get_node_param_by_index(*node, 0), params, params_size);
	}
	eiCHECK_FREE(params);
	
	/* copy the default value to parameter value */
	/* none is used to pad dummy node parameter */
	if (param_type != EI_DATA_TYPE_NONE)
	{
		memcpy(param_table + param.offset, default_value, param.size);
	}
	else
	{
		memset(param_table + param.offset, 0, param.size);
	}

	param_index = (*node)->num_params;
	
	memcpy(get_node_param_by_index(*node, param_index), &param, sizeof(eiNodeParam));
	++ (*node)->num_params;

	/* dirt the parameter table automatically */
	/* dirt the node such that the parameter map will be re-built */
	ei_db_dirt(nodesys->m_db, (*node)->tag);

	return param_index;
}

static void ei_node_build_parameters(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	eiNodeDesc *desc)
{
	eiInt	num_param_descs;
	eiInt	i;

	/* TODO: a performance optimization, resize parameter table once 
	   here since we already know all parameters from the node description */

	num_param_descs = ei_data_array_size(nodesys->m_db, desc->param_descs);

	for (i = 0; i < num_param_descs; ++i)
	{
		eiTag				param_desc_tag;
		eiNodeParamDesc		*param_desc;

		param_desc_tag = *((eiTag *)ei_data_array_read(nodesys->m_db, desc->param_descs, i));
		ei_data_array_end(nodesys->m_db, desc->param_descs, i);

		param_desc = (eiNodeParamDesc *)ei_db_access(nodesys->m_db, param_desc_tag);
			
		ei_nodesys_declare_parameter(
			nodesys, 
			node, 
			param_desc->storage_class, 
			param_desc->type, 
			param_desc->name, 
			param_desc + 1);

		ei_db_end(nodesys->m_db, param_desc_tag);
	}
}

static void ei_nodesys_node_set_desc(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const void *param_value)
{
	eiNodeDesc				*desc;

	if ((*node)->desc != eiNULL_TAG)
	{
		ei_warning("The node desc has been set, cannot set it twice.\n");
		return;
	}

	/* begin access node desc */
	(*node)->desc = ei_nodesys_find_node_desc(nodesys, (char *)param_value);

	/* try to create the node desc, notice that this is NOT thread-safe, 
	   so we must avoid creating such nodes in multi-threaded processing */
	/* this is specifically designed for deferred shader class creation, 
	   which is only supposed to be used in single-threaded mode */
	if ((*node)->desc == eiNULL_TAG)
	{
		ei_plugsys_declare(&nodesys->m_plugsys, (char *)param_value, NULL);

		/* try the search again */
		(*node)->desc = ei_nodesys_find_node_desc(nodesys, (char *)param_value);
	}

	if ((*node)->desc == eiNULL_TAG)
	{
		ei_error("Cannot find the node desc %s", (char *)param_value);
		return;
	}

	desc = (eiNodeDesc *)ei_db_access(nodesys->m_db, (*node)->desc);

	/* build node parameters from node desc */
	ei_node_build_parameters(nodesys, node, desc);

	/* invoke the node constructor for derived classes */
	if (desc->object->init_node != NULL)
	{
		desc->object->init_node(nodesys, *node);
	}

	/* end access node desc */
	ei_db_end(nodesys->m_db, (*node)->desc);
}

void ei_nodesys_set_parameter(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const void *param_value)
{
	eiIndex			param_index;
	eiNodeParam		*param;
	eiByte			*param_table;

	if (node == NULL || *node == NULL)
	{
		ei_error("Invalid node.\n");
		return;
	}

	if (param_value == NULL)
	{
		ei_error("Invalid parameter value.\n");
		return;
	}

	/* set built-in parameters */
	if (strcmp(param_name, "desc") == 0)
	{
		ei_nodesys_node_set_desc(nodesys, node, param_value);
		return;
	}

	/* lookup the node parameter */
	param_index = ei_nodesys_find_parameter(nodesys, *node, param_name);
	
	if (param_index == eiNULL_INDEX)
	{
		ei_warning("Failed to lookup node parameter %s\n", param_name);
		return;
	}

	/* set custom parameters */
	param_table = (eiByte *)((*node) + 1);

	param = get_node_param_by_index(*node, param_index);
	
	/* copy the parameter value, assume the user 
	   would always pass the value of correct type, 
	   not type casting is required here. */
	memcpy(param_table + param->offset, param_value, param->size);

	/* dirt the parameter table automatically */
	ei_db_dirt(nodesys->m_db, (*node)->tag);
}

void ei_nodesys_link_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name, 
	const char *src_node_name, 
    const char *src_param_name)
{
	eiTag			src_node_tag;
	eiNode			*src_node;
	eiTag			src_param_index;
	eiIndex			param_index;
	eiNodeParam		*param;

	if (node == NULL)
	{
		ei_error("Invalid node.\n");
		return;
	}

	/* lookup the node */
	src_node_tag = ei_nodesys_find_node(nodesys, src_node_name);

	if (src_node_tag == eiNULL_TAG)
	{
		ei_warning("Failed to lookup connected node %s\n", src_node_name);
		return;
	}

	/* lookup the connected node parameter */
	src_node = (eiNode *)ei_db_access(nodesys->m_db, src_node_tag);
	src_param_index = ei_nodesys_find_parameter(nodesys, src_node, src_param_name);
	ei_db_end(nodesys->m_db, src_node_tag);

	if (src_param_index == eiNULL_INDEX)
	{
		ei_warning("Failed to lookup connected node parameter %s\n", src_param_name);
		return;
	}
	
	/* lookup the node parameter */
	param_index = ei_nodesys_find_parameter(nodesys, node, param_name);
	
	if (param_index == eiNULL_INDEX)
	{
		ei_warning("Failed to lookup node parameter %s\n", param_name);
		return;
	}

	param = get_node_param_by_index(node, param_index);

	param->inst = src_node_tag;
	param->param = src_param_index;

	/* dirt the parameter table automatically */
	ei_db_dirt(nodesys->m_db, node->tag);
}

void ei_nodesys_unlink_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name)
{
	eiIndex			param_index;
	eiNodeParam		*param;

	if (node == NULL)
	{
		ei_error("Invalid node.\n");
		return;
	}
	
	/* lookup the node parameter */
	param_index = ei_nodesys_find_parameter(nodesys, node, param_name);
	
	if (param_index == eiNULL_INDEX)
	{
		ei_warning("Failed to lookup node parameter %s\n", param_name);
		return;
	}

	param = get_node_param_by_index(node, param_index);

	param->inst = eiNULL_TAG;
	param->param = eiNULL_TAG;

	/* dirt the parameter table automatically */
	ei_db_dirt(nodesys->m_db, node->tag);
}

eiUint ei_nodesys_get_parameter_count(
	eiNodeSystem *nodesys, 
	eiNode *node)
{
	return (eiUint)node->num_params;
}

eiNodeParam *ei_nodesys_read_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const eiIndex param_index)
{
	return get_node_param_by_index(node, param_index);
}

eiNodeParam *ei_nodesys_write_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const eiIndex param_index)
{
	/* dirt the parameter table automatically */
	ei_db_dirt(nodesys->m_db, node->tag);

	return get_node_param_by_index(node, param_index);
}

void ei_nodesys_get_parameter_value(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	eiNodeParam *param, 
	void *param_value)
{
	eiByte		*param_table;

	param_table = (eiByte *)(node + 1);
	
	/* copy the parameter value, assume the user 
	   would always pass the value of correct type, 
	   no type casting is required here. */
	memcpy(param_value, param_table + param->offset, param->size);
}

void ei_nodesys_cast_parameter_value(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	eiNodeParam *param, 
	void *param_value, 
	const eiInt type)
{
	eiByte		*param_table;

	param_table = (eiByte *)(node + 1);

	/* cast the parameter value into the destination type. */
	ei_db_cast(
		nodesys->m_db, 
		param_value, 
		type, 
		param_table + param->offset, 
		param->type);
}

void ei_nodesys_get_parameter_at(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const eiIndex param_index, 
	void *param_value)
{
	eiNodeParam		*param;

	param = get_node_param_by_index(node, param_index);
	
	ei_nodesys_get_parameter_value(nodesys, node, param, param_value);
}

void ei_nodesys_cast_parameter_at(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const eiIndex param_index, 
	void *param_value, 
	const eiInt type)
{
	eiNodeParam		*param;

	param = get_node_param_by_index(node, param_index);
	
	ei_nodesys_cast_parameter_value(nodesys, node, param, param_value, type);
}

void ei_nodesys_get_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name, 
	void *param_value)
{
	eiIndex			param_index;

	param_index = (eiIndex)ei_symbol_table_find(&node->param_table, param_name);

	if (param_index == eiNULL_INDEX)
	{
		ei_warning("Failed to lookup node parameter %s\n", param_name);
		return;
	}

	ei_nodesys_get_parameter_at(nodesys, node, param_index, param_value);
}

void ei_nodesys_end_node(
	eiNodeSystem *nodesys, 
	eiNode *node)
{
	if (node == NULL)
	{
		ei_error("Invalid node.\n");
		return;
	}

	/* time-stamp the modification to the node */
	++ node->time;

	if (node->desc != eiNULL_TAG)
	{
		eiNodeDesc		*desc;

		desc = (eiNodeDesc *)ei_db_access(nodesys->m_db, node->desc);

		/* notify node changed for derived classes */
		if (desc->object->node_changed != NULL)
		{
			desc->object->node_changed(nodesys, node);
		}

		/* end access node desc */
		ei_db_end(nodesys->m_db, node->desc);
	}

	ei_db_end(nodesys->m_db, node->tag);
}

void ei_nodesys_delete_node(
	eiNodeSystem *nodesys, 
	const eiTag tag)
{
	eiNode		*node;
	eiNodeDesc	*desc;

	node = (eiNode *)ei_db_access(nodesys->m_db, tag);

	/* remove the node from symbol map */
	ei_symbol_table_remove(&nodesys->m_node_table, node->name);

	desc = (eiNodeDesc *)ei_db_access(nodesys->m_db, node->desc);

	/* invoke the node destructor for derived classes */
	if (desc->object->exit_node != NULL)
	{
		desc->object->exit_node(nodesys, node);
	}

	ei_db_end(nodesys->m_db, node->desc);

	/* call base node destructor automatically */
	ei_node_exit(node, nodesys->m_db);

	ei_db_end(nodesys->m_db, tag);

	ei_db_delete(nodesys->m_db, tag);
}

void ei_nodesys_add_int(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiInt default_value)
{
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		param_storage_class, 
		EI_DATA_TYPE_INT, 
		param_name, 
		&default_value);
}

void ei_nodesys_add_scalar(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiScalar default_value)
{
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		param_storage_class, 
		EI_DATA_TYPE_SCALAR, 
		param_name, 
		&default_value);
}

void ei_nodesys_add_vector(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiScalar x, const eiScalar y, const eiScalar z)
{
	eiVector	value;
	setv(&value, x, y, z);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		param_storage_class, 
		EI_DATA_TYPE_VECTOR, 
		param_name, 
		&value);
}

void ei_nodesys_add_vector4(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiScalar x, const eiScalar y, const eiScalar z, const eiScalar w)
{
	eiVector4	value;
	setv4(&value, x, y, z, w);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		param_storage_class, 
		EI_DATA_TYPE_VECTOR4, 
		param_name, 
		&value);
}

void ei_nodesys_add_tag(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiTag default_value)
{
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		param_storage_class, 
		EI_DATA_TYPE_TAG, 
		param_name, 
		&default_value);
}

void ei_nodesys_add_index(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiIndex default_value)
{
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		param_storage_class, 
		EI_DATA_TYPE_INDEX, 
		param_name, 
		&default_value);
}

void ei_nodesys_add_bool(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiBool default_value)
{
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		param_storage_class, 
		EI_DATA_TYPE_BOOL, 
		param_name, 
		&default_value);
}

eiInt ei_nodesys_get_int(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name)
{
	eiInt	value;
	ei_nodesys_get_parameter(nodesys, node, param_name, &value);
	return value;
}

eiScalar ei_nodesys_get_scalar(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name)
{
	eiScalar	value;
	ei_nodesys_get_parameter(nodesys, node, param_name, &value);
	return value;
}

eiTag ei_nodesys_get_tag(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name)
{
	eiTag	value;
	ei_nodesys_get_parameter(nodesys, node, param_name, &value);
	return value;
}

eiIndex ei_nodesys_get_index(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name)
{
	eiIndex	value;
	ei_nodesys_get_parameter(nodesys, node, param_name, &value);
	return value;
}

eiBool ei_nodesys_get_bool(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name)
{
	eiBool	value;
	ei_nodesys_get_parameter(nodesys, node, param_name, &value);
	return value;
}

void ei_nodesys_set_string(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const char *param_value)
{
	ei_nodesys_set_parameter(nodesys, node, param_name, param_value);
}

void ei_nodesys_set_int(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiInt param_value)
{
	ei_nodesys_set_parameter(nodesys, node, param_name, &param_value);
}

void ei_nodesys_set_scalar(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiScalar param_value)
{
	ei_nodesys_set_parameter(nodesys, node, param_name, &param_value);
}

void ei_nodesys_set_vector(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiScalar x, const eiScalar y, const eiScalar z)
{
	eiVector	value;
	setv(&value, x, y, z);
	ei_nodesys_set_parameter(nodesys, node, param_name, &value);
}

void ei_nodesys_set_vector4(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiScalar x, const eiScalar y, const eiScalar z, const eiScalar w)
{
	eiVector4	value;
	setv4(&value, x, y, z, w);
	ei_nodesys_set_parameter(nodesys, node, param_name, &value);
}

void ei_nodesys_set_tag(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiTag param_value)
{
	ei_nodesys_set_parameter(nodesys, node, param_name, &param_value);
}

void ei_nodesys_set_index(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiIndex param_value)
{
	ei_nodesys_set_parameter(nodesys, node, param_name, &param_value);
}

void ei_nodesys_set_bool(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiBool param_value)
{
	ei_nodesys_set_parameter(nodesys, node, param_name, &param_value);
}

void generate_node_desc(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls)
{
	eiNodeSystem				*nodesys;
	eiNodeDesc					*desc;
	ei_create_node_object_func	creator;

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	eiDBG_ASSERT(pData != NULL && pData->ptr != NULL);
	desc = (eiNodeDesc *)pData->ptr;

	/* lookup built-in creator */
	creator = ei_nodesys_find_creator(nodesys, desc->name);

	if (creator != NULL)
	{
		/* create built-in node object */
		desc->object = creator(NULL);

		/* flag build-in node object by setting its plugin 
		   pointer to NULL */
		desc->object->base.plugin = NULL;
	}
	else
	{
		/* create the node plugin object */
		desc->object = (eiNodeObject *)ei_plugsys_create(
			&nodesys->m_plugsys, 
			desc->name, 
			NULL);
	}
}

void clear_node_desc(eiDatabase *db, void *data)
{
	eiNodeSystem	*nodesys;
	eiNodeDesc		*desc;

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	eiDBG_ASSERT(data != NULL);
	desc = (eiNodeDesc *)data;

	if (desc->object == NULL)
	{
		return;
	}

	/* the built-in node object has no plugin pointer */
	if (desc->object->base.plugin == NULL)
	{
		/* we can delete built-in one directly by our own heap */
		eiCHECK_FREE(desc->object);
	}
	else
	{
		/* delete the node plugin object */
		ei_plugsys_delete(&nodesys->m_plugsys, &desc->object->base);
	}

	desc->object = NULL;
}

void generate_node(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls)
{
	eiNodeSystem	*nodesys;
	eiNode			*node;
	eiInt			i;
	eiNodeDesc		*desc;

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	eiDBG_ASSERT(pData != NULL && pData->ptr != NULL);
	node = (eiNode *)pData->ptr;

	/* build parameter map */
	ei_symbol_table_init(&node->param_table, (void *)eiNULL_INDEX);

	for (i = 0; i < node->num_params; ++i)
	{
		eiNodeParam		*param;

		param = get_node_param_by_index(node, i);
		/* add the mapping from parameter name to index */
		ei_symbol_table_add(&node->param_table, param->name, (void *)i);
	}

	/* cache node object for fast access */
	desc = (eiNodeDesc *)ei_db_access(db, node->desc);
	node->object = desc->object;
	ei_db_end(db, node->desc);
}

void clear_node(eiDatabase *db, void *data)
{
	eiNodeSystem	*nodesys;
	eiNode			*node;

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	eiDBG_ASSERT(data != NULL);
	node = (eiNode *)data;

	/* clear node object */
	node->object = NULL;

	/* clear parameter map */
	ei_symbol_table_exit(&node->param_table);
}

void byteswap_node_desc(eiDatabase *db, void *data, const eiUint size)
{
	eiNodeDesc *desc = (eiNodeDesc *)data;

	ei_byteswap_int(&desc->param_descs);
}

void byteswap_node_param_desc(eiDatabase *db, void *data, const eiUint size)
{
	eiNodeParamDesc		*desc;

	desc = (eiNodeParamDesc *)data;

	/* none is used to pad dummy node parameter */
	if (desc->type != EI_DATA_TYPE_NONE)
	{
		ei_db_byteswap_typed(db, desc->type, desc + 1, desc->size);
	}

	ei_byteswap_int(&desc->storage_class);
	ei_byteswap_int(&desc->type);
	ei_byteswap_int(&desc->size);
}

void byteswap_node(eiDatabase *db, void *data, const eiUint size)
{
	eiNode		*node;
	eiByte		*param_table;
	eiInt		i;

	node = (eiNode *)data;
	param_table = (eiByte *)(node + 1);

	/* byte-swap all parameter values in parameter table */
	for (i = 0; i < node->num_params; ++i)
	{
		eiNodeParam		*param;

		param = get_node_param_by_index(node, i);

		ei_db_byteswap_typed(db, param->type, param_table + param->offset, param->size);

		byteswap_node_param(db, param, sizeof(eiNodeParam));
	}

	ei_byteswap_int(&node->type);
	ei_byteswap_int(&node->tag);
	ei_byteswap_int(&node->desc);
	ei_byteswap_int(&node->num_params);
	ei_byteswap_int(&node->param_table_size);
	ei_byteswap_int(&node->time);
}

void byteswap_node_param(eiDatabase *db, void *data, const eiUint size)
{
	eiNodeParam *param = (eiNodeParam *)data;

	ei_byteswap_int(&param->storage_class);
	ei_byteswap_int(&param->type);
	ei_byteswap_int(&param->offset);
	ei_byteswap_int(&param->size);
	ei_byteswap_int(&param->inst);
	ei_byteswap_int(&param->param);
	ei_byteswap_int(&param->channel_offset);
	ei_byteswap_int(&param->channel_dim);
}
