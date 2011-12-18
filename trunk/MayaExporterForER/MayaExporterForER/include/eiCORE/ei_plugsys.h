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

#ifndef EI_PLUGSYS_H
#define EI_PLUGSYS_H

/** \brief A generic plugin management system which manages the dynamic 
 * loading and unloading of modules and plugins. this system is designed 
 * to be thread-safe, no extra locking is needed.
 * \file ei_plugsys.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>
#include <eiCORE/ei_platform.h>
#include <eiCORE/ei_btree.h>
#include <eiCORE/ei_array.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief A generic module representation. */
typedef struct eiModule			eiModule;
/** \brief A generic plugin representation. */
typedef struct eiPlugin			eiPlugin;
/** \brief The object instance created by a plugin. */
typedef struct eiPluginObject	eiPluginObject;
/** \brief A generic plugin management system. */
typedef struct eiPluginSystem	eiPluginSystem;

/** \brief The function to initialize a module on loading. */
typedef void (*ei_module_init_func)();
/** \brief The function to cleanup a module on unloading. */
typedef void (*ei_module_exit_func)();
/** \brief The callback to delete the plugin object itself. */
typedef void (*ei_plugin_object_deletethis)(eiPluginObject *object);

/** \brief The base class of plugin object.
 * all objects created by plugins should 
 * derive from this class. */
struct eiPluginObject {
	/* the plugin that created this object */
	eiPlugin						*plugin;
	/* this function pointer must be filled by 
	   derived classes to delete the plugin 
	   object itself. */
	ei_plugin_object_deletethis		deletethis;
};

/** \brief Initialize the base plugin object. */
eiCORE_API void ei_plugin_object_init(eiPluginObject *object);
/** \brief Cleanup the base plugin object. */
eiCORE_API void ei_plugin_object_exit(eiPluginObject *object);

/** \brief Get a symbol pointer from a module by name.
 */
eiCORE_API eiSymbolHandle ei_module_get_symbol(
	eiModule *module, 
	const char *name);

/** \brief Get member symbol of a plugin, which should be 
 * implemented in format pluginName_symbolName.
 */
eiCORE_API eiSymbolHandle ei_plugin_get_symbol(
	eiPlugin *plugin, 
	const char *symbolName);

/** \brief Create a plugin object instance from the plugin.
 * @param param The custom parameters which will be passed to 
 * the create function in plugin.
 */
eiCORE_API eiPluginObject *ei_plugin_create_object(
	eiPlugin *plugin, 
	void *param);

/** \brief Declare a plugin object instance from the plugin.
 * @param param The custom parameters which will be passed to 
 * the declare function in plugin.
 */
eiCORE_API eiBool ei_plugin_declare_object(
	eiPlugin *plugin, 
	void *param);

/** \brief The generic plug-in management system. */
struct eiPluginSystem {
	/* a module map containing all modules managed by 
	   this plugin system. */
	ei_btree				m_module_map;
	/* a plugin map containing all plugins managed by 
	   this plugin system. */
	ei_btree				m_plugin_map;
	/* the array of search pathes */
	ei_array				m_search_pathes;
	/* the lock for performing plugin management operations. */
	eiLock					m_lock;
};

/** \brief Initialize a plugin system.
 */
eiCORE_API void ei_plugsys_init(eiPluginSystem *plugsys);

/** \brief Cleanup a plugin system.
 */
eiCORE_API void ei_plugsys_exit(eiPluginSystem *plugsys);

/** \brief Add a search path to the plugin system for 
 * searching modules.
 */
eiCORE_API eiBool ei_plugsys_add_search_path(
	eiPluginSystem *plugsys, 
	const char *search_path);

/** \brief Link a module by name. if the module has been loaded, 
 * no extra loading will be performed. add reference to the 
 * module. you can explicitly call this function to link a module.
 */
eiCORE_API eiModule *ei_plugsys_link(
	eiPluginSystem *plugsys, 
	const char *filename);

/** \brief Unlink a module, de-reference the module, if it's not 
 * longer referenced by any plugins, the module will be unloaded.
 */
eiCORE_API eiBool ei_plugsys_unlink(
	eiPluginSystem *plugsys, 
	eiModule *module);

/** \brief Open a plugin by name.
 * @param name The name of the plugin. it can be in format 
 * module_name.plugin_name, so the system will try to search 
 * in module module_name, or it can just be in format 
 * plugin_name, then the system will try to search in all 
 * modules to match plugin_name. if the corresponding module 
 * is not loaded, it will be automatically loaded.
 */
eiCORE_API eiPlugin *ei_plugsys_open(
	eiPluginSystem *plugsys, 
	const char *name);

/** \brief Close a plugin. the corresponding module will be 
 * automatically unloaded if there is no more plugins 
 * reference it.
 */
eiCORE_API eiBool ei_plugsys_close(
	eiPluginSystem *plugsys, 
	eiPlugin *plugin);

/** \brief Create a plugin object by plugin name.
* @param name The name of the plugin. it can be in format 
 * module_name.plugin_name, so the system will try to search 
 * in module module_name, or it can just be in format 
 * plugin_name, then the system will try to search in all 
 * modules to match plugin_name. if the corresponding module 
 * is not loaded, it will be automatically loaded.
 * @param param The custom parameters which will be passed to 
 * the create function in plugin.
 */
eiCORE_API eiPluginObject *ei_plugsys_create(
	eiPluginSystem *plugsys, 
	const char *name, 
	void *param);

/** \brief Declare a plugin object by plugin name.
* @param name The name of the plugin. it can be in format 
 * module_name.plugin_name, so the system will try to search 
 * in module module_name, or it can just be in format 
 * plugin_name, then the system will try to search in all 
 * modules to match plugin_name. if the corresponding module 
 * is not loaded, it will be automatically loaded.
 * @param param The custom parameters which will be passed to 
 * the declare function in plugin.
 */
eiCORE_API eiBool ei_plugsys_declare(
	eiPluginSystem *plugsys, 
	const char *name, 
	void *param);

/** \brief Delete a plugin object from the system.
 */
eiCORE_API eiBool ei_plugsys_delete(
	eiPluginSystem *plugsys, 
	eiPluginObject *object);

#ifdef __cplusplus
}
#endif

#endif
