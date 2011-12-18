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
 
#ifndef EI_CONFIG_H
#define EI_CONFIG_H

/** \brief The host configurations.
 * \file ei_config.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>
#include <eiCORE/ei_array.h>
#include <eiCORE/ei_platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The host description. */
typedef struct eiHostDesc {
	char		host_name[ EI_MAX_HOST_NAME_LEN ];
	eiUshort	port_number;
} eiHostDesc;

typedef struct eiSearchPath {
	char		path[ EI_MAX_FILE_NAME_LEN ];
} eiSearchPath;

/** \brief The host configurations. */
typedef struct eiConfig {
	eiInt		nthreads;
	eiInt		memlimit;
	eiBool		distributed;
	/* server only: port number to listen */
	eiUshort	port;
	/* server only: maximum number of allowed clients */
	eiInt		maxclients;
	/* manager only: an array of eiHostDesc */
	ei_array	servers;
	/* an array of eiSearchPath */
	ei_array	searchpaths;
} eiConfig;

/** \brief Set configurations to default values. */
eiCORE_API void ei_config_init(eiConfig *config);
/** \brief Cleanup configurations. */
eiCORE_API void ei_config_exit(eiConfig *config);

/** \brief Load configurations from file. */
eiCORE_API eiBool ei_config_load(eiConfig *config, const char *filename);

#ifdef __cplusplus
}
#endif

#endif
