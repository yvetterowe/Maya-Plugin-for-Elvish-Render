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

void deriv_u_scalar(
	eiScalar * const dXdu, 
	const eiScalar dX1, 
	const eiScalar dX2, 
	eiState * const state)
{
	*dXdu = dX1 * state->duv.m1 + dX2 * state->duv.m2;
}

void deriv_u_vector(
	eiVector * const dXdu, 
	const eiVector *dX1, 
	const eiVector *dX2, 
	eiState * const state)
{
	eiVector	tmp;

	mulvf(dXdu, dX1, state->duv.m1);
	mulvf(&tmp, dX2, state->duv.m2);
	addi(dXdu, &tmp);
}

void deriv_u_vector2(
	eiVector2 * const dXdu, 
	const eiVector2 *dX1, 
	const eiVector2 *dX2, 
	eiState * const state)
{
	eiVector2	tmp;

	mulvf2(dXdu, dX1, state->duv.m1);
	mulvf2(&tmp, dX2, state->duv.m2);
	add2i(dXdu, &tmp);
}

void deriv_u_vector4(
	eiVector4 * const dXdu, 
	const eiVector4 *dX1, 
	const eiVector4 *dX2, 
	eiState * const state)
{
	eiVector4	tmp;

	mulvf4(dXdu, dX1, state->duv.m1);
	mulvf4(&tmp, dX2, state->duv.m2);
	add4i(dXdu, &tmp);
}

void deriv_v_scalar(
	eiScalar * const dXdv, 
	const eiScalar dX1, 
	const eiScalar dX2, 
	eiState * const state)
{
	*dXdv = dX1 * state->duv.m3 + dX2 * state->duv.m4;
}

void deriv_v_vector(
	eiVector * const dXdv, 
	const eiVector *dX1, 
	const eiVector *dX2, 
	eiState * const state)
{
	eiVector	tmp;

	mulvf(dXdv, dX1, state->duv.m3);
	mulvf(&tmp, dX2, state->duv.m4);
	addi(dXdv, &tmp);
}

void deriv_v_vector2(
	eiVector2 * const dXdv, 
	const eiVector2 *dX1, 
	const eiVector2 *dX2, 
	eiState * const state)
{
	eiVector2	tmp;

	mulvf2(dXdv, dX1, state->duv.m3);
	mulvf2(&tmp, dX2, state->duv.m4);
	add2i(dXdv, &tmp);
}

void deriv_v_vector4(
	eiVector4 * const dXdv, 
	const eiVector4 *dX1, 
	const eiVector4 *dX2, 
	eiState * const state)
{
	eiVector4	tmp;

	mulvf4(dXdv, dX1, state->duv.m3);
	mulvf4(&tmp, dX2, state->duv.m4);
	add4i(dXdv, &tmp);
}

void deriv_u_any(
	eiByte * const dXdu, 
	const eiByte *dX1, 
	const eiByte *dX2, 
	eiState * const state, 
	const eiInt type)
{
	eiScalar	sval;

	switch (type)
	{
	case EI_DATA_TYPE_BYTE:
		deriv_u_scalar(&sval, (eiScalar)(*((const eiByte *)dX1)), (eiScalar)(*((const eiByte *)dX2)), state);
		*((eiByte * const)dXdu) = (eiByte)sval;
		break;
	case EI_DATA_TYPE_SHORT:
		deriv_u_scalar(&sval, (eiScalar)(*((const eiShort *)dX1)), (eiScalar)(*((const eiShort *)dX2)), state);
		*((eiShort * const)dXdu) = (eiShort)sval;
		break;
	case EI_DATA_TYPE_INT:
		deriv_u_scalar(&sval, (eiScalar)(*((const eiInt *)dX1)), (eiScalar)(*((const eiInt *)dX2)), state);
		*((eiInt * const)dXdu) = (eiInt)sval;
		break;
	case EI_DATA_TYPE_BOOL:
		deriv_u_scalar(&sval, (eiScalar)(*((const eiByte *)dX1)), (eiScalar)(*((const eiByte *)dX2)), state);
		*((eiBool * const)dXdu) = (sval != 0.0f) ? eiTRUE : eiFALSE;
		break;
	case EI_DATA_TYPE_TAG:
		deriv_u_scalar(&sval, (eiScalar)(*((const eiTag *)dX1)), (eiScalar)(*((const eiTag *)dX2)), state);
		*((eiTag * const)dXdu) = (eiTag)sval;
		break;
	case EI_DATA_TYPE_INDEX:
		deriv_u_scalar(&sval, (eiScalar)(*((const eiIndex *)dX1)), (eiScalar)(*((const eiIndex *)dX2)), state);
		*((eiIndex * const)dXdu) = (eiInt)sval;
		break;
	case EI_DATA_TYPE_SCALAR:
		deriv_u_scalar((eiScalar * const)dXdu, *((const eiScalar *)dX1), *((const eiScalar *)dX2), state);
		break;
	case EI_DATA_TYPE_VECTOR:
		deriv_u_vector((eiVector * const)dXdu, (const eiVector *)dX1, (const eiVector *)dX2, state);
		break;
	case EI_DATA_TYPE_VECTOR2:
		deriv_u_vector2((eiVector2 * const)dXdu, (const eiVector2 *)dX1, (const eiVector2 *)dX2, state);
		break;
	case EI_DATA_TYPE_VECTOR4:
		deriv_u_vector4((eiVector4 * const)dXdu, (const eiVector4 *)dX1, (const eiVector4 *)dX2, state);
		break;
	default:
		break;
	}
}

void deriv_v_any(
	eiByte * const dXdv, 
	const eiByte *dX1, 
	const eiByte *dX2, 
	eiState * const state, 
	const eiInt type)
{
	eiScalar	sval;

	switch (type)
	{
	case EI_DATA_TYPE_BYTE:
		deriv_v_scalar(&sval, (eiScalar)(*((const eiByte *)dX1)), (eiScalar)(*((const eiByte *)dX2)), state);
		*((eiByte * const)dXdv) = (eiByte)sval;
		break;
	case EI_DATA_TYPE_SHORT:
		deriv_v_scalar(&sval, (eiScalar)(*((const eiShort *)dX1)), (eiScalar)(*((const eiShort *)dX2)), state);
		*((eiShort * const)dXdv) = (eiShort)sval;
		break;
	case EI_DATA_TYPE_INT:
		deriv_v_scalar(&sval, (eiScalar)(*((const eiInt *)dX1)), (eiScalar)(*((const eiInt *)dX2)), state);
		*((eiInt * const)dXdv) = (eiInt)sval;
		break;
	case EI_DATA_TYPE_BOOL:
		deriv_v_scalar(&sval, (eiScalar)(*((const eiByte *)dX1)), (eiScalar)(*((const eiByte *)dX2)), state);
		*((eiBool * const)dXdv) = (sval != 0.0f) ? eiTRUE : eiFALSE;
		break;
	case EI_DATA_TYPE_TAG:
		deriv_v_scalar(&sval, (eiScalar)(*((const eiTag *)dX1)), (eiScalar)(*((const eiTag *)dX2)), state);
		*((eiTag * const)dXdv) = (eiTag)sval;
		break;
	case EI_DATA_TYPE_INDEX:
		deriv_v_scalar(&sval, (eiScalar)(*((const eiIndex *)dX1)), (eiScalar)(*((const eiIndex *)dX2)), state);
		*((eiIndex * const)dXdv) = (eiInt)sval;
		break;
	case EI_DATA_TYPE_SCALAR:
		deriv_v_scalar((eiScalar * const)dXdv, *((const eiScalar *)dX1), *((const eiScalar *)dX2), state);
		break;
	case EI_DATA_TYPE_VECTOR:
		deriv_v_vector((eiVector * const)dXdv, (const eiVector *)dX1, (const eiVector *)dX2, state);
		break;
	case EI_DATA_TYPE_VECTOR2:
		deriv_v_vector2((eiVector2 * const)dXdv, (const eiVector2 *)dX1, (const eiVector2 *)dX2, state);
		break;
	case EI_DATA_TYPE_VECTOR4:
		deriv_v_vector4((eiVector4 * const)dXdv, (const eiVector4 *)dX1, (const eiVector4 *)dX2, state);
		break;
	default:
		break;
	}
}
