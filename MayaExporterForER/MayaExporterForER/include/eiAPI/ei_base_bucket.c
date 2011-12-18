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

#include <eiAPI/ei_base_bucket.h>
#include <eiCORE/ei_assert.h>

void ei_build_approx_bucket(
	eiBaseBucket *bucket, 
	eiDatabase *db, 
	const eiInt random_offset)
{
	eiASSERT(bucket != NULL);
	eiASSERT(db != NULL);

	bucket->db = db;
	bucket->rt = (eiRayTracer *)ei_db_globals_interface(db, EI_INTERFACE_TYPE_RAYTRACER);
	bucket->nodesys = (eiNodeSystem *)ei_db_globals_interface(db, EI_INTERFACE_TYPE_NODE_SYSTEM);
	bucket->opt = NULL;
	bucket->cam = NULL;
	bucket->type = EI_BUCKET_TYPE_NONE;

	ei_data_table_reset_iterator(&bucket->light_insts_iter);

	/* decorelate random generator */
	ei_random_reset(&bucket->randGen, EI_DEFAULT_RANDOM_SEED + random_offset);
}

void ei_base_bucket_init(
	eiBaseBucket *bucket, 
	eiDatabase *db, 
	const eiTag opt_tag, 
	const eiTag cam_tag, 
	const eiTag lightInstances, 
	const eiInt random_offset)
{
	eiASSERT(bucket != NULL);
	eiASSERT(db != NULL);
	eiASSERT(opt_tag != eiNULL_TAG);
	eiASSERT(cam_tag != eiNULL_TAG);
	eiASSERT(lightInstances != eiNULL_TAG);

	bucket->db = db;
	bucket->rt = (eiRayTracer *)ei_db_globals_interface(db, EI_INTERFACE_TYPE_RAYTRACER);
	bucket->nodesys = (eiNodeSystem *)ei_db_globals_interface(db, EI_INTERFACE_TYPE_NODE_SYSTEM);
	bucket->opt = (eiOptions *)ei_db_access(db, opt_tag);
	bucket->cam = (eiCamera *)ei_db_access(db, cam_tag);
	bucket->type = EI_BUCKET_TYPE_NONE;

	ei_data_table_begin(db, lightInstances, &bucket->light_insts_iter);

	/* decorelate random generator */
	ei_random_reset(&bucket->randGen, EI_DEFAULT_RANDOM_SEED + random_offset);
}

void ei_base_bucket_exit(
	eiBaseBucket *bucket, 
	const eiTag opt_tag, 
	const eiTag cam_tag)
{
	eiASSERT(bucket != NULL);
	eiASSERT(opt_tag != eiNULL_TAG);
	eiASSERT(cam_tag != eiNULL_TAG);

	ei_data_table_end(&bucket->light_insts_iter);

	ei_db_end(bucket->db, opt_tag);
	ei_db_end(bucket->db, cam_tag);
}
