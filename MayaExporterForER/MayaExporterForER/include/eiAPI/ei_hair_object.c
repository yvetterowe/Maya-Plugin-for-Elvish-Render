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

#include <eiAPI/ei_hair_object.h>
#include <eiAPI/ei_rayhair.h>
#include <eiCORE/ei_data_table.h>
#include <eiCORE/ei_assert.h>

/** \brief The internal tessellable representation(sub-object) of 
 * hair object. */
typedef struct eiHairTessel {
	/* the source object node */
	eiTag		object;
	eiInt		degree;
	eiTag		vertex_list;
	eiTag		motion_vertex_list;
	eiTag		hair_list;
} eiHairTessel;

void ei_hair_object_init(eiNodeSystem *nodesys, eiNode *node)
{
	eiDatabase		*db;
	eiHairObject	*hair;
	eiRayHairTree	*bsptree;

	ei_object_init(nodesys, node);

	db = nodesys->m_db;
	hair = (eiHairObject *)node;

	hair->degree = 1;
	hair->vertex_list = eiNULL_TAG;
	hair->motion_vertex_list = eiNULL_TAG;
	hair->hair_list = eiNULL_TAG;
	hair->bsp_size = 10;
	hair->bsp_depth = 20;
	hair->bsptree = eiNULL_TAG;

	bsptree = (eiRayHairTree *)ei_db_create(
		db, 
		&hair->bsptree, 
		EI_DATA_TYPE_RAY_HAIR_TREE, 
		sizeof(eiRayHairTree), 
		EI_DB_FLUSHABLE);

	bsptree->obj_tag = node->tag;
	bsptree->num_nodes = 0;
	bsptree->gpit_padding = 0;
	initb(&bsptree->box);

	ei_db_end(db, hair->bsptree);
}

void ei_hair_object_exit(eiNodeSystem *nodesys, eiNode *node)
{
	eiDatabase		*db;
	eiHairObject	*hair;

	db = nodesys->m_db;
	hair = (eiHairObject *)node;

	if (hair->bsptree != eiNULL_TAG)
	{
		ei_db_delete(db, hair->bsptree);
		hair->bsptree = eiNULL_TAG;
	}

	if (hair->hair_list != eiNULL_TAG)
	{
		if (ei_db_unref(db, hair->hair_list) == 0)
		{
			ei_delete_data_table(db, hair->hair_list);
		}
		hair->hair_list = eiNULL_TAG;
	}

	if (hair->motion_vertex_list != eiNULL_TAG)
	{
		if (ei_db_unref(db, hair->motion_vertex_list) == 0)
		{
			ei_delete_data_table(db, hair->motion_vertex_list);
		}
		hair->motion_vertex_list = eiNULL_TAG;
	}

	if (hair->vertex_list != eiNULL_TAG)
	{
		if (ei_db_unref(db, hair->vertex_list) == 0)
		{
			ei_delete_data_table(db, hair->vertex_list);
		}
		hair->vertex_list = eiNULL_TAG;
	}

	ei_object_exit(nodesys, node);
}

void ei_hair_object_changed(eiNodeSystem *nodesys, eiNode *node)
{
	eiDatabase		*db;
	eiHairObject	*hair;

	db = nodesys->m_db;
	hair = (eiHairObject *)node;

	/* dirt the BSP-tree to issue a rebuild when node changed */
	if (hair->bsptree != eiNULL_TAG)
	{
		ei_db_dirt(db, hair->bsptree);
	}
}

eiTag ei_hair_object_create(
	eiDatabase *db, 
	eiObject *src_obj, 
	eiTesselJob *job)
{
	eiNodeSystem	*nodesys;
	eiHairObject	*src_hair;
	eiHairTessel	*hair;
	eiTag			tessellable;

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);

	src_hair = (eiHairObject *)src_obj;

	hair = (eiHairTessel *)ei_db_create(
		db, 
		&tessellable, 
		EI_DATA_TYPE_INTARRAY, 
		sizeof(eiHairTessel), 
		EI_DB_FLUSHABLE);

	/* record the tag of the source object node */
	hair->object = src_obj->node.tag;
	hair->degree = src_hair->degree;

	/* reference vertex list */
	hair->vertex_list = src_hair->vertex_list;
	ei_db_ref(db, hair->vertex_list);

	/* reference motion vertex list */
	hair->motion_vertex_list = src_hair->motion_vertex_list;
	ei_db_ref(db, hair->motion_vertex_list);

	/* reference hair list */
	hair->hair_list = src_hair->hair_list;
	ei_db_ref(db, hair->hair_list);

	ei_db_end(db, tessellable);

	return tessellable;
}

void ei_hair_object_delete(
	eiDatabase *db, 
	eiTesselJob *job)
{
	eiHairTessel	*hair;

	if (job->tessellable != eiNULL_TAG)
	{
		hair = (eiHairTessel *)ei_db_access(db, job->tessellable);

		if (hair->hair_list != eiNULL_TAG)
		{
			if (ei_db_unref(db, hair->hair_list) == 0)
			{
				ei_delete_data_table(db, hair->hair_list);
			}
			hair->hair_list = eiNULL_TAG;
		}

		if (hair->motion_vertex_list != eiNULL_TAG)
		{
			if (ei_db_unref(db, hair->motion_vertex_list) == 0)
			{
				ei_delete_data_table(db, hair->motion_vertex_list);
			}
			hair->motion_vertex_list = eiNULL_TAG;
		}

		if (hair->vertex_list != eiNULL_TAG)
		{
			if (ei_db_unref(db, hair->vertex_list) == 0)
			{
				ei_delete_data_table(db, hair->vertex_list);
			}
			hair->vertex_list = eiNULL_TAG;
		}

		ei_db_end(db, job->tessellable);

		ei_db_delete(db, job->tessellable);
		job->tessellable = eiNULL_TAG;
	}
}

void ei_hair_object_bound(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	eiBound *box)
{
	eiHairTessel			*hair;
	eiInt					num_vertices;
	eiInt					i;
	eiDataTableIterator		vertex_list_iter;
	eiDataTableIterator		motion_vertex_list_iter;

	hair = (eiHairTessel *)obj;

	ei_data_table_begin(db, hair->vertex_list, &vertex_list_iter);
	num_vertices = vertex_list_iter.tab->item_count;

	if (job->motion)
	{
		ei_data_table_begin(db, hair->motion_vertex_list, &motion_vertex_list_iter);

		/* iterate all vertices and compute the bound */
		for (i = 0; i < num_vertices; ++i)
		{
			/* add position to the bound */
			addbv(box, (eiVector *)ei_data_table_read(&vertex_list_iter, i));

			/* add motion position to the bound */
			addbv(box, (eiVector *)ei_data_table_read(&motion_vertex_list_iter, i));
		}

		ei_data_table_end(&motion_vertex_list_iter);
	}
	else
	{
		/* iterate all vertices and compute the bound */
		for (i = 0; i < num_vertices; ++i)
		{
			/* add position to the bound */
			addbv(box, (eiVector *)ei_data_table_read(&vertex_list_iter, i));
		}
	}

	ei_data_table_end(&vertex_list_iter);
}

eiBool ei_hair_object_diceable(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box)
{
	/* currently hair object will be diced directly without splitting */
	return eiTRUE;
}

void ei_hair_object_deferred_dice(
	eiDatabase *db, 
	eiTesselJob *job, 
	eiObject *obj, 
	const eiBound *box, 
	const eiTag deferred_tessel_tag)
{
	ei_error("Hair does not support deferred dicing.\n");
}

eiTag ei_hair_object_dice(
	eiDatabase *db, 
	eiTesselJob *job, 
	eiObject *obj, 
	const eiBound *box)
{
	eiRayTracer		*rt;
	eiNodeSystem	*nodesys;
	eiHairTessel	*hair;
	eiRayTessel		*tessel;
	eiTag			tessel_tag;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);

	hair = (eiHairTessel *)obj;

	/* create procedural tessellation */
	tessel = ei_rt_proc_tessel(rt, &tessel_tag);

	ei_rt_tessel_box(tessel, box);
	/* for hair tessellation, we simply want a placeholder 
	   in the main BSP-tree, we don't need to dynamically 
	   generate any tessellated micro-triangles in render-time, 
	   so we simply set the job to NULL. */
	ei_rt_tessel_job(tessel, eiNULL_TAG);

	ei_rt_end_proc_tessel(rt, tessel, tessel_tag);

	return tessel_tag;
}

void ei_hair_object_split(
	eiDatabase *db, 
	eiTesselJob *job, 
	eiObject *obj, 
	const eiBound *box, 
	eiJobQueue *queue)
{
	ei_error("Hair does not support splitting.\n");
}

eiBool ei_hair_object_interp_varying(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	eiNodeParam *param, 
	const eiTag tessel_tag, 
	const eiIndex tri_index, 
	const eiIndex prim_index, 
	const eiVector *bary, 
	const eiScalar *user_data, 
	const char *name, 
	eiScalar * const x, 
	eiScalar * const dx1, 
	eiScalar * const dx2)
{
	eiHairObject			*hair;
	eiDataTableIterator		hair_curve_iter;
	eiDataTableIterator		hair_data_iter;
	eiIndex					hair_vtx_offset;
	eiTag					tab_tag;
	eiInt					dim;
	eiScalar				v0[ EI_MAX_USER_CHANNEL_DIM ];
	eiScalar				v1[ EI_MAX_USER_CHANNEL_DIM ];
	eiInt					i;

	hair = (eiHairObject *)node;

	if (param == NULL)
	{
		if (strcmp(name, "u") == 0)
		{
			*x = bary->x;
			*dx1 = 1.0f;
			*dx2 = 0.0f;

			return eiTRUE;
		}
		else if (strcmp(name, "v") == 0)
		{
			*x = 0.0f;
			*dx1 = 0.0f;
			*dx2 = 1.0f;

			return eiTRUE;
		}

		return eiFALSE;
	}
	else
	{
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

		ei_data_table_begin(nodesys->m_db, hair->hair_list, &hair_curve_iter);
		ei_data_table_begin(nodesys->m_db, tab_tag, &hair_data_iter);

		hair_vtx_offset = *((eiIndex *)ei_data_table_read(&hair_curve_iter, prim_index * 2));
		hair_vtx_offset += tri_index;

		dim = ei_type_dim(hair_data_iter.tab->item_type);

		ei_type_to_scalars(
			hair_data_iter.tab->item_type, 
			v0, 
			ei_data_table_read(&hair_data_iter, hair_vtx_offset + 0));
		ei_type_to_scalars(
			hair_data_iter.tab->item_type, 
			v1, 
			ei_data_table_read(&hair_data_iter, hair_vtx_offset + 1));

		for (i = 0; i < dim; ++i)
		{
			lerp(&x[i], v0[i], v1[i], bary->x);

			dx1[i] = v1[i] - v0[i];
		}

		ei_data_table_end(&hair_data_iter);
		ei_data_table_end(&hair_curve_iter);

		return eiTRUE;
	}
}

eiBool ei_hair_object_interp_vertex(
	eiNodeSystem *nodesys, 
	eiNode *node, 
	eiNodeParam *param, 
	const eiTag tessel_tag, 
	const eiIndex tri_index, 
	const eiIndex prim_index, 
	const eiVector *bary, 
	const eiScalar *user_data, 
	const char *name, 
	eiScalar * const x, 
	eiScalar * const dx1, 
	eiScalar * const dx2)
{
	eiHairObject			*hair;
	eiDataTableIterator		hair_curve_iter;
	eiDataTableIterator		hair_data_iter;
	eiIndex					hair_vtx_offset;
	eiTag					tab_tag;

	hair = (eiHairObject *)node;

	if (param == NULL)
	{
		if (strcmp(name, "P") == 0)
		{
			eiDataTableIterator		hair_vtx_iter;

			ei_data_table_begin(nodesys->m_db, hair->hair_list, &hair_curve_iter);
			ei_data_table_begin(nodesys->m_db, hair->vertex_list, &hair_vtx_iter);

			hair_vtx_offset = *((eiIndex *)ei_data_table_read(&hair_curve_iter, prim_index * 2));
			hair_vtx_offset += (hair->degree * tri_index);

			switch (hair->degree)
			{
			case 1:
				{
					eiVector4	*v0, *v1;
					eiCurve1	c;
					eiVector4	p;

					v0 = (eiVector4 *)ei_data_table_read(&hair_vtx_iter, hair_vtx_offset + 0);
					movv(&c.p[0].xyz, &v0->xyz);
					c.p[0].w = v0->w;
					v1 = (eiVector4 *)ei_data_table_read(&hair_vtx_iter, hair_vtx_offset + 1);
					movv(&c.p[1].xyz, &v1->xyz);
					c.p[1].w = v1->w;

					ei_curve1_eval_exported(&c, &p, bary->x);
					movv((eiVector *)x, &p.xyz);
					ei_curve1_evalT_exported(&c, (eiVector *)dx1, bary->x);
					cross((eiVector *)dx2, (eiVector *)dx1, (eiVector *)user_data);
				}
				break;

			case 2:
				{
					eiVector4	*v0, *v1, *v2;
					eiCurve2	c;
					eiVector4	p;

					v0 = (eiVector4 *)ei_data_table_read(&hair_vtx_iter, hair_vtx_offset + 0);
					movv(&c.p[0].xyz, &v0->xyz);
					c.p[0].w = v0->w;
					v1 = (eiVector4 *)ei_data_table_read(&hair_vtx_iter, hair_vtx_offset + 1);
					movv(&c.p[1].xyz, &v1->xyz);
					c.p[1].w = v1->w;
					v2 = (eiVector4 *)ei_data_table_read(&hair_vtx_iter, hair_vtx_offset + 2);
					movv(&c.p[2].xyz, &v2->xyz);
					c.p[2].w = v2->w;

					ei_curve2_eval_exported(&c, &p, bary->x);
					movv((eiVector *)x, &p.xyz);
					ei_curve2_evalT_exported(&c, (eiVector *)dx1, bary->x);
					cross((eiVector *)dx2, (eiVector *)dx1, (eiVector *)user_data);
				}
				break;

			case 3:
				{
					eiVector4	*v0, *v1, *v2, *v3;
					eiCurve3	c;
					eiVector4	p;

					v0 = (eiVector4 *)ei_data_table_read(&hair_vtx_iter, hair_vtx_offset + 0);
					movv(&c.p[0].xyz, &v0->xyz);
					c.p[0].w = v0->w;
					v1 = (eiVector4 *)ei_data_table_read(&hair_vtx_iter, hair_vtx_offset + 1);
					movv(&c.p[1].xyz, &v1->xyz);
					c.p[1].w = v1->w;
					v2 = (eiVector4 *)ei_data_table_read(&hair_vtx_iter, hair_vtx_offset + 2);
					movv(&c.p[2].xyz, &v2->xyz);
					c.p[2].w = v2->w;
					v3 = (eiVector4 *)ei_data_table_read(&hair_vtx_iter, hair_vtx_offset + 3);
					movv(&c.p[3].xyz, &v3->xyz);
					c.p[3].w = v3->w;

					ei_curve3_eval_exported(&c, &p, bary->x);
					movv((eiVector *)x, &p.xyz);
					ei_curve3_evalT_exported(&c, (eiVector *)dx1, bary->x);
					cross((eiVector *)dx2, (eiVector *)dx1, (eiVector *)user_data);
				}
				break;
			}

			ei_data_table_end(&hair_vtx_iter);
			ei_data_table_end(&hair_curve_iter);

			return eiTRUE;
		}
		else if (strcmp(name, "Pm") == 0)
		{
			eiDataTableIterator		hair_motion_vtx_iter;

			ei_data_table_begin(nodesys->m_db, hair->hair_list, &hair_curve_iter);
			ei_data_table_begin(nodesys->m_db, hair->motion_vertex_list, &hair_motion_vtx_iter);

			hair_vtx_offset = *((eiIndex *)ei_data_table_read(&hair_curve_iter, prim_index * 2));
			hair_vtx_offset += (hair->degree * tri_index);

			switch (hair->degree)
			{
			case 1:
				{
					eiVector4	*v0, *v1;
					eiCurve1	c;
					eiVector4	p;

					v0 = (eiVector4 *)ei_data_table_read(&hair_motion_vtx_iter, hair_vtx_offset + 0);
					movv(&c.p[0].xyz, &v0->xyz);
					c.p[0].w = v0->w;
					v1 = (eiVector4 *)ei_data_table_read(&hair_motion_vtx_iter, hair_vtx_offset + 1);
					movv(&c.p[1].xyz, &v1->xyz);
					c.p[1].w = v1->w;

					ei_curve1_eval_exported(&c, &p, bary->x);
					movv((eiVector *)x, &p.xyz);
					ei_curve1_evalT_exported(&c, (eiVector *)dx1, bary->x);
					cross((eiVector *)dx2, (eiVector *)dx1, (eiVector *)user_data);
				}
				break;

			case 2:
				{
					eiVector4	*v0, *v1, *v2;
					eiCurve2	c;
					eiVector4	p;

					v0 = (eiVector4 *)ei_data_table_read(&hair_motion_vtx_iter, hair_vtx_offset + 0);
					movv(&c.p[0].xyz, &v0->xyz);
					c.p[0].w = v0->w;
					v1 = (eiVector4 *)ei_data_table_read(&hair_motion_vtx_iter, hair_vtx_offset + 1);
					movv(&c.p[1].xyz, &v1->xyz);
					c.p[1].w = v1->w;
					v2 = (eiVector4 *)ei_data_table_read(&hair_motion_vtx_iter, hair_vtx_offset + 2);
					movv(&c.p[2].xyz, &v2->xyz);
					c.p[2].w = v2->w;

					ei_curve2_eval_exported(&c, &p, bary->x);
					movv((eiVector *)x, &p.xyz);
					ei_curve2_evalT_exported(&c, (eiVector *)dx1, bary->x);
					cross((eiVector *)dx2, (eiVector *)dx1, (eiVector *)user_data);
				}
				break;

			case 3:
				{
					eiVector4	*v0, *v1, *v2, *v3;
					eiCurve3	c;
					eiVector4	p;

					v0 = (eiVector4 *)ei_data_table_read(&hair_motion_vtx_iter, hair_vtx_offset + 0);
					movv(&c.p[0].xyz, &v0->xyz);
					c.p[0].w = v0->w;
					v1 = (eiVector4 *)ei_data_table_read(&hair_motion_vtx_iter, hair_vtx_offset + 1);
					movv(&c.p[1].xyz, &v1->xyz);
					c.p[1].w = v1->w;
					v2 = (eiVector4 *)ei_data_table_read(&hair_motion_vtx_iter, hair_vtx_offset + 2);
					movv(&c.p[2].xyz, &v2->xyz);
					c.p[2].w = v2->w;
					v3 = (eiVector4 *)ei_data_table_read(&hair_motion_vtx_iter, hair_vtx_offset + 3);
					movv(&c.p[3].xyz, &v3->xyz);
					c.p[3].w = v3->w;

					ei_curve3_eval_exported(&c, &p, bary->x);
					movv((eiVector *)x, &p.xyz);
					ei_curve3_evalT_exported(&c, (eiVector *)dx1, bary->x);
					cross((eiVector *)dx2, (eiVector *)dx1, (eiVector *)user_data);
				}
				break;
			}

			ei_data_table_end(&hair_motion_vtx_iter);
			ei_data_table_end(&hair_curve_iter);

			return eiTRUE;
		}

		return eiFALSE;
	}
	else
	{
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

		ei_data_table_begin(nodesys->m_db, hair->hair_list, &hair_curve_iter);
		ei_data_table_begin(nodesys->m_db, tab_tag, &hair_data_iter);

		hair_vtx_offset = *((eiIndex *)ei_data_table_read(&hair_curve_iter, prim_index * 2));
		hair_vtx_offset += (hair->degree * tri_index);

		switch (hair->degree)
		{
		case 1:
			{
				eiInt		dim;
				eiScalar	v0[ EI_MAX_USER_CHANNEL_DIM ];
				eiScalar	v1[ EI_MAX_USER_CHANNEL_DIM ];

				dim = ei_type_dim(hair_data_iter.tab->item_type);

				ei_type_to_scalars(
					hair_data_iter.tab->item_type, 
					v0, 
					ei_data_table_read(&hair_data_iter, hair_vtx_offset + 0));
				ei_type_to_scalars(
					hair_data_iter.tab->item_type, 
					v1, 
					ei_data_table_read(&hair_data_iter, hair_vtx_offset + 1));

				ei_curve1_eval_scalars(dim, v0, v1, x, bary->x);
				ei_curve1_evalT_scalars(dim, v0, v1, dx1, bary->x);
			}
			break;

		case 2:
			{
				eiInt		dim;
				eiScalar	v0[ EI_MAX_USER_CHANNEL_DIM ];
				eiScalar	v1[ EI_MAX_USER_CHANNEL_DIM ];
				eiScalar	v2[ EI_MAX_USER_CHANNEL_DIM ];

				dim = ei_type_dim(hair_data_iter.tab->item_type);

				ei_type_to_scalars(
					hair_data_iter.tab->item_type, 
					v0, 
					ei_data_table_read(&hair_data_iter, hair_vtx_offset + 0));
				ei_type_to_scalars(
					hair_data_iter.tab->item_type, 
					v1, 
					ei_data_table_read(&hair_data_iter, hair_vtx_offset + 1));
				ei_type_to_scalars(
					hair_data_iter.tab->item_type, 
					v2, 
					ei_data_table_read(&hair_data_iter, hair_vtx_offset + 2));

				ei_curve2_eval_scalars(dim, v0, v1, v2, x, bary->x);
				ei_curve2_evalT_scalars(dim, v0, v1, v2, dx1, bary->x);
			}
			break;

		case 3:
			{
				eiInt		dim;
				eiScalar	v0[ EI_MAX_USER_CHANNEL_DIM ];
				eiScalar	v1[ EI_MAX_USER_CHANNEL_DIM ];
				eiScalar	v2[ EI_MAX_USER_CHANNEL_DIM ];
				eiScalar	v3[ EI_MAX_USER_CHANNEL_DIM ];

				dim = ei_type_dim(hair_data_iter.tab->item_type);

				ei_type_to_scalars(
					hair_data_iter.tab->item_type, 
					v0, 
					ei_data_table_read(&hair_data_iter, hair_vtx_offset + 0));
				ei_type_to_scalars(
					hair_data_iter.tab->item_type, 
					v1, 
					ei_data_table_read(&hair_data_iter, hair_vtx_offset + 1));
				ei_type_to_scalars(
					hair_data_iter.tab->item_type, 
					v2, 
					ei_data_table_read(&hair_data_iter, hair_vtx_offset + 2));
				ei_type_to_scalars(
					hair_data_iter.tab->item_type, 
					v3, 
					ei_data_table_read(&hair_data_iter, hair_vtx_offset + 3));

				ei_curve3_eval_scalars(dim, v0, v1, v2, v3, x, bary->x);
				ei_curve3_evalT_scalars(dim, v0, v1, v2, v3, dx1, bary->x);
			}
			break;
		}

		ei_data_table_end(&hair_data_iter);
		ei_data_table_end(&hair_curve_iter);

		return eiTRUE;
	}
}

eiNodeObject *ei_create_hair_object_node_object(void *param)
{
	eiObjectElement	*element;

	element = ei_create_object_element();

	((eiPluginObject *)element)->deletethis = ei_object_element_deletethis;
	((eiNodeObject *)element)->init_node = ei_hair_object_init;
	((eiNodeObject *)element)->exit_node = ei_hair_object_exit;
	((eiNodeObject *)element)->node_changed = ei_hair_object_changed;
	((eiElement *)element)->update_instance = ei_object_update_instance;
	element->create_obj = ei_hair_object_create;
	element->delete_obj = ei_hair_object_delete;
	element->bound = ei_hair_object_bound;
	element->diceable = ei_hair_object_diceable;
	element->dice = ei_hair_object_dice;
	element->deferred_dice = ei_hair_object_deferred_dice;
	element->split = ei_hair_object_split;
	element->intersect = ei_hair_object_intersect;
	element->interp_varying = ei_hair_object_interp_varying;
	element->interp_vertex = ei_hair_object_interp_vertex;

	return ((eiNodeObject *)element);
}

void ei_install_hair_object_node(eiNodeSystem *nodesys)
{
	eiTag		desc_tag;
	eiNodeDesc	*desc;
	eiTag		default_tag;
	eiInt		default_int;

	desc = ei_nodesys_node_desc(nodesys, &desc_tag, "hair");
	if (desc == NULL)
	{
		return;
	}

	default_tag = eiNULL_TAG;
	default_int = 0;

	ei_install_object_node(nodesys, desc);

	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"degree", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"vertex_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"motion_vertex_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"hair_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"bsp_size", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"bsp_depth", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"bsptree", 
		&default_tag);

	ei_nodesys_end_node_desc(nodesys, desc, desc_tag);
}
