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
 
#ifndef EI_NODESYS_H
#define EI_NODESYS_H

/** \brief The pluggable node system for scene management.
 * \file ei_nodesys.h
 * \author Elvic Liang, Bo Zhou
 */

#include <eiAPI/ei_api.h>
#include <eiCORE/ei_dataflow.h>
#include <eiCORE/ei_plugsys.h>
#include <eiCORE/ei_symbol.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The storage class of primitive variables. */
typedef enum eiStorageClass {
	eiCONSTANT = 0,	/* constant per-object */
	eiUNIFORM,		/* constant per-primitive */
	eiVARYING,		/* bilinearly interpolated */
	eiVERTEX,		/* vertex interpolated */
} eiStorageClass;

typedef struct eiNodeObject		eiNodeObject;
typedef struct eiNodeDesc		eiNodeDesc;
typedef struct eiNode			eiNode;
typedef struct eiNodeSystem		eiNodeSystem;

/** \brief Node object constructor.
 * @param param The node description which is creating this node object. */
typedef void (*ei_node_object_init_func)(eiNodeObject *object, void *param);
/** \brief Node object destructor. */
typedef void (*ei_node_object_exit_func)(eiNodeObject *object);
/** \brief The custom node constructor for derived classes. */
typedef void (*ei_node_init_func)(eiNodeSystem *nodesys, eiNode *node);
/** \brief The custom node destructor for derived classes. */
typedef void (*ei_node_exit_func)(eiNodeSystem *nodesys, eiNode *node);
/** \brief The node changed callback, called when finished node editing. */
typedef void (*ei_node_changed_func)(eiNodeSystem *nodesys, eiNode *node);

/** \brief The function to create built-in node object. */
typedef eiNodeObject * (*ei_create_node_object_func)(void *param);

/** \brief The node object loaded from DSO. it's 
 * derived from plugin object. */
struct eiNodeObject {
	/* the base plugin object */
	eiPluginObject				base;
	/* the node constructor */
	ei_node_init_func			init_node;
	/* the node destructor */
	ei_node_exit_func			exit_node;
	/* node changed, called when finished node editing */
	ei_node_changed_func		node_changed;
};

eiAPI void ei_node_object_init(eiNodeObject *object);
eiAPI void ei_node_object_exit(eiNodeObject *object);

/** \brief Get the name of a node description. */
eiAPI const char *ei_node_desc_name(eiNodeDesc *desc);

/** \brief A class encapsulates the node parameter. */
typedef struct eiNodeParam {
	char				name[ EI_MAX_PARAM_NAME_LEN ];
	/* the storage class of the node parameter */
	eiInt				storage_class;
	/* the data type of the node parameter */
	eiInt				type;
	/* the offset of this parameter in node parameter 
	   table in bytes */
	eiUint				offset;
	/* the size of this parameter in bytes */
	eiUint				size;
	/* the connected node instance tag as the input */
	eiTag				inst;
	/* the connected node parameter index as the input */
	eiIndex				param;
	/* the offset in varying/vertex channels */
	eiInt				channel_offset;
	/* the dimension in varying/vertex channels */
	eiInt				channel_dim;
} eiNodeParam;

/** \brief The node instance in scene graph. */
struct eiNode {
	char				name[ EI_MAX_NODE_NAME_LEN ];
	/* the user type flag, will not be modified by the node 
	   system. */
	eiInt				type;
	/* the tag of this node in database */
	eiTag				tag;
	/* the tag of the node desc */
	eiTag				desc;
	/* the number of all node parameters */
	eiInt				num_params;
	/* the size of all node parameters for this node instance 
	   in bytes, not including connected node parameters */
	eiUint				param_table_size;
	/* the local symbol table for node parameters */
	eiSymbolTable		param_table;
	/* this node plugin object must be re-created for 
	   each local host. this is also a member of eiNodeDesc, 
	   cached in each node for fast access. */
	eiNodeObject		*object;
	/* the time-stamp for the last modification to this node */
	eiUint				time;
};

/** \brief The node system. */
struct eiNodeSystem {
	/* the database we are working on */
	eiDatabase			*m_db;
	/* the plugin system we are linking to */
	eiPluginSystem		m_plugsys;
	/* the global symbol table for node descs */
	eiSymbolTable		m_desc_table;
	/* the global symbol table for nodes */
	eiSymbolTable		m_node_table;
	/* the global symbol table for built-in creators of 
	   node objects */
	eiSymbolTable		m_creator_table;
};

/** \brief Initialize the node system. */
eiAPI void ei_nodesys_init(
	eiNodeSystem *nodesys, 
	eiDatabase *db);
/** \brief Cleanup the node system. */
eiAPI void ei_nodesys_exit(eiNodeSystem *nodesys);

/** \brief Get the plugin system for the node system. */
eiAPI eiPluginSystem *ei_nodesys_plugin_system(eiNodeSystem *nodesys);

/** \brief Lookup a creator of built-in node object by name. */
eiAPI ei_create_node_object_func ei_nodesys_find_creator(
	eiNodeSystem *nodesys, 
	const char *name);
/** \brief Register a creator for built-in node object.
 * in order to call this function from every local host, 
 * we must register all built-in creators in global object 
 * creator. */
eiAPI void ei_nodesys_register_creator(
	eiNodeSystem *nodesys, 
	const char *name, 
	ei_create_node_object_func creator);

/** \brief Find a node desc by name. */
eiAPI eiTag ei_nodesys_find_node_desc(
	eiNodeSystem *nodesys, 
	const char *desc_name);
/** \brief Begin editing a node desc, create it if 
 * it's not created before. */
eiAPI eiNodeDesc *ei_nodesys_node_desc(
	eiNodeSystem *nodesys, 
	eiTag * const tag, 
	const char *desc_name);
/** \brief Add a parameter to a node desc. */
eiAPI void ei_nodesys_add_parameter(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const eiInt param_type, 
	const char *param_name, 
	const void *default_value);
/** \brief Get the plugin object of a node desc. do NOT use 
 * this function to retreive node object for each node, use 
 * the node object member in each node directly is faster. */
eiAPI eiNodeObject *ei_node_desc_get_object(eiNodeDesc *desc);
/** \brief End editing a node desc. */
eiAPI void ei_nodesys_end_node_desc(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiTag tag);
/** \brief Delete a node desc. */
eiAPI void ei_nodesys_delete_node_desc(
	eiNodeSystem *nodesys, 
	const eiTag tag);

/** \brief Find a node by name. */
eiAPI eiTag ei_nodesys_find_node(
	eiNodeSystem *nodesys, 
	const char *node_name);
/** \brief Lookup a node parameter by name from symbol table. 
 * this function should NOT be called when editing the node, 
 * because symbol table has not been re-built while editing. */
eiAPI eiIndex ei_nodesys_lookup_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name);
/** \brief Find a node parameter by name in parameter list, 
 * this function can be used while editing the node, but it's 
 * slower. */
eiAPI eiIndex ei_nodesys_find_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name);
/** \brief Begin editing a node, create it if it's not 
 * created before. */
eiAPI eiNode *ei_nodesys_node(
	eiNodeSystem *nodesys, 
	const char *node_name, 
	eiBool *need_init);
/** \brief Declare a user parameter specific for the node, returns 
 * the index of the new node parameter. */
eiAPI eiIndex ei_nodesys_declare_parameter(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const eiInt param_storage_class, 
	const eiInt param_type, 
	const char *param_name, 
	const void *default_value);
/** \brief Set a parameter value by name. */
eiAPI void ei_nodesys_set_parameter(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const void *param_value);
/** \brief Link an input parameter to the output parameter of 
 * another node. the value of the linked parameter will depend 
 * on the source parameter. */
eiAPI void ei_nodesys_link_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name, 
    const char *src_node_name, 
    const char *src_param_name);
/** \brief Unlink a parameter. */
eiAPI void ei_nodesys_unlink_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name);
/** \brief Get the number of parameters in a node. */
eiAPI eiUint ei_nodesys_get_parameter_count(
	eiNodeSystem *nodesys, 
	eiNode *node);
/** \brief Begin reading a parameter information by index. */
eiAPI eiNodeParam *ei_nodesys_read_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const eiIndex param_index);
/** \brief Begin writing a parameter information by index. */
eiAPI eiNodeParam *ei_nodesys_write_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const eiIndex param_index);
/** \brief Get the constant parameter value directly from a 
 * node parameter information. */
eiAPI void ei_nodesys_get_parameter_value(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	eiNodeParam *param, 
	void *param_value);
/** \brief Get the constant parameter value directly from a 
 * node parameter information, then cast the value into 
 * another type. */
eiAPI void ei_nodesys_cast_parameter_value(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	eiNodeParam *param, 
	void *param_value, 
	const eiInt type);
/** \brief Get the constant parameter value by index. */
eiAPI void ei_nodesys_get_parameter_at(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const eiIndex param_index, 
	void *param_value);
/** \brief Get the constant parameter value by index, then 
 * cast the value into another type. */
eiAPI void ei_nodesys_cast_parameter_at(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const eiIndex param_index, 
	void *param_value, 
	const eiInt type);
/** \brief Get the constant parameter value by name. this 
 * function will not evaluate connected nodes automatically.
 * this function should NOT be called when editing the node, 
 * because symbol table has not been re-built while editing. */
eiAPI void ei_nodesys_get_parameter(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name, 
	void *param_value);
/** \brief End editing a node. */
eiAPI void ei_nodesys_end_node(
	eiNodeSystem *nodesys, 
	eiNode *node);
/** \brief Delete a node. */
eiAPI void ei_nodesys_delete_node(
	eiNodeSystem *nodesys, 
	const eiTag tag);

/* parameter declarations */
eiAPI void ei_nodesys_add_int(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiInt default_value);
eiAPI void ei_nodesys_add_scalar(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiScalar default_value);
eiAPI void ei_nodesys_add_vector(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiScalar x, const eiScalar y, const eiScalar z);
eiAPI void ei_nodesys_add_vector4(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiScalar x, const eiScalar y, const eiScalar z, const eiScalar w);
eiAPI void ei_nodesys_add_tag(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiTag default_value);
eiAPI void ei_nodesys_add_index(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiIndex default_value);
eiAPI void ei_nodesys_add_bool(
	eiNodeSystem *nodesys, 
	eiNodeDesc *desc, 
	const eiInt param_storage_class, 
	const char *param_name, 
	const eiBool default_value);

/* parameter readers
 * these functions should NOT be called when editing the node, 
 * because symbol table has not been re-built while editing. */
eiAPI eiInt ei_nodesys_get_int(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name);
eiAPI eiScalar ei_nodesys_get_scalar(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name);
eiAPI eiTag ei_nodesys_get_tag(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name);
eiAPI eiIndex ei_nodesys_get_index(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name);
eiAPI eiBool ei_nodesys_get_bool(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	const char *param_name);

/* parameter writers */
eiAPI void ei_nodesys_set_string(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const char *param_value);
eiAPI void ei_nodesys_set_int(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiInt param_value);
eiAPI void ei_nodesys_set_scalar(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiScalar param_value);
eiAPI void ei_nodesys_set_vector(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiScalar x, const eiScalar y, const eiScalar z);
eiAPI void ei_nodesys_set_vector4(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiScalar x, const eiScalar y, const eiScalar z, const eiScalar w);
eiAPI void ei_nodesys_set_tag(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiTag param_value);
eiAPI void ei_nodesys_set_index(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiIndex param_value);
eiAPI void ei_nodesys_set_bool(
	eiNodeSystem *nodesys, 
	eiNode **node, 
	const char *param_name, 
	const eiBool param_value);

/** \brief Generate node desc by re-creating 
 * the node plugin object. for internal use only. */
void generate_node_desc(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls);
/** \brief Delete local data of node desc. for internal 
 * use only. */
void clear_node_desc(eiDatabase *db, void *data);
/** \brief Generate node, build parameter map.
 * for internal use only. */
void generate_node(
	eiDatabase *db, 
	const eiTag data_tag, 
	eiData *pData, 
	eiTLS *pTls);
/** \brief Delete local data of node. for internal 
 * use only. */
void clear_node(eiDatabase *db, void *data);

/* for internal use only */
void byteswap_node_desc(eiDatabase *db, void *data, const eiUint size);
void byteswap_node_param_desc(eiDatabase *db, void *data, const eiUint size);
void byteswap_node(eiDatabase *db, void *data, const eiUint size);
void byteswap_node_param(eiDatabase *db, void *data, const eiUint size);

#ifdef __cplusplus
}
#endif

#endif
