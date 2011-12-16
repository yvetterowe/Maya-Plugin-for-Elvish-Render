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
 
#ifndef EI_COMMON_H
#define EI_COMMON_H

/** \brief The common header for all modules except for core module, 
 * the core module must not include this header otherwise it will 
 * results in cyclic dependency.
 * \file ei_common.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/* global system configurations */
#define EI_MAX_NODE_NAME_LEN			128
/* use a small value to save memory for nodes */
#define EI_MAX_PARAM_NAME_LEN			32

/** \brief The custom data types */
enum {
	/* ray-tracing module */
	EI_DATA_TYPE_BSP_NODE = EI_DATA_TYPE_USER,				/* BSP-node */
	EI_DATA_TYPE_BSP_TREE, 									/* BSP-tree */
	EI_DATA_TYPE_RAY_SUBSCENE,								/* ray-traceable sub-scene */
	EI_DATA_TYPE_RAY_SCENE,									/* ray-traceable scene */
	EI_DATA_TYPE_RAY_OBJECT,								/* ray-traceable object */
	EI_DATA_TYPE_RAY_OBJECT_INST,							/* ray-traceable object instance */
	EI_DATA_TYPE_RAY_TESSEL,								/* ray-traceable tessellation */
	EI_DATA_TYPE_RAY_ACCEL_TRIANGLES,						/* ray-traceable accelerated triangles */
	EI_DATA_TYPE_RAY_SUBTREE,								/* ray-traceable sub-tree */
	EI_DATA_TYPE_RAY_TESSEL_INST,							/* ray-traceable tessellation instance */
	EI_DATA_TYPE_RAY_HAIR_TREE,								/* ray-traceable hair tree */
	/* image module */
	EI_DATA_TYPE_BUFFER,									/* dynamic data buffer stored in database */
	EI_DATA_TYPE_FRAMEBUFFER_TILE,							/* a tile on frame buffer */
	EI_DATA_TYPE_FRAMEBUFFER,								/* frame buffer */
	EI_DATA_TYPE_TEXTURE_MAP,								/* texture map */
	EI_DATA_TYPE_TEXTURE_TILE,								/* texture tile */
	/* node system */
	EI_DATA_TYPE_NODE_DESC,									/* node description */
	EI_DATA_TYPE_NODE_PARAM_DESC,							/* node parameter description */
	EI_DATA_TYPE_NODE,										/* node */
	EI_DATA_TYPE_NODE_PARAM,								/* node parameter */
	/* shading system */
	EI_DATA_TYPE_SHADER_INST_PARAM_TABLE,					/* shader instance parameter table */
	/* scene manager */
	EI_DATA_TYPE_OUTPUT_VARIABLE,							/* output variable */
	EI_DATA_TYPE_OUTPUT,									/* output */
	EI_DATA_TYPE_APPROX,									/* approximation */
	EI_DATA_TYPE_JOB_TESSEL,								/* tessellation job */
	EI_DATA_TYPE_LIGHT_INST,								/* light instance */
	EI_DATA_TYPE_MAP,										/* map */
	/* global illumination */
	EI_DATA_TYPE_IRRADIANCE,								/* irradiance */
	EI_DATA_TYPE_PHOTON,									/* photon */
	EI_DATA_TYPE_LIGHT_FLUX,								/* light flux */
	/* renderer */
	EI_DATA_TYPE_JOB_PHOTON,								/* photon emission job */
	EI_DATA_TYPE_JOB_BUCKET,								/* bucket rendering job */
	EI_DATA_TYPE_COUNT, 
};

/** \brief The custom interface types */
enum {
	EI_INTERFACE_TYPE_RAYTRACER = EI_INTERFACE_TYPE_USER,	/* ray-tracer interface */
	EI_INTERFACE_TYPE_NODE_SYSTEM,							/* node system interface */
	EI_INTERFACE_TYPE_COUNT, 
};

/** \brief The custom TLS interface types */
enum {
	EI_TLS_TYPE_RAYTRACER = EI_TLS_TYPE_USER,	/* ray-tracer TLS interface */
	EI_TLS_TYPE_GLOBILLUM,						/* global illumination TLS interface */
	EI_TLS_TYPE_COUNT, 
};

#ifdef __cplusplus
}
#endif

#endif 
