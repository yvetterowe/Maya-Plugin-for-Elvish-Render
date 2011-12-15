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

#include <eiAPI/ei_state.h>
#include <eiAPI/ei_base_bucket.h>
#include <eiAPI/ei_material.h>
#include <eiCORE/ei_dataflow.h>
#include <eiCORE/ei_assert.h>

void ei_state_init(
	eiState *state, 
	const eiInt type, 
	eiBaseBucket *bucket)
{
	eiASSERT(state != NULL);
	eiASSERT(type > eiRAY_TYPE_NONE && type < eiRAY_TYPE_COUNT);
	eiASSERT(bucket != NULL);

	state->type = type;
	/* retrieve thread local storage for this state */
	state->tls = ei_db_get_tls(bucket->db);
	state->result = NULL;
	state->bucket = bucket;
	state->db = bucket->db;
	state->opt = bucket->opt;
	state->cam = bucket->cam;
	state->shader_cache = NULL;
	state->shader = eiNULL_TAG;
	state->current_volumes = NULL;
	state->prev_hit_t = 0.0f;

	ei_new_state(state);

	initv(&state->org);
	initv(&state->dir);
	initv(&state->inv_dir);
	state->time = 0.0f;
	state->max_t = eiMAX_SCALAR;
	state->distance = 0.0f;
	state->reflect_depth = 0;
	state->refract_depth = 0;
	state->finalgather_diffuse_depth = 0;
	state->caustic_reflect_depth = 0;
	state->caustic_refract_depth = 0;
	state->globillum_reflect_depth = 0;
	state->globillum_refract_depth = 0;
}

void ei_state_exit(eiState *state)
{
	if (state->current_volumes != NULL)
	{
		ei_tls_free(state->tls, state->current_volumes);
		state->current_volumes = NULL;
	}
}

void ei_flush_cache(eiState *state)
{
	eiShaderCache	*shader_cache;

	shader_cache = (eiShaderCache *)state->shader_cache;

	if (shader_cache != NULL && shader_cache->size != 0)
	{
		memset(shader_cache + 1, 0, shader_cache->size);
	}
}

void ei_new_state(eiState *state)
{
	state->t_near = 0.0f;
	state->t_far = eiMAX_SCALAR;
	state->found_hit = eiFALSE;
	state->hit_t = eiMAX_SCALAR;
	state->hit_bsp = eiNULL_TAG;
	state->hit_tessel_inst	= eiNULL_INDEX;
	state->hit_tessel = eiNULL_TAG;
	state->hit_inst = eiNULL_TAG;
	state->hit_obj = eiNULL_TAG;
	state->hit_mtl = eiNULL_TAG;
	state->hit_tri = eiNULL_INDEX;
	state->hit_prim = eiNULL_INDEX;
	state->pass_motion = eiFALSE;
	state->hit_motion = eiFALSE;
	initv(&state->bary);
	state->bias = EI_RAY_BIAS;
	state->bias_scale = EI_RAY_BIAS_SCALE;
	memset(state->user_data, 0, sizeof(eiScalar) * EI_MAX_USER_DATA_SIZE);

	ei_flush_cache(state);

	/*	reset the iterators */
	state->current_light_index = -1;
	state->current_area_sample = 0;
	state->current_surface = 0;
	state->num_current_volumes = 0;
	if (state->current_volumes != NULL)
	{
		ei_tls_free(state->tls, state->current_volumes);
		state->current_volumes = NULL;
	}
}

void ei_state_init_volume(eiState *state, const eiTag volume)
{
	if (volume == eiNULL_TAG)
	{
		ei_warning("Invalid volume shader to add.");
		return;
	}

	if (state->current_volumes != NULL)
	{
		ei_tls_free(state->tls, state->current_volumes);
		state->current_volumes = NULL;
	}

	state->num_current_volumes = 1;
	state->current_volumes = (eiTag *)ei_tls_allocate(state->tls, sizeof(eiTag) * state->num_current_volumes);
	state->current_volumes[0] = volume;
}

void ei_state_inherit_volume(eiState *state, eiState * const parent)
{
	eiTag		hit_volume;
	eiUint		hit_volume_index;
	eiUint		i;

	/* get the current hit volume shader */
	hit_volume = eiNULL_TAG;

	if (parent->hit_mtl != eiNULL_TAG)
	{
		eiMaterial	*mtl;

		mtl = (eiMaterial *)ei_db_access(state->db, parent->hit_mtl);
		hit_volume = mtl->volume_list;
		ei_db_end(state->db, parent->hit_mtl);
	}

	/* check NULL hit instance, just copy the current volumes */
	if (hit_volume == eiNULL_TAG)
	{
		if (parent->num_current_volumes != 0)
		{
			state->num_current_volumes = parent->num_current_volumes;
			state->current_volumes = (eiTag *)ei_tls_allocate(state->tls, sizeof(eiTag) * state->num_current_volumes);
			memcpy(state->current_volumes, parent->current_volumes, sizeof(eiTag) * state->num_current_volumes);
		}
		return;
	}

	/* search the current hit instance in existing volume list */
	hit_volume_index = eiNULL_INDEX;

	for (i = 0; i < parent->num_current_volumes; ++i)
	{
		if (parent->current_volumes[i] == hit_volume)
		{
			hit_volume_index = i;
			break;
		}
	}

	/* build new volume list */
	if (parent->dot_nd < 0.0f)
	{
		/* ray is entering the volume of current hit instance */
		if (hit_volume_index == eiNULL_INDEX)
		{
			/* the hit instance does not reside in existing volume list, append it */
			state->num_current_volumes = parent->num_current_volumes + 1;
			state->current_volumes = (eiTag *)ei_tls_allocate(state->tls, sizeof(eiTag) * state->num_current_volumes);
			if (parent->num_current_volumes != 0)
			{
				memcpy(state->current_volumes, parent->current_volumes, sizeof(eiTag) * parent->num_current_volumes);
			}
			state->current_volumes[parent->num_current_volumes] = hit_volume;
		}
		else
		{
			/* the hit instance already existed, just copy */
			state->num_current_volumes = parent->num_current_volumes;
			state->current_volumes = (eiTag *)ei_tls_allocate(state->tls, sizeof(eiTag) * state->num_current_volumes);
			memcpy(state->current_volumes, parent->current_volumes, sizeof(eiTag) * state->num_current_volumes);
		}
	}
	else
	{
		/* ray is leaving the volume of current hit instance */
		if (hit_volume_index == eiNULL_INDEX)
		{
			/* the hit instance does not reside in existing volume list, just copy */
			if (parent->num_current_volumes != 0)
			{
				state->num_current_volumes = parent->num_current_volumes;
				state->current_volumes = (eiTag *)ei_tls_allocate(state->tls, sizeof(eiTag) * state->num_current_volumes);
				memcpy(state->current_volumes, parent->current_volumes, sizeof(eiTag) * state->num_current_volumes);
			}
		}
		else
		{
			/* the hit instance existed, remove it */
			state->num_current_volumes = parent->num_current_volumes - 1;
			state->current_volumes = (eiTag *)ei_tls_allocate(state->tls, sizeof(eiTag) * state->num_current_volumes);
			/* copy the portion before index */
			if (hit_volume_index != 0)
			{
				memcpy(state->current_volumes, parent->current_volumes, sizeof(eiTag) * hit_volume_index);
			}
			/* copy the portion after index */
			if ((parent->num_current_volumes - hit_volume_index - 1) != 0)
			{
				memcpy(state->current_volumes + hit_volume_index, parent->current_volumes + hit_volume_index + 1, 
					sizeof(eiTag) * (parent->num_current_volumes - hit_volume_index - 1));
			}
		}
	}
}
