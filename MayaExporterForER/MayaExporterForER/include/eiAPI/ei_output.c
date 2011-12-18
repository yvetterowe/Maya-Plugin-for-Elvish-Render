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

#include <eiAPI/ei_output.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_assert.h>

void ei_output_init(eiOutput *output, 
	const char *filename, const char *fileformat, const eiInt datatype, 
	eiDatabase *db)
{
	strncpy(output->filename, filename, EI_MAX_FILE_NAME_LEN - 1);
	strncpy(output->fileformat, fileformat, EI_MAX_NODE_NAME_LEN - 1);
	output->datatype = datatype;
	output->variables = ei_create_data_array(db, EI_DATA_TYPE_OUTPUT_VARIABLE);
}

void ei_output_exit(eiOutput *output, eiDatabase *db)
{
	ei_delete_data_array(db, output->variables);
}

void ei_output_add_variable(eiOutput *output, const char *name, const eiInt datatype, 
	eiDatabase *db)
{
	eiOutputVariable	outvar;

	strncpy(outvar.name, name, EI_MAX_PARAM_NAME_LEN - 1);
	outvar.datatype = datatype;
	outvar.framebuffer = eiNULL_TAG;

	ei_data_array_push_back(db, output->variables, &outvar);
}

void byteswap_output_variable(eiDatabase *db, void *data, const eiUint size)
{
	eiOutputVariable *outvar = (eiOutputVariable *)data;

	ei_byteswap_int(&outvar->datatype);
	ei_byteswap_int(&outvar->framebuffer);
}

void byteswap_output(eiDatabase *db, void *data, const eiUint size)
{
	eiOutput *output = (eiOutput *)data;

	ei_byteswap_int(&output->datatype);
	ei_byteswap_int(&output->variables);
}
