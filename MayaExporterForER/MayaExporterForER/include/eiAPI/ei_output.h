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
 
#ifndef EI_OUTPUT_H
#define EI_OUTPUT_H

/** \brief The output of rendering.
 * \file ei_output.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_element.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief An output variable. */
typedef struct eiOutputVariable {
	char			name[ EI_MAX_PARAM_NAME_LEN ];
	eiInt			datatype;
	/* the frame buffer allocated by the renderer which 
	   is associated with this output variable */
	eiTag			framebuffer;
} eiOutputVariable;

/** \brief A class defines an output of a rendering, its name 
 * is the filename of this output. */
typedef struct eiOutput {
	char			filename[ EI_MAX_FILE_NAME_LEN ];
	char			fileformat[ EI_MAX_NODE_NAME_LEN ];
	eiInt			datatype;
	/* the data array of output variables */
	eiTag			variables;
} eiOutput;

void ei_output_init(eiOutput *output, 
	const char *filename, const char *fileformat, const eiInt datatype, 
	eiDatabase *db);
void ei_output_exit(eiOutput *output, eiDatabase *db);

void ei_output_add_variable(eiOutput *output, const char *name, const eiInt datatype, 
	eiDatabase *db);

/* for internal use only */
void byteswap_output_variable(eiDatabase *db, void *data, const eiUint size);
void byteswap_output(eiDatabase *db, void *data, const eiUint size);

#ifdef __cplusplus
}
#endif

#endif
