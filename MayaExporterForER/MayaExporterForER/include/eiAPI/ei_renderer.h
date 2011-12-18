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
 
#ifndef EI_RENDERER_H
#define EI_RENDERER_H

/** \brief The implementation of the renderer.
 * \file ei_renderer.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_sampler.h>
#include <eiCORE/ei_array.h>
#include <eiCORE/ei_dataflow.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eiDatabase		eiDatabase;
typedef struct eiExecutor		eiExecutor;
typedef struct eiMaster			eiMaster;
typedef struct eiRayTracer		eiRayTracer;
typedef struct eiNodeSystem		eiNodeSystem;
typedef struct eiConnection		eiConnection;
typedef struct eiOptions		eiOptions;
typedef struct eiCamera			eiCamera;
typedef struct eiRenderer		eiRenderer;

/** \brief Initialize session global objects for the renderer. */
eiAPI void ei_init_globals(eiGlobals *globals, eiDatabase *db);
/** \brief Cleanup session global objects for the renderer. */
eiAPI void ei_exit_globals(eiGlobals *globals, eiDatabase *db);
/** \brief Initialize thread global objects for the renderer. */
eiAPI void ei_init_tls(eiTLS *tls);
/** \brief Cleanup thread global objects for the renderer. */
eiAPI void ei_exit_tls(eiTLS *tls);
/** \brief Set renderable scene for the renderer. */
eiAPI eiBool ei_set_scene_callback(eiGlobals *globals, eiDatabase *db, const eiTag scene_tag);
/** \brief End editing renderable scene. */
eiAPI eiBool ei_end_scene_callback(eiGlobals *globals, eiDatabase *db);
/** \brief Update editing renderable scene. */
eiAPI eiBool ei_update_scene_callback(eiGlobals *globals, eiDatabase *db);
/** \brief Link a module. */
eiAPI eiBool ei_link_callback(eiGlobals *globals, eiDatabase *db, const char *module_name);

/** \brief Create a renderer. */
eiRenderer *ei_create_renderer();
/** \brief Delete a renderer. */
void ei_delete_renderer(eiRenderer *rend);

/** \brief Set application connection. */
void ei_renderer_set_connection(
	eiRenderer *rend, 
	eiConnection *con);

/** \brief Get node system of this renderer. */
eiNodeSystem * const ei_renderer_node_system(eiRenderer *rend);

/** \brief Start rendering a scene described 
 * by the root instance group. */
void ei_renderer_render(
	eiRenderer *rend, 
	const eiTag root_instgrp_tag, 
	const eiTag cam_inst_tag, 
	const eiTag opt_tag);

#ifdef __cplusplus
}
#endif

#endif
