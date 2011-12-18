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

#include <eiAPI/ei_object.h>
#include <eiAPI/ei_element.h>
#include <eiAPI/ei_approx.h>
#include <eiAPI/ei_instance.h>
#include <eiAPI/ei_material.h>
#include <eiAPI/ei_shadesys.h>
#include <eiAPI/ei_base_bucket.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_data_table.h>
#include <eiCORE/ei_assert.h>

/* the actual expanded key for comparing object representations, 
   we expand this on the fly because this structure is relatively 
   too big to store for each object representation. */
typedef struct eiObjectRepKey {
	eiBool			view_dep;
	eiMatrix		transform;
	eiMatrix		motion_transform;
	eiApprox		approx;
	eiBool			motion;
	eiTag			displace_list;
	eiUint			time;
	eiUint			view_time;
} eiObjectRepKey;

static void ei_expand_object_rep_key(
	eiObjectRepKey *key, 
	eiObjectRepNode *node, 
	eiDatabase *db)
{
	eiRayObjectInstance	*inst;
	eiApprox			*approx;

	if (node->inst == eiNULL_TAG)
	{
		key->view_dep = eiFALSE;
		initm(&key->transform, 0.0f);
		initm(&key->motion_transform, 0.0f);
	}
	else
	{
		key->view_dep = eiTRUE;
		inst = (eiRayObjectInstance *)ei_db_access(db, node->inst);
		movm(&key->transform, &inst->object_to_world);
		movm(&key->motion_transform, &inst->motion_object_to_world);
		ei_db_end(db, node->inst);
	}

	approx = (eiApprox *)ei_db_access(db, node->approx);
	ei_approx_copy(&key->approx, approx);
	ei_db_end(db, node->approx);

	key->motion = node->motion;
	key->displace_list = node->displace_list;
	key->time = node->time;
	key->view_time = node->view_time;
}

static void ei_object_rep_node_init(
	eiObjectRepNode *node, 
	const eiTag inst, 
	const eiTag approx, 
	const eiBool motion, 
	const eiTag displace_list, 
	const eiUint time, 
	const eiUint view_time, 
	const eiTag rep)
{
	ei_btree_node_init(&node->node);
	node->inst = inst;
	node->approx = approx;
	node->motion = motion;
	node->displace_list = displace_list;
	node->time = time;
	node->view_time = view_time;
	node->rep = rep;
}

static void ei_object_rep_node_exit(eiObjectRepNode *node)
{
	ei_btree_node_clear(&node->node);
}

static eiObjectRepNode *ei_create_object_rep_node(
	const eiTag inst, 
	const eiTag approx, 
	const eiBool motion, 
	const eiTag displace_list, 
	const eiUint time, 
	const eiUint view_time, 
	const eiTag rep)
{
	eiObjectRepNode *node = (eiObjectRepNode *)ei_allocate(sizeof(eiObjectRepNode));
	
	ei_object_rep_node_init(node, inst, approx, motion, displace_list, time, view_time, rep);

	return node;
}

static eiIntptr ei_object_rep_map_compare(void *lhs, void *rhs, void *param)
{
	eiDatabase		*db;
	eiObjectRepKey	lkey;
	eiObjectRepKey	rkey;

	db = (eiDatabase *)param;

	ei_expand_object_rep_key(&lkey, (eiObjectRepNode *)lhs, db);
	ei_expand_object_rep_key(&rkey, (eiObjectRepNode *)rhs, db);

	return memcmp(&lkey, &rkey, sizeof(eiObjectRepKey));
}

static void ei_object_rep_map_delete_node(ei_btree_node *node, void *param)
{
	if (node == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_object_rep_node_exit((eiObjectRepNode *)node);

	eiCHECK_FREE(node);
}

void ei_object_rep_map_init(eiObjectRepMap *map)
{
	ei_btree_init(&map->map, ei_object_rep_map_compare, ei_object_rep_map_delete_node, NULL);
}

void ei_object_rep_map_exit(eiObjectRepMap *map)
{
	ei_btree_clear(&map->map);
}

void ei_object_rep_map_add(
	eiObjectRepMap *map, 
	const eiTag inst, 
	const eiTag approx, 
	const eiBool motion, 
	const eiTag displace_list, 
	const eiUint time, 
	const eiUint view_time, 
	const eiTag rep, 
	eiDatabase *db)
{
	eiObjectRepNode		*node;

	node = ei_create_object_rep_node(inst, approx, motion, displace_list, time, view_time, rep);

	ei_btree_insert(&map->map, &node->node, (void *)db);
}

eiTag ei_object_rep_map_find(
	eiObjectRepMap *map, 
	const eiTag inst, 
	const eiTag approx, 
	const eiBool motion, 
	const eiTag displace_list, 
	const eiUint time, 
	const eiUint view_time, 
	eiDatabase *db)
{
	eiObjectRepNode		key;
	eiObjectRepNode		*node;

	ei_object_rep_node_init(&key, inst, approx, motion, displace_list, time, view_time, eiNULL_TAG);
	node = (eiObjectRepNode *)ei_btree_lookup(&map->map, &key.node, (void *)db);

	if (node == NULL)
	{
		return eiNULL_TAG;
	}

	return node->rep;
}

/** \brief The node for tag set. */
typedef struct eiTagNode {
	/* the base node */
	ei_btree_node	node;
	/* the tag as value */
	eiTag			tag;
} eiTagNode;

static void ei_tag_node_init(eiTagNode *node, const eiTag tag)
{
	ei_btree_node_init(&node->node);
	node->tag = tag;
}

static void ei_tag_node_exit(eiTagNode *node)
{
	ei_btree_node_clear(&node->node);
}

static eiTagNode *ei_create_tag_node(const eiTag tag)
{
	eiTagNode	*node;

	node = (eiTagNode *)ei_allocate(sizeof(eiTagNode));
	ei_tag_node_init(node, tag);

	return node;
}

static eiIntptr ei_tag_node_compare(void *lhs, void *rhs, void *param)
{
	/* must cast to signed types to minus correctly */
	return (((eiIntptr)((eiTagNode *)lhs)->tag) - ((eiIntptr)((eiTagNode *)rhs)->tag));
}

static void ei_tag_node_delete(ei_btree_node *node, void *param)
{
	if (node == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_tag_node_exit((eiTagNode *)node);

	eiCHECK_FREE(node);
}

void ei_object_rep_cache_init(
	eiObjectRepCache *cache, 
	eiDatabase *db, 
	const eiTag cam_tag, 
	const eiTag light_insts)
{
	cache->cam_tag = cam_tag;
	cache->light_instances = light_insts;
	ei_btree_init(&cache->objects, ei_tag_node_compare, ei_tag_node_delete, NULL);
	cache->object_rep_jobs = ei_create_job_queue();
	cache->object_instances = ei_create_data_array(db, EI_DATA_TYPE_TAG);
}

void ei_object_rep_cache_exit(eiObjectRepCache *cache, eiDatabase *db)
{
	if (cache->object_instances != eiNULL_TAG)
	{
		ei_delete_data_array(db, cache->object_instances);
		cache->object_instances = eiNULL_TAG;
	}
	if (cache->object_rep_jobs != NULL)
	{
		ei_delete_job_queue(cache->object_rep_jobs);
		cache->object_rep_jobs = NULL;
	}
	ei_btree_clear(&cache->objects);
}

typedef struct eiCollectRepParams {
	eiDatabase		*db;
	ei_array		*reps;
} eiCollectRepParams;

static eiInt ei_collect_unref_object_reps(ei_btree_node *node, void *param)
{
	eiObjectRepNode		*rep_node;
	eiCollectRepParams	*params;

	rep_node = (eiObjectRepNode *)node;
	params = (eiCollectRepParams *)param;

	if (ei_db_getref(params->db, rep_node->rep) == 0)
	{
		ei_array_push_back(params->reps, (void *)(&rep_node));
	}

	return eiTRUE;
}

typedef struct eiClearRepParams {
	eiDatabase		*db;
} eiClearRepParams;

static eiInt ei_clear_unref_object_reps(ei_btree_node *node, void *param)
{
	eiClearRepParams	*args;
	eiRayTracer			*rt;
	eiTag				obj_tag;
	eiObject			*obj;
	eiCollectRepParams	params;
	ei_array			unref_reps;
	eiIntptr			i;

	args = (eiClearRepParams *)param;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		args->db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* collect all unreferenced object representations */
	params.db = args->db;
	params.reps = &unref_reps;

	ei_array_init(&unref_reps, sizeof(eiObjectRepNode *));

	obj_tag = ((eiTagNode *)node)->tag;

	obj = (eiObject *)ei_db_access(args->db, obj_tag);

	ei_btree_traverse(&obj->object_reps.map, ei_collect_unref_object_reps, (void *)(&params));

	ei_db_end(args->db, obj_tag);

	/* remove all unreferenced object representations */
	for (i = 0; i < ei_array_size(&unref_reps); ++i)
	{
		eiObjectRepNode		*node;
		eiRayObject			*ray_obj;

		node = *((eiObjectRepNode **)ei_array_get(&unref_reps, i));

		/* destruct the ray-traceable object */
		ray_obj = (eiRayObject *)ei_db_access(args->db, node->rep);
		ei_ray_object_exit(rt, ray_obj);
		ei_db_end(args->db, node->rep);

		/* delete the ray-traceable object */
		ei_db_delete(args->db, node->rep);

		/* remove the ray-traceable object from object representation map */
		ei_btree_delete(&obj->object_reps.map, &node->node, (void *)args->db);
	}

	ei_array_clear(&unref_reps);

	return eiTRUE;
}

void ei_object_rep_cache_add_object(eiObjectRepCache *cache, const eiTag tag)
{
	eiTagNode	key;
	eiTagNode	*node;

	key.tag = tag;
	node = (eiTagNode *)ei_btree_lookup(&cache->objects, &key.node, NULL);

	if (node == NULL)
	{
		node = ei_create_tag_node(tag);
		ei_btree_insert(&cache->objects, &node->node, NULL);
	}
}

void ei_object_rep_cache_clear_unref_object_reps(eiObjectRepCache *cache, eiDatabase *db)
{
	eiClearRepParams	params;

	params.db = db;

	ei_btree_traverse(&cache->objects, ei_clear_unref_object_reps, (void *)(&params));
}

void ei_object_init(eiNodeSystem *nodesys, eiNode *node)
{
	eiObject	*obj;

	obj = (eiObject *)node;

	obj->node.type = EI_ELEMENT_OBJECT;

	ei_object_rep_map_init(&obj->object_reps);
}

void ei_object_exit(eiNodeSystem *nodesys, eiNode *node)
{
	eiObject	*obj;

	obj = (eiObject *)node;

	ei_object_rep_map_exit(&obj->object_reps);
}

static eiBool get_constant_var(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	eiNodeParam *param, 
	const eiInt type, 
	eiByte * const x)
{
	ei_nodesys_cast_parameter_value(
		nodesys, 
		node, 
		param, 
		(void *)x, 
		type);

	return eiTRUE;
}

static eiFORCEINLINE void cast_prim_var(
	const eiInt type, 
	eiByte * const x, 
	const eiScalar *value)
{
	switch (type)
	{
	case EI_DATA_TYPE_BYTE:
		*((eiByte * const)x) = (eiByte)(*value);
		break;
	case EI_DATA_TYPE_SHORT:
		*((eiShort * const)x) = (eiShort)(*value);
		break;
	case EI_DATA_TYPE_INT:
		*((eiInt * const)x) = (eiInt)(*value);
		break;
	case EI_DATA_TYPE_BOOL:
		*((eiBool * const)x) = (*value != 0.0f) ? eiTRUE : eiFALSE;
		break;
	case EI_DATA_TYPE_TAG:
		*((eiTag * const)x) = (eiTag)(*value);
		break;
	case EI_DATA_TYPE_INDEX:
		*((eiIndex * const)x) = (eiIndex)(*value);
		break;
	case EI_DATA_TYPE_SCALAR:
		*((eiScalar * const)x) = (eiScalar)(*value);
		break;
	case EI_DATA_TYPE_VECTOR:
		movv((eiVector * const)x, (eiVector *)value);
		break;
	case EI_DATA_TYPE_VECTOR2:
		movv2((eiVector2 * const)x, (eiVector2 *)value);
		break;
	case EI_DATA_TYPE_VECTOR4:
		movv4((eiVector4 * const)x, (eiVector4 *)value);
		break;
	default:
		break;
	}
}

static eiBool get_uniform_prim_var(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	eiNodeParam *param, 
	const eiIndex prim_index, 
	const eiInt type, 
	eiByte * const x)
{
	eiTag				tab_tag;
	eiDataTableIterator	tab_iter;

	if (param->type != EI_DATA_TYPE_TAG)
	{
		return eiFALSE;
	}

	tab_tag = eiNULL_TAG;

	ei_nodesys_get_parameter_value(nodesys, node, param, &tab_tag);

	if (tab_tag == eiNULL_TAG || 
		ei_db_type(nodesys->m_db, tab_tag) != EI_DATA_TYPE_TABLE)
	{
		return eiFALSE;
	}

	ei_data_table_begin(nodesys->m_db, tab_tag, &tab_iter);

	ei_db_cast(
		nodesys->m_db, 
		x, 
		type, 
		ei_data_table_read(&tab_iter, prim_index), 
		tab_iter.tab->item_type);

	ei_data_table_end(&tab_iter);

	return eiTRUE;
}

static eiBool interp_varying_prim_var(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	eiNodeParam *param, 
	const eiTag tessel_tag, 
	const eiIndex tri_index, 
	const eiIndex prim_index, 
	const eiVector *bary, 
	const eiScalar *user_data, 
	const char *name, 
	const eiInt type, 
	eiByte * const x, 
	eiByte * const dx1, 
	eiByte * const dx2)
{
	eiObjectElement		*obj_fn;
	eiScalar			value[ EI_MAX_USER_CHANNEL_DIM ];
	eiScalar			dvalue1[ EI_MAX_USER_CHANNEL_DIM ];
	eiScalar			dvalue2[ EI_MAX_USER_CHANNEL_DIM ];

	obj_fn = (eiObjectElement *)node->object;
	eiDBG_ASSERT(obj_fn != NULL);

	if (!obj_fn->interp_varying(
		nodesys, 
		node, 
		param, 
		tessel_tag, 
		tri_index, 
		prim_index, 
		bary, 
		user_data, 
		name, 
		value, 
		dvalue1, 
		dvalue2))
	{
		return eiFALSE;
	}

	cast_prim_var(type, x, value);
	
	if (dx1 != NULL)
	{
		cast_prim_var(type, dx1, dvalue1);
	}
	
	if (dx2 != NULL)
	{
		cast_prim_var(type, dx2, dvalue2);
	}

	return eiTRUE;
}

static eiBool interp_vertex_prim_var(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	eiNodeParam *param, 
	const eiTag tessel_tag, 
	const eiIndex tri_index, 
	const eiIndex prim_index, 
	const eiVector *bary, 
	const eiScalar *user_data, 
	const char *name, 
	const eiInt type, 
	eiByte * const x, 
	eiByte * const dx1, 
	eiByte * const dx2)
{
	eiObjectElement		*obj_fn;
	eiScalar			value[ EI_MAX_USER_CHANNEL_DIM ];
	eiScalar			dvalue1[ EI_MAX_USER_CHANNEL_DIM ];
	eiScalar			dvalue2[ EI_MAX_USER_CHANNEL_DIM ];

	obj_fn = (eiObjectElement *)node->object;
	eiDBG_ASSERT(obj_fn != NULL);

	if (!obj_fn->interp_vertex(
		nodesys, 
		node, 
		param, 
		tessel_tag, 
		tri_index, 
		prim_index, 
		bary, 
		user_data, 
		name, 
		value, 
		dvalue1, 
		dvalue2))
	{
		return eiFALSE;
	}

	cast_prim_var(type, x, value);
	
	if (dx1 != NULL)
	{
		cast_prim_var(type, dx1, dvalue1);
	}
	
	if (dx2 != NULL)
	{
		cast_prim_var(type, dx2, dvalue2);
	}

	return eiTRUE;
}

eiBool ei_get_prim_var(
	eiState * const state, 
	const char *name, 
	const eiInt type, 
	eiByte * const x, 
	eiByte * const dx1, 
	eiByte * const dx2)
{
	eiDatabase			*db;
	eiSizet				type_size;
	eiNodeSystem		*nodesys;
	eiNode				*node;
	eiIndex				param_index;
	eiNodeParam			*param;
	eiBool				result;

	/* must have hit object for binding */
	if (state->hit_obj == eiNULL_TAG)
	{
		return eiFALSE;
	}

	db = state->db;
	type_size = ei_db_type_size(db, type);

	/* cannot interpolate for varying-sized data element */
	if (type_size == 0)
	{
		return eiFALSE;
	}

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);
	eiDBG_ASSERT(nodesys != NULL);

	/* lookup the node parameter */
	node = (eiNode *)ei_db_access(db, state->hit_obj);

	param_index = ei_nodesys_lookup_parameter(nodesys, node, name);

	if (param_index == eiNULL_INDEX)
	{
		/* cannot find the parameter, try custom parameter binding, 
		   this is for some built-in parameters that are not added 
		   as node parameters */
		if (interp_varying_prim_var(
			nodesys, 
			node, 
			NULL, 
			state->hit_tessel, 
			state->hit_tri, 
			state->hit_prim, 
			&state->bary, 
			state->user_data, 
			name, 
			type, 
			x, 
			dx1, 
			dx2))
		{
			ei_db_end(db, state->hit_obj);

			return eiTRUE;
		}

		if (interp_vertex_prim_var(
			nodesys, 
			node, 
			NULL, 
			state->hit_tessel, 
			state->hit_tri, 
			state->hit_prim, 
			&state->bary, 
			state->user_data, 
			name, 
			type, 
			x, 
			dx1, 
			dx2))
		{
			ei_db_end(db, state->hit_obj);

			return eiTRUE;
		}

		/* still cannot bind, failed */
		ei_db_end(db, state->hit_obj);

		return eiFALSE;
	}

	param = ei_nodesys_read_parameter(nodesys, node, param_index);

	/* bind node parameter according to storage class */
	result = eiFALSE;

	switch (param->storage_class)
	{
	case eiCONSTANT:
		{
			result = get_constant_var(
				nodesys, 
				node, 
				param, 
				type, 
				x);
		}
		break;

	case eiUNIFORM:
		{
			result = get_uniform_prim_var(
				nodesys, 
				node, 
				param, 
				state->hit_prim, 
				type, 
				x);
		}
		break;

	case eiVARYING:
		{
			result = interp_varying_prim_var(
				nodesys, 
				node, 
				param, 
				state->hit_tessel, 
				state->hit_tri, 
				state->hit_prim, 
				&state->bary, 
				state->user_data, 
				name, 
				type, 
				x, 
				dx1, 
				dx2);
		}
		break;

	case eiVERTEX:
		{
			result = interp_vertex_prim_var(
				nodesys, 
				node, 
				param, 
				state->hit_tessel, 
				state->hit_tri, 
				state->hit_prim, 
				&state->bary, 
				state->user_data, 
				name, 
				type, 
				x, 
				dx1, 
				dx2);
		}
		break;

	default:
		{
			result = eiFALSE;
		}
		break;
	}

	ei_db_end(db, state->hit_obj);

	return result;
}

void ei_object_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer)
{
	eiObject				*obj;
	eiNodeDesc				*desc;
	eiObjectElement			*obj_fn;
	eiInstance				*inst;
	eiRayTracer				*rt;
	eiRayObjectInstance		*ray_inst;
	eiTag					ray_inst_tag;
	eiBool					motion;
	eiTag					displace_list;
	eiApprox				*approx;
	eiBool					approx_is_enabled;
	eiTag					obj_rep_inst_tag;
	eiUint					view_time;
	eiTag					obj_rep_tag;

	/* don't create renderable representation for this leaf if it's invisible */
	if (!ei_attr_get_flag(attr, EI_ATTR_VISIBLE))
	{
		return;
	}

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		nodesys->m_db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	obj = (eiObject *)node;
	/* begin accessing node description */
	desc = (eiNodeDesc *)ei_db_access(rt->db, obj->node.desc);
	obj_fn = (eiObjectElement *)ei_node_desc_get_object(desc);
	inst = (eiInstance *)instancer;

	/* create a new ray-traceable object instance */
	ray_inst = (eiRayObjectInstance *)ei_db_create(
		rt->db, 
		&ray_inst_tag, 
		EI_DATA_TYPE_RAY_OBJECT_INST, 
		sizeof(eiRayObjectInstance), 
		EI_DB_FLUSHABLE);

	/* the ray-traceable object representation is still not created 
	   at this moment, we just use this object instance to lookup 
	   the object representation from cache */
	ei_ray_object_instance_init(
		ray_inst, 
		eiNULL_TAG, 
		attr, 
		transform, 
		motion_transform);

	/* lookup the ray-traceable object representation to re-use 
	   previous tessellations */
	motion = ei_attr_get_flag(attr, EI_ATTR_MOTION_BLUR);
	/* we only use the first material for displacement shaders since 
	   we do displacement mapping on vertices, where we don't know 
	   per-primitive material index */
	displace_list = eiNULL_TAG;

	/* get displace shader only when displacement is on */
	if (ei_attr_get_flag(attr, EI_ATTR_DISPLACE) && 
		attr->material_list != eiNULL_TAG && 
		!ei_data_array_empty(rt->db, attr->material_list))
	{
		eiTag	mtl_tag;

		mtl_tag = *((eiTag *)ei_data_array_read(rt->db, attr->material_list, 0));
		ei_data_array_end(rt->db, attr->material_list, 0);

		if (mtl_tag != eiNULL_TAG)
		{
			eiMaterial	*mtl;

			mtl = (eiMaterial *)ei_db_access(rt->db, mtl_tag);

			if (!ei_data_array_empty(rt->db, mtl->displace_list))
			{
				displace_list = mtl->displace_list;
			}

			ei_db_end(rt->db, mtl_tag);
		}
	}

	/* determine the approximation type */
	approx = (eiApprox *)ei_db_access(rt->db, attr->approx);

	approx_is_enabled = ei_approx_is_enabled(approx);
		
	/* disable approximation if the object is polygon and displacement is off */
	if (strcmp(ei_node_desc_name(desc), "poly") == 0 && displace_list == eiNULL_TAG)
	{
		approx_is_enabled = eiFALSE;
	}

	view_time = eiNULL_INDEX;

	if (approx_is_enabled && ei_approx_is_view_dep(approx))
	{
		eiCamera		*cam;

		/* the approximation statement is view-dependent, we need 
		   to encode the transformation of object instance into 
		   object representation key */
		obj_rep_inst_tag = ray_inst_tag;

		/* get the time-stamp of camera if the approximation is view-dependent */
		cam = (eiCamera *)ei_db_access(rt->db, cache->cam_tag);
		
		view_time = cam->node.time;

		ei_db_end(rt->db, cache->cam_tag);
	}
	else
	{
		/* the approximation statement is view-independent, so we 
		   leave the object instance field alone to allow mutiple 
		   object instances share this object representation */
		obj_rep_inst_tag = eiNULL_TAG;
	}

	ei_db_end(rt->db, attr->approx);

	obj_rep_tag = ei_object_rep_map_find(
		&obj->object_reps, 
		obj_rep_inst_tag, 
		attr->approx, 
		motion, 
		displace_list, 
		node->time, 
		view_time, 
		rt->db);

	if (obj_rep_tag == eiNULL_TAG)
	{
		eiRayObject	*ray_obj;
		eiTag		tessel_job_tag;
		eiTesselJob	*tessel_job;

		/* create a new ray-traceable object representation */
		ray_obj = (eiRayObject *)ei_db_create(
			rt->db, 
			&obj_rep_tag, 
			EI_DATA_TYPE_RAY_OBJECT, 
			sizeof(eiRayObject), 
			EI_DB_FLUSHABLE);

		/* set this object node as the source object */
		ei_ray_object_init(
			ray_obj, 
			rt->db, 
			node->tag);

		ei_db_end(rt->db, obj_rep_tag);

		/* add this object representation to the cache */
		ei_object_rep_map_add(
			&obj->object_reps, 
			obj_rep_inst_tag, 
			attr->approx, 
			motion, 
			displace_list, 
			node->time, 
			view_time, 
			obj_rep_tag, 
			rt->db);

		/* we don't do the tessellation job now, instead, we schedule 
		   a tessellation job for this object representation and execute 
		   the job later, after the scene manager have deleted unreferenced 
		   object representations to save memory, and with multi-threaded 
		   processing. */
		tessel_job = (eiTesselJob *)ei_db_create(
			rt->db, 
			&tessel_job_tag, 
			EI_DATA_TYPE_JOB_TESSEL, 
			sizeof(eiTesselJob), 
			EI_DB_FLUSHABLE);

		tessel_job->cam = cache->cam_tag;
		tessel_job->inst = obj_rep_inst_tag;
		tessel_job->approx = attr->approx;
		tessel_job->motion = motion;
		tessel_job->displace_list = displace_list;
		tessel_job->raytraceable = obj_rep_tag;
		tessel_job->subdiv = 0;
		tessel_job->deferred_dice = eiFALSE;
		tessel_job->object_desc = obj->node.desc;
		tessel_job->tessellable = obj_fn->create_obj(rt->db, obj, tessel_job);

		ei_db_end(rt->db, tessel_job_tag);

		ei_job_queue_add(cache->object_rep_jobs, tessel_job_tag);
	}

	/* use the object representation as the referenced object */
	ray_inst->object = obj_rep_tag;

	/* add reference to the ray-traceable object. */
	ei_db_ref(rt->db, obj_rep_tag);

	/* finished creating the ray-traceable object instance */
	ei_db_end(rt->db, ray_inst_tag);

	/* add the newly created object instance to the cache */
	ei_data_array_push_back(rt->db, cache->object_instances, &ray_inst_tag);

	/* done accessing node description */
	ei_db_end(rt->db, obj->node.desc);

	/* add travesed object tag to cache */
	ei_object_rep_cache_add_object(cache, obj->node.tag);
}

static void ei_object_element_init(eiObjectElement *element)
{
	ei_element_init(&element->base);
}

static void ei_object_element_exit(eiObjectElement *element)
{
	ei_element_exit(&element->base);
}

eiObjectElement *ei_create_object_element()
{
	eiObjectElement	*element;

	element = (eiObjectElement *)ei_allocate(sizeof(eiObjectElement));

	ei_object_element_init(element);

	return element;
}

void ei_object_element_deletethis(eiPluginObject *object)
{
	if (object == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_object_element_exit((eiObjectElement *)object);

	ei_free(object);
}

eiUint ei_type_dim(const eiInt type)
{
	switch (type)
	{
	case EI_DATA_TYPE_BYTE:
	case EI_DATA_TYPE_SHORT:
	case EI_DATA_TYPE_INT:
	case EI_DATA_TYPE_BOOL:
	case EI_DATA_TYPE_TAG:
	case EI_DATA_TYPE_INDEX:
	case EI_DATA_TYPE_SCALAR:
		return 1;
	case EI_DATA_TYPE_VECTOR:
		return 3;
	case EI_DATA_TYPE_VECTOR2:
		return 2;
	case EI_DATA_TYPE_VECTOR4:
		return 4;
	default:
		return 0;
	}
}

void ei_type_to_scalars(
	const eiInt type, 
	eiScalar *sval, 
	void *data)
{
	switch (type)
	{
	case EI_DATA_TYPE_BYTE:
		{
			sval[0] = (eiScalar)(*((eiByte *)data));
		}
		break;

	case EI_DATA_TYPE_SHORT:
		{
			sval[0] = (eiScalar)(*((eiShort *)data));
		}
		break;

	case EI_DATA_TYPE_INT:
	case EI_DATA_TYPE_BOOL:
		{
			sval[0] = (eiScalar)(*((eiInt *)data));
		}
		break;

	case EI_DATA_TYPE_TAG:
	case EI_DATA_TYPE_INDEX:
		{
			sval[0] = (eiScalar)(*((eiIndex *)data));
		}
		break;

	case EI_DATA_TYPE_SCALAR:
		{
			sval[0] = *((eiScalar *)data);
		}
		break;

	case EI_DATA_TYPE_VECTOR:
		{
			movv((eiVector *)sval, (eiVector *)data);
		}
		break;

	case EI_DATA_TYPE_VECTOR2:
		{
			movv2((eiVector2 *)sval, (eiVector2 *)data);
		}
		break;

	case EI_DATA_TYPE_VECTOR4:
		{
			movv4((eiVector4 *)sval, (eiVector4 *)data);
		}
		break;
	}
}

void append_user_data_array(
	eiDatabase *db, 
	const eiTag tab_tag, 
	ei_array *user_data_array, 
	const eiInt num_elements)
{
	eiDataTable		*tab;
	eiIntptr		i, j;

	tab = (eiDataTable *)ei_db_access(db, tab_tag);

	for (i = 0; i < num_elements; ++i)
	{
		for (j = 0; j < ei_array_size(user_data_array); ++j)
		{
			eiUserData			*user_data;
			eiDataTableIterator	user_data_iter;
			eiScalar			sval;
			eiVector			vval;
			eiVector2			v2val;
			eiVector4			v4val;

			user_data = (eiUserData *)ei_array_get(user_data_array, j);

			ei_data_table_begin(db, user_data->tag, &user_data_iter);

			switch (user_data->type)
			{
			case EI_DATA_TYPE_BYTE:
				{
					sval = (eiScalar)(*((eiByte *)ei_data_table_read(&user_data_iter, i)));

					ei_data_table_push_back(db, &tab, &sval);
				}
				break;

			case EI_DATA_TYPE_SHORT:
				{
					sval = (eiScalar)(*((eiShort *)ei_data_table_read(&user_data_iter, i)));

					ei_data_table_push_back(db, &tab, &sval);
				}
				break;

			case EI_DATA_TYPE_INT:
			case EI_DATA_TYPE_BOOL:
				{
					sval = (eiScalar)(*((eiInt *)ei_data_table_read(&user_data_iter, i)));

					ei_data_table_push_back(db, &tab, &sval);
				}
				break;

			case EI_DATA_TYPE_TAG:
			case EI_DATA_TYPE_INDEX:
				{
					sval = (eiScalar)(*((eiIndex *)ei_data_table_read(&user_data_iter, i)));

					ei_data_table_push_back(db, &tab, &sval);
				}
				break;

			case EI_DATA_TYPE_SCALAR:
				{
					sval = *((eiScalar *)ei_data_table_read(&user_data_iter, i));

					ei_data_table_push_back(db, &tab, &sval);
				}
				break;

			case EI_DATA_TYPE_VECTOR:
				{
					movv(&vval, (eiVector *)ei_data_table_read(&user_data_iter, i));

					ei_data_table_push_back(db, &tab, &vval.x);
					ei_data_table_push_back(db, &tab, &vval.y);
					ei_data_table_push_back(db, &tab, &vval.z);
				}
				break;

			case EI_DATA_TYPE_VECTOR2:
				{
					movv2(&v2val, (eiVector2 *)ei_data_table_read(&user_data_iter, i));

					ei_data_table_push_back(db, &tab, &v2val.x);
					ei_data_table_push_back(db, &tab, &v2val.y);
				}
				break;

			case EI_DATA_TYPE_VECTOR4:
				{
					movv4(&v4val, (eiVector4 *)ei_data_table_read(&user_data_iter, i));

					ei_data_table_push_back(db, &tab, &v4val.x);
					ei_data_table_push_back(db, &tab, &v4val.y);
					ei_data_table_push_back(db, &tab, &v4val.z);
					ei_data_table_push_back(db, &tab, &v4val.w);
				}
				break;
			}

			ei_data_table_end(&user_data_iter);
		}
	}

	ei_db_end(db, tab_tag);
}

void ei_user_data_array_init(
	ei_array *varyings, 
	ei_array *vertices, 
	eiUint * const varying_dim, 
	eiUint * const vertex_dim, 
	eiNodeSystem *nodesys, 
	eiNode *node)
{
	eiUint		num_params;
	eiUint		i;

	*varying_dim = 0;
	*vertex_dim = 0;

	ei_array_init(varyings, sizeof(eiUserData));
	ei_array_init(vertices, sizeof(eiUserData));

	num_params = ei_nodesys_get_parameter_count(nodesys, node);

	for (i = 0; i < num_params; ++i)
	{
		eiTag			tab_tag;
		eiDataTable		*tab;
		eiNodeParam		*param;
		eiUserData		user_data;

		tab_tag = eiNULL_TAG;

		param = ei_nodesys_read_parameter(nodesys, node, i);

		/* the parameter must be a tag */
		if ((param->storage_class == eiVARYING || 
			param->storage_class == eiVERTEX) && 
			param->type == EI_DATA_TYPE_TAG)
		{
			ei_nodesys_get_parameter_value(
				nodesys, 
				node, 
				param, 
				(void *)(&tab_tag));
		}

		/* the tag must points to a data table */
		if (tab_tag == eiNULL_TAG || 
			ei_db_type(nodesys->m_db, tab_tag) != EI_DATA_TYPE_TABLE)
		{
			continue;
		}

		param = ei_nodesys_write_parameter(nodesys, node, i);

		user_data.param_index = i;
		user_data.tag = tab_tag;
		tab = (eiDataTable *)ei_db_access(nodesys->m_db, tab_tag);
		user_data.type = ei_data_table_type(tab);
		ei_db_end(nodesys->m_db, tab_tag);
		user_data.channel_dim = ei_type_dim(user_data.type);
		/* update the channel dimension in node parameter */
		param->channel_dim = user_data.channel_dim;

		switch (param->storage_class)
		{
		case eiVARYING:
			{
				user_data.channel_offset = *varying_dim;
				(*varying_dim) += user_data.channel_dim;
				/* update the channel offset in node parameter */
				param->channel_offset = user_data.channel_offset;

				ei_array_push_back(varyings, &user_data);
			}
			break;

		case eiVERTEX:
			{
				user_data.channel_offset = *vertex_dim;
				(*vertex_dim) += user_data.channel_dim;
				/* update the channel offset in node parameter */
				param->channel_offset = user_data.channel_offset;

				ei_array_push_back(vertices, &user_data);
			}
			break;

		default:
			{
				ei_error("Unexpected storage class for user data.\n");
			}
			break;
		}
	}

	/* offset vertex user data by varying dimension */
	for (i = 0; i < (eiUint)ei_array_size(vertices); ++i)
	{
		eiUserData		*user_data;
		eiNodeParam		*param;

		user_data = (eiUserData *)ei_array_get(vertices, i);

		param = ei_nodesys_write_parameter(nodesys, node, user_data->param_index);

		param->channel_offset += (*varying_dim);
	}
}

void ei_user_data_array_exit(
	ei_array *varyings, 
	ei_array *vertices)
{
	ei_array_clear(vertices);
	ei_array_clear(varyings);
}

void byteswap_job_tessel(eiDatabase *db, void *data, const eiUint size)
{
	eiTesselJob *job = (eiTesselJob *)data;

	ei_byteswap_int(&job->cam);
	ei_byteswap_int(&job->inst);
	ei_byteswap_int(&job->approx);
	ei_byteswap_int(&job->motion);
	ei_byteswap_int(&job->displace_list);
	ei_byteswap_int(&job->object_desc);
	ei_byteswap_int(&job->tessellable);
	ei_byteswap_int(&job->raytraceable);
	ei_byteswap_int(&job->deferred_dice);
}

static eiFORCEINLINE eiScalar get_edge_angle(
	const eiVector *e0, 
	const eiVector *e1)
{
	eiScalar a = - dot(e0, e1);
	clampi(a, -1.0f, 1.0f);
	return acosf(a);
}

static void ei_calc_tessel_vertex_normals(
	eiDatabase *db, 
	eiRayTessel *tessel, 
	const eiTag obj_tag)
{
	eiNodeSystem		*nodesys;
	eiNode				*node;
	eiIndex				param_index;
	eiNodeParam			*param;
	eiScalar			*N0, *N1, *N2;
	eiUint				num_vertices;
	eiUint				num_triangles;
	eiUint				i;

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);

	node = (eiNode *)ei_db_access(db, obj_tag);

	param_index = ei_nodesys_lookup_parameter(nodesys, node, "N");

	if (param_index == eiNULL_INDEX)
	{
		ei_db_end(db, obj_tag);

		return;
	}

	param = ei_nodesys_read_parameter(nodesys, node, param_index);

	if (param->storage_class != eiVARYING)
	{
		ei_db_end(db, obj_tag);

		return;
	}

	N0 = ei_rt_tessel_get_vertex_channel(tessel, param->channel_offset + 0);
	N1 = ei_rt_tessel_get_vertex_channel(tessel, param->channel_offset + 1);
	N2 = ei_rt_tessel_get_vertex_channel(tessel, param->channel_offset + 2);

	ei_db_end(db, obj_tag);

	if (N0 == NULL || N1 == NULL || N2 == NULL)
	{
		return;
	}

	num_vertices = ei_rt_tessel_get_num_vertices(tessel);
	num_triangles = ei_rt_tessel_get_num_triangles(tessel);

	/* zero vertex normals */
	for (i = 0; i < num_vertices; ++i)
	{
		N0[i] = 0.0f;
		N1[i] = 0.0f;
		N2[i] = 0.0f;
	}

	/* calculate face normal and average for each sharing vertices */
	for (i = 0; i < num_triangles; ++i)
	{
		eiRayTriangle	*tri;
		eiRayVertex		*v1, *v2, *v3;
		eiVector4		fn;
		eiVector		e1, e2, e3;
		eiScalar		w1, w2, w3;

		tri = ei_rt_tessel_get_triangle(tessel, i);
		v1 = ei_rt_tessel_get_vertex(tessel, tri->v1);
		v2 = ei_rt_tessel_get_vertex(tessel, tri->v2);
		v3 = ei_rt_tessel_get_vertex(tessel, tri->v3);

		get_normal(&v1->pos, &v2->pos, &v3->pos, &fn);

		sub(&e1, &v2->pos, &v1->pos);
		sub(&e2, &v3->pos, &v2->pos);
		sub(&e3, &v1->pos, &v3->pos);

		normalizei(&e1);
		normalizei(&e2);
		normalizei(&e3);
		
		w1 = get_edge_angle(&e1, &e3);
		w2 = get_edge_angle(&e2, &e1);
		w3 = get_edge_angle(&e3, &e2);

		N0[tri->v1] += w1 * fn.x;
		N1[tri->v1] += w1 * fn.y;
		N2[tri->v1] += w1 * fn.z;

		N0[tri->v2] += w2 * fn.x;
		N1[tri->v2] += w2 * fn.y;
		N2[tri->v2] += w2 * fn.z;

		N0[tri->v3] += w3 * fn.x;
		N1[tri->v3] += w3 * fn.y;
		N2[tri->v3] += w3 * fn.z;
	}

	/* normalize vertex normals */
	for (i = 0; i < num_vertices; ++i)
	{
		eiVector	vn;

		setv(&vn, N0[i], N1[i], N2[i]);
		normalizei(&vn);

		N0[i] = vn.x;
		N1[i] = vn.y;
		N2[i] = vn.z;
	}
}

void ei_displace_tessel(
	eiDatabase *db, 
	const eiTag displace_list, 
	const eiTag tessel_tag, 
	const eiTag obj_tag, 
	const eiBool motion)
{
	eiNodeSystem	*nodesys;
	eiRayTessel		*tessel;
	eiBound			displaced_box;
	eiBaseBucket	displace_bucket;
	eiUint			num_vertices;
	eiUint			i;

	if (displace_list == eiNULL_TAG)
	{
		return;
	}

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);

	tessel = (eiRayTessel *)ei_db_access(db, tessel_tag);

	/* call displacement shaders for each vertex in the tessellation, re-compute 
	   the bounding box since vertices may be displaced by shaders */
	initb(&displaced_box);

	ei_build_approx_bucket(
		&displace_bucket, 
		db, 
		0);

	num_vertices = ei_rt_tessel_get_num_vertices(tessel);

	for (i = 0; i < num_vertices; ++i)
	{
		eiRayVertex		*vtx;
		eiState			state;
		eiVector4		result;

		vtx = ei_rt_tessel_get_vertex(tessel, i);

		ei_state_init(&state, eiRAY_DISPLACE, &displace_bucket);
		
		state.result = NULL;
		state.P = vtx->pos;
		sub(&state.dPdtime, &vtx->m_pos, &vtx->pos);
		state.dtime = 1.0f;

		initv(&state.Ng);
		initv(&state.N);
		ei_get_prim_var(
			&state, 
			"N", 
			EI_DATA_TYPE_VECTOR, 
			(eiByte *)(&state.N), 
			NULL, 
			NULL);

		initv4(&result);

		ei_call_shader_instance_list(nodesys, &result, &state, displace_list, NULL);

		vtx->pos = state.P;
		addbv(&displaced_box, &state.P);

		if (motion)
		{
			mulvf(&vtx->m_pos, &state.dPdtime, state.dtime);
			addi(&vtx->m_pos, &state.P);

			addbv(&displaced_box, &vtx->m_pos);
		}

		ei_state_exit(&state);
	}

	ei_rt_tessel_box(tessel, &displaced_box);

	/* recalculate vertex normals since we displaced vertices */
	ei_calc_tessel_vertex_normals(db, tessel, obj_tag);

	ei_db_end(db, tessel_tag);
}

eiBool execute_job_tessel(eiDatabase *db, eiBaseWorker *pWorker, void *job, void *param)
{
	eiRayTracer			*rt;
	eiTesselJob			*pJob;
	void				*obj;
	eiNodeDesc			*desc;
	eiObjectElement		*obj_fn;
	eiBound				box;
	eiTag				tessellable;
	eiTag				tessel_tag;

	/* handle user cancellation, it's possible that worker is NULL */
	if (pWorker != NULL && !ei_base_worker_is_running(pWorker))
	{
		return eiTRUE;
	}

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_RAYTRACER);
	
	pJob = (eiTesselJob *)job;

	/* copy this because it may be cleared to eiNULL_TAG from the 
	   job if the dicing is deferred */
	tessellable = pJob->tessellable;
	obj = (void *)ei_db_access(db, tessellable);
	desc = (eiNodeDesc *)ei_db_access(db, pJob->object_desc);
	obj_fn = (eiObjectElement *)ei_node_desc_get_object(desc);

	/* Bound the object in object space 
	   including motion blur(no displacement) */
	initb(&box);

	obj_fn->bound(db, pJob, obj, &box);

	if (pJob->deferred_dice)
	{
		tessel_tag = (eiTag)param;

		/* dice directly if this is a deferred dicing job */
		obj_fn->deferred_dice(db, pJob, obj, &box, tessel_tag);
	}
	else
	{
		/* small enough to dice */
		if (obj_fn->diceable(db, pJob, obj, &box))
		{
			/* dice into grids */
			tessel_tag = obj_fn->dice(db, pJob, obj, &box);

			/* add this tessellation to object representation */
			if (tessel_tag != eiNULL_TAG)
			{
				ei_rt_add_tessel(rt, pJob->raytraceable, tessel_tag);
			}
			else
			{
				ei_warning("Object generates invalid tessellation.\n");
			}
		}
		else
		{
			/* too large, divide and conquer, 
			   add back to job queue */
			obj_fn->split(db, pJob, obj, &box, (eiJobQueue *)param);
		}
	}

	ei_db_end(db, pJob->object_desc);
	ei_db_end(db, tessellable);

	/* delete the tessellable object since it has been processed */
	obj_fn->delete_obj(db, pJob);

	return eiTRUE;
}

eiUint count_job_tessel(eiDatabase *db, void *job)
{
	eiRayObject		*ray_obj;
	eiObject		*obj;
	eiUint			count;

	ray_obj = (eiRayObject *)job;

	obj = (eiObject *)ei_db_access(db, ray_obj->source);
	/* use the volume of the object bounding boxes to estimate 
	   the splitting job amount of the object, we assume bigger 
	   objects will take longer to split */
	count = (eiUint)(bvol(&obj->box) + bvol(&obj->motion_box));
	ei_db_end(db, ray_obj->source);

	return MAX(1, count);
}

void ei_procedural_intersect(
	eiRayObjectInstance *ray_obj_inst, 
	const eiTag tessel_tag, 
	const eiIndex tessel_instance_index, 
	const eiIndex parent_bsptree, 
	eiState *state, 
	ei_array *hit_info_array, 
	const eiBool sort_by_distance)
{
	eiDatabase			*db;
	eiRayObject			*ray_obj;
	eiObject			*obj;
	eiObjectElement		*obj_fn;

	db = state->db;

	ray_obj = (eiRayObject *)ei_db_access(db, ray_obj_inst->object);
	obj = (eiObject *)ei_db_access(db, ray_obj->source);
	obj_fn = (eiObjectElement *)obj->node.object;

	obj_fn->intersect(
		obj, 
		tessel_tag, 
		tessel_instance_index, 
		parent_bsptree, 
		state, 
		hit_info_array, 
		sort_by_distance);

	ei_db_end(db, ray_obj->source);
	ei_db_end(db, ray_obj_inst->object);
}

void ei_install_object_node(eiNodeSystem *nodesys, eiNodeDesc *desc)
{
	eiBound		default_box;

	initb(&default_box);

	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOUND, 
		"box", 
		&default_box);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_BOUND, 
		"motion_box", 
		&default_box);
	/* pad a dummy parameter, this parameter must NOT 
	   be accessed by other remote hosts */
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_NONE, 
		"object_reps", 
		(void *)(sizeof(eiObjectRepMap)));
}
