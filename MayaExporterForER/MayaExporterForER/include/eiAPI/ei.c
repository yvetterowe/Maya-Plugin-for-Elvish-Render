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

#include <eiAPI/ei.h>
#include <eiAPI/ei_renderer.h>
#include <eiAPI/ei_scenemgr.h>
#include <eiAPI/ei_shadesys.h>
#include <eiAPI/ei_texture.h>
#include <eiAPI/ei_texmake.h>
#include <eiCORE/ei_data_table.h>
#include <eiCORE/ei_network.h>
#include <eiCORE/ei_assert.h>

#define EI_MIN_BUCKET_SIZE				32
#define EI_MAX_BUCKET_SIZE				96

/** \brief Definitions of pair types */
enum {
	EI_PAIR_NONE = 0, 
	EI_SCENE_PAIR, 
	EI_TAB_PAIR, 
	EI_OPTIONS_PAIR, 
	EI_CAMERA_PAIR, 
	EI_MATERIAL_PAIR, 
	EI_LIGHT_PAIR, 
	EI_OBJECT_PAIR, 
	EI_INSTANCE_PAIR, 
	EI_INSTGROUP_PAIR, 
	EI_PAIR_COUNT, 
};

/** \brief A utility class for checking if a non-nested begin/end function pair is 
 * valid, it can automatically detect irregular calling orders, and can 
 * detect missing calls by invoking check_missing from outer scope at the 
 * end. */
typedef struct eiNonNestedPair {
	eiInt		status;
} eiNonNestedPair;

static void ei_non_nested_pair_init(eiNonNestedPair *pair)
{
	pair->status = eiFALSE;
}

static eiBool ei_non_nested_pair_begin(eiNonNestedPair *pair)
{
	/* check if this pair has begun. */
	if (pair->status == eiTRUE)
	{
		/* report a syntax error */
		return eiFALSE;
	}

	pair->status = eiTRUE;

	return eiTRUE;
}

static eiBool ei_non_nested_pair_end(eiNonNestedPair *pair)
{
	/* check if there's nothing begun. */
	if (pair->status == eiFALSE)
	{
		/* report a syntax error */
		return eiFALSE;
	}

	pair->status = eiFALSE;

	return eiTRUE;
}

static eiBool ei_non_nested_pair_check_missing(eiNonNestedPair *pair)
{
	if (pair->status == eiFALSE)
	{
		return eiFALSE;
	}
	else
	{
		/* report a syntax error */
		return eiTRUE;
	}
}

/* if it's inside the pair */
static eiBool ei_non_nested_pair_inside(eiNonNestedPair *pair)
{
	if (pair->status == eiTRUE)
	{
		return eiTRUE;
	}
	else
	{
		return eiFALSE;
	}
}

/** \brief a graphics state context for the interface. */
typedef struct eiContext {
	eiRenderer			*renderer;
	eiNodeSystem		*nodesys;
	eiNonNestedPair		shader_decl_pair;
	eiNodeDesc			*current_shader_decl;
	eiTag				current_shader_decl_tag;
	eiNonNestedPair		node_pair;
	eiNode				*current_node;
	eiNonNestedPair		tab_pair;
	eiTag				current_tab;
	eiNonNestedPair		output_pair;
	eiOutput			current_output;
} eiContext;

/** \brief Initialize the state context. */
static void ei_context_init(eiContext *context)
{
	context->renderer = NULL;
	context->nodesys = NULL;

	ei_non_nested_pair_init(&context->shader_decl_pair);
	context->current_shader_decl = NULL;
	context->current_shader_decl_tag = eiNULL_TAG;
	ei_non_nested_pair_init(&context->node_pair);
	context->current_node = NULL;
	ei_non_nested_pair_init(&context->tab_pair);
	context->current_tab = eiNULL_TAG;
	ei_non_nested_pair_init(&context->output_pair);
}

/** \brief Cleanup the state context. */
static void ei_context_exit(eiContext *context)
{
	/* check statement missings */
	ei_non_nested_pair_check_missing(&context->shader_decl_pair);
	ei_non_nested_pair_check_missing(&context->node_pair);
	ei_non_nested_pair_check_missing(&context->tab_pair);
	ei_non_nested_pair_check_missing(&context->output_pair);
}

/* global rendering context */
static eiContext	*g_Context = NULL;

/** \brief Store the connection pointer, this will be called by the renderer. */
void ei_connection(eiConnection *con)
{
	ei_renderer_set_connection(g_Context->renderer, con);
}

/** \brief Create a new rendering context, return the context handle. */
eiContext *ei_create_context()
{
	eiContext *context = (eiContext *)ei_allocate(sizeof(eiContext));

	ei_context_init(context);

	/* create the renderer */
	context->renderer = ei_create_renderer(context);
	context->nodesys = ei_renderer_node_system(context->renderer);

	return context;
}

/** \brief Delete a rendering context. */
void ei_delete_context(eiContext *context)
{
	if (context == NULL)
	{
		eiASSERT(0);
		return;
	}

	/* create the renderer */
	ei_delete_renderer(context->renderer);

	ei_context_exit(context);

	eiCHECK_FREE(context);
}

/** \brief Create a new rendering context, return the context handle. */
eiContext *ei_create_server_context(eiGlobals *globals)
{
	eiContext *context = (eiContext *)ei_allocate(sizeof(eiContext));

	ei_context_init(context);

	context->nodesys = (eiNodeSystem *)globals->interfaces[ EI_INTERFACE_TYPE_NODE_SYSTEM ];

	return context;
}

/** \brief Delete a rendering context. */
void ei_delete_server_context(eiContext *context)
{
	if (context == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_context_exit(context);

	eiCHECK_FREE(context);
}

/** \brief Get current rendering context. */
eiContext *ei_current_context()
{
	return g_Context;
}

/** \brief Set current rendering context, return previous rendering 
 * context for backup. */
eiContext *ei_context(eiContext *context)
{
	eiContext	*prev_context;

	prev_context = g_Context;
	g_Context = context;

	return prev_context;
}

eiDatabase *ei_context_database(eiContext *context)
{
	return context->nodesys->m_db;
}

/** \brief Declare a shader class. if any error detected in 
 * the parameter list, parsing will stop immediately. defaults 
 * are forced to be specified. */
void ei_declare_shader(const char *shader_name)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_begin(&g_Context->shader_decl_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;
	g_Context->current_shader_decl_tag = eiNULL_TAG;

	g_Context->current_shader_decl = ei_nodesys_node_desc(
		nodesys, 
		&g_Context->current_shader_decl_tag, 
		shader_name);
}

/** \brief Declare a shader parameter. */
void ei_declare_shader_param(
	const eiInt param_type, 
	const char *param_name, 
	const void *default_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->shader_decl_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_add_parameter(
		nodesys, 
		g_Context->current_shader_decl, 
		eiCONSTANT, 
		param_type, 
		param_name, 
		default_value);
}

void ei_declare_shader_param_int(
	const char *param_name, 
	const eiInt default_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->shader_decl_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_add_int(
		nodesys, 
		g_Context->current_shader_decl, 
		eiCONSTANT, 
		param_name, 
		default_value);
}

void ei_declare_shader_param_scalar(
	const char *param_name, 
	const eiScalar default_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->shader_decl_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_add_scalar(
		nodesys, 
		g_Context->current_shader_decl, 
		eiCONSTANT, 
		param_name, 
		default_value);
}

void ei_declare_shader_param_vector(
	const char *param_name, 
	const eiScalar x, const eiScalar y, const eiScalar z)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->shader_decl_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_add_vector(
		nodesys, 
		g_Context->current_shader_decl, 
		eiCONSTANT, 
		param_name, 
		x, y, z);
}

void ei_declare_shader_param_vector4(
	const char *param_name, 
	const eiScalar x, const eiScalar y, const eiScalar z, const eiScalar w)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->shader_decl_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_add_vector4(
		nodesys, 
		g_Context->current_shader_decl, 
		eiCONSTANT, 
		param_name, 
		x, y, z, w);
}

void ei_declare_shader_param_tag(
	const char *param_name, 
	const eiTag default_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->shader_decl_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_add_tag(
		nodesys, 
		g_Context->current_shader_decl, 
		eiCONSTANT, 
		param_name, 
		default_value);
}

void ei_declare_shader_param_index(
	const char *param_name, 
	const eiIndex default_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->shader_decl_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_add_index(
		nodesys, 
		g_Context->current_shader_decl, 
		eiCONSTANT, 
		param_name, 
		default_value);
}

void ei_declare_shader_param_bool(
	const char *param_name, 
	const eiBool default_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->shader_decl_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_add_bool(
		nodesys, 
		g_Context->current_shader_decl, 
		eiCONSTANT, 
		param_name, 
		default_value);
}

/** \brief Finish declaring a shader class. */
void ei_end_declare_shader()
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_end(&g_Context->shader_decl_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_end_node_desc(
		nodesys, 
		g_Context->current_shader_decl, 
		g_Context->current_shader_decl_tag);

	g_Context->current_shader_decl = NULL;
	g_Context->current_shader_decl_tag = eiNULL_TAG;
}

/** \brief Create a new shader instance from a shader class, 
 * or modify parameter values of an existing shader instance. */
void ei_shader(const char *instance_name)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_begin(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	g_Context->current_node = ei_shader_instance(
		nodesys, 
		instance_name);
}

/** \brief Set a shader parameter value by name. */
void ei_shader_param(
	const char *param_name, 
    const void *param_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_set_parameter(
		nodesys, 
		&g_Context->current_node, 
		param_name, 
		param_value);
}

eiAPI void ei_shader_param_string(
	const char *param_name, 
    const char *param_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_set_string(
		nodesys, 
		&g_Context->current_node, 
		param_name, 
		param_value);
}

void ei_shader_param_int(
	const char *param_name, 
    const eiInt param_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_set_int(
		nodesys, 
		&g_Context->current_node, 
		param_name, 
		param_value);
}

void ei_shader_param_scalar(
	const char *param_name, 
    const eiScalar param_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_set_scalar(
		nodesys, 
		&g_Context->current_node, 
		param_name, 
		param_value);
}

void ei_shader_param_vector(
	const char *param_name, 
    const eiScalar x, const eiScalar y, const eiScalar z)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_set_vector(
		nodesys, 
		&g_Context->current_node, 
		param_name, 
		x, y, z);
}

void ei_shader_param_vector4(
	const char *param_name, 
    const eiScalar x, const eiScalar y, const eiScalar z, const eiScalar w)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_set_vector4(
		nodesys, 
		&g_Context->current_node, 
		param_name, 
		x, y, z, w);
}

void ei_shader_param_tag(
	const char *param_name, 
	const eiTag param_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_set_tag(
		nodesys, 
		&g_Context->current_node, 
		param_name, 
		param_value);
}

void ei_shader_param_texture(
	const char *param_name, 
	const char *texture_name)
{
	eiNodeSystem	*nodesys;
	eiTag			tex_tag;
	eiTexture		*tex;
	eiTag			texmap;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	tex_tag = ei_nodesys_find_node(nodesys, texture_name);

	if (tex_tag == eiNULL_TAG)
	{
		ei_error("Cannot find texture %s\n", texture_name);
		return;
	}

	/* get the tag of the underlying texture map */
	tex = (eiTexture *)ei_db_access(nodesys->m_db, tex_tag);
	texmap = tex->tag;
	ei_db_end(nodesys->m_db, tex_tag);

	ei_nodesys_set_tag(
		nodesys, 
		&g_Context->current_node, 
		param_name, 
		texmap);
}

void ei_shader_param_index(
	const char *param_name, 
	const eiIndex param_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_set_index(
		nodesys, 
		&g_Context->current_node, 
		param_name, 
		param_value);
}

void ei_shader_param_bool(
	const char *param_name, 
	const eiBool param_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_set_bool(
		nodesys, 
		&g_Context->current_node, 
		param_name, 
		param_value);
}

/** \brief Link the output parameter of another shader to the 
 * current shader parameter by name. */
eiAPI void ei_shader_link_param(
    const char *param_name, 
    const char *src_shader_name, 
    const char *src_param_name)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_link_parameter(
		nodesys, 
		g_Context->current_node, 
		param_name, 
		src_shader_name, 
		src_param_name);
}

/** \brief Finish editing a shader instance. */
void ei_end_shader()
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_end(&g_Context->node_pair))
	{
		return;
	}

	/* we don't describe shader from server side */
	if (g_Context->renderer == NULL)
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_end_shader_instance(
		nodesys, 
		g_Context->current_node);

	/* dirt the parameter table automatically */
	ei_db_dirt(nodesys->m_db, g_Context->current_node->tag);

	g_Context->current_node = NULL;
}

static eiBool link_message_proc(eiDatabase *db, SOCKET sock, void *param)
{
	char	*module_name;

	module_name = (char *)param;

	ei_net_send(sock, (eiByte *)module_name, EI_MAX_FILE_NAME_LEN);

	return eiTRUE;
}

/** \brief Link a DSO to the renderer for shader lookups, 
 * add the DSO to the scene. */
void ei_link(const char *filename)
{
	eiNodeSystem	*nodesys;
	eiPluginSystem	*plugsys;
	eiMaster		*master;
	eiMessage		req;
	char			module_name[ EI_MAX_FILE_NAME_LEN ];

	nodesys = g_Context->nodesys;
	plugsys = ei_nodesys_plugin_system(nodesys);

	ei_plugsys_link(plugsys, filename);

	/* synchronize the linking */
	master = ei_db_net_master(nodesys->m_db);

	strncpy(module_name, filename, EI_MAX_FILE_NAME_LEN - 1);

	req.type = EI_MSG_REQ_LINK;

	ei_master_broadcast(master, &req, 0, link_message_proc, (void *)module_name);
}

/** \brief Set the verbose level. */
void ei_verbose(const eiInt level)
{
	ei_verbose_set(level);
}

/** \brief Delete a named scene element, such as objects, materials, lights, 
 * textures, instances, and instance groups. */
void ei_delete(const char *element_name)
{
	eiNodeSystem	*nodesys;
	eiTag			tag;

	nodesys = g_Context->nodesys;

	tag = ei_nodesys_find_node(nodesys, element_name);

	if (tag == eiNULL_TAG)
	{
		return;
	}

	ei_nodesys_delete_node(nodesys, tag);
}

/** \brief This command renders the scene. the root instance group, a camera instance 
 * element (which must also have been attached to the root instance group), 
 * and the name of an options element must be given. */
void ei_render(const char *root_instgroup, const char *camera_inst, const char *options)
{
	eiNodeSystem	*nodesys;
	eiTag			root_instgrp_tag;
	eiTag			cam_inst_tag;
	eiTag			opt_tag;

	nodesys = g_Context->nodesys;

	root_instgrp_tag = ei_nodesys_find_node(nodesys, root_instgroup);

	if (root_instgrp_tag == eiNULL_TAG)
	{
		/* error */
		return;
	}

	/* the camera instance must be attached to the root instance group. */
	cam_inst_tag = ei_nodesys_find_node(nodesys, camera_inst);

	if (cam_inst_tag == eiNULL_TAG)
	{
		/* error */
		return;
	}

	opt_tag = ei_nodesys_find_node(nodesys, options);

	if (opt_tag == eiNULL_TAG)
	{
		/* error */
		return;
	}

	/* start rendering */
	ei_renderer_render(g_Context->renderer, root_instgrp_tag, cam_inst_tag, opt_tag);
}

void ei_declare(const char *name, const eiInt storage_class, const eiInt type, const void *default_value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_declare_parameter(
		nodesys, 
		&g_Context->current_node, 
		storage_class, 
		type, 
		name, 
		default_value);
}

void ei_variable(const char *name, const void *value)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_set_parameter(
		nodesys, 
		&g_Context->current_node, 
		name, 
		value);
}

void ei_node(const char *name, const char *type)
{
	eiNodeSystem	*nodesys;
	eiBool			need_init;

	if (!ei_non_nested_pair_begin(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	g_Context->current_node = ei_nodesys_node(
		nodesys, 
		name, 
		&need_init);

	if (need_init)
	{
		/* set node description to built-in class */
		ei_nodesys_set_string(nodesys, 
			&g_Context->current_node, 
			"desc", 
			type);
	}
}

void ei_end_node()
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_end(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	/* dirt the parameter table automatically */
	ei_db_dirt(nodesys->m_db, g_Context->current_node->tag);

	ei_nodesys_end_node(
		nodesys, 
		g_Context->current_node);

	g_Context->current_node = NULL;
}

eiTag ei_tab(const eiInt type, const eiInt items_per_slot)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_begin(&g_Context->tab_pair))
	{
		return eiNULL_TAG;
	}

	nodesys = g_Context->nodesys;

	g_Context->current_tab = ei_create_data_table(
		nodesys->m_db, 
		type, 
		items_per_slot);

	return g_Context->current_tab;
}

void ei_tab_add(const void *value)
{
	eiNodeSystem	*nodesys;
	eiDataTable		*tab;

	if (!ei_non_nested_pair_inside(&g_Context->tab_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	tab = (eiDataTable *)ei_db_access(nodesys->m_db, g_Context->current_tab);
	ei_data_table_push_back(nodesys->m_db, &tab, value);
	ei_db_end(nodesys->m_db, g_Context->current_tab);
}

void ei_tab_add_int(const eiInt value)
{
	ei_tab_add(&value);
}

void ei_tab_add_uint(const eiUint value)
{
	ei_tab_add(&value);
}

void ei_tab_add_scalar(const eiScalar value)
{
	ei_tab_add(&value);
}

void ei_tab_add_vector(const eiScalar x, const eiScalar y, const eiScalar z)
{
	eiVector	vec;
	setv(&vec, x, y, z);
	ei_tab_add(&vec);
}

void ei_tab_add_vector4(const eiScalar x, const eiScalar y, const eiScalar z, const eiScalar w)
{
	eiVector4	vec;
	setv4(&vec, x, y, z, w);
	ei_tab_add(&vec);
}

void ei_tab_add_tag(const eiTag value)
{
	ei_tab_add(&value);
}

void ei_tab_add_index(const eiIndex value)
{
	ei_tab_add(&value);
}

void ei_tab_add_bool(const eiBool value)
{
	ei_tab_add(&value);
}

void ei_end_tab()
{
	if (!ei_non_nested_pair_end(&g_Context->tab_pair))
	{
		return;
	}

	g_Context->current_tab = eiNULL_TAG;
}

/** \brief Begin describing an options, an options can be passed to "render" 
 * function to specify global rendering settings. */
void ei_options(const char *name)
{
	ei_node(name, "options");
}

/** \brief The contrast controls oversampling. If neighboring samples differ by 
 * more than the color, supersampling is done as specified by the sampling 
 * parameters. */
void ei_contrast(eiScalar r, eiScalar g, eiScalar b, eiScalar a)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	setv4(&opt->contrast, r, g, b, a);
}

/** \brief This statement determines the minimum and maximum sample rate. */
void ei_samples(eiInt min, eiInt max)
{
	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	if (g_Context->current_node->type == EI_ELEMENT_OPTIONS)
	{
		eiOptions	*opt;

		opt = (eiOptions *)g_Context->current_node;

		opt->min_samples = min;
		opt->max_samples = max;
	}
	else if (g_Context->current_node->type == EI_ELEMENT_INSTANCE)
	{
		eiInstance	*inst;

		inst = (eiInstance *)g_Context->current_node;

		inst->attr.min_samples = min;
		inst->attr.max_samples = max;
	}
	else
	{
		ei_error("Current node does not support this statement.\n");
	}
}

/** \brief Set the bucket size of tiled rendering. */
void ei_bucket_size(eiInt size)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	clampi(size, EI_MIN_BUCKET_SIZE, EI_MAX_BUCKET_SIZE);

	opt = (eiOptions *)g_Context->current_node;

	opt->bucket_size = size;
}

/** \brief The filter statement specifies how multiple samples are to 
* be combined into a single pixel value. */
void ei_filter(eiInt filter, eiScalar size)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->filter = filter;
	opt->filter_size = size;
}

/** \brief This statment specifies the geometric approximation settings. */
void ei_approx(const eiApprox *approx)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	if (g_Context->current_node->type == EI_ELEMENT_OPTIONS)
	{
		eiOptions	*opt;
		eiApprox	*opt_approx;

		opt = (eiOptions *)g_Context->current_node;

		opt_approx = (eiApprox *)ei_db_access(nodesys->m_db, opt->approx);
		ei_approx_copy(opt_approx, approx);
		ei_db_end(nodesys->m_db, opt->approx);
	}
	else if (g_Context->current_node->type == EI_ELEMENT_INSTANCE)
	{
		eiInstance	*inst;

		inst = (eiInstance *)g_Context->current_node;

		ei_attr_set_approx(&inst->attr, approx, nodesys->m_db);
	}
	else
	{
		ei_error("Current node does not support this statement.\n");
	}
}

/** \brief This statement overrides all max displace statements in 
 * displacement-mapped objects with the maximum displacement distance dist. */
void ei_max_displace(eiScalar dist)
{
	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	if (g_Context->current_node->type == EI_ELEMENT_OPTIONS)
	{
		eiOptions	*opt;

		opt = (eiOptions *)g_Context->current_node;

		opt->max_displace = dist;
	}
	else if (g_Context->current_node->type == EI_ELEMENT_INSTANCE)
	{
		eiInstance	*inst;

		inst = (eiInstance *)g_Context->current_node;

		inst->attr.max_displace = dist;
	}
	else
	{
		ei_error("Current node does not support this statement.\n");
	}
}

/** \brief This statement controls motion blurring. The camera shutter 
 * opens at time "open" and closes at time "close". */
void ei_shutter(eiScalar open, eiScalar close)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->shutter_open = open;
	opt->shutter_close = close;
}

/** \brief The motion statement turns motion blurring on or off explicitly. */
void ei_motion(eiInt type)
{
	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	if (g_Context->current_node->type == EI_ELEMENT_OPTIONS)
	{
		eiOptions	*opt;

		opt = (eiOptions *)g_Context->current_node;

		opt->motion = type;
	}
	else if (g_Context->current_node->type == EI_ELEMENT_INSTANCE)
	{
		eiInstance	*inst;

		inst = (eiInstance *)g_Context->current_node;

		ei_attr_set_flag(&inst->attr, EI_ATTR_MOTION_BLUR, type);
	}
	else
	{
		ei_error("Current node does not support this statement.\n");
	}
}

/** \brief This option specifies how many motion path segments should be 
 * created for all motion transforms in the scene. */
void ei_motion_segments(eiInt num)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->motion_segments = num;
}

/** \brief The reflect parameter limits the number of recursive reflection rays. 
 * refract controls the maximum depth of refraction and transparency rays. 
 * Sum limits the sum of reflection and refraction rays. */
void ei_trace_depth(eiInt reflect, eiInt refract, eiInt sum)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->trace_reflect_depth = reflect;
	opt->trace_refract_depth = refract;
	opt->trace_sum_depth = sum;
}

/** \brief Selects the mode of accleration for ray-tracing. */
void ei_acceleration(eiInt type)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->acceleration = type;
}

/** \brief The maximum number of primitives in a leaf of the BSP tree. */
void ei_bsp_size(eiInt size)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->bsp_size = size;
}

/** \brief The maximum number of levels in the BSP tree. */
void ei_bsp_depth(eiInt depth)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->bsp_depth = depth;
}

/** \brief Ignore all lens shaders if set to off. */
void ei_lens(eiInt type)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->lens = type;
}

/** \brief This statement specifies the mode of shadows. */
void ei_shadow(eiInt type)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->shadow = type;
}

/** \brief Ignore all volume shaders if set to off. */
void ei_volume(eiInt type)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->volume = type;
}

/** \brief Ignore all geometry shaders if set to off. */
void ei_geometry(eiInt type)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->geometry = type;
}

/** \brief Ignore all displacement shaders if set to off. */
void ei_displace(eiInt type)
{
	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	if (g_Context->current_node->type == EI_ELEMENT_OPTIONS)
	{
		eiOptions	*opt;

		opt = (eiOptions *)g_Context->current_node;

		opt->displace = type;
	}
	else if (g_Context->current_node->type == EI_ELEMENT_INSTANCE)
	{
		eiInstance	*inst;

		inst = (eiInstance *)g_Context->current_node;

		ei_attr_set_flag(&inst->attr, EI_ATTR_DISPLACE, type);
	}
	else
	{
		ei_error("Current node does not support this statement.\n");
	}
}

/** \brief Ignore all imager shaders if set to off. */
void ei_imager(eiInt type)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->imager = type;
}

void ei_caustic(eiInt type)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->caustic = type;
}

void ei_caustic_photons(eiInt photons)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->caustic_photons = photons;
}

void ei_caustic_accuracy(eiInt samples, eiScalar radius)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->caustic_samples = samples;
	opt->caustic_radius = radius;
}

void ei_caustic_scale(eiScalar r, eiScalar g, eiScalar b)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	setv(&opt->caustic_scale, r, g, b);
}

void ei_caustic_filter(eiInt filter, eiScalar filter_const)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->caustic_filter = filter;
	opt->caustic_filter_const = filter_const;
}

void ei_photon_trace_depth(eiInt reflect, eiInt refract, eiInt sum)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->photon_reflect_depth = reflect;
	opt->photon_refract_depth = refract;
	opt->photon_sum_depth = sum;
}

void ei_photon_decay(eiScalar decay)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->photon_decay = decay;
}

void ei_globillum(eiInt type)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->globillum = type;
}

void ei_globillum_photons(eiInt photons)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->globillum_photons = photons;
}

void ei_globillum_accuracy(eiInt samples, eiScalar radius)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->globillum_samples = samples;
	opt->globillum_radius = radius;
}

void ei_globillum_scale(eiScalar r, eiScalar g, eiScalar b)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	setv(&opt->globillum_scale, r, g, b);
}

void ei_photonvol_accuracy(eiInt samples, eiScalar radius)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->photonvol_samples = samples;
	opt->photonvol_radius = radius;
}

void ei_finalgather(eiInt type)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->finalgather = type;
}

void ei_finalgather_accuracy(eiInt rays, eiInt samples, eiScalar density, eiScalar radius)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->finalgather_rays = rays;
	opt->finalgather_samples = samples;
	opt->finalgather_density = density;
	opt->finalgather_radius = radius;
}

void ei_finalgather_falloff(eiInt type)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->finalgather_falloff = type;
}

void ei_finalgather_falloff_range(eiScalar start, eiScalar stop)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->finalgather_falloff_start = start;
	opt->finalgather_falloff_stop = stop;
}

void ei_finalgather_filter(eiScalar size)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->finalgather_filter_size = size;
}

void ei_finalgather_trace_depth(eiInt reflect, eiInt refract, eiInt diffuse, eiInt sum)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->finalgather_reflect_depth = reflect;
	opt->finalgather_refract_depth = refract;
	opt->finalgather_diffuse_bounces = diffuse;
	opt->finalgather_sum_depth = sum;
}

void ei_finalgather_scale(eiScalar r, eiScalar g, eiScalar b)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	setv(&opt->finalgather_scale, r, g, b);
}

/** \brief Gamma correction can be applied to rendered and quantized color 
 * pixels to compensate for output devices with a nonlinear color response. */
void ei_exposure(eiScalar gain, eiScalar gamma)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->exposure_gain = gain;
	opt->exposure_gamma = gamma;
}

/** \brief The value one defines the mapping from floating-point values to fixed 
 * point values. Dithering is performed by adding a random number to the 
 * floating-point values before they are rounded to the nearest integer. 
 * The added value is scaled to lie between plus and minus the 
 * dither_amplitude. If dither_amplitude is 0, dithering is turned off. */
void ei_quantize(eiScalar one, eiScalar min, eiScalar max, eiScalar dither_amplitude)
{
	eiOptions	*opt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	opt = (eiOptions *)g_Context->current_node;

	opt->quantize_one = one;
	opt->quantize_min = min;
	opt->quantize_max = max;
	opt->quantize_dither_amplitude = dither_amplitude;
}

/** \brief Whether the front side, the back side or both of a face is visible. */
void ei_face(eiInt type)
{
	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	if (g_Context->current_node->type == EI_ELEMENT_OPTIONS)
	{
		eiOptions	*opt;

		opt = (eiOptions *)g_Context->current_node;

		opt->face = type;
	}
	else if (g_Context->current_node->type == EI_ELEMENT_INSTANCE)
	{
		eiInstance	*inst;

		inst = (eiInstance *)g_Context->current_node;

		inst->attr.face = type;
	}
	else
	{
		ei_error("Current node does not support this statement.\n");
	}
}

void ei_end_options()
{
	ei_end_node();
}

/** \brief Begin describing a camera, an instance of camera can be passed to 
 * "render" function to specify the camera setting for the frame. */
void ei_camera(const char *name)
{
	eiNodeSystem	*nodesys;
	eiCamera		*cam;

	ei_node(name, "camera");

	nodesys = g_Context->nodesys;
	cam = (eiCamera *)g_Context->current_node;

	/* clear all outputs for each new incremental change, thus 
	   we can re-define outputs for every new frame */
	ei_camera_clear_outputs(cam, nodesys->m_db);
}

/** \brief This statement specifies an output file, more than one output file can 
 * be created by multiple output statements. */
void ei_output(const char *filename, const char *fileformat, const eiInt datatype)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	if (!ei_non_nested_pair_begin(&g_Context->output_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_output_init(&g_Context->current_output, 
		filename, fileformat, datatype, nodesys->m_db);
}

/** \brief Specify a shader parameter to be output for current output statement. */
void ei_output_variable(const char *name, const eiInt datatype)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->output_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_output_add_variable(&g_Context->current_output, name, datatype, nodesys->m_db);
}

/** \brief Finish current output statement. */
void ei_end_output()
{
	eiNodeSystem	*nodesys;
	eiCamera		*cam;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	if (!ei_non_nested_pair_end(&g_Context->output_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;
	cam = (eiCamera *)g_Context->current_node;

	ei_camera_add_output(cam, nodesys->m_db, &g_Context->current_output);
}

/** \brief The focal distance is set to distance. The focal distance is the distance 
 * from the camera to the viewing plane. If infinity is used in place of the 
 * distance, an orthographic view is rendered. */
void ei_focal(eiScalar distance)
{
	eiCamera	*cam;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	cam = (eiCamera *)g_Context->current_node;

	cam->focal = distance;
}

/** \brief The aperture is the width of the viewing plane. */
void ei_aperture(eiScalar aperture)
{
	eiCamera	*cam;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	cam = (eiCamera *)g_Context->current_node;

	cam->aperture = aperture;
}

/** \brief This is the aspect ratio of the camera. */
void ei_aspect(eiScalar aspect)
{
	eiCamera	*cam;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	cam = (eiCamera *)g_Context->current_node;

	cam->aspect = aspect;
}

/** \brief Specifies the width and height of the output image in pixels. */
void ei_resolution(eiInt x, eiInt y)
{
	eiCamera	*cam;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	cam = (eiCamera *)g_Context->current_node;

	cam->res_x = x;
	cam->res_y = y;
}

/** \brief Only the sub-rectangle of the image specified by the four bounds 
 * will be rendered. */
void ei_window(eiInt xmin, eiInt xmax, eiInt ymin, eiInt ymax)
{
	eiCamera	*cam;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	cam = (eiCamera *)g_Context->current_node;

	if (xmin > xmax)
	{
		cam->window_xmin = xmax;
		cam->window_xmax = xmin;
	}
	else
	{
		cam->window_xmin = xmin;
		cam->window_xmax = xmax;
	}
	if (ymin > ymax)
	{
		cam->window_ymin = ymax;
		cam->window_ymax = ymin;
	}
	else
	{
		cam->window_ymin = ymin;
		cam->window_ymax = ymax;
	}
}

/** \brief The hither (near) and yon (far) planes are planes parallel to the viewing 
 * plane that delimit the rendered scene. */
void ei_clip(eiScalar hither, eiScalar yon)
{
	eiCamera	*cam;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	cam = (eiCamera *)g_Context->current_node;

	cam->clip_hither = hither;
	cam->clip_yon = yon;
}

/** \brief A lens shader accepts the origin and direction of the camera ray, modifies 
 * them, and casts a new primary ray. */
void ei_add_lens(const char *shader_name)
{
	eiNodeSystem	*nodesys;
	eiCamera		*cam;
	eiTag			shader;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	shader = ei_nodesys_find_node(nodesys, shader_name);

	if (shader == eiNULL_TAG)
	{
		ei_error("Cannot find shader %s\n", shader_name);
		return;
	}

	cam = (eiCamera *)g_Context->current_node;

	ei_camera_add_lens(cam, nodesys->m_db, shader);
}

/** \brief The imager statement calls a list of imager shaders, that may operate on 
 * all available frame buffers or shader parameters. */
void ei_add_imager(const char *shader_name)
{
	eiNodeSystem	*nodesys;
	eiCamera		*cam;
	eiTag			shader;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	shader = ei_nodesys_find_node(nodesys, shader_name);

	if (shader == eiNULL_TAG)
	{
		ei_error("Cannot find shader %s\n", shader_name);
		return;
	}

	cam = (eiCamera *)g_Context->current_node;

	ei_camera_add_imager(cam, nodesys->m_db, shader);
}

/** \brief End describing the current camera. */
void ei_end_camera()
{
	ei_end_node();
}

/** \brief Begin describing a material, materials determine the look of geometric 
 * objects, and can be attached to instances of geometric objects. */
void ei_material(const char *name)
{
	ei_node(name, "material");
}

/** \brief The surface shader then calculates a color (and certain other optional 
 * values such as labels, Z depths, and normal vectors that can be written 
 * to special output files). This color may then be modified by the optional 
 * volume shader if present. The resulting color is stored in the output 
 * frame buffer. In order to calculate the color, the surface shader may 
 * cast secondary reflection, refraction, or transparency rays, which in 
 * turn may hit objects and cause other surface shaders to be called. */
void ei_add_surface(const char *shader_name)
{
	eiNodeSystem	*nodesys;
	eiMaterial		*mtl;
	eiTag			shader;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	shader = ei_nodesys_find_node(nodesys, shader_name);

	if (shader == eiNULL_TAG)
	{
		ei_error("Cannot find shader %s\n", shader_name);
		return;
	}

	mtl = (eiMaterial *)g_Context->current_node;

	ei_material_add_surface(mtl, nodesys->m_db, shader);
}

/** \brief The displacement shader is a function returning a scalar that displaces 
 * the object's surface in the direction of its normal, or in arbitrary 
 * directions. */
void ei_add_displace(const char *shader_name)
{
	eiNodeSystem	*nodesys;
	eiMaterial		*mtl;
	eiTag			shader;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	shader = ei_nodesys_find_node(nodesys, shader_name);

	if (shader == eiNULL_TAG)
	{
		ei_error("Cannot find shader %s\n", shader_name);
		return;
	}

	mtl = (eiMaterial *)g_Context->current_node;

	ei_material_add_displace(mtl, nodesys->m_db, shader);
}

/** \brief The shadow shader is called when a shadow calculation is done, and the 
 * shadow ray from the light source towards the point in shadow intersects 
 * with this material. The shadow shader then changes the color of the ray. */
void ei_add_shadow(const char *shader_name)
{
	eiNodeSystem	*nodesys;
	eiMaterial		*mtl;
	eiTag			shader;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	shader = ei_nodesys_find_node(nodesys, shader_name);

	if (shader == eiNULL_TAG)
	{
		ei_error("Cannot find shader %s\n", shader_name);
		return;
	}

	mtl = (eiMaterial *)g_Context->current_node;

	ei_material_add_shadow(mtl, nodesys->m_db, shader);
}

/** \brief A volume shader affects rays traveling inside an object. When a ray 
 * (either from the eye or from a light source) hits this material, the 
 * volume shader, if present, is called to change the color returned by the 
 * ray based on the distance the ray has traveled, and atmospheric or 
 * material parameters. */
void ei_add_volume(const char *shader_name)
{
	eiNodeSystem	*nodesys;
	eiTag			shader;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	shader = ei_nodesys_find_node(nodesys, shader_name);

	if (shader == eiNULL_TAG)
	{
		ei_error("Cannot find shader %s\n", shader_name);
		return;
	}

	if (g_Context->current_node->type == EI_ELEMENT_MATERIAL)
	{
		eiMaterial		*mtl;

		mtl = (eiMaterial *)g_Context->current_node;
		
		ei_material_add_volume(mtl, nodesys->m_db, shader);
	}
	else if (g_Context->current_node->type == EI_ELEMENT_CAMERA)
	{
		eiCamera		*cam;

		cam = (eiCamera *)g_Context->current_node;
		
		ei_camera_add_volume(cam, nodesys->m_db, shader);
	}
	else
	{
		ei_error("Current node does not support this statement.\n");
	}
}

/** \brief The environment shader is called when a reflection or refraction ray cast 
 * by the surface shader leaves the scene entirely without striking another 
 * object. */
void ei_add_environment(const char *shader_name)
{
	eiNodeSystem	*nodesys;
	eiTag			shader;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	shader = ei_nodesys_find_node(nodesys, shader_name);

	if (shader == eiNULL_TAG)
	{
		ei_error("Cannot find shader %s\n", shader_name);
		return;
	}

	if (g_Context->current_node->type == EI_ELEMENT_MATERIAL)
	{
		eiMaterial		*mtl;

		mtl = (eiMaterial *)g_Context->current_node;
		
		ei_material_add_environment(mtl, nodesys->m_db, shader);
	}
	else if (g_Context->current_node->type == EI_ELEMENT_CAMERA)
	{
		eiCamera		*cam;

		cam = (eiCamera *)g_Context->current_node;
		
		ei_camera_add_environment(cam, nodesys->m_db, shader);
	}
	else
	{
		ei_error("Current node does not support this statement.\n");
	}
}

/** \brief The photon shader is called when a photon ray hits a surface. */
void ei_add_photon(const char *shader_name)
{
	eiNodeSystem	*nodesys;
	eiTag			shader;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	shader = ei_nodesys_find_node(nodesys, shader_name);

	if (shader == eiNULL_TAG)
	{
		ei_error("Cannot find shader %s\n", shader_name);
		return;
	}

	if (g_Context->current_node->type == EI_ELEMENT_MATERIAL)
	{
		eiMaterial		*mtl;

		mtl = (eiMaterial *)g_Context->current_node;
		
		ei_material_add_photon(mtl, nodesys->m_db, shader);
	}
	else
	{
		ei_error("Current node does not support this statement.\n");
	}
}

/** \brief End describing the current material. */
void ei_end_material()
{
	ei_end_node();
}

void ei_make_texture(
	const char *picturename, const char *texturename, 
	eiInt swrap, eiInt twrap, eiInt filter, eiScalar swidth, eiScalar twidth)
{
	eiPluginSystem	*plugsys;

	plugsys = ei_nodesys_plugin_system(g_Context->nodesys);

	ei_make_texture_imp(
		plugsys, 
		picturename, 
		texturename, 
		swrap, twrap, 
		filter, 
		swidth, twidth);
}

/** \brief Begin describing a texture. */
void ei_texture(const char *name)
{
	ei_node(name, "texture");
}

/** \brief Create texture from file. */
void ei_file_texture(const char *filename, const eiBool local)
{
	eiNodeSystem	*nodesys;
	eiTexture		*tex;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	tex = (eiTexture *)g_Context->current_node;

	ei_texture_create_from_file(tex, nodesys->m_db, filename, local);
}

/** \brief End describing the current texture. */
void ei_end_texture()
{
	ei_end_node();
}

/** \brief Begin describing a light, lights can be multiple instanced to illuminate 
 * the scene. */
void ei_light(const char *name)
{
	ei_node(name, "light");
}

/** \brief This statement specifies a list of lightsource shader instances for the 
 * current light. */
void ei_add_light(const char *shader_name)
{
	eiNodeSystem	*nodesys;
	eiLight			*lgt;
	eiTag			shader;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	shader = ei_nodesys_find_node(nodesys, shader_name);

	if (shader == eiNULL_TAG)
	{
		ei_error("Cannot find shader %s\n", shader_name);
		return;
	}

	lgt = (eiLight *)g_Context->current_node;

	ei_light_add_light(lgt, nodesys->m_db, shader);
}

/** \brief This statement specifies a list of photon emitter shader 
 * instances for the current light. */
void ei_add_emitter(const char *shader_name)
{
	eiNodeSystem	*nodesys;
	eiLight			*lgt;
	eiTag			shader;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	shader = ei_nodesys_find_node(nodesys, shader_name);

	if (shader == eiNULL_TAG)
	{
		ei_error("Cannot find shader %s\n", shader_name);
		return;
	}

	lgt = (eiLight *)g_Context->current_node;

	ei_light_add_emitter(lgt, nodesys->m_db, shader);
}

void ei_origin(eiScalar x, eiScalar y, eiScalar z)
{
	eiLight		*lgt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	lgt = (eiLight *)g_Context->current_node;

	setv(&lgt->origin, x, y, z);
}

void ei_energy(eiScalar r, eiScalar g, eiScalar b)
{
	eiLight		*lgt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	lgt = (eiLight *)g_Context->current_node;

	setv(&lgt->energy, r, g, b);
}

/** \brief The u_samples and v_samples parameters subdivide the area light source 
 * primitive.
 * If the optional low_level exists and is greater than 0, then low_u_samples and 
 * low_v_samples will be used instead of u_samples and v_samples, respectively, 
 * if the sum of the reflection and refraction trace level exceeds low_level. */
void ei_area_samples(eiInt u_samples, eiInt v_samples, 
					 eiInt low_level, eiInt low_u_samples, eiInt low_v_samples)
{
	eiLight		*lgt;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	lgt = (eiLight *)g_Context->current_node;

	lgt->u_samples = u_samples;
	lgt->v_samples = v_samples;
	lgt->low_level = low_level;
	lgt->low_u_samples = low_u_samples;
	lgt->low_v_samples = low_v_samples;
}

/** \brief End describing the current light. */
void ei_end_light()
{
	ei_end_node();
}

/** \brief Begin describing an object, objects contain the geometric information. */
void ei_object(const char *name, const char *type)
{
	ei_node(name, type);
}

void ei_box(eiScalar xmin, eiScalar ymin, eiScalar zmin, 
		eiScalar xmax, eiScalar ymax, eiScalar zmax)
{
	eiObject	*obj;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	obj = (eiObject *)g_Context->current_node;

	setb(&obj->box, xmin, xmax, ymin, ymax, zmin, zmax);
}

void ei_motion_box(eiScalar xmin, eiScalar ymin, eiScalar zmin, 
		eiScalar xmax, eiScalar ymax, eiScalar zmax)
{
	eiObject	*obj;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	obj = (eiObject *)g_Context->current_node;

	setb(&obj->motion_box, xmin, xmax, ymin, ymax, zmin, zmax);
}

void ei_add_geometry(const char *shader_name)
{
	eiNodeSystem	*nodesys;
	eiProcObject	*proc;
	eiTag			shader;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	shader = ei_nodesys_find_node(nodesys, shader_name);

	if (shader == eiNULL_TAG)
	{
		ei_error("Cannot find shader %s\n", shader_name);
		return;
	}

	proc = (eiProcObject *)g_Context->current_node;

	ei_proc_object_add_geometry(proc, nodesys->m_db, shader);
}

void ei_pos_list(const eiTag tab)
{
	eiNodeSystem	*nodesys;
	eiIndex			param_index;
	eiTag			prev_tab;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	/* delete previous list if there's any */
	prev_tab = eiNULL_TAG;

	param_index = ei_nodesys_find_parameter(
		nodesys, 
		g_Context->current_node, 
		"pos_list");

	if (param_index != eiNULL_INDEX)
	{
		ei_nodesys_get_parameter_at(
			nodesys, 
			g_Context->current_node, 
			param_index, 
			&prev_tab);
	}

	if (prev_tab != eiNULL_TAG)
	{
		/* delete the data table according to reference count */
		if (ei_db_unref(nodesys->m_db, prev_tab) == 0)
		{
			ei_delete_data_table(nodesys->m_db, prev_tab);
		}
	}

	/* add reference to the new table */
	ei_db_ref(nodesys->m_db, tab);

	ei_nodesys_set_tag(
		nodesys, 
		&g_Context->current_node, 
		"pos_list", 
		tab);

	ei_motion_pos_list(tab);
}

void ei_motion_pos_list(const eiTag tab)
{
	eiNodeSystem	*nodesys;
	eiIndex			param_index;
	eiTag			prev_tab;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	/* delete previous list if there's any */
	prev_tab = eiNULL_TAG;

	param_index = ei_nodesys_find_parameter(
		nodesys, 
		g_Context->current_node, 
		"motion_pos_list");

	if (param_index != eiNULL_INDEX)
	{
		ei_nodesys_get_parameter_at(
			nodesys, 
			g_Context->current_node, 
			param_index, 
			&prev_tab);
	}

	if (prev_tab != eiNULL_TAG)
	{
		/* delete the data table according to reference count */
		if (ei_db_unref(nodesys->m_db, prev_tab) == 0)
		{
			ei_delete_data_table(nodesys->m_db, prev_tab);
		}
	}

	/* add reference to the new table */
	ei_db_ref(nodesys->m_db, tab);

	ei_nodesys_set_tag(
		nodesys, 
		&g_Context->current_node, 
		"motion_pos_list", 
		tab);
}

void ei_triangle_list(const eiTag tab)
{
	eiNodeSystem	*nodesys;
	eiIndex			param_index;
	eiTag			prev_tab;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	/* delete previous list if there's any */
	prev_tab = eiNULL_TAG;

	param_index = ei_nodesys_find_parameter(
		nodesys, 
		g_Context->current_node, 
		"triangle_list");

	if (param_index != eiNULL_INDEX)
	{
		ei_nodesys_get_parameter_at(
			nodesys, 
			g_Context->current_node, 
			param_index, 
			&prev_tab);
	}

	if (prev_tab != eiNULL_TAG)
	{
		/* delete the data table according to reference count */
		if (ei_db_unref(nodesys->m_db, prev_tab) == 0)
		{
			ei_delete_data_table(nodesys->m_db, prev_tab);
		}
	}

	/* add reference to the new table */
	ei_db_ref(nodesys->m_db, tab);

	ei_nodesys_set_tag(
		nodesys, 
		&g_Context->current_node, 
		"triangle_list", 
		tab);
}

void ei_disc_list(const eiTag tab)
{
	eiNodeSystem	*nodesys;
	eiIndex			param_index;
	eiTag			prev_tab;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	/* delete previous list if there's any */
	prev_tab = eiNULL_TAG;

	param_index = ei_nodesys_find_parameter(
		nodesys, 
		g_Context->current_node, 
		"disc_list");

	if (param_index != eiNULL_INDEX)
	{
		ei_nodesys_get_parameter_at(
			nodesys, 
			g_Context->current_node, 
			param_index, 
			&prev_tab);
	}
	
	if (prev_tab != eiNULL_TAG)
	{
		/* delete the data table according to reference count */
		if (ei_db_unref(nodesys->m_db, prev_tab) == 0)
		{
			ei_delete_data_table(nodesys->m_db, prev_tab);
		}
	}

	/* add reference to the new table */
	ei_db_ref(nodesys->m_db, tab);

	ei_nodesys_set_tag(
		nodesys, 
		&g_Context->current_node, 
		"disc_list", 
		tab);
}

void ei_degree(eiInt degree)
{
	eiNodeSystem	*nodesys;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	ei_nodesys_set_int(
		nodesys, 
		&g_Context->current_node, 
		"degree", 
		degree);
}

void ei_vertex_list(const eiTag tab)
{
	eiNodeSystem	*nodesys;
	eiIndex			param_index;
	eiTag			prev_tab;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	/* delete previous list if there's any */
	prev_tab = eiNULL_TAG;

	param_index = ei_nodesys_find_parameter(
		nodesys, 
		g_Context->current_node, 
		"vertex_list");

	if (param_index != eiNULL_INDEX)
	{
		ei_nodesys_get_parameter_at(
			nodesys, 
			g_Context->current_node, 
			param_index, 
			&prev_tab);
	}
	
	if (prev_tab != eiNULL_TAG)
	{
		/* delete the data table according to reference count */
		if (ei_db_unref(nodesys->m_db, prev_tab) == 0)
		{
			ei_delete_data_table(nodesys->m_db, prev_tab);
		}
	}

	/* add reference to the new table */
	ei_db_ref(nodesys->m_db, tab);

	ei_nodesys_set_tag(
		nodesys, 
		&g_Context->current_node, 
		"vertex_list", 
		tab);

	ei_motion_vertex_list(tab);
}

void ei_motion_vertex_list(const eiTag tab)
{
	eiNodeSystem	*nodesys;
	eiIndex			param_index;
	eiTag			prev_tab;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	/* delete previous list if there's any */
	prev_tab = eiNULL_TAG;

	param_index = ei_nodesys_find_parameter(
		nodesys, 
		g_Context->current_node, 
		"motion_vertex_list");

	if (param_index != eiNULL_INDEX)
	{
		ei_nodesys_get_parameter_at(
			nodesys, 
			g_Context->current_node, 
			param_index, 
			&prev_tab);
	}
	
	if (prev_tab != eiNULL_TAG)
	{
		/* delete the data table according to reference count */
		if (ei_db_unref(nodesys->m_db, prev_tab) == 0)
		{
			ei_delete_data_table(nodesys->m_db, prev_tab);
		}
	}

	/* add reference to the new table */
	ei_db_ref(nodesys->m_db, tab);

	ei_nodesys_set_tag(
		nodesys, 
		&g_Context->current_node, 
		"motion_vertex_list", 
		tab);
}

void ei_hair_list(const eiTag tab)
{
	eiNodeSystem	*nodesys;
	eiIndex			param_index;
	eiTag			prev_tab;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	/* delete previous list if there's any */
	prev_tab = eiNULL_TAG;

	param_index = ei_nodesys_find_parameter(
		nodesys, 
		g_Context->current_node, 
		"hair_list");

	if (param_index != eiNULL_INDEX)
	{
		ei_nodesys_get_parameter_at(
			nodesys, 
			g_Context->current_node, 
			param_index, 
			&prev_tab);
	}

	if (prev_tab != eiNULL_TAG)
	{
		/* delete the data table according to reference count */
		if (ei_db_unref(nodesys->m_db, prev_tab) == 0)
		{
			ei_delete_data_table(nodesys->m_db, prev_tab);
		}
	}

	/* add reference to the new table */
	ei_db_ref(nodesys->m_db, tab);

	ei_nodesys_set_tag(
		nodesys, 
		&g_Context->current_node, 
		"hair_list", 
		tab);
}

/** \brief End describing the current object. */
void ei_end_object()
{
	ei_end_node();
}

/** \brief Begin describing an instance, it contains the scene element to be 
 * instanced. */
void ei_instance(const char *name)
{
	ei_node(name, "instance");
}

/** \brief Instance a scene element into this instance. Instances place cameras, 
 * lights, objects, and instance groups into the scene. Without instances, 
 * these entities have no effect; they are not tessellated and are not 
 * scheduled for processing. An instance has a name that identifies the 
 * instance when it is placed into an instance group. Every instance 
 * references exactly one element element, which must be the name of a 
 * camera, a light, an object, or an instance group. */
void ei_element(const char *element_name)
{
	eiNodeSystem	*nodesys;
	eiTag			element;
	eiInstance		*inst;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	element = ei_nodesys_find_node(nodesys, element_name);

	if (element == eiNULL_TAG)
	{
		ei_error("Cannot find element %s\n", element_name);
		return;
	}

	inst = (eiInstance *)g_Context->current_node;

	inst->element = element;
}

/** \brief The transform statement is followed by 16 numbers that define a 4x4 matrix 
 * in row-major order. The matrix establishes the transformation from the 
 * object space of the instanced element to the parent coordinate space. If 
 * the instance is directly attached to the root instance group, the parent 
 * coordinate space is world space. */
void ei_transform(eiScalar m1, eiScalar m2, eiScalar m3, eiScalar m4, 
		eiScalar m5, eiScalar m6, eiScalar m7, eiScalar m8, 
		eiScalar m9, eiScalar m10, eiScalar m11, eiScalar m12, 
		eiScalar m13, eiScalar m14, eiScalar m15, eiScalar m16)
{
	eiInstance	*inst;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	inst = (eiInstance *)g_Context->current_node;

	setm(&inst->transform, 
		m1, m2, m3, m4, 
		m5, m6, m7, m8, 
		m9, m10, m11, m12, 
		m13, m14, m15, m16);
	setm(&inst->motion_transform, 
		m1, m2, m3, m4, 
		m5, m6, m7, m8, 
		m9, m10, m11, m12, 
		m13, m14, m15, m16);
}

/** \brief The motion transform matrix specifies a transformation from local space to 
 * parent space for motion blurred geometry. If not specified, the instance 
 * transformation is used for the motion blur transformation. In this case the 
 * parent instance determines whether motion blur is active or not. Motion 
 * blur is activated by specifying a motion transformation in the scene DAG. 
 * This transformation is propagated through the scene DAG in the same way 
 * as the instance transformations. The motion off statement turns off all 
 * motion information inherited up to this point, as if the camera and all 
 * instances above did not have motion transforms. */
void ei_motion_transform(eiScalar m1, eiScalar m2, eiScalar m3, eiScalar m4, 
		eiScalar m5, eiScalar m6, eiScalar m7, eiScalar m8, 
		eiScalar m9, eiScalar m10, eiScalar m11, eiScalar m12, 
		eiScalar m13, eiScalar m14, eiScalar m15, eiScalar m16)
{
	eiInstance	*inst;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	inst = (eiInstance *)g_Context->current_node;

	setm(&inst->motion_transform, 
		m1, m2, m3, m4, 
		m5, m6, m7, m8, 
		m9, m10, m11, m12, 
		m13, m14, m15, m16);
}

/** \brief The material_name is the name of a previously defined material. It is 
 * stored along with the instance. Instance materials are inherited down the 
 * scene DAG. Materials in instances lower in the scene DAG (closer to the 
 * leaves) override materials in instances higher up. The material defined 
 * lowest becomes the default material for any polygon or surface in a 
 * geometrical object that has no material of its own. */
void ei_add_material(const char *material_name)
{
	eiNodeSystem	*nodesys;
	eiInstance		*inst;
	eiTag			mtl;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	mtl = ei_nodesys_find_node(nodesys, material_name);

	if (mtl == eiNULL_TAG)
	{
		ei_error("Cannot find material %s\n", material_name);
		return;
	}

	inst = (eiInstance *)g_Context->current_node;

	ei_instance_add_material(inst, mtl, nodesys->m_db);
}

void ei_attribute(const eiInt flag, const eiBool value)
{
	eiInstance	*inst;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	inst = (eiInstance *)g_Context->current_node;

	ei_attr_set_flag(&inst->attr, flag, value);
}

/** \brief End describing the current instance. */
void ei_end_instance()
{
	ei_end_node();
}

/** \brief Begin describing an instance group, it's a collection of instances. */
void ei_instgroup(const char *name)
{
	ei_node(name, "instgroup");
}

/** \brief Add an instance into the current instance group. */
void ei_add_instance(const char *name)
{
	eiNodeSystem	*nodesys;
	eiInstgroup		*instgroup;
	eiTag			inst;

	if (!ei_non_nested_pair_inside(&g_Context->node_pair))
	{
		return;
	}

	nodesys = g_Context->nodesys;

	inst = ei_nodesys_find_node(nodesys, name);

	if (inst == eiNULL_TAG)
	{
		ei_error("Cannot find instance %s\n", name);
		return;
	}

	instgroup = (eiInstgroup *)g_Context->current_node;

	ei_instgroup_add_instance(instgroup, inst, nodesys->m_db);
}

/** \brief End describing the current instance group. */
void ei_end_instgroup()
{
	ei_end_node();
}
