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
 
#ifndef EI_DATA_GEN_H
#define EI_DATA_GEN_H

/** \brief data generator interface for on-demand dynamic job execution, part of our 
 * dataflow architecture. most of the rendering data should have their data 
 * generator, which access multiple input data, then generate single output 
 * data. the data generator is only called once to initialize the data at 
 * the first time when we access the data, so most of the rendering data are 
 * generated on-demand, deferred to the first access time. in order to support 
 * transparent data transfer over the network, we only need the data type 
 * to generate a data, this is why function pointers should not be used here.
 * \file ei_data_gen.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>
#include <eiCORE/ei_dataflow.h>

#ifdef __cplusplus
extern "C" {
#endif

/* job only for testing purpose. */
typedef struct eiTestJob {
	eiShort			checkShort;
	eiUshort		checkUshort;
	eiInt			checkInt;
	eiUint			checkUint;
	eiLong			checkLong;
	eiUlong			checkUlong;
	eiScalar		checkFloat;
	eiGeoScalar		checkDouble;
} eiTestJob;

/** \brief The table of data generators, user must fill this array. */
eiCORE_EXTERN eiDataGenTable g_DataGenTable;

/** \brief The global object constructor, user must fill this callback. */
eiCORE_EXTERN eiInitGlobals g_InitGlobals;
/** \brief The global object destructor, user must fill this callback. */
eiCORE_EXTERN eiExitGlobals g_ExitGlobals;

/** \brief The custom TLS constructor, user must fill this callback. */
eiCORE_EXTERN eiInitTLS g_InitTLS;
/** \brief The custom TLS destructor, user must fill this callback. */
eiCORE_EXTERN eiExitTLS g_ExitTLS;

/** \brief The callback for setting renderable scene. */
eiCORE_EXTERN eiSetSceneCallback g_SetSceneCallback;
/** \brief The callback for ending editing renderable scene. */
eiCORE_EXTERN eiEndSceneCallback g_EndSceneCallback;
/** \brief The callback for updating editing renderable scene. */
eiCORE_EXTERN eiUpdateSceneCallback g_UpdateSceneCallback;
/** \brief The callback for linking a module */
eiCORE_EXTERN eiLinkCallback g_LinkCallback;

/* initialize default data generator table. for internal use only. */
eiCORE_API void ei_init_default_data_gen_table();

/** \brief Run a rendering server. */
eiCORE_API void ei_run_server();

#ifdef __cplusplus
}
#endif

#endif 
