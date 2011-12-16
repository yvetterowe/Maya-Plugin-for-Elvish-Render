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

#include <eiCORE/ei_plugsys.h>
#include <eiCORE/ei_assert.h>
#include <eiCORE/ei_util.h>

struct eiModule {
	ei_btree_node			node;
	/* the resolved module DSO filename. */
	char					filename[ EI_MAX_FILE_NAME_LEN ];
	/* the plugin system which manages this module. */
	eiPluginSystem			*plugin_system;
	eiModuleHandle			module_handle;
	/* reference count by plugins, when it's 0, the module 
	   will be automatically deleted.
	   it's to be used by the plugin system for performing 
	   reference counted automatic garbage collections. */
	eiInt					ref_count;
	eiUint					padding;
};

/** \brief The function for declaring the object instance itself */
typedef void (*ei_plugin_declare_object_func)(void *param);
/** \brief The function to create object instance from plugin */
typedef eiPluginObject * (*ei_plugin_create_object_func)(void *param);

struct eiPlugin {
	ei_btree_node			node;
	/* the full name of this plugin, in format 
	   module_name.plugin_name */
	char					name[ EI_MAX_FILE_NAME_LEN ];
	/* the resolved plugin name */
	char					plugin_name[ EI_MAX_FILE_NAME_LEN ];
	/* the module that this plugin resides in */
	eiModule				*module;
	/* the function pointer for declaring plugin object */
	ei_plugin_declare_object_func	declare_object;
	/* the function pointer for creating plugin object */
	ei_plugin_create_object_func	create_object;
	/* reference count by plugin instance, when it's 0, 
	   the plugin will be automatically deleted.
	   it's to be used by the plugin system for performing 
	   reference counted automatic garbage collections. */
	eiInt					ref_count;
	eiUint					padding;
};

static eiIntptr ei_module_compare(ei_btree_node *lhs, ei_btree_node *rhs, void *param)
{
	return strcmp(((eiModule *)lhs)->filename, ((eiModule *)rhs)->filename);
}

static void ei_module_init(
	eiModule *module, 
	eiPluginSystem *plugin_system, 
	const char *filename)
{
	eiSymbolHandle	init_func;

	ei_btree_node_init(&module->node);

	strcpy(module->filename, filename);
	module->plugin_system = plugin_system;
	module->module_handle = ei_load_module(module->filename);
	module->ref_count = 0;

	/* initialize the module on loading */
	init_func = ei_module_get_symbol(module, "module_init");
	if (init_func != NULL)
	{
		ei_module_init_func	module_init;

		module_init = (ei_module_init_func)init_func;
		module_init();
	}
}

static void ei_module_exit(eiModule *module)
{
	if (module->module_handle != NULL)
	{
		/* cleanup the module on unloading */
		eiSymbolHandle	exit_func;

		exit_func = ei_module_get_symbol(module, "module_exit");
		if (exit_func != NULL)
		{
			ei_module_exit_func	module_exit;

			module_exit = (ei_module_exit_func)exit_func;
			module_exit();
		}

		ei_free_module(module->module_handle);
		module->module_handle = NULL;
	}

	ei_btree_node_clear(&module->node);
}

static eiModule *ei_create_module(
	eiPluginSystem *plugin_system, 
	const char *filename)
{
	eiModule	*module;

	module = (eiModule *)ei_allocate(sizeof(eiModule));

	ei_module_init(module, plugin_system, filename);

	return module;
}

static void ei_module_delete_node(ei_btree_node *node, void *param)
{
	if (node == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_module_exit((eiModule *)node);

	eiCHECK_FREE(node);
}

static void ei_plugsys_lock(eiPluginSystem *plugsys)
{
	ei_lock(&plugsys->m_lock);
}

static void ei_plugsys_unlock(eiPluginSystem *plugsys)
{
	ei_unlock(&plugsys->m_lock);
}

static eiSymbolHandle ei_module_get_symbol_imp(
	eiModule *module, 
	const char *name)
{
	eiSymbolHandle	symbol;

	if (module == NULL || 
		module->plugin_system == NULL || 
		module->module_handle == NULL)
	{
		eiASSERT(0);
		return NULL;
	}

	symbol = ei_get_symbol(module->module_handle, name);

	return symbol;
}

void ei_plugin_object_init(eiPluginObject *object)
{
	/* derived class must fill this function pointer */
	object->deletethis = NULL;
}

void ei_plugin_object_exit(eiPluginObject *object)
{
}

eiSymbolHandle ei_module_get_symbol(
	eiModule *module, 
	const char *name)
{
	eiSymbolHandle	symbol;

	ei_plugsys_lock(module->plugin_system);

	symbol = ei_module_get_symbol_imp(module, name);

	ei_plugsys_unlock(module->plugin_system);

	return symbol;
}

eiSymbolHandle ei_plugin_get_symbol(
	eiPlugin *plugin, 
	const char *symbolName)
{
	char	resolved_name[ EI_MAX_FILE_NAME_LEN ];

	if (plugin == NULL || 
		symbolName == NULL)
	{
		eiASSERT(0);
		return NULL;
	}

	sprintf(resolved_name, "%s_%s", plugin->name, symbolName);

	return ei_module_get_symbol(plugin->module, resolved_name);
}

eiPluginObject *ei_plugin_create_object(
	eiPlugin *plugin, 
	void *param)
{
	eiPluginObject	*object;

	if (plugin == NULL || 
		plugin->create_object == NULL)
	{
		eiASSERT(0);
		return NULL;
	}

	object = plugin->create_object(param);
	/* set the plugin which created the object */
	object->plugin = plugin;

	return object;
}

eiBool ei_plugin_declare_object(
	eiPlugin *plugin, 
	void *param)
{
	if (plugin == NULL || 
		plugin->declare_object == NULL)
	{
		return eiFALSE;
	}

	plugin->declare_object(param);

	return TRUE;
}

static eiIntptr ei_plugin_compare(ei_btree_node *lhs, ei_btree_node *rhs, void *param)
{
	return strcmp(((eiPlugin *)lhs)->name, ((eiPlugin *)rhs)->name);
}

static void ei_plugin_init(
	eiPlugin *plugin, 
	const char *name, 
	const char *plugin_name, 
	const char *create_plugin_name, 
	eiModule *module)
{
	char		declare_plugin_name[ EI_MAX_FILE_NAME_LEN ];

	ei_btree_node_init(&plugin->node);

	strncpy(plugin->name, name, EI_MAX_FILE_NAME_LEN - 1);
	strncpy(plugin->plugin_name, plugin_name, EI_MAX_FILE_NAME_LEN - 1);
	plugin->module = module;
	
	plugin->create_object = (ei_plugin_create_object_func)ei_module_get_symbol_imp(
		plugin->module, 
		create_plugin_name);
	eiASSERT(plugin->create_object != NULL);

	/* the name of the function for declaring plugin object */
	sprintf(declare_plugin_name, "declare_%s", plugin_name);
	plugin->declare_object = (ei_plugin_declare_object_func)ei_module_get_symbol_imp(
		plugin->module, 
		declare_plugin_name);

	plugin->ref_count = 0;
}

static void ei_plugin_exit(eiPlugin *plugin)
{
	ei_btree_node_clear(&plugin->node);
}

static eiPlugin *ei_create_plugin(
	const char *name, 
	const char *plugin_name, 
	const char *create_plugin_name, 
	eiModule *module)
{
	eiPlugin	*plugin;

	plugin = (eiPlugin *)ei_allocate(sizeof(eiPlugin));

	ei_plugin_init(plugin, name, plugin_name, create_plugin_name, module);

	return plugin;
}

static void ei_plugin_delete_node(ei_btree_node *node, void *param)
{
	if (node == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_plugin_exit((eiPlugin *)node);

	eiCHECK_FREE(node);
}

void ei_plugsys_init(eiPluginSystem *plugsys)
{
	char	cur_dir[ EI_MAX_FILE_NAME_LEN ];
	char	module_dir[ EI_MAX_FILE_NAME_LEN ];

	ei_create_lock(&plugsys->m_lock);
	ei_btree_init(&plugsys->m_module_map, ei_module_compare, ei_module_delete_node, NULL);
	ei_btree_init(&plugsys->m_plugin_map, ei_plugin_compare, ei_plugin_delete_node, NULL);
	ei_array_init(&plugsys->m_search_pathes, sizeof(char *));

	/* add built-in search pathes */
	ei_get_current_directory(cur_dir);
	ei_plugsys_add_search_path(plugsys, cur_dir);
	ei_append_filename(module_dir, cur_dir, "plugins");
	ei_plugsys_add_search_path(plugsys, module_dir);
}

void ei_plugsys_exit(eiPluginSystem *plugsys)
{
	eiIntptr	i;

	for (i = 0; i < ei_array_size(&plugsys->m_search_pathes); ++i)
	{
		char	*str;
		str = *((char **)ei_array_get(&plugsys->m_search_pathes, i));
		eiCHECK_FREE(str);
	}
	ei_array_clear(&plugsys->m_search_pathes);
	ei_btree_clear(&plugsys->m_plugin_map);
	ei_btree_clear(&plugsys->m_module_map);
	ei_delete_lock(&plugsys->m_lock);
}

eiBool ei_plugsys_add_search_path(
	eiPluginSystem *plugsys, 
	const char *search_path)
{
	char	*str;

	if (search_path == NULL)
	{
		return eiFALSE;
	}

	ei_plugsys_lock(plugsys);

	/* we don't need to check duplicated search pathes 
	   because they will finally be resolved to the same 
	   thing */
	str = (char *)ei_allocate(strlen(search_path) + 1);
	strcpy(str, search_path);
	ei_array_push_back(&plugsys->m_search_pathes, &str);

	ei_plugsys_unlock(plugsys);

	return eiTRUE;
}

static eiBool ei_resolve_filename_in(
	char *resolved_filename, 
	const char *filename, 
	const char *search_path)
{
	ei_append_filename(resolved_filename, search_path, filename);

	if (!ei_file_exists(resolved_filename))
	{
		char	dso_name[ EI_MAX_FILE_NAME_LEN ];

		sprintf(dso_name, "%s.so", resolved_filename);

		if (ei_file_exists(dso_name))
		{
			strcpy(resolved_filename, dso_name);
		}
		else
		{
			char	dll_name[ EI_MAX_FILE_NAME_LEN ];

			sprintf(dll_name, "%s.dll", resolved_filename);
			
			if (ei_file_exists(dll_name))
			{
				strcpy(resolved_filename, dll_name);
			}
			else
			{
				return eiFALSE;
			}
		}
	}

	return eiTRUE;
}

static eiBool ei_resolve_filename(
   char *resolved_filename, 
   const char *filename, 
   const eiSizet num_search_pathes, 
   char **search_pathes)
{
	eiSizet		i;

	/* try to resolve with absolute name */
	if (ei_resolve_filename_in(resolved_filename, filename, ""))
	{
		return eiTRUE;
	}

	/* try to resolve with search pathes */
	for (i = 0; i < num_search_pathes; ++i)
	{
		if (ei_resolve_filename_in(resolved_filename, filename, search_pathes[i]))
		{
			return eiTRUE;
		}
	}

	return eiFALSE;
}

static eiModule *ei_plugsys_link_imp(
	eiPluginSystem *plugsys, 
	const char *filename)
{
	eiModule	key;
	eiModule	*module;

	/* resolve the module filename */
	if (!ei_resolve_filename(key.filename, filename, 
		ei_array_size(&plugsys->m_search_pathes), 
		(char **)ei_array_data(&plugsys->m_search_pathes)))
	{
		ei_error("Failed to resolve %s, please check its existence.\n", filename);

		return NULL;
	}

	/* lookup in loaded modules */
	module = (eiModule *)ei_btree_lookup(&plugsys->m_module_map, &key.node, NULL);

	/* load the module and add it to the map if not found */
	if (module == NULL)
	{
		module = ei_create_module(plugsys, key.filename);
		ei_btree_insert(&plugsys->m_module_map, &module->node, NULL);
	}

	/* add reference to the module */
	++ module->ref_count;

	return module;
}

static eiBool ei_plugsys_unlink_imp(
	eiPluginSystem *plugsys, 
	eiModule *module)
{
	/* do not close the module if it does not 
	   belong to this plugin system */
	if (module == NULL || 
		module->plugin_system != plugsys)
	{
		return eiFALSE;
	}

	/* decrement reference to the module */
	-- module->ref_count;

	/* unload the module and remove it from the map 
	   if no one references it */
	if (module->ref_count == 0)
	{
		ei_btree_delete(&plugsys->m_module_map, &module->node, NULL);
	}

	return eiTRUE;
}

eiModule *ei_plugsys_link(
	eiPluginSystem *plugsys, 
	const char *filename)
{
	eiModule	*module;

	ei_plugsys_lock(plugsys);

	module = ei_plugsys_link_imp(plugsys, filename);

	ei_plugsys_unlock(plugsys);

	return module;
}

eiBool ei_plugsys_unlink(
	eiPluginSystem *plugsys, 
	eiModule *module)
{
	eiBool		result;

	ei_plugsys_lock(plugsys);

	result = ei_plugsys_unlink_imp(plugsys, module);

	ei_plugsys_unlock(plugsys);

	return result;
}

typedef struct ei_search_plugin_params {
	char		*create_plugin_name;
	eiModule	*module;
} ei_search_plugin_params;

eiInt ei_search_plugin(ei_btree_node *node, void *param)
{
	eiModule				*module;
	ei_search_plugin_params	*params;

	module = (eiModule *)node;
	params = (ei_search_plugin_params *)param;

	if (ei_module_get_symbol_imp(module, params->create_plugin_name) != NULL)
	{
		params->module = module;

		/* found the plugin, terminate the traversal */
		return eiFALSE;
	}

	return eiTRUE;
}

eiPlugin *ei_plugsys_open(
	eiPluginSystem *plugsys, 
	const char *name)
{
	eiPlugin	*plugin;
	eiModule	*module;
	char		module_name[ EI_MAX_FILE_NAME_LEN ];
	char		plugin_name[ EI_MAX_FILE_NAME_LEN ];
	char		create_plugin_name[ EI_MAX_FILE_NAME_LEN ];
	eiPlugin	key;

	ei_plugsys_lock(plugsys);

	/* make the key with full name, so we can support 
	   multiple plugins in different modules having the 
	   same name */
	strncpy(key.name, name, EI_MAX_FILE_NAME_LEN - 1);
	ei_split_name(module_name, plugin_name, key.name, EI_MAX_FILE_NAME_LEN, '.');
	/* the name of the function for creating plugin object */
	sprintf(create_plugin_name, "create_%s", plugin_name);

	/* lookup in cached plugins */
	plugin = (eiPlugin *)ei_btree_lookup(&plugsys->m_plugin_map, &key.node, NULL);

	if (plugin != NULL)
	{
		/* add reference to the plugin */
		++ plugin->ref_count;

		ei_plugsys_unlock(plugsys);

		return plugin;
	}

	/* cannot find the plugin, create it from module */
	module = NULL;

	if (strlen(module_name) != 0)
	{
		/* module name specified, link the module */
		module = ei_plugsys_link_imp(plugsys, module_name);

		if (module != NULL && 
			ei_module_get_symbol_imp(module, create_plugin_name) == NULL)
		{
			/* cannot find the plugin in this module */
			module = NULL;
		}
	}

	if (module == NULL)
	{
		/* no module name specified, or failed to find 
		   plugin in the specified module, try to 
		   search plugin in all linked modules */
		ei_search_plugin_params		params;
		
		params.create_plugin_name = create_plugin_name;
		params.module = NULL;

		ei_btree_traverse(&plugsys->m_module_map, ei_search_plugin, &params);

		module = params.module;

		/* add reference to the module */
		if (module != NULL)
		{
			++ module->ref_count;
		}
	}

	if (module == NULL)
	{
		eiASSERT(0);

		ei_plugsys_unlock(plugsys);

		return NULL;
	}

	/* create the plugin from the module */
	plugin = ei_create_plugin(
		key.name, 
		plugin_name, 
		create_plugin_name, 
		module);
	ei_btree_insert(&plugsys->m_plugin_map, &plugin->node, NULL);

	/* add reference to the plugin */
	++ plugin->ref_count;

	ei_plugsys_unlock(plugsys);

	return plugin;
}

eiBool ei_plugsys_close(
	eiPluginSystem *plugsys, 
	eiPlugin *plugin)
{
	if (plugin == NULL || 
		plugin->module == NULL)
	{
		eiASSERT(0);
		return eiFALSE;
	}

	ei_plugsys_lock(plugsys);

	/* unlink from the module */
	ei_plugsys_unlink_imp(plugsys, plugin->module);

	/* decrement reference to the plugin */
	-- plugin->ref_count;

	/* delete the plugin and remove it from the map 
	   if no one references it */
	if (plugin->ref_count == 0)
	{
		ei_btree_delete(&plugsys->m_plugin_map, &plugin->node, NULL);
	}

	ei_plugsys_unlock(plugsys);

	return eiTRUE;
}

eiPluginObject *ei_plugsys_create(
	eiPluginSystem *plugsys, 
	const char *name, 
	void *param)
{
	eiPlugin	*plugin;

	if (plugsys == NULL || 
		name == NULL)
	{
		return NULL;
	}
	
	/* open the plugin by name */
	plugin = ei_plugsys_open(plugsys, name);

	if (plugin == NULL)
	{
		eiASSERT(0);
		return NULL;
	}

	/* create plugin object */
	return ei_plugin_create_object(plugin, param);
}

eiBool ei_plugsys_declare(
	eiPluginSystem *plugsys, 
	const char *name, 
	void *param)
{
	eiPlugin	*plugin;

	if (plugsys == NULL || 
		name == NULL)
	{
		return eiFALSE;
	}
	
	/* open the plugin by name */
	plugin = ei_plugsys_open(plugsys, name);

	if (plugin == NULL)
	{
		eiASSERT(0);
		return eiFALSE;
	}

	/* declare plugin object */
	return ei_plugin_declare_object(plugin, param);
}

eiBool ei_plugsys_delete(
	eiPluginSystem *plugsys, 
	eiPluginObject *object)
{
	eiPlugin	*plugin;

	if (plugsys == NULL || 
		object == NULL || 
		object->plugin == NULL || 
		object->deletethis == NULL)
	{
		eiASSERT(0);
		return eiFALSE;
	}

	plugin = object->plugin;

	/* delete the plugin object */
	object->deletethis(object);

	/* close the plugin */
	return ei_plugsys_close(plugsys, plugin);
}
