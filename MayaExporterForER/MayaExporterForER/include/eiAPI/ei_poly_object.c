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

#include <eiAPI/ei_poly_object.h>
#include <eiAPI/ei_approx.h>
#include <eiAPI/ei_camera.h>
#include <eiAPI/ei_raytracer.h>
#include <eiAPI/ei_buffer.h>
#include <eiCORE/ei_algorithm.h>
#include <eiCORE/ei_pool.h>
#include <eiCORE/ei_assert.h>

#define NUM_TRI_CHANS	7

/** \brief Get barycentric coordinates from positions */
static eiFORCEINLINE void get_bary(eiVector *bary, const eiVector *pos, 
	const eiVector *pos1, const eiVector *pos2, const eiVector *pos3)
{
	eiVector	fpos, fpos1, fpos2, fpos3;
	eiScalar	area;

	movv(&fpos, pos);
	movv(&fpos1, pos1);
	movv(&fpos2, pos2);
	movv(&fpos3, pos3);

	if (almost_zero(fpos1.x, eiSCALAR_EPS) && 
		almost_zero(fpos2.x, eiSCALAR_EPS) && 
		almost_zero(fpos3.x, eiSCALAR_EPS))
	{
		fpos.x = 1.0f;
		fpos1.x = 1.0f;
		fpos2.x = 1.0f;
		fpos3.x = 1.0f;
	}

	if (almost_zero(fpos1.y, eiSCALAR_EPS) && 
		almost_zero(fpos2.y, eiSCALAR_EPS) && 
		almost_zero(fpos3.y, eiSCALAR_EPS))
	{
		fpos.y = 1.0f;
		fpos1.y = 1.0f;
		fpos2.y = 1.0f;
		fpos3.y = 1.0f;
	}

	if (almost_zero(fpos1.z, eiSCALAR_EPS) && 
		almost_zero(fpos2.z, eiSCALAR_EPS) && 
		almost_zero(fpos3.z, eiSCALAR_EPS))
	{
		fpos.z = 1.0f;
		fpos1.z = 1.0f;
		fpos2.z = 1.0f;
		fpos3.z = 1.0f;
	}

	area = tri_area(&fpos1, &fpos2, &fpos3);

	bary->x = tri_area(&fpos, &fpos2, &fpos3) / area;
	bary->y = tri_area(&fpos, &fpos3, &fpos1) / area;
	bary->z = 1.0f - bary->x - bary->y;
}

void ei_poly_object_init(eiNodeSystem *nodesys, eiNode *node)
{
	eiPolyObject	*poly;

	ei_object_init(nodesys, node);

	poly = (eiPolyObject *)node;

	poly->pos_list = eiNULL_TAG;
	poly->motion_pos_list = eiNULL_TAG;
	poly->triangle_list = eiNULL_TAG;
}

void ei_poly_object_exit(eiNodeSystem *nodesys, eiNode *node)
{
	eiDatabase		*db;
	eiPolyObject	*poly;

	db = nodesys->m_db;
	poly = (eiPolyObject *)node;

	if (poly->pos_list != eiNULL_TAG)
	{
		if (ei_db_unref(db, poly->pos_list) == 0)
		{
			ei_delete_data_table(db, poly->pos_list);
		}
		poly->pos_list = eiNULL_TAG;
	}

	if (poly->motion_pos_list != eiNULL_TAG)
	{
		if (ei_db_unref(db, poly->motion_pos_list) == 0)
		{
			ei_delete_data_table(db, poly->motion_pos_list);
		}
		poly->motion_pos_list = eiNULL_TAG;
	}

	if (poly->triangle_list != eiNULL_TAG)
	{
		if (ei_db_unref(db, poly->triangle_list) == 0)
		{
			ei_delete_data_table(db, poly->triangle_list);
		}
		poly->triangle_list = eiNULL_TAG;
	}

	ei_object_exit(nodesys, node);
}

/** \brief Generic edge segments estimator. */
typedef eiInt (*estimate_num_sub_edges_func)(
	const eiVector *pos1, const eiVector *pos2, 
	const eiVector *motion_pos1, const eiVector *motion_pos2, 
	void *param);

/** \brief Estimate edge segments in regular mode. */
typedef struct estimate_num_sub_edges_regular_params {
	eiInt			num_sub_edges;
} estimate_num_sub_edges_regular_params;

static eiInt estimate_num_sub_edges_regular(
	const eiVector *pos1, const eiVector *pos2, 
	const eiVector *motion_pos1, const eiVector *motion_pos2, 
	void *param)
{
	estimate_num_sub_edges_regular_params *params;

	params = (estimate_num_sub_edges_regular_params *)param;

	return params->num_sub_edges;
}

/** \brief Estimate edge segments in length mode. */
typedef struct estimate_num_sub_edges_length_params {
	eiBool			view_dep;
	eiBool			motion;
	eiScalar		length;
	eiScalar		motion_factor;
	eiMatrix		object_to_view;
	eiMatrix		motion_object_to_view;
	eiTag			cam;
	eiDatabase		*db;
} estimate_num_sub_edges_length_params;

static eiInt estimate_num_sub_edges_length(
	const eiVector *pos1, const eiVector *pos2, 
	const eiVector *motion_pos1, const eiVector *motion_pos2, 
	void *param)
{
	estimate_num_sub_edges_length_params *params;
	eiCamera	*cam;
	eiVector	vpos1, vpos2;
	eiScalar	length;
	eiScalar	shading_rate;

	params = (estimate_num_sub_edges_length_params *)param;

	cam = (eiCamera *)ei_db_access(params->db, params->cam);

	if (params->view_dep)
	{
		ei_camera_object_to_screen(cam, &vpos1, pos1, &params->object_to_view);
		ei_camera_object_to_screen(cam, &vpos2, pos2, &params->object_to_view);

		length = dist2(&vpos1.xy, &vpos2.xy);
	}
	else
	{
		length = dist(pos1, pos2);
	}

	shading_rate = params->length;

	/* scale up the shading rate if the object is moving fast */
	if (params->motion)
	{
		eiScalar	length_ratio;

		if (params->view_dep)
		{
			eiVector	motion_vpos1, motion_vpos2;

			ei_camera_object_to_screen(cam, &motion_vpos1, motion_pos1, &params->motion_object_to_view);
			ei_camera_object_to_screen(cam, &motion_vpos2, motion_pos2, &params->motion_object_to_view);

			length_ratio = (dist2(&motion_vpos1.xy, &vpos1.xy) + dist2(&motion_vpos2.xy, &vpos2.xy)) * 0.5f;
		}
		else
		{
			length_ratio = (dist(motion_pos1, pos1) + dist(motion_pos2, pos2)) * 0.5f;
		}

		length_ratio = MAX(1.0f, params->motion_factor * length_ratio);

		shading_rate = shading_rate * length_ratio;
	}

	ei_db_end(params->db, params->cam);

	/* at least one edge */
	return MAX(1, (eiInt)(length / shading_rate));
}

static void build_edge_estimator(
	eiDatabase *db, 
	eiTesselJob *job, 
	estimate_num_sub_edges_func				*estimator, 
	estimate_num_sub_edges_regular_params	*estimate_default_params, 
	estimate_num_sub_edges_regular_params	*estimate_regular_params, 
	estimate_num_sub_edges_length_params	*estimate_length_params, 
	void									**estimator_params)
{
	eiApprox		*approx;

	*estimator = estimate_num_sub_edges_regular;
	estimate_default_params->num_sub_edges = 1;
	*estimator_params = (void *)estimate_default_params;

	/* disable approximation if the object is polygon and displacement is off */
	if (job->displace_list == eiNULL_TAG)
	{
		return;
	}

	approx = (eiApprox *)ei_db_access(db, job->approx);

	if (approx->method == EI_APPROX_METHOD_REGULAR)
	{
		if (approx->args[EI_APPROX_U] != 0.0f)
		{
			/* polygons only use U parameter for regular approximation */
			estimate_regular_params->num_sub_edges = (1 << (eiInt)(approx->args[EI_APPROX_U]));

			*estimator = estimate_num_sub_edges_regular;
			*estimator_params = (void *)estimate_regular_params;
		}
	}
	else if (approx->method == EI_APPROX_METHOD_LENGTH)
	{
		if (approx->args[EI_APPROX_LENGTH] != 0.0f)
		{
			estimate_length_params->view_dep = approx->view_dep;
			estimate_length_params->motion = job->motion;
			estimate_length_params->length = approx->args[EI_APPROX_LENGTH];
			estimate_length_params->motion_factor = approx->motion_factor;
			estimate_length_params->cam = job->cam;
			estimate_length_params->db = db;
			
			if (approx->view_dep && job->inst != eiNULL_TAG)
			{
				eiCamera				*cam;
				eiRayObjectInstance		*inst;

				cam = (eiCamera *)ei_db_access(db, job->cam);
				inst = (eiRayObjectInstance *)ei_db_access(db, job->inst);

				mulmm(&estimate_length_params->object_to_view, 
					&inst->object_to_world, &cam->world_to_camera);
				mulmm(&estimate_length_params->motion_object_to_view, 
					&inst->motion_object_to_world, &cam->motion_world_to_camera);

				ei_db_end(db, job->inst);
				ei_db_end(db, job->cam);
			}
			else
			{
				initm(&estimate_length_params->object_to_view, 1.0f);
				initm(&estimate_length_params->motion_object_to_view, 1.0f);
			}

			*estimator = estimate_num_sub_edges_length;
			*estimator_params = (void *)estimate_length_params;
		}
	}

	ei_db_end(db, job->approx);
}

eiTag ei_poly_object_create(
	eiDatabase *db, 
	eiObject *src_obj, 
	eiTesselJob *job)
{
	eiNodeSystem	*nodesys;
	eiPolyObject	*src_poly;
	eiPolyTessel	*poly;
	eiTag			tessellable;
	ei_array		varyings, vertices;
	eiInt			num_vertices;
	eiInt			num_triangles;
	eiInt			items_per_slot;
	eiInt			i;
	eiDataTableIterator	triangle_list_iter;
	eiDataTableIterator	pos_list_iter;
	eiDataTableIterator	motion_pos_list_iter;
	eiDataTable			*triangle_tab;
	
	estimate_num_sub_edges_func				estimator;
	estimate_num_sub_edges_regular_params	estimate_default_params;
	estimate_num_sub_edges_regular_params	estimate_regular_params;
	estimate_num_sub_edges_length_params	estimate_length_params;
	void									*estimator_params;

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);

	src_poly = (eiPolyObject *)src_obj;

	poly = (eiPolyTessel *)ei_db_create(
		db, 
		&tessellable, 
		EI_DATA_TYPE_INTARRAY, 
		sizeof(eiPolyTessel), 
		EI_DB_FLUSHABLE);

	/* record the tag of the source object node */
	poly->object = src_obj->node.tag;

	ei_user_data_array_init(
		&varyings, 
		&vertices, 
		&poly->varying_dim, 
		&poly->vertex_dim, 
		nodesys, 
		(eiNode *)src_poly);

	/* reference position list and motion position list */
	poly->pos_list = src_poly->pos_list;
	ei_db_ref(db, poly->pos_list);

	poly->motion_pos_list = src_poly->motion_pos_list;
	ei_db_ref(db, poly->motion_pos_list);

	/* create lists for user data */
	poly->varying_list = eiNULL_TAG;
	poly->vertex_list = eiNULL_TAG;

	ei_data_table_begin(db, src_poly->triangle_list, &triangle_list_iter);
	ei_data_table_begin(db, poly->pos_list, &pos_list_iter);
	ei_data_table_begin(db, poly->motion_pos_list, &motion_pos_list_iter);

	num_vertices = pos_list_iter.tab->item_count;
	num_triangles = triangle_list_iter.tab->item_count / 3;
	items_per_slot = pos_list_iter.tab->items_per_slot;

	if (poly->varying_dim > 0)
	{
		poly->varying_list = ei_create_data_table(db, EI_DATA_TYPE_SCALAR, items_per_slot);
		ei_db_ref(db, poly->varying_list);
		append_user_data_array(db, poly->varying_list, &varyings, num_vertices);
	}

	if (poly->vertex_dim > 0)
	{
		poly->vertex_list = ei_create_data_table(db, EI_DATA_TYPE_SCALAR, items_per_slot);
		ei_db_ref(db, poly->vertex_list);
		append_user_data_array(db, poly->vertex_list, &vertices, num_triangles);
	}

	ei_user_data_array_exit(&varyings, &vertices);

	/* create new triangle list with edge segments channel */
	items_per_slot = triangle_list_iter.tab->items_per_slot;

	poly->triangle_list = ei_create_data_table(db, EI_DATA_TYPE_INDEX, items_per_slot);
	ei_db_ref(db, poly->triangle_list);

	/* build edge estimator */
	build_edge_estimator(
		db, 
		job, 
		&estimator, 
		&estimate_default_params, 
		&estimate_regular_params, 
		&estimate_length_params, 
		&estimator_params);

	triangle_tab = (eiDataTable *)ei_db_access(db, poly->triangle_list);

	for (i = 0; i < num_triangles; ++i)
	{
		eiIndex		v1, v2, v3;
		eiVector	pos1, pos2, pos3;
		eiVector	motion_pos1, motion_pos2, motion_pos3;
		eiIndex		e1, e2, e3;

		/* access 3 vertex indices of triangle */
		v1 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * 3 + 0));
		v2 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * 3 + 1));
		v3 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * 3 + 2));

		/* access 3 vertices */
		movv(&pos1, (eiVector *)ei_data_table_read(&pos_list_iter, v1));
		movv(&motion_pos1, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v1));

		movv(&pos2, (eiVector *)ei_data_table_read(&pos_list_iter, v2));
		movv(&motion_pos2, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v2));

		movv(&pos3, (eiVector *)ei_data_table_read(&pos_list_iter, v3));
		movv(&motion_pos3, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v3));

		/* add triangle indices channel */
		ei_data_table_push_back(db, &triangle_tab, &v1);
		ei_data_table_push_back(db, &triangle_tab, &v2);
		ei_data_table_push_back(db, &triangle_tab, &v3);

		/* call estimator for all 3 edges */
		e1 = estimator(&pos1, &pos2, &motion_pos1, &motion_pos2, estimator_params);
		e2 = estimator(&pos2, &pos3, &motion_pos2, &motion_pos3, estimator_params);
		e3 = estimator(&pos3, &pos1, &motion_pos3, &motion_pos1, estimator_params);

		/* add edge segments channel */
		ei_data_table_push_back(db, &triangle_tab, &e1);
		ei_data_table_push_back(db, &triangle_tab, &e2);
		ei_data_table_push_back(db, &triangle_tab, &e3);

		/* add primitive index channel */
		ei_data_table_push_back(db, &triangle_tab, &i);
	}

	ei_db_end(db, poly->triangle_list);

	ei_data_table_end(&motion_pos_list_iter);
	ei_data_table_end(&pos_list_iter);
	ei_data_table_end(&triangle_list_iter);

	ei_db_end(db, tessellable);

	return tessellable;
}

void ei_poly_object_delete(
	eiDatabase *db, 
	eiTesselJob *job)
{
	eiPolyTessel	*poly;

	if (job->tessellable != eiNULL_TAG)
	{
		poly = (eiPolyTessel *)ei_db_access(db, job->tessellable);

		if (poly->triangle_list != eiNULL_TAG)
		{
			if (ei_db_unref(db, poly->triangle_list) == 0)
			{
				ei_delete_data_table(db, poly->triangle_list);
			}
			poly->triangle_list = eiNULL_TAG;
		}

		if (poly->vertex_list != eiNULL_TAG)
		{
			if (ei_db_unref(db, poly->vertex_list) == 0)
			{
				ei_delete_data_table(db, poly->vertex_list);
			}
			poly->vertex_list = eiNULL_TAG;
		}

		if (poly->varying_list != eiNULL_TAG)
		{
			if (ei_db_unref(db, poly->varying_list) == 0)
			{
				ei_delete_data_table(db, poly->varying_list);
			}
			poly->varying_list = eiNULL_TAG;
		}

		if (poly->motion_pos_list != eiNULL_TAG)
		{
			if (ei_db_unref(db, poly->motion_pos_list) == 0)
			{
				ei_delete_data_table(db, poly->motion_pos_list);
			}
			poly->motion_pos_list = eiNULL_TAG;
		}

		if (poly->pos_list != eiNULL_TAG)
		{
			if (ei_db_unref(db, poly->pos_list) == 0)
			{
				ei_delete_data_table(db, poly->pos_list);
			}
			poly->pos_list = eiNULL_TAG;
		}

		ei_db_end(db, job->tessellable);

		ei_db_delete(db, job->tessellable);
		job->tessellable = eiNULL_TAG;
	}
}

void ei_poly_object_bound(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	eiBound *box)
{
	eiPolyTessel			*poly;
	eiInt					num_vertices;
	eiInt					i;
	eiDataTableIterator		pos_list_iter;
	eiDataTableIterator		motion_pos_list_iter;

	poly = (eiPolyTessel *)obj;

	ei_data_table_begin(db, poly->pos_list, &pos_list_iter);
	num_vertices = pos_list_iter.tab->item_count;

	if (job->motion)
	{
		ei_data_table_begin(db, poly->motion_pos_list, &motion_pos_list_iter);

		/* iterate all vertices and compute the bound */
		for (i = 0; i < num_vertices; ++i)
		{
			/* add position to the bound */
			addbv(box, (eiVector *)ei_data_table_read(&pos_list_iter, i));
			
			/* add motion position to the bound */
			addbv(box, (eiVector *)ei_data_table_read(&motion_pos_list_iter, i));
		}

		ei_data_table_end(&motion_pos_list_iter);
	}
	else
	{
		/* iterate all vertices and compute the bound */
		for (i = 0; i < num_vertices; ++i)
		{
			/* add position to the bound */
			addbv(box, (eiVector *)ei_data_table_read(&pos_list_iter, i));
		}
	}

	ei_data_table_end(&pos_list_iter);
}

static eiFORCEINLINE eiUint ei_poly_object_estimate_grid_size(
	eiDatabase *db, 
	eiTesselJob *job, 
	eiPolyTessel *poly, 
	const eiBound *box, 
	eiApprox *approx)
{
	eiInt				num_triangles;
	eiUint				grid_size;
	eiInt				i;
	eiDataTableIterator	triangle_list_iter;

	grid_size = 0;

	ei_data_table_begin(db, poly->triangle_list, &triangle_list_iter);
	num_triangles = triangle_list_iter.tab->item_count / NUM_TRI_CHANS;

	for (i = 0; i < num_triangles; ++i)
	{
		eiIndex		e1, e2, e3;

		/* access 3 edge segments */
		e1 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 3));
		e2 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 4));
		e3 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 5));

		grid_size += (e1 * e2 + e2 * e3 + e3 * e1) / 3;
	}

	ei_data_table_end(&triangle_list_iter);

	return grid_size;
}

eiBool ei_poly_object_diceable(
	eiDatabase *db, 
	eiTesselJob *job, 
	void *obj, 
	const eiBound *box)
{
	eiPolyTessel	*poly;
	eiApprox		*approx;
	eiDataTable		*triangle_tab;
	eiInt			num_triangles;
	eiUint			max_grid_size;
	eiUint			grid_size;

	approx = (eiApprox *)ei_db_access(db, job->approx);

	/* reached maximum subdivison level, no subdivision anymore, 
	   dice immediately */
	if (job->subdiv >= approx->max_subdiv)
	{
		ei_db_end(db, job->approx);

		return eiTRUE;
	}

	/* max grid size should not be less than 1 */
	max_grid_size = MAX(1, approx->max_grid_size);

	poly = (eiPolyTessel *)obj;

	triangle_tab = (eiDataTable *)ei_db_access(db, poly->triangle_list);
	num_triangles = ei_data_table_size(triangle_tab) / NUM_TRI_CHANS;
	ei_db_end(db, poly->triangle_list);

	/* disable approximation if the object is polygon and displacement is off */
	if (job->displace_list == eiNULL_TAG)
	{
		grid_size = (eiUint)num_triangles;
	}
	else
	{
		if (approx->method == EI_APPROX_METHOD_REGULAR)
		{
			if (approx->args[EI_APPROX_U] == 0.0f)
			{
				grid_size = (eiUint)num_triangles;
			}
			else
			{
				grid_size = ei_poly_object_estimate_grid_size(
					db, job, poly, box, approx);
			}
		}
		else if (approx->method == EI_APPROX_METHOD_LENGTH)
		{
			if (approx->args[EI_APPROX_LENGTH] == 0.0f)
			{
				grid_size = (eiUint)num_triangles;
			}
			else
			{
				grid_size = ei_poly_object_estimate_grid_size(
					db, job, poly, box, approx);
			}
		}
	}

	ei_db_end(db, job->approx);

	return (grid_size <= max_grid_size);
}

static eiTag ei_poly_object_direct_dice(
	eiDatabase *db, 
	eiRayTracer *rt, 
	eiPolyTessel *poly, 
	const eiBound *box)
{
	eiRayTessel				*tessel;
	eiTag					tessel_tag;
	eiInt					num_vertices;
	eiInt					num_triangles;
	eiInt					i, j;
	eiDataTableIterator		pos_list_iter;
	eiDataTableIterator		motion_pos_list_iter;
	eiDataTableIterator		varying_list_iter;
	eiDataTableIterator		vertex_list_iter;
	eiDataTableIterator		triangle_list_iter;

	ei_data_table_begin(db, poly->pos_list, &pos_list_iter);
	ei_data_table_begin(db, poly->motion_pos_list, &motion_pos_list_iter);
	ei_data_table_begin(db, poly->varying_list, &varying_list_iter);
	ei_data_table_begin(db, poly->vertex_list, &vertex_list_iter);
	ei_data_table_begin(db, poly->triangle_list, &triangle_list_iter);

	num_vertices = pos_list_iter.tab->item_count;
	num_triangles = triangle_list_iter.tab->item_count / NUM_TRI_CHANS;

	/* no approximation, dice directly */
	tessel = ei_rt_tessel(
		rt, 
		&tessel_tag, 
		num_vertices, 
		num_triangles, 
		0, 
		poly->varying_dim + poly->vertex_dim);

	ei_rt_tessel_box(tessel, box);

	/* iterate all vertices and build vertices */
	for (i = 0; i < num_vertices; ++i)
	{
		eiRayVertex		vtx;

		/* copy position */
		movv(&vtx.pos, (eiVector *)ei_data_table_read(&pos_list_iter, i));

		/* copy motion position */
		movv(&vtx.m_pos, (eiVector *)ei_data_table_read(&motion_pos_list_iter, i));

		ei_rt_tessel_add_vertex(tessel, &vtx);

		/* copy user-defined vertex data */
		for (j = 0; j < poly->varying_dim; ++j)
		{
			ei_rt_tessel_add_vertex_data(
				tessel, 
				j, 
				ei_data_table_read(&varying_list_iter, i * poly->varying_dim + j));
		}

		for (j = 0; j < poly->vertex_dim; ++j)
		{
			ei_rt_tessel_add_vertex_data(
				tessel, 
				poly->varying_dim + j, 
				ei_data_table_read(&vertex_list_iter, i * poly->vertex_dim + j));
		}
	}

	for (i = 0; i < num_triangles; ++i)
	{
		eiRayTriangle	tri;

		tri.v1 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 0));
		tri.v2 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 1));
		tri.v3 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 2));
		tri.prim_index = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 6));

		ei_rt_tessel_add_triangle(tessel, &tri);
	}

	ei_data_table_end(&triangle_list_iter);
	ei_data_table_end(&vertex_list_iter);
	ei_data_table_end(&varying_list_iter);
	ei_data_table_end(&motion_pos_list_iter);
	ei_data_table_end(&pos_list_iter);

	ei_rt_end_tessel(rt, tessel, tessel_tag);

	return tessel_tag;
}

static eiTag ei_poly_object_indirect_dice(
	eiDatabase *db, 
	eiRayTracer *rt, 
	eiTesselJob *job, 
	eiPolyTessel *poly, 
	const eiBound *box)
{
	eiRayTessel		*tessel;
	eiTag			tessel_tag;
	eiTesselJob		*deferred_job;
	eiTag			dicer;

	/* clone the tessellation job as the deferred job */
	deferred_job = (eiTesselJob *)ei_db_create(
		db, 
		&dicer, 
		EI_DATA_TYPE_JOB_TESSEL, 
		sizeof(eiTesselJob), 
		EI_DB_FLUSHABLE);

	memcpy(deferred_job, job, sizeof(eiTesselJob));

	/* flag this job as deferred dicing job */
	deferred_job->deferred_dice = eiTRUE;

	ei_db_end(db, dicer);

	/* clear the tessellation from the source job so that it won't be 
	   deleted, we will delete it after the deferred job being executed. */
	job->tessellable = eiNULL_TAG;

	/* create deferred tessellation */
	tessel = ei_rt_defer_tessel(rt, &tessel_tag);

	ei_rt_tessel_box(tessel, box);
	ei_rt_tessel_job(tessel, dicer);

	ei_rt_end_defer_tessel(rt, tessel, tessel_tag);

	return tessel_tag;
}

typedef struct eiPolyEdge {
	/* the number of line segments that this edge has */
	eiInt		num_sub_edges;
	/* the index to the first tessellated vertex 
	   that this edge references, NOT including the 
	   triangle corners, only the vertices in between 
	   triangle corners */
	eiIndex		vtx_index;
} eiPolyEdge;

void ei_poly_edge_init(void *item)
{
	eiPolyEdge *edge = (eiPolyEdge *)item;

	/* use -1 to flag that this edge has not been referenced 
	   and initialized by any triangle before */
	edge->num_sub_edges = -1;
	edge->vtx_index = 0;
}

/** \brief Interpolate user-defined vertex data by barycentric coordinates. */
static eiFORCEINLINE void interp_vertex_data(
	eiDatabase *db, 
	eiPolyTessel *poly, 
	ei_array *vertex_data, 
	eiDataTableIterator *varying_list_iter, 
	eiDataTableIterator *vertex_list_iter, 
	const eiIndex v1, 
	const eiIndex v2, 
	const eiIndex v3, 
	const eiVector *bary)
{
	eiInt	i;

	for (i = 0; i < poly->varying_dim; ++i)
	{
		eiScalar	sval;

		interp_scalar(&sval, 
			*((eiScalar *)ei_data_table_read(varying_list_iter, v1 * poly->varying_dim + i)), 
			*((eiScalar *)ei_data_table_read(varying_list_iter, v2 * poly->varying_dim + i)), 
			*((eiScalar *)ei_data_table_read(varying_list_iter, v3 * poly->varying_dim + i)), 
			bary);

		ei_array_push_back(vertex_data, &sval);
	}

	for (i = 0; i < poly->vertex_dim; ++i)
	{
		eiScalar	sval;

		interp_scalar(&sval, 
			*((eiScalar *)ei_data_table_read(vertex_list_iter, v1 * poly->vertex_dim + i)), 
			*((eiScalar *)ei_data_table_read(vertex_list_iter, v2 * poly->vertex_dim + i)), 
			*((eiScalar *)ei_data_table_read(vertex_list_iter, v3 * poly->vertex_dim + i)), 
			bary);

		ei_array_push_back(vertex_data, &sval);
	}
}

/** \brief Linearly interpolate user-defined vertex data */
static eiFORCEINLINE void lerp_vertex_data(
	eiDatabase *db, 
	eiPolyTessel *poly, 
	ei_array *vertex_data, 
	eiDataTableIterator *varying_list_iter, 
	eiDataTableIterator *vertex_list_iter, 
	const eiIndex v1, 
	const eiIndex v2, 
	const eiScalar t)
{
	eiInt	i;

	for (i = 0; i < poly->varying_dim; ++i)
	{
		eiScalar	sval;

		lerp(&sval, 
			*((eiScalar *)ei_data_table_read(varying_list_iter, v1 * poly->varying_dim + i)), 
			*((eiScalar *)ei_data_table_read(varying_list_iter, v2 * poly->varying_dim + i)), 
			t);

		ei_array_push_back(vertex_data, &sval);
	}

	for (i = 0; i < poly->vertex_dim; ++i)
	{
		eiScalar	sval;

		lerp(&sval, 
			*((eiScalar *)ei_data_table_read(vertex_list_iter, v1 * poly->vertex_dim + i)), 
			*((eiScalar *)ei_data_table_read(vertex_list_iter, v2 * poly->vertex_dim + i)), 
			t);

		ei_array_push_back(vertex_data, &sval);
	}
}

/** \brief Add user-defined vertex data */
static eiFORCEINLINE void add_vertex_data(
	eiDatabase *db, 
	eiPolyTessel *poly, 
	ei_array *vertex_data, 
	eiDataTableIterator *varying_list_iter, 
	eiDataTableIterator *vertex_list_iter, 
	const eiIndex v)
{
	eiInt	i;

	for (i = 0; i < poly->varying_dim; ++i)
	{
		eiScalar	sval;

		sval = *((eiScalar *)ei_data_table_read(varying_list_iter, v * poly->varying_dim + i));

		ei_array_push_back(vertex_data, &sval);
	}

	for (i = 0; i < poly->vertex_dim; ++i)
	{
		eiScalar	sval;

		sval = *((eiScalar *)ei_data_table_read(vertex_list_iter, v * poly->vertex_dim + i));

		ei_array_push_back(vertex_data, &sval);
	}
}

static eiFORCEINLINE eiPolyEdge *build_edge_vertices(
	const eiIndex tv1, 
	const eiIndex tv2, 
	eiBool * const edge_flipped, 
	eiBuffer *edges, 
	ei_array *vertices, 
	ei_array *vertex_data, 
	eiDataTableIterator *varying_list_iter, 
	eiDataTableIterator *vertex_list_iter, 
	const eiVector *pos1, const eiVector *pos2, 
	const eiVector *motion_pos1, const eiVector *motion_pos2, 
	const eiIndex num_sub_edges, 
	eiDatabase *db, 
	eiPolyTessel *poly)
{
	eiIndex			v1, v2, temp_v;
	eiPolyEdge		*edge;
	eiScalar		vtx_dt, vtx_t;
	eiRayVertex		new_vtx;
	eiInt			i;

	v1 = tv1;
	v2 = tv2;
	*edge_flipped = eiFALSE;

	if (v1 > v2)
	{
		temp_v = v1;
		v1 = v2;
		v2 = temp_v;
		*edge_flipped = eiTRUE;
	}
	
	edge = (eiPolyEdge *)ei_buffer_getptr(edges, v1, v2);

	/* only estimate once when it has not been estimated before */
	if (edge->num_sub_edges == -1)
	{
		edge->num_sub_edges = num_sub_edges;
		edge->vtx_index = (eiIndex)ei_array_size(vertices);

		vtx_dt = 1.0f / (eiScalar)edge->num_sub_edges;
		vtx_t = vtx_dt;
		
		if (*edge_flipped)
		{
			vtx_dt = -vtx_dt;
			vtx_t = 1.0f - vtx_t;
		}

		for (i = 1; i < edge->num_sub_edges; ++i)
		{
			lerp3(&new_vtx.pos, pos1, pos2, vtx_t);
			lerp3(&new_vtx.m_pos, motion_pos1, motion_pos2, vtx_t);

			ei_array_push_back(vertices, &new_vtx);

			lerp_vertex_data(db, poly, vertex_data, varying_list_iter, vertex_list_iter, tv1, tv2, vtx_t);

			vtx_t += vtx_dt;
		}
	}

	return edge;
}

/** \brief Connect outer edge and inner edge of tessellated triangle 
 * in a regular way */
static eiFORCEINLINE void connect_two_edges(
	ei_array *triangles, 
	const eiIndex prim_index, 
	eiIndex *e1, const eiInt e1_size, 
	eiIndex *e2, const eiInt e2_size, 
	eiDatabase *db, 
	eiPolyTessel *poly)
{
	eiRayTriangle	new_tri;
	eiInt			i;

	for (i = 0; i < (e2_size - 1); ++i)
	{
		new_tri.v1 = e1[i];
		new_tri.v2 = e1[i + 1];
		new_tri.v3 = e2[i];
		new_tri.prim_index = prim_index;

		ei_array_push_back(triangles, &new_tri);

		new_tri.v1 = e2[i];
		new_tri.v2 = e1[i + 1];
		new_tri.v3 = e2[i + 1];
		new_tri.prim_index = prim_index;

		ei_array_push_back(triangles, &new_tri);
	}

	new_tri.v1 = e1[e2_size - 1];
	new_tri.v2 = e1[e2_size];
	new_tri.v3 = e2[e2_size - 1];
	new_tri.prim_index = prim_index;

	ei_array_push_back(triangles, &new_tri);
}

void connect_two_edges_exported(
	ei_array *triangles, 
	const eiIndex prim_index, 
	eiIndex *e1, const eiInt e1_size, 
	eiIndex *e2, const eiInt e2_size, 
	eiDatabase *db, 
	eiPolyTessel *poly)
{
	connect_two_edges(
		triangles, 
		prim_index, 
		e1, e1_size, 
		e2, e2_size, 
		db, 
		poly);
}

/** \brief Build a regular tessellated grid for a triangle */
static eiFORCEINLINE void build_triangle_grid(
	ei_array *vertices, 
	ei_array *vertex_data, 
	eiDataTableIterator *varying_list_iter, 
	eiDataTableIterator *vertex_list_iter, 
	ei_array *triangles, 
	const eiIndex prim_index, 
	const eiIndex v1, const eiIndex v2, const eiIndex v3, 
	const eiVector *pos1, const eiVector *pos2, const eiVector *pos3, 
	const eiVector *motion_pos1, const eiVector *motion_pos2, const eiVector *motion_pos3, 
	const eiInt num_edges, 
	const eiIndex vertex_offset, 
	eiDatabase *db, 
	eiPolyTessel *poly)
{
	eiInt		row, col;
	eiVector	bary1, bary2, bary3;
	eiVector	spos1, spos2, spos3;

	bary1.x = 1.0f - 2.0f / (eiScalar)(num_edges + 3);
	bary1.y = (1.0f - bary1.x) * 0.5f;
	bary1.z = bary1.y;

	bary2.x = bary1.y;
	bary2.y = bary1.x;
	bary2.z = bary1.y;

	bary3.x = bary1.y;
	bary3.y = bary1.y;
	bary3.z = bary1.x;

	/* make inner triangle grid smaller */
	interp_point(&spos1, pos1, pos2, pos3, &bary1);
	interp_point(&spos2, pos1, pos2, pos3, &bary2);
	interp_point(&spos3, pos1, pos2, pos3, &bary3);

	/* build tessellated vertices */
	for (row = 0; row <= num_edges; ++row)
	{
		eiVector		A, B;
		eiVector		uv;
		eiRayVertex		new_vtx;

		lerp3(&A, &spos1, &spos2, (eiScalar)row / (eiScalar)num_edges);
		lerp3(&B, &spos1, &spos3, (eiScalar)row / (eiScalar)num_edges);

		if (row == 0)
		{
			new_vtx.pos = A;
			get_bary(&uv, &new_vtx.pos, pos1, pos2, pos3);

			interp_point(&new_vtx.m_pos, motion_pos1, motion_pos2, motion_pos3, &uv);

			ei_array_push_back(vertices, &new_vtx);

			interp_vertex_data(db, poly, vertex_data, varying_list_iter, vertex_list_iter, v1, v2, v3, &uv);
		}
		else
		{
			for (col = 0; col <= row; ++col)
			{
				lerp3(&new_vtx.pos, &A, &B, (eiScalar)col / (eiScalar)row);
				get_bary(&uv, &new_vtx.pos, pos1, pos2, pos3);

				interp_point(&new_vtx.m_pos, motion_pos1, motion_pos2, motion_pos3, &uv);

				ei_array_push_back(vertices, &new_vtx);

				interp_vertex_data(db, poly, vertex_data, varying_list_iter, vertex_list_iter, v1, v2, v3, &uv);
			}
		}
	}

	/* build tessellated triangles */
	for (row = 0; row < num_edges; ++row)
	{
		for (col = 0; col < (2 * row + 1); ++col)
		{
			eiRayTriangle	new_tri;

			if ((col % 2) == 0)
			{
				new_tri.v1 = vertex_offset + sum(row) + col / 2;
				new_tri.v2 = vertex_offset + sum(row + 1) + col / 2;
				new_tri.v3 = vertex_offset + sum(row + 1) + col / 2 + 1;
			}
			else
			{
				new_tri.v1 = vertex_offset + sum(row) + (col - 1) / 2;
				new_tri.v2 = vertex_offset + sum(row + 1) + (col + 1) / 2;
				new_tri.v3 = vertex_offset + sum(row) + (col + 1) / 2;
			}

			new_tri.prim_index = prim_index;

			ei_array_push_back(triangles, &new_tri);
		}
	}
}

void build_triangle_grid_exported(
	ei_array *vertices, 
	ei_array *vertex_data, 
	eiDataTableIterator *varying_list_iter, 
	eiDataTableIterator *vertex_list_iter, 
	ei_array *triangles, 
	const eiIndex prim_index, 
	const eiIndex v1, const eiIndex v2, const eiIndex v3, 
	const eiVector *pos1, const eiVector *pos2, const eiVector *pos3, 
	const eiVector *motion_pos1, const eiVector *motion_pos2, const eiVector *motion_pos3, 
	const eiInt num_edges, 
	const eiIndex vertex_offset, 
	eiDatabase *db, 
	eiPolyTessel *poly)
{
	build_triangle_grid(
		vertices, 
		vertex_data, 
		varying_list_iter, 
		vertex_list_iter, 
		triangles, 
		prim_index, 
		v1, v2, v3, 
		pos1, pos2, pos3, 
		motion_pos1, motion_pos2, motion_pos3, 
		num_edges, 
		vertex_offset, 
		db, 
		poly);
}

static void dice_triangle_regular(
	ei_array *vertices, 
	ei_array *vertex_data, 
	eiDataTableIterator *varying_list_iter, 
	eiDataTableIterator *vertex_list_iter, 
	ei_array *triangles, 
	const eiIndex prim_index, 
	const eiIndex v1, const eiIndex v2, const eiIndex v3, 
	const eiVector *pos1, const eiVector *pos2, const eiVector *pos3, 
	const eiVector *motion_pos1, const eiVector *motion_pos2, const eiVector *motion_pos3, 
	eiPolyEdge *e1, const eiBool e1_flipped, 
	eiPolyEdge *e2, const eiBool e2_flipped, 
	eiPolyEdge *e3, const eiBool e3_flipped, 
	const eiInt num_edges, 
	ei_pool *pool, 
	eiDatabase *db, 
	eiPolyTessel *poly)
{
	eiInt		num_inner_edges;
	eiInt		num_inner_vertices;
	eiIndex		inner_vertex_offset;
	eiInt		row, col;
	eiInt		max_num_sub_edges;
	eiIndex		*outer_edge, *inner_edge;
	eiInt		inner_edge_size;
	eiIndex		vtx_index;
	eiInt		i;

	/* handle degenerated cases */
	if (num_edges <= 1)
	{
		/* just need to form 1 triangle from the source triangle */
		eiRayTriangle	new_tri;

		new_tri.v1 = v1;
		new_tri.v2 = v2;
		new_tri.v3 = v3;
		new_tri.prim_index = prim_index;

		ei_array_push_back(triangles, &new_tri);

		return;
	}
	else if (num_edges == 2)
	{
		/* just need to form 4 triangles by connecting edge vertices */
		eiRayTriangle	new_tri;

		new_tri.v1 = v1;
		new_tri.v2 = e1->vtx_index;
		new_tri.v3 = e3->vtx_index;
		new_tri.prim_index = prim_index;

		ei_array_push_back(triangles, &new_tri);

		new_tri.v1 = e1->vtx_index;
		new_tri.v2 = v2;
		new_tri.v3 = e2->vtx_index;
		new_tri.prim_index = prim_index;

		ei_array_push_back(triangles, &new_tri);

		new_tri.v1 = e3->vtx_index;
		new_tri.v2 = e2->vtx_index;
		new_tri.v3 = v3;
		new_tri.prim_index = prim_index;

		ei_array_push_back(triangles, &new_tri);

		new_tri.v1 = e1->vtx_index;
		new_tri.v2 = e2->vtx_index;
		new_tri.v3 = e3->vtx_index;
		new_tri.prim_index = prim_index;

		ei_array_push_back(triangles, &new_tri);

		return;
	}
	else
	{
		num_inner_edges = num_edges - 3;
		num_inner_vertices = (num_inner_edges + 2) * (num_inner_edges + 1) / 2;
		inner_vertex_offset = (eiIndex)ei_array_size(vertices);

		if (num_edges == 3)
		{
			/* the inner grid becomes a single point */
			eiVector		uv;
			eiRayVertex		new_vtx;

			add(&new_vtx.pos, pos1, pos2);
			addi(&new_vtx.pos, pos3);
			mulvfi(&new_vtx.pos, 1.0f / 3.0f);
			get_bary(&uv, &new_vtx.pos, pos1, pos2, pos3);

			interp_point(&new_vtx.m_pos, motion_pos1, motion_pos2, motion_pos3, &uv);

			ei_array_push_back(vertices, &new_vtx);

			interp_vertex_data(db, poly, vertex_data, varying_list_iter, vertex_list_iter, v1, v2, v3, &uv);
		}
		else
		{
			/* build inner grid */
			build_triangle_grid(
				vertices, 
				vertex_data, 
				varying_list_iter, 
				vertex_list_iter, 
				triangles, 
				prim_index, 
				v1, v2, v3, 
				pos1, pos2, pos3, 
				motion_pos1, motion_pos2, motion_pos3, 
				num_inner_edges, inner_vertex_offset, 
				db, 
				poly);
		}
	}

	/* build outer grids */
	max_num_sub_edges = MAX(e1->num_sub_edges + 1, MAX(e2->num_sub_edges + 1, e3->num_sub_edges + 1));
	outer_edge = (eiIndex *)ei_pool_allocate(pool, 
		sizeof(eiIndex) * (max_num_sub_edges + (num_inner_edges + 2)));
	inner_edge = outer_edge + max_num_sub_edges;

	/* connect inner edge and outer edge 1 */
	outer_edge[0] = v1;
	if (e1_flipped)
	{
		for (i = 1; i < e1->num_sub_edges; ++i)
		{
			outer_edge[e1->num_sub_edges - i] = e1->vtx_index + i - 1;
		}
	}
	else
	{
		for (i = 1; i < e1->num_sub_edges; ++i)
		{
			outer_edge[i] = e1->vtx_index + i - 1;
		}
	}
	outer_edge[e1->num_sub_edges] = v2;

	if (e3_flipped)
	{
		inner_edge[0] = e3->vtx_index;
	}
	else
	{
		inner_edge[0] = e3->vtx_index + e3->num_sub_edges - 2;
	}
	inner_edge_size = 1;
	vtx_index = inner_vertex_offset;

	for (row = 0; row <= num_inner_edges; ++row)
	{
		inner_edge[inner_edge_size] = vtx_index;
		++ inner_edge_size;
		vtx_index += (row + 1);
	}

	connect_two_edges(
		triangles, 
		prim_index, 
		outer_edge, 
		e1->num_sub_edges + 1, 
		inner_edge, 
		inner_edge_size, 
		db, 
		poly);

	/* connect inner edge and outer edge 2 */
	outer_edge[0] = v2;
	if (e2_flipped)
	{
		for (i = 1; i < e2->num_sub_edges; ++i)
		{
			outer_edge[e2->num_sub_edges - i] = e2->vtx_index + i - 1;
		}
	}
	else
	{
		for (i = 1; i < e2->num_sub_edges; ++i)
		{
			outer_edge[i] = e2->vtx_index + i - 1;
		}
	}
	outer_edge[e1->num_sub_edges] = v3;

	if (e1_flipped)
	{
		inner_edge[0] = e1->vtx_index;
	}
	else
	{
		inner_edge[0] = e1->vtx_index + e1->num_sub_edges - 2;
	}
	inner_edge_size = 1;
	vtx_index = inner_vertex_offset + num_inner_vertices - 1 - num_inner_edges;

	for (col = 0; col <= num_inner_edges; ++col)
	{
		inner_edge[inner_edge_size] = vtx_index;
		++ inner_edge_size;
		++ vtx_index;
	}
	
	connect_two_edges(
		triangles, 
		prim_index, 
		outer_edge, 
		e2->num_sub_edges + 1, 
		inner_edge, 
		inner_edge_size, 
		db, 
		poly);

	/* connect inner edge and outer edge 3 */
	outer_edge[0] = v3;
	if (e3_flipped)
	{
		for (i = 1; i < e3->num_sub_edges; ++i)
		{
			outer_edge[e3->num_sub_edges - i] = e3->vtx_index + i - 1;
		}
	}
	else
	{
		for (i = 1; i < e3->num_sub_edges; ++i)
		{
			outer_edge[i] = e3->vtx_index + i - 1;
		}
	}
	outer_edge[e1->num_sub_edges] = v1;

	if (e2_flipped)
	{
		inner_edge[0] = e2->vtx_index;
	}
	else
	{
		inner_edge[0] = e2->vtx_index + e2->num_sub_edges - 2;
	}
	inner_edge_size = 1;
	vtx_index = inner_vertex_offset + num_inner_vertices - 1;

	for (row = num_inner_edges; row >= 0; --row)
	{
		inner_edge[inner_edge_size] = vtx_index;
		++ inner_edge_size;
		vtx_index -= (row + 1);
	}

	connect_two_edges(
		triangles, 
		prim_index, 
		outer_edge, 
		e3->num_sub_edges + 1, 
		inner_edge, 
		inner_edge_size, 
		db, 
		poly);

	ei_pool_free(pool, outer_edge);
}

static eiFORCEINLINE eiScalar calc_triangle_max_angle(
	const eiVector *v1, 
	const eiVector *v2, 
	const eiVector *v3)
{
	eiVector	e1, e2, e3;
	eiScalar	a1, a2, a3;

	sub(&e1, v2, v1);
	sub(&e2, v3, v2);
	sub(&e3, v1, v3);

	normalizei(&e1);
	normalizei(&e2);
	normalizei(&e3);

	a1 = -dot(&e1, &e3);
	a2 = -dot(&e2, &e1);
	a3 = -dot(&e3, &e2);

	clampi(a1, -1.0f, 1.0f);
	clampi(a2, -1.0f, 1.0f);
	clampi(a3, -1.0f, 1.0f);

	a1 = acosf(a1);
	a2 = acosf(a2);
	a3 = acosf(a3);

	return MAX(a1, MAX(a2, a3));
}

/** \brief This routine is a simplified subset of Delaunay triangulation 
   which minimizes the maximum inner angle of all triangles */
static eiFORCEINLINE void stitch_two_edges(
	ei_array *vertices, 
	ei_array *triangles, 
	const eiIndex prim_index, 
	eiIndex *e1, const eiInt e1_size, 
	eiIndex *e2, const eiInt e2_size, 
	eiDatabase *db, 
	eiPolyTessel *poly)
{
	eiInt		v1, max_v1, v2, max_v2;

	v1 = 0;
	max_v1 = e1_size - 1;
	v2 = 0;
	max_v2 = e2_size - 1;

	while (eiTRUE)
	{
		eiInt			v1_next, v2_next;
		eiRayVertex		*vtx1, *vtx2, *vtx1_next, *vtx2_next;
		eiScalar		max_angle1, max_angle2;
		eiRayTriangle	new_tri;

		/* we reached the last edge, no need to go further */
		if (v1 == max_v1 && v2 == max_v2)
		{
			break;
		}

		v1_next = v1;
		v2_next = v2;

		if (v1_next < max_v1)
		{
			++ v1_next;
		}

		if (v2_next < max_v2)
		{
			++ v2_next;
		}

		vtx1 = (eiRayVertex *)ei_array_get(vertices, e1[v1]);
		vtx2 = (eiRayVertex *)ei_array_get(vertices, e2[v2]);
		vtx1_next = (eiRayVertex *)ei_array_get(vertices, e1[v1_next]);
		vtx2_next = (eiRayVertex *)ei_array_get(vertices, e2[v2_next]);

		/* we choose the triangle which minimizes the maximum inner angle */
		if (v1 == v1_next)
		{
			/* degenerated case, set to max scalar so we never choose it */
			max_angle1 = eiMAX_SCALAR;
		}
		else
		{
			max_angle1 = calc_triangle_max_angle(&vtx1->pos, &vtx1_next->pos, &vtx2->pos);
		}

		if (v2 == v2_next)
		{
			/* degenerated case, set to max scalar so we never choose it */
			max_angle2 = eiMAX_SCALAR;
		}
		else
		{
			max_angle2 = calc_triangle_max_angle(&vtx2->pos, &vtx1->pos, &vtx2_next->pos);
		}

		if (max_angle1 < max_angle2)
		{
			new_tri.v1 = e1[v1];
			new_tri.v2 = e1[v1_next];
			new_tri.v3 = e2[v2];
			new_tri.prim_index = prim_index;
			v1 = v1_next;
		}
		else
		{
			new_tri.v1 = e2[v2];
			new_tri.v2 = e1[v1];
			new_tri.v3 = e2[v2_next];
			new_tri.prim_index = prim_index;
			v2 = v2_next;
		}

		ei_array_push_back(triangles, &new_tri);
	}
}

void stitch_two_edges_exported(
	ei_array *vertices, 
	ei_array *triangles, 
	const eiIndex prim_index, 
	eiIndex *e1, const eiInt e1_size, 
	eiIndex *e2, const eiInt e2_size, 
	eiDatabase *db, 
	eiPolyTessel *poly)
{
	stitch_two_edges(
		vertices, 
		triangles, 
		prim_index, 
		e1, e1_size, 
		e2, e2_size, 
		db, 
		poly);
}

static void dice_triangle(
	ei_array *vertices, 
	ei_array *vertex_data, 
	eiDataTableIterator *varying_list_iter, 
	eiDataTableIterator *vertex_list_iter, 
	ei_array *triangles, 
	const eiIndex prim_index, 
	const eiIndex v1, const eiIndex v2, const eiIndex v3, 
	const eiVector *pos1, const eiVector *pos2, const eiVector *pos3, 
	const eiVector *motion_pos1, const eiVector *motion_pos2, const eiVector *motion_pos3, 
	eiPolyEdge *e1, const eiBool e1_flipped, 
	eiPolyEdge *e2, const eiBool e2_flipped, 
	eiPolyEdge *e3, const eiBool e3_flipped, 
	ei_pool *pool, 
	eiDatabase *db, 
	eiPolyTessel *poly)
{
	eiInt		num_edges;
	eiInt		num_inner_edges;
	eiInt		num_inner_vertices;
	eiIndex		inner_vertex_offset;
	eiInt		row, col;
	eiInt		max_num_sub_edges;
	eiIndex		*outer_edge, *inner_edge;
	eiInt		inner_edge_size;
	eiIndex		vtx_index;
	eiInt		i;

	/* get the minimum number of sub-edges from 3 edges */
	num_edges = MIN(e1->num_sub_edges, MIN(e2->num_sub_edges, e3->num_sub_edges));

	/* handle degenerated cases */
	if (num_edges <= 1)
	{
		eiInt		num_collapsed_edges;
		eiIndex		collapsed_edges[3];
		eiPolyEdge	*edges[3];
		eiBool		edges_flipped[3];
		eiIndex		corner_vertices[3][3];
		eiIndex		left_edge_index, right_edge_index;
		eiPolyEdge	*left_edge, *right_edge;
		eiBool		left_edge_flipped, right_edge_flipped;

		/* count the number of edges with num_sub_edges == 1 */
		num_collapsed_edges = 0;

		if (e1->num_sub_edges <= 1)
		{
			collapsed_edges[num_collapsed_edges] = 0;
			++ num_collapsed_edges;
		}
		if (e2->num_sub_edges <= 1)
		{
			collapsed_edges[num_collapsed_edges] = 1;
			++ num_collapsed_edges;
		}
		if (e3->num_sub_edges <= 1)
		{
			collapsed_edges[num_collapsed_edges] = 2;
			++ num_collapsed_edges;
		}

		edges[0] = e1;
		edges_flipped[0] = e1_flipped;
		edges[1] = e2;
		edges_flipped[1] = e2_flipped;
		edges[2] = e3;
		edges_flipped[2] = e3_flipped;

		corner_vertices[0][0] = v1;
		corner_vertices[0][1] = v2;
		corner_vertices[0][2] = v3;
		corner_vertices[1][0] = v2;
		corner_vertices[1][1] = v3;
		corner_vertices[1][2] = v1;
		corner_vertices[2][0] = v3;
		corner_vertices[2][1] = v1;
		corner_vertices[2][2] = v2;

		if (num_collapsed_edges == 1)
		{
			/* there is only 1 edge with num_sub_edges == 1 */
			static const eiIndex other_edges[3][2] = {
				{2, 1}, {0, 2}, {1, 0}, 
			};
			eiRayTriangle	new_tri;

			left_edge_index = other_edges[collapsed_edges[0]][0];
			right_edge_index = other_edges[collapsed_edges[0]][1];

			left_edge = edges[left_edge_index];
			left_edge_flipped = edges_flipped[left_edge_index];
			right_edge = edges[right_edge_index];
			right_edge_flipped = edges_flipped[right_edge_index];

			/* form the triangle at the corner */
			new_tri.v1 = corner_vertices[left_edge_index][0];
			if (left_edge_flipped)
			{
				new_tri.v2 = left_edge->vtx_index + left_edge->num_sub_edges - 2;
			}
			else
			{
				new_tri.v2 = left_edge->vtx_index;
			}
			if (right_edge_flipped)
			{
				new_tri.v3 = right_edge->vtx_index;
			}
			else
			{
				new_tri.v3 = right_edge->vtx_index + right_edge->num_sub_edges - 2;
			}
			new_tri.prim_index = prim_index;

			ei_array_push_back(triangles, &new_tri);

			/* form the ribbon grid */
			outer_edge = (eiIndex *)ei_pool_allocate(pool, 
				sizeof(eiIndex) * (left_edge->num_sub_edges + right_edge->num_sub_edges));
			inner_edge = outer_edge + left_edge->num_sub_edges;

			if (left_edge_flipped)
			{
				for (i = 0; i < (left_edge->num_sub_edges - 1); ++i)
				{
					outer_edge[left_edge->num_sub_edges - i - 2] = left_edge->vtx_index + i;
				}
			}
			else
			{
				for (i = 0; i < (left_edge->num_sub_edges - 1); ++i)
				{
					outer_edge[i] = left_edge->vtx_index + i;
				}
			}
			outer_edge[left_edge->num_sub_edges - 1] = corner_vertices[left_edge_index][1];

			if (right_edge_flipped)
			{
				for (i = 0; i < (right_edge->num_sub_edges - 1); ++i)
				{
					inner_edge[i] = right_edge->vtx_index + i;
				}
			}
			else
			{
				for (i = 0; i < (right_edge->num_sub_edges - 1); ++i)
				{
					inner_edge[right_edge->num_sub_edges - i - 2] = right_edge->vtx_index + i;
				}
			}
			inner_edge[right_edge->num_sub_edges - 1] = corner_vertices[right_edge_index][0];

			stitch_two_edges(vertices, triangles, 
				prim_index, 
				outer_edge, left_edge->num_sub_edges, 
				inner_edge, right_edge->num_sub_edges, 
				db, poly);

			ei_pool_free(pool, outer_edge);
		}
		else
		{
			/* there must be 2 edges with num_sub_edges == 1 */
			static const eiIndex middle_edge[3][3] = {
				{eiNULL_INDEX, 2, 1}, {2, eiNULL_INDEX, 0}, {1, 0, eiNULL_INDEX}, 
			};

			eiIndex		mid_edge_index;
			eiPolyEdge	*mid_edge;
			eiBool		mid_edge_flipped;

			mid_edge_index = middle_edge[collapsed_edges[0]][collapsed_edges[1]];
			mid_edge = edges[mid_edge_index];
			mid_edge_flipped = edges_flipped[mid_edge_index];

			outer_edge = (eiIndex *)ei_pool_allocate(pool, 
				sizeof(eiIndex) * ((mid_edge->num_sub_edges + 1) + 1));
			inner_edge = outer_edge + (mid_edge->num_sub_edges + 1);

			outer_edge[0] = corner_vertices[mid_edge_index][0];
			if (mid_edge_flipped)
			{
				for (i = 1; i < mid_edge->num_sub_edges; ++i)
				{
					outer_edge[mid_edge->num_sub_edges - i] = mid_edge->vtx_index + i - 1;
				}
			}
			else
			{
				for (i = 1; i < mid_edge->num_sub_edges; ++i)
				{
					outer_edge[i] = mid_edge->vtx_index + i - 1;
				}
			}
			outer_edge[mid_edge->num_sub_edges] = corner_vertices[mid_edge_index][1];

			inner_edge[0] = corner_vertices[mid_edge_index][2];

			stitch_two_edges(vertices, triangles, 
				prim_index, 
				outer_edge, mid_edge->num_sub_edges + 1, 
				inner_edge, 1, 
				db, poly);

			ei_pool_free(pool, outer_edge);
		}

		return;
	}
	else if (num_edges <= 3)
	{
		eiVector		uv;
		eiRayVertex		new_vtx;

		/* force to num_edges == 3 case */
		num_inner_edges = 0;
		num_inner_vertices = (num_inner_edges + 2) * (num_inner_edges + 1) / 2;
		inner_vertex_offset = (eiIndex)ei_array_size(vertices);

		/* the inner grid becomes a single point */
		add(&new_vtx.pos, pos1, pos2);
		addi(&new_vtx.pos, pos3);
		mulvfi(&new_vtx.pos, 1.0f / 3.0f);
		get_bary(&uv, &new_vtx.pos, pos1, pos2, pos3);

		interp_point(&new_vtx.m_pos, motion_pos1, motion_pos2, motion_pos3, &uv);

		ei_array_push_back(vertices, &new_vtx);

		interp_vertex_data(db, poly, vertex_data, varying_list_iter, vertex_list_iter, v1, v2, v3, &uv);
	}
	else
	{
		num_inner_edges = num_edges - 3;
		num_inner_vertices = (num_inner_edges + 2) * (num_inner_edges + 1) / 2;
		inner_vertex_offset = (eiIndex)ei_array_size(vertices);

		/* build inner grid */
		build_triangle_grid(
			vertices, 
			vertex_data, 
			varying_list_iter, 
			vertex_list_iter, 
			triangles, 
			prim_index, 
			v1, v2, v3, 
			pos1, pos2, pos3, 
			motion_pos1, motion_pos2, motion_pos3, 
			num_inner_edges, inner_vertex_offset, 
			db, 
			poly);
	}

	/* build outer grids */
	max_num_sub_edges = MAX(e1->num_sub_edges + 1, MAX(e2->num_sub_edges + 1, e3->num_sub_edges + 1));
	outer_edge = (eiIndex *)ei_pool_allocate(pool, 
		sizeof(eiIndex) * (max_num_sub_edges + (num_inner_edges + 1)));
	inner_edge = outer_edge + max_num_sub_edges;

	/* connect inner edge and outer edge 1 */
	outer_edge[0] = v1;
	if (e1_flipped)
	{
		for (i = 1; i < e1->num_sub_edges; ++i)
		{
			outer_edge[e1->num_sub_edges - i] = e1->vtx_index + i - 1;
		}
	}
	else
	{
		for (i = 1; i < e1->num_sub_edges; ++i)
		{
			outer_edge[i] = e1->vtx_index + i - 1;
		}
	}
	outer_edge[e1->num_sub_edges] = v2;

	inner_edge_size = 0;
	vtx_index = inner_vertex_offset;

	for (row = 0; row <= num_inner_edges; ++row)
	{
		inner_edge[inner_edge_size] = vtx_index;
		++ inner_edge_size;
		vtx_index += (row + 1);
	}

	stitch_two_edges(vertices, triangles, 
		prim_index, 
		outer_edge, e1->num_sub_edges + 1, 
		inner_edge, inner_edge_size, 
		db, poly);

	/* connect inner edge and outer edge 2 */
	outer_edge[0] = v2;
	if (e2_flipped)
	{
		for (i = 1; i < e2->num_sub_edges; ++i)
		{
			outer_edge[e2->num_sub_edges - i] = e2->vtx_index + i - 1;
		}
	}
	else
	{
		for (i = 1; i < e2->num_sub_edges; ++i)
		{
			outer_edge[i] = e2->vtx_index + i - 1;
		}
	}
	outer_edge[e2->num_sub_edges] = v3;

	inner_edge_size = 0;
	vtx_index = inner_vertex_offset + num_inner_vertices - 1 - num_inner_edges;

	for (col = 0; col <= num_inner_edges; ++col)
	{
		inner_edge[inner_edge_size] = vtx_index;
		++ inner_edge_size;
		++ vtx_index;
	}
	
	stitch_two_edges(vertices, triangles, 
		prim_index, 
		outer_edge, e2->num_sub_edges + 1, 
		inner_edge, inner_edge_size, 
		db, poly);

	/* connect inner edge and outer edge 3 */
	outer_edge[0] = v3;
	if (e3_flipped)
	{
		for (i = 1; i < e3->num_sub_edges; ++i)
		{
			outer_edge[e3->num_sub_edges - i] = e3->vtx_index + i - 1;
		}
	}
	else
	{
		for (i = 1; i < e3->num_sub_edges; ++i)
		{
			outer_edge[i] = e3->vtx_index + i - 1;
		}
	}
	outer_edge[e3->num_sub_edges] = v1;

	inner_edge_size = 0;
	vtx_index = inner_vertex_offset + num_inner_vertices - 1;

	for (row = num_inner_edges; row >= 0; --row)
	{
		inner_edge[inner_edge_size] = vtx_index;
		++ inner_edge_size;
		vtx_index -= (row + 1);
	}

	stitch_two_edges(vertices, triangles, 
		prim_index, 
		outer_edge, e3->num_sub_edges + 1, 
		inner_edge, inner_edge_size, 
		db, poly);

	ei_pool_free(pool, outer_edge);
}

static void ei_poly_object_dice_imp(
	eiDatabase *db, 
	eiRayTracer *rt, 
	eiTesselJob *job, 
	eiPolyTessel *poly, 
	const eiBound *box, 
	const eiTag tessel_tag)
{
	eiRayTessel				*tessel;
	eiInt					num_vertices;
	eiInt					num_triangles;
	eiBuffer				edges;
	ei_array				vertices;
	ei_array				vertex_data;
	ei_array				triangles;
	ei_pool					pool;
	eiInt					num_tessellated_vertices;
	eiInt					num_tessellated_triangles;
	eiInt					i, j;
	eiDataTableIterator		pos_list_iter;
	eiDataTableIterator		motion_pos_list_iter;
	eiDataTableIterator		varying_list_iter;
	eiDataTableIterator		vertex_list_iter;
	eiDataTableIterator		triangle_list_iter;

	ei_data_table_begin(db, poly->pos_list, &pos_list_iter);
	ei_data_table_begin(db, poly->motion_pos_list, &motion_pos_list_iter);
	ei_data_table_begin(db, poly->varying_list, &varying_list_iter);
	ei_data_table_begin(db, poly->vertex_list, &vertex_list_iter);
	ei_data_table_begin(db, poly->triangle_list, &triangle_list_iter);

	num_vertices = pos_list_iter.tab->item_count;
	num_triangles = triangle_list_iter.tab->item_count / NUM_TRI_CHANS;

	/* build edge list */
	ei_buffer_init(
		&edges, 
		sizeof(eiPolyEdge), 
		ei_poly_edge_init, 
		NULL, 
		NULL, 
		NULL, 
		NULL, 
		NULL);
	
	ei_buffer_allocate(&edges, num_vertices, num_vertices);
	ei_array_init(&vertices, sizeof(eiRayVertex));
	ei_array_init(&vertex_data, sizeof(eiScalar) * (poly->varying_dim + poly->vertex_dim));
	ei_array_init(&triangles, sizeof(eiRayTriangle));
	ei_pool_init(&pool);

	/* add original vertices first, we will use them */
	for (i = 0; i < num_vertices; ++i)
	{
		eiRayVertex		new_vtx;

		movv(&new_vtx.pos, (eiVector *)ei_data_table_read(&pos_list_iter, i));
		movv(&new_vtx.m_pos, (eiVector *)ei_data_table_read(&motion_pos_list_iter, i));

		ei_array_push_back(&vertices, &new_vtx);

		add_vertex_data(db, poly, &vertex_data, &varying_list_iter, &vertex_list_iter, i);
	}

	for (i = 0; i < num_triangles; ++i)
	{
		eiIndex			v1, v2, v3;
		eiIndex			edge_segs1, edge_segs2, edge_segs3;
		eiIndex			prim_index;
		eiVector		pos1, pos2, pos3;
		eiVector		motion_pos1, motion_pos2, motion_pos3;
		eiPolyEdge		*e1, *e2, *e3;
		eiBool			e1_flipped, e2_flipped, e3_flipped;

		/* access 3 vertex indices of triangle */
		v1 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 0));
		v2 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 1));
		v3 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 2));

		/* access 3 edge segments */
		edge_segs1 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 3));
		edge_segs2 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 4));
		edge_segs3 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 5));

		/* access primitive index */
		prim_index = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 6));

		/* access vertex 1 */
		movv(&pos1, (eiVector *)ei_data_table_read(&pos_list_iter, v1));
		movv(&motion_pos1, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v1));

		/* access vertex 2 */
		movv(&pos2, (eiVector *)ei_data_table_read(&pos_list_iter, v2));
		movv(&motion_pos2, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v2));

		/* access vertex 3 */
		movv(&pos3, (eiVector *)ei_data_table_read(&pos_list_iter, v3));
		movv(&motion_pos3, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v3));

		/* estimate sub-division for 3 edges of triangle */
		e1 = build_edge_vertices(
			v1, v2, 
			&e1_flipped, 
			&edges, 
			&vertices, &vertex_data, 
			&varying_list_iter, 
			&vertex_list_iter, 
			&pos1, &pos2, 
			&motion_pos1, &motion_pos2, 
			edge_segs1, 
			db, 
			poly);

		e2 = build_edge_vertices(
			v2, v3, 
			&e2_flipped, 
			&edges, 
			&vertices, &vertex_data, 
			&varying_list_iter, 
			&vertex_list_iter, 
			&pos2, &pos3, 
			&motion_pos2, &motion_pos3, 
			edge_segs2, 
			db, 
			poly);

		e3 = build_edge_vertices(
			v3, v1, 
			&e3_flipped, 
			&edges, 
			&vertices, &vertex_data, 
			&varying_list_iter, 
			&vertex_list_iter, 
			&pos3, &pos1, 
			&motion_pos3, &motion_pos1, 
			edge_segs3, 
			db, 
			poly);

		/* construct tessellated triangles from 3 edges */
		if (e1->num_sub_edges == e2->num_sub_edges && 
			e2->num_sub_edges == e3->num_sub_edges)
		{
			/* 3 edges have the same sub-division level, good case */
			dice_triangle_regular(
				&vertices, 
				&vertex_data, 
				&varying_list_iter, 
				&vertex_list_iter, 
				&triangles, 
				prim_index, 
				v1, v2, v3, 
				&pos1, &pos2, &pos3, 
				&motion_pos1, &motion_pos2, &motion_pos3, 
				e1, e1_flipped, 
				e2, e2_flipped, 
				e3, e3_flipped, 
				e1->num_sub_edges, 
				&pool, 
				db, 
				poly);
		}
		else
		{
			/* sub-divison levels of 3 edges are different, bad case */
			dice_triangle(
				&vertices, 
				&vertex_data, 
				&varying_list_iter, 
				&vertex_list_iter, 
				&triangles, 
				prim_index, 
				v1, v2, v3, 
				&pos1, &pos2, &pos3, 
				&motion_pos1, &motion_pos2, &motion_pos3, 
				e1, e1_flipped, 
				e2, e2_flipped, 
				e3, e3_flipped, 
				&pool, 
				db, 
				poly);
		}
	}

	ei_data_table_end(&triangle_list_iter);
	ei_data_table_end(&vertex_list_iter);
	ei_data_table_end(&varying_list_iter);
	ei_data_table_end(&motion_pos_list_iter);
	ei_data_table_end(&pos_list_iter);

	/* clear auxiliary data */
	ei_pool_clear(&pool);
	ei_buffer_clear(&edges);

	/* copy cached data into ray-traceable tessellation */
	num_tessellated_vertices = (eiUint)ei_array_size(&vertices);
	num_tessellated_triangles = (eiUint)ei_array_size(&triangles);

	tessel = ei_rt_tessel_resize(
		rt, 
		tessel_tag, 
		num_tessellated_vertices, 
		num_tessellated_triangles, 
		0, 
		poly->varying_dim + poly->vertex_dim);

	for (i = 0; i < num_tessellated_vertices; ++i)
	{
		ei_rt_tessel_add_vertex(
			tessel, 
			(eiRayVertex *)ei_array_get(&vertices, i));

		/* copy user-defined vertex data */
		for (j = 0; j < (poly->varying_dim + poly->vertex_dim); ++j)
		{
			ei_rt_tessel_add_vertex_data(
				tessel, 
				j, 
				ei_array_get(&vertex_data, i * (poly->varying_dim + poly->vertex_dim) + j));
		}
	}

	for (i = 0; i < num_tessellated_triangles; ++i)
	{
		ei_rt_tessel_add_triangle(
			tessel, 
			(eiRayTriangle *)ei_array_get(&triangles, i));
	}

	ei_rt_end_tessel(rt, tessel, tessel_tag);

	/* clear cached data */
	ei_array_clear(&triangles);
	ei_array_clear(&vertex_data);
	ei_array_clear(&vertices);
}

void ei_poly_object_deferred_dice(
	eiDatabase *db, 
	eiTesselJob *job, 
	eiObject *obj, 
	const eiBound *box, 
	const eiTag deferred_tessel_tag)
{
	eiRayTracer		*rt;
	eiNodeSystem	*nodesys;
	eiPolyTessel	*poly;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);

	poly = (eiPolyTessel *)obj;

	ei_poly_object_dice_imp(
		db, rt, job, poly, box, deferred_tessel_tag);

	/* call displacement shader right after dicing */
	ei_displace_tessel(db, job->displace_list, deferred_tessel_tag, poly->object, job->motion);
}

eiTag ei_poly_object_dice(
	eiDatabase *db, 
	eiTesselJob *job, 
	eiObject *obj, 
	const eiBound *box)
{
	eiRayTracer		*rt;
	eiNodeSystem	*nodesys;
	eiPolyTessel	*poly;
	eiTag			tessel_tag;

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* get node system interface */
	nodesys = (eiNodeSystem *)ei_db_globals_interface(
		db, 
		EI_INTERFACE_TYPE_NODE_SYSTEM);

	poly = (eiPolyTessel *)obj;

	/* disable approximation if the object is polygon and displacement is off */
	if (job->displace_list == eiNULL_TAG)
	{
		/* no approximation, dice directly */
		tessel_tag = ei_poly_object_direct_dice(db, rt, poly, box);
	}
	else
	{
		eiApprox	*approx;

		approx = (eiApprox *)ei_db_access(db, job->approx);

		if (approx->method == EI_APPROX_METHOD_REGULAR)
		{
			if (approx->args[EI_APPROX_U] == 0.0f)
			{
				/* no approximation, dice directly */
				tessel_tag = ei_poly_object_direct_dice(db, rt, poly, box);

				/* call displacement shader right after dicing */
				ei_displace_tessel(db, job->displace_list, tessel_tag, poly->object, job->motion);
			}
			else
			{
				/* create a deferred tessellation to dice on-demand */
				tessel_tag = ei_poly_object_indirect_dice(db, rt, job, poly, box);
			}
		}
		else if (approx->method == EI_APPROX_METHOD_LENGTH)
		{
			if (approx->args[EI_APPROX_LENGTH] == 0.0f)
			{
				/* no approximation, dice directly */
				tessel_tag = ei_poly_object_direct_dice(db, rt, poly, box);

				/* call displacement shader right after dicing */
				ei_displace_tessel(db, job->displace_list, tessel_tag, poly->object, job->motion);
			}
			else
			{
				/* create a deferred tessellation to dice on-demand */
				tessel_tag = ei_poly_object_indirect_dice(db, rt, job, poly, box);
			}
		}

		ei_db_end(db, job->approx);
	}

	return tessel_tag;
}

typedef struct eiTriSortElem {
	eiScalar		tri_mid_pos;
	eiIndex			tri_index;
} eiTriSortElem;

eiInt ei_tri_sort_elem_compare(void *lhs, void *rhs)
{
	return (eiInt)(((eiTriSortElem *)lhs)->tri_mid_pos - ((eiTriSortElem *)rhs)->tri_mid_pos);
}

typedef struct eiVtxMapElem {
	/* the index of this vertex in the vertex list of left sub-object, 
	   will be not eiNULL_TAG if this vertex belongs to left sub-object */
	eiIndex			left_index;
	/* the index of this vertex in the vertex list of right sub-object, 
	   will be not eiNULL_TAG if this vertex belongs to right sub-object */
	eiIndex			right_index;
} eiVtxMapElem;

static eiFORCEINLINE void add_new_vertex(
	eiDatabase *db, 
	eiPolyTessel *poly, 
	eiDataTable **pos_tab, 
	eiDataTable **motion_pos_tab, 
	eiDataTable **varying_tab, 
	eiDataTableIterator *varying_list_iter, 
	eiDataTable **vertex_tab, 
	eiDataTableIterator *vertex_list_iter, 
	const eiVector *pos, 
	const eiVector *motion_pos, 
	const eiIndex v)
{
	eiInt	i;

	ei_data_table_push_back(db, pos_tab, pos);

	if ((*motion_pos_tab) != NULL)
	{
		ei_data_table_push_back(db, motion_pos_tab, motion_pos);
	}

	for (i = 0; i < poly->varying_dim; ++i)
	{
		ei_data_table_push_back(
			db, 
			varying_tab, 
			ei_data_table_read(varying_list_iter, v * poly->varying_dim + i));
	}

	for (i = 0; i < poly->vertex_dim; ++i)
	{
		ei_data_table_push_back(
			db, 
			vertex_tab, 
			ei_data_table_read(vertex_list_iter, v * poly->vertex_dim + i));
	}
}

static eiFORCEINLINE void lerp_new_vertex(
	eiDatabase *db, 
	eiPolyTessel *poly, 
	eiDataTable **pos_tab, 
	eiDataTableIterator *pos_list_iter, 
	eiDataTable **motion_pos_tab, 
	eiDataTableIterator *motion_pos_list_iter, 
	eiDataTable **varying_tab, 
	eiDataTableIterator *varying_list_iter, 
	eiDataTable **vertex_tab, 
	eiDataTableIterator *vertex_list_iter, 
	const eiVector *pos1, const eiVector *pos2, 
	const eiVector *motion_pos1, const eiVector *motion_pos2, 
	const eiIndex v1, const eiIndex v2, 
	const eiScalar t, 
	eiVector *pos, 
	eiVector *motion_pos)
{
	eiInt	i;

	lerp3(pos, pos1, pos2, t);
	lerp3(motion_pos, motion_pos1, motion_pos2, t);

	ei_data_table_push_back(db, pos_tab, pos);
	
	if ((*motion_pos_tab) != NULL)
	{
		ei_data_table_push_back(db, motion_pos_tab, motion_pos);
	}

	for (i = 0; i < poly->varying_dim; ++i)
	{
		eiScalar	sval;

		lerp(&sval, 
			*((eiScalar *)ei_data_table_read(varying_list_iter, v1 * poly->varying_dim + i)), 
			*((eiScalar *)ei_data_table_read(varying_list_iter, v2 * poly->varying_dim + i)), 
			t);

		ei_data_table_push_back(
			db, 
			varying_tab, 
			&sval);
	}

	for (i = 0; i < poly->vertex_dim; ++i)
	{
		eiScalar	sval;

		lerp(&sval, 
			*((eiScalar *)ei_data_table_read(vertex_list_iter, v1 * poly->vertex_dim + i)), 
			*((eiScalar *)ei_data_table_read(vertex_list_iter, v2 * poly->vertex_dim + i)), 
			t);

		ei_data_table_push_back(
			db, 
			vertex_tab, 
			&sval);
	}
}

static eiFORCEINLINE void add_new_triangle(
	eiDatabase *db, 
	eiDataTable **triangle_tab, 
	const eiIndex v1, 
	const eiIndex v2, 
	const eiIndex v3, 
	const eiIndex e1, 
	const eiIndex e2, 
	const eiIndex e3, 
	const eiIndex prim_index)
{
	ei_data_table_push_back(db, triangle_tab, &v1);
	ei_data_table_push_back(db, triangle_tab, &v2);
	ei_data_table_push_back(db, triangle_tab, &v3);
	ei_data_table_push_back(db, triangle_tab, &e1);
	ei_data_table_push_back(db, triangle_tab, &e2);
	ei_data_table_push_back(db, triangle_tab, &e3);
	ei_data_table_push_back(db, triangle_tab, &prim_index);
}

static void ei_poly_object_parametric_split(
	eiDatabase *db, 
	eiTesselJob *job, 
	eiPolyTessel *poly, 
	const eiBound *box, 
	eiJobQueue *queue)
{
	eiMaster		*master;
	eiPolyTessel	*poly1, *poly2, *poly3, *poly4;
	eiTag			poly1_tag, poly2_tag, poly3_tag, poly4_tag;
	eiInt			items_per_slot;
	eiTag			pos_list;
	eiTag			motion_pos_list;
	eiTag			varying_list;
	eiTag			vertex_list;
	eiIndex			v1, v2, v3;
	eiIndex			e1, e2, e3;
	eiIndex			prim_index;
	eiVector		pos1, pos2, pos3;
	eiVector		motion_pos1, motion_pos2, motion_pos3;
	eiVector		mid_pos1, mid_pos2, mid_pos3;
	eiVector		mid_motion_pos1, mid_motion_pos2, mid_motion_pos3;
	eiIndex			splitter[3];
	eiScalar		t;
	eiIndex			t0_edge_segs[3];
	eiIndex			t1_edge_segs[3];
	eiIndex			t3_edge_segs[3];
	eiTag			tessel_job_tag1, 
					tessel_job_tag2, 
					tessel_job_tag3, 
					tessel_job_tag4;
	eiTesselJob		*tessel_job;

	estimate_num_sub_edges_func				estimator;
	estimate_num_sub_edges_regular_params	estimate_default_params;
	estimate_num_sub_edges_regular_params	estimate_regular_params;
	estimate_num_sub_edges_length_params	estimate_length_params;
	void									*estimator_params;

	eiDataTableIterator		pos_list_iter;
	eiDataTableIterator		motion_pos_list_iter;
	eiDataTableIterator		varying_list_iter;
	eiDataTableIterator		vertex_list_iter;
	eiDataTableIterator		triangle_list_iter;
	eiDataTable				*pos_tab;
	eiDataTable				*motion_pos_tab;
	eiDataTable				*varying_tab;
	eiDataTable				*vertex_tab;
	eiDataTable				*triangle_tab1;
	eiDataTable				*triangle_tab2;
	eiDataTable				*triangle_tab3;
	eiDataTable				*triangle_tab4;

	/* get rendering manager */
	master = ei_db_net_master(db);

	/* create poly 1 */
	poly1 = (eiPolyTessel *)ei_db_create(
		db, 
		&poly1_tag, 
		EI_DATA_TYPE_INTARRAY, 
		sizeof(eiPolyTessel), 
		EI_DB_FLUSHABLE);

	/* create poly 2 */
	poly2 = (eiPolyTessel *)ei_db_create(
		db, 
		&poly2_tag, 
		EI_DATA_TYPE_INTARRAY, 
		sizeof(eiPolyTessel), 
		EI_DB_FLUSHABLE);

	/* create poly 3 */
	poly3 = (eiPolyTessel *)ei_db_create(
		db, 
		&poly3_tag, 
		EI_DATA_TYPE_INTARRAY, 
		sizeof(eiPolyTessel), 
		EI_DB_FLUSHABLE);

	/* create poly 4 */
	poly4 = (eiPolyTessel *)ei_db_create(
		db, 
		&poly4_tag, 
		EI_DATA_TYPE_INTARRAY, 
		sizeof(eiPolyTessel), 
		EI_DB_FLUSHABLE);

	/* record the tag of the source object node */
	poly1->object = poly->object;
	poly2->object = poly->object;
	poly3->object = poly->object;
	poly4->object = poly->object;

	ei_data_table_begin(db, poly->pos_list, &pos_list_iter);
	ei_data_table_begin(db, poly->motion_pos_list, &motion_pos_list_iter);
	ei_data_table_begin(db, poly->varying_list, &varying_list_iter);
	ei_data_table_begin(db, poly->vertex_list, &vertex_list_iter);
	ei_data_table_begin(db, poly->triangle_list, &triangle_list_iter);

	/* build pos list, we can share it among sub-objects */
	items_per_slot = pos_list_iter.tab->items_per_slot;
	pos_list = ei_create_data_table(db, EI_DATA_TYPE_VECTOR, items_per_slot);
	poly1->pos_list = pos_list;
	ei_db_ref(db, poly1->pos_list);
	poly2->pos_list = pos_list;
	ei_db_ref(db, poly2->pos_list);
	poly3->pos_list = pos_list;
	ei_db_ref(db, poly3->pos_list);
	poly4->pos_list = pos_list;
	ei_db_ref(db, poly4->pos_list);

	/* build motion pos list, we can share it among sub-objects */
	if (poly->motion_pos_list == poly->pos_list)
	{
		motion_pos_list = pos_list;
	}
	else
	{
		items_per_slot = motion_pos_list_iter.tab->items_per_slot;
		motion_pos_list = ei_create_data_table(db, EI_DATA_TYPE_VECTOR, items_per_slot);
	}
	poly1->motion_pos_list = motion_pos_list;
	ei_db_ref(db, poly1->motion_pos_list);
	poly2->motion_pos_list = motion_pos_list;
	ei_db_ref(db, poly2->motion_pos_list);
	poly3->motion_pos_list = motion_pos_list;
	ei_db_ref(db, poly3->motion_pos_list);
	poly4->motion_pos_list = motion_pos_list;
	ei_db_ref(db, poly4->motion_pos_list);

	/* build optinal varying list, we can share it among sub-objects */
	poly1->varying_dim = poly->varying_dim;
	poly2->varying_dim = poly->varying_dim;
	poly3->varying_dim = poly->varying_dim;
	poly4->varying_dim = poly->varying_dim;
	varying_list = eiNULL_TAG;
	poly1->varying_list = eiNULL_TAG;
	poly2->varying_list = eiNULL_TAG;
	poly3->varying_list = eiNULL_TAG;
	poly4->varying_list = eiNULL_TAG;

	if (poly->varying_list != eiNULL_TAG)
	{
		items_per_slot = varying_list_iter.tab->items_per_slot;
		varying_list = ei_create_data_table(db, EI_DATA_TYPE_SCALAR, items_per_slot);
		poly1->varying_list = varying_list;
		ei_db_ref(db, poly1->varying_list);
		poly2->varying_list = varying_list;
		ei_db_ref(db, poly2->varying_list);
		poly3->varying_list = varying_list;
		ei_db_ref(db, poly3->varying_list);
		poly4->varying_list = varying_list;
		ei_db_ref(db, poly4->varying_list);
	}

	/* build vertex list, we can share it among sub-objects */
	poly1->vertex_dim = poly->vertex_dim;
	poly2->vertex_dim = poly->vertex_dim;
	poly3->vertex_dim = poly->vertex_dim;
	poly4->vertex_dim = poly->vertex_dim;
	vertex_list = eiNULL_TAG;
	poly1->vertex_list = eiNULL_TAG;
	poly2->vertex_list = eiNULL_TAG;
	poly3->vertex_list = eiNULL_TAG;
	poly4->vertex_list = eiNULL_TAG;

	if (poly->vertex_list != eiNULL_TAG)
	{
		items_per_slot = vertex_list_iter.tab->items_per_slot;
		vertex_list = ei_create_data_table(db, EI_DATA_TYPE_SCALAR, items_per_slot);
		poly1->vertex_list = vertex_list;
		ei_db_ref(db, poly1->vertex_list);
		poly2->vertex_list = vertex_list;
		ei_db_ref(db, poly2->vertex_list);
		poly3->vertex_list = vertex_list;
		ei_db_ref(db, poly3->vertex_list);
		poly4->vertex_list = vertex_list;
		ei_db_ref(db, poly4->vertex_list);
	}

	/* build triangle list */
	items_per_slot = triangle_list_iter.tab->items_per_slot;
	poly1->triangle_list = ei_create_data_table(db, EI_DATA_TYPE_INDEX, items_per_slot);
	ei_db_ref(db, poly1->triangle_list);
	poly2->triangle_list = ei_create_data_table(db, EI_DATA_TYPE_INDEX, items_per_slot);
	ei_db_ref(db, poly2->triangle_list);
	poly3->triangle_list = ei_create_data_table(db, EI_DATA_TYPE_INDEX, items_per_slot);
	ei_db_ref(db, poly3->triangle_list);
	poly4->triangle_list = ei_create_data_table(db, EI_DATA_TYPE_INDEX, items_per_slot);
	ei_db_ref(db, poly4->triangle_list);

	/* we must only have one triangle, subdivide it */
	v1 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, 0 * NUM_TRI_CHANS + 0));
	v2 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, 0 * NUM_TRI_CHANS + 1));
	v3 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, 0 * NUM_TRI_CHANS + 2));

	/* access 3 edge segments */
	e1 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, 0 * NUM_TRI_CHANS + 3));
	e2 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, 0 * NUM_TRI_CHANS + 4));
	e3 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, 0 * NUM_TRI_CHANS + 5));

	/* access primitive index */
	prim_index = *((eiIndex *)ei_data_table_read(&triangle_list_iter, 0 * NUM_TRI_CHANS + 6));

	/* access vertex 1 */
	movv(&pos1, (eiVector *)ei_data_table_read(&pos_list_iter, v1));
	movv(&motion_pos1, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v1));

	/* access vertex 2 */
	movv(&pos2, (eiVector *)ei_data_table_read(&pos_list_iter, v2));
	movv(&motion_pos2, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v2));

	/* access vertex 3 */
	movv(&pos3, (eiVector *)ei_data_table_read(&pos_list_iter, v3));
	movv(&motion_pos3, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v3));

	/* select splitters */
	splitter[0] = e1 / 2;
	splitter[1] = e2 / 2;
	splitter[2] = e3 / 2;

	pos_tab = (eiDataTable *)ei_db_access(db, pos_list);
	motion_pos_tab = NULL;
	if (pos_list != motion_pos_list)
	{
		motion_pos_tab = (eiDataTable *)ei_db_access(db, motion_pos_list);
	}
	if (varying_list != eiNULL_TAG)
	{
		varying_tab = (eiDataTable *)ei_db_access(db, varying_list);
	}
	if (vertex_list != eiNULL_TAG)
	{
		vertex_tab = (eiDataTable *)ei_db_access(db, vertex_list);
	}
	triangle_tab1 = (eiDataTable *)ei_db_access(db, poly1->triangle_list);
	triangle_tab2 = (eiDataTable *)ei_db_access(db, poly2->triangle_list);
	triangle_tab3 = (eiDataTable *)ei_db_access(db, poly3->triangle_list);
	triangle_tab4 = (eiDataTable *)ei_db_access(db, poly4->triangle_list);

	/* build new vertex 0 */
	add_new_vertex(
		db, 
		poly, 
		&pos_tab, 
		&motion_pos_tab, 
		&varying_tab, 
		&varying_list_iter, 
		&vertex_tab, 
		&vertex_list_iter, 
		&pos1, 
		&motion_pos1, 
		v1);

	/* build new vertex 1 */
	t = (eiScalar)splitter[0] / (eiScalar)e1;

	lerp_new_vertex(
		db, 
		poly, 
		&pos_tab, 
		&pos_list_iter, 
		&motion_pos_tab, 
		&motion_pos_list_iter, 
		&varying_tab, 
		&varying_list_iter, 
		&vertex_tab, 
		&vertex_list_iter, 
		&pos1, &pos2, 
		&motion_pos1, &motion_pos2, 
		v1, v2, 
		t, 
		&mid_pos1, 
		&mid_motion_pos1);

	/* build new vertex 2 */
	t = (eiScalar)splitter[2] / (eiScalar)e3;

	lerp_new_vertex(
		db, 
		poly, 
		&pos_tab, 
		&pos_list_iter, 
		&motion_pos_tab, 
		&motion_pos_list_iter, 
		&varying_tab, 
		&varying_list_iter, 
		&vertex_tab, 
		&vertex_list_iter, 
		&pos3, &pos1, 
		&motion_pos3, &motion_pos1, 
		v3, v1, 
		t, 
		&mid_pos3, 
		&mid_motion_pos3);

	/* build new vertex 3 */
	add_new_vertex(
		db, 
		poly, 
		&pos_tab, 
		&motion_pos_tab, 
		&varying_tab, 
		&varying_list_iter, 
		&vertex_tab, 
		&vertex_list_iter, 
		&pos2, 
		&motion_pos2, 
		v2);

	/* build new vertex 4 */
	t = (eiScalar)splitter[1] / (eiScalar)e2;

	lerp_new_vertex(
		db, 
		poly, 
		&pos_tab, 
		&pos_list_iter, 
		&motion_pos_tab, 
		&motion_pos_list_iter, 
		&varying_tab, 
		&varying_list_iter, 
		&vertex_tab, 
		&vertex_list_iter, 
		&pos2, &pos3, 
		&motion_pos2, &motion_pos3, 
		v2, v3, 
		t, 
		&mid_pos2, 
		&mid_motion_pos2);

	/* build new vertex 5 */
	add_new_vertex(
		db, 
		poly, 
		&pos_tab, 
		&motion_pos_tab, 
		&varying_tab, 
		&varying_list_iter, 
		&vertex_tab, 
		&vertex_list_iter, 
		&pos3, 
		&motion_pos3, 
		v3);

	/* build edge estimator */
	build_edge_estimator(
		db, 
		job, 
		&estimator, 
		&estimate_default_params, 
		&estimate_regular_params, 
		&estimate_length_params, 
		&estimator_params);

	/* build new sub-edges */
	t0_edge_segs[0] = splitter[0];
	t0_edge_segs[1] = estimator(&mid_pos3, &mid_pos1, &mid_motion_pos3, &mid_motion_pos1, estimator_params);
	t0_edge_segs[2] = e3 - splitter[2];

	t1_edge_segs[0] = e1 - splitter[0];
	t1_edge_segs[1] = splitter[1];
	t1_edge_segs[2] = estimator(&mid_pos1, &mid_pos2, &mid_motion_pos1, &mid_motion_pos2, estimator_params);

	t3_edge_segs[0] = estimator(&mid_pos2, &mid_pos3, &mid_motion_pos2, &mid_motion_pos3, estimator_params);
	t3_edge_segs[1] = e2 - splitter[1];
	t3_edge_segs[2] = splitter[2];

	/* build new triangle 1 */
	add_new_triangle(
		db, 
		&triangle_tab1, 
		0, 1, 2, 
		t0_edge_segs[0], 
		t0_edge_segs[1], 
		t0_edge_segs[2], 
		prim_index);

	/* build new triangle 2 */
	add_new_triangle(
		db, 
		&triangle_tab2, 
		1, 3, 4, 
		t1_edge_segs[0], 
		t1_edge_segs[1], 
		t1_edge_segs[2], 
		prim_index);

	/* build new triangle 3 */
	add_new_triangle(
		db, 
		&triangle_tab3, 
		1, 4, 2, 
		t1_edge_segs[2], 
		t3_edge_segs[0], 
		t0_edge_segs[1], 
		prim_index);

	/* build new triangle 4 */
	add_new_triangle(
		db, 
		&triangle_tab4, 
		2, 4, 5, 
		t3_edge_segs[0], 
		t3_edge_segs[1], 
		t3_edge_segs[2], 
		prim_index);

	ei_db_end(db, poly4->triangle_list);
	ei_db_end(db, poly3->triangle_list);
	ei_db_end(db, poly2->triangle_list);
	ei_db_end(db, poly1->triangle_list);
	if (vertex_list != eiNULL_TAG)
	{
		ei_db_end(db, vertex_list);
	}
	if (varying_list != eiNULL_TAG)
	{
		ei_db_end(db, varying_list);
	}
	if (motion_pos_tab != NULL)
	{
		ei_db_end(db, motion_pos_list);
	}
	ei_db_end(db, pos_list);

	ei_data_table_end(&triangle_list_iter);
	ei_data_table_end(&vertex_list_iter);
	ei_data_table_end(&varying_list_iter);
	ei_data_table_end(&motion_pos_list_iter);
	ei_data_table_end(&pos_list_iter);

	/* finish creating poly 4 */
	ei_db_end(db, poly4_tag);

	/* finish creating poly 3 */
	ei_db_end(db, poly3_tag);

	/* finish creating poly 2 */
	ei_db_end(db, poly2_tag);

	/* finish creating poly 1 */
	ei_db_end(db, poly1_tag);

	/* schedule tessellation jobs for generated sub-objects */
	tessel_job = (eiTesselJob *)ei_db_create(
		db, 
		&tessel_job_tag1, 
		EI_DATA_TYPE_JOB_TESSEL, 
		sizeof(eiTesselJob), 
		EI_DB_FLUSHABLE);

	tessel_job->cam = job->cam;
	tessel_job->inst = job->inst;
	tessel_job->approx = job->approx;
	tessel_job->motion = job->motion;
	tessel_job->object_desc = job->object_desc;
	tessel_job->tessellable = poly1_tag;
	tessel_job->raytraceable = job->raytraceable;
	tessel_job->subdiv = job->subdiv + 1;
	tessel_job->displace_list = job->displace_list;
	tessel_job->deferred_dice = eiFALSE;

	ei_db_end(db, tessel_job_tag1);

	tessel_job = (eiTesselJob *)ei_db_create(
		db, 
		&tessel_job_tag2, 
		EI_DATA_TYPE_JOB_TESSEL, 
		sizeof(eiTesselJob), 
		EI_DB_FLUSHABLE);

	tessel_job->cam = job->cam;
	tessel_job->inst = job->inst;
	tessel_job->approx = job->approx;
	tessel_job->motion = job->motion;
	tessel_job->object_desc = job->object_desc;
	tessel_job->tessellable = poly2_tag;
	tessel_job->raytraceable = job->raytraceable;
	tessel_job->subdiv = job->subdiv + 1;
	tessel_job->displace_list = job->displace_list;
	tessel_job->deferred_dice = eiFALSE;

	ei_db_end(db, tessel_job_tag2);

	tessel_job = (eiTesselJob *)ei_db_create(
		db, 
		&tessel_job_tag3, 
		EI_DATA_TYPE_JOB_TESSEL, 
		sizeof(eiTesselJob), 
		EI_DB_FLUSHABLE);

	tessel_job->cam = job->cam;
	tessel_job->inst = job->inst;
	tessel_job->approx = job->approx;
	tessel_job->motion = job->motion;
	tessel_job->object_desc = job->object_desc;
	tessel_job->tessellable = poly3_tag;
	tessel_job->raytraceable = job->raytraceable;
	tessel_job->subdiv = job->subdiv + 1;
	tessel_job->displace_list = job->displace_list;
	tessel_job->deferred_dice = eiFALSE;

	ei_db_end(db, tessel_job_tag3);

	tessel_job = (eiTesselJob *)ei_db_create(
		db, 
		&tessel_job_tag4, 
		EI_DATA_TYPE_JOB_TESSEL, 
		sizeof(eiTesselJob), 
		EI_DB_FLUSHABLE);

	tessel_job->cam = job->cam;
	tessel_job->inst = job->inst;
	tessel_job->approx = job->approx;
	tessel_job->motion = job->motion;
	tessel_job->object_desc = job->object_desc;
	tessel_job->tessellable = poly4_tag;
	tessel_job->raytraceable = job->raytraceable;
	tessel_job->subdiv = job->subdiv + 1;
	tessel_job->displace_list = job->displace_list;
	tessel_job->deferred_dice = eiFALSE;

	ei_db_end(db, tessel_job_tag4);

	if (queue != NULL)
	{
		/* add to thread local job queue if available */
		ei_job_queue_add(queue, tessel_job_tag1);
		ei_job_queue_add(queue, tessel_job_tag2);
		ei_job_queue_add(queue, tessel_job_tag3);
		ei_job_queue_add(queue, tessel_job_tag4);
	}
	else if (master != NULL)
	{
		/* otherwise, add to global job queue */
		ei_master_add_job(master, tessel_job_tag1);
		ei_master_add_job(master, tessel_job_tag2);
		ei_master_add_job(master, tessel_job_tag3);
		ei_master_add_job(master, tessel_job_tag4);
	}
	else
	{
		ei_error("No valid job queue for geometric approximation.\n");
	}
}

static void ei_poly_object_spatial_split(
	eiDatabase *db, 
	eiTesselJob *job, 
	eiPolyTessel *poly, 
	const eiBound *box, 
	eiJobQueue *queue)
{
	eiMaster		*master;
	eiPolyTessel	*lpoly, *rpoly;
	eiTag			lpoly_tag, rpoly_tag;
	eiInt			axis;
	eiInt			items_per_slot;
	eiInt			num_vertices;
	eiInt			num_triangles;
	eiInt			num_half_triangles;
	eiTriSortElem	*tri_sort_elems;
	eiVtxMapElem	*vtx_map_elems;
	eiTag			left_tessel_job_tag, right_tessel_job_tag;
	eiTesselJob		*tessel_job;
	eiInt			i;

	eiDataTableIterator		pos_list_iter;
	eiDataTableIterator		motion_pos_list_iter;
	eiDataTableIterator		varying_list_iter;
	eiDataTableIterator		vertex_list_iter;
	eiDataTableIterator		triangle_list_iter;
	eiDataTable				*lpos_tab;
	eiDataTable				*lmotion_pos_tab;
	eiDataTable				*lvarying_tab;
	eiDataTable				*lvertex_tab;
	eiDataTable				*ltriangle_tab;
	eiDataTable				*rpos_tab;
	eiDataTable				*rmotion_pos_tab;
	eiDataTable				*rvarying_tab;
	eiDataTable				*rvertex_tab;
	eiDataTable				*rtriangle_tab;

	/* get rendering manager */
	master = ei_db_net_master(db);

	/* create left poly */
	lpoly = (eiPolyTessel *)ei_db_create(
		db, 
		&lpoly_tag, 
		EI_DATA_TYPE_INTARRAY, 
		sizeof(eiPolyTessel), 
		EI_DB_FLUSHABLE);

	/* create right poly */
	rpoly = (eiPolyTessel *)ei_db_create(
		db, 
		&rpoly_tag, 
		EI_DATA_TYPE_INTARRAY, 
		sizeof(eiPolyTessel), 
		EI_DB_FLUSHABLE);

	/* record the tag of the source object node */
	lpoly->object = poly->object;
	rpoly->object = poly->object;

	/* choose a splitting axis of the maximum length */
	axis = bmax_axis(box);

	ei_data_table_begin(db, poly->pos_list, &pos_list_iter);
	ei_data_table_begin(db, poly->motion_pos_list, &motion_pos_list_iter);
	ei_data_table_begin(db, poly->varying_list, &varying_list_iter);
	ei_data_table_begin(db, poly->vertex_list, &vertex_list_iter);
	ei_data_table_begin(db, poly->triangle_list, &triangle_list_iter);

	/* choose a splitting plane which splits primitives into two sets of equal primitive count */
	num_vertices = pos_list_iter.tab->item_count;
	num_triangles = triangle_list_iter.tab->item_count / NUM_TRI_CHANS;

	tri_sort_elems = (eiTriSortElem *)ei_allocate(sizeof(eiTriSortElem) * num_triangles);

	for (i = 0; i < num_triangles; ++i)
	{
		eiIndex			v1, v2, v3;
		eiVector		pos1, pos2, pos3;
		eiVector		motion_pos1, motion_pos2, motion_pos3;

		v1 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 0));
		v2 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 1));
		v3 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, i * NUM_TRI_CHANS + 2));

		/* access vertex 1 */
		movv(&pos1, (eiVector *)ei_data_table_read(&pos_list_iter, v1));
		movv(&motion_pos1, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v1));

		/* access vertex 2 */
		movv(&pos2, (eiVector *)ei_data_table_read(&pos_list_iter, v2));
		movv(&motion_pos2, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v2));

		/* access vertex 3 */
		movv(&pos3, (eiVector *)ei_data_table_read(&pos_list_iter, v3));
		movv(&motion_pos3, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v3));

		/* add the sort element for current triangle */
		tri_sort_elems[i].tri_mid_pos = (pos1.comp[axis] + pos2.comp[axis] + pos3.comp[axis] + 
			motion_pos1.comp[axis] + motion_pos2.comp[axis] + motion_pos3.comp[axis]) * (1.0f / 6.0f);
		tri_sort_elems[i].tri_index = i;
	}

	ei_heapsort((void *)tri_sort_elems, num_triangles, sizeof(eiTriSortElem), ei_tri_sort_elem_compare);

	num_half_triangles = num_triangles / 2;

	/* build pos list */
	items_per_slot = pos_list_iter.tab->items_per_slot;
	lpoly->pos_list = ei_create_data_table(db, EI_DATA_TYPE_VECTOR, items_per_slot);
	ei_db_ref(db, lpoly->pos_list);
	rpoly->pos_list = ei_create_data_table(db, EI_DATA_TYPE_VECTOR, items_per_slot);
	ei_db_ref(db, rpoly->pos_list);

	/* build motion pos list */
	if (poly->motion_pos_list == poly->pos_list)
	{
		lpoly->motion_pos_list = lpoly->pos_list;
		rpoly->motion_pos_list = rpoly->pos_list;
	}
	else
	{
		items_per_slot = motion_pos_list_iter.tab->items_per_slot;
		lpoly->motion_pos_list = ei_create_data_table(db, EI_DATA_TYPE_VECTOR, items_per_slot);
		rpoly->motion_pos_list = ei_create_data_table(db, EI_DATA_TYPE_VECTOR, items_per_slot);
	}

	ei_db_ref(db, lpoly->motion_pos_list);
	ei_db_ref(db, rpoly->motion_pos_list);

	/* build varying list */
	lpoly->varying_dim = poly->varying_dim;
	rpoly->varying_dim = poly->varying_dim;
	lpoly->varying_list = eiNULL_TAG;
	rpoly->varying_list = eiNULL_TAG;

	if (poly->varying_list != eiNULL_TAG)
	{
		items_per_slot = varying_list_iter.tab->items_per_slot;
		lpoly->varying_list = ei_create_data_table(db, EI_DATA_TYPE_SCALAR, items_per_slot);
		ei_db_ref(db, lpoly->varying_list);
		rpoly->varying_list = ei_create_data_table(db, EI_DATA_TYPE_SCALAR, items_per_slot);
		ei_db_ref(db, rpoly->varying_list);
	}

	/* build vertex list */
	lpoly->vertex_dim = poly->vertex_dim;
	rpoly->vertex_dim = poly->vertex_dim;
	lpoly->vertex_list = eiNULL_TAG;
	rpoly->vertex_list = eiNULL_TAG;

	if (poly->vertex_list != eiNULL_TAG)
	{
		items_per_slot = vertex_list_iter.tab->items_per_slot;
		lpoly->vertex_list = ei_create_data_table(db, EI_DATA_TYPE_SCALAR, items_per_slot);
		ei_db_ref(db, lpoly->vertex_list);
		rpoly->vertex_list = ei_create_data_table(db, EI_DATA_TYPE_SCALAR, items_per_slot);
		ei_db_ref(db, rpoly->vertex_list);
	}

	/* build triangle list */
	items_per_slot = triangle_list_iter.tab->items_per_slot;
	lpoly->triangle_list = ei_create_data_table(db, EI_DATA_TYPE_INDEX, items_per_slot);
	ei_db_ref(db, lpoly->triangle_list);
	rpoly->triangle_list = ei_create_data_table(db, EI_DATA_TYPE_INDEX, items_per_slot);
	ei_db_ref(db, rpoly->triangle_list);

	vtx_map_elems = (eiVtxMapElem *)ei_allocate(sizeof(eiVtxMapElem) * num_vertices);

	/* initialize all vertices to unreference status */
	for (i = 0; i < num_vertices; ++i)
	{
		vtx_map_elems[i].left_index = eiNULL_TAG;
		vtx_map_elems[i].right_index = eiNULL_TAG;
	}

	lpos_tab = (eiDataTable *)ei_db_access(db, lpoly->pos_list);
	lmotion_pos_tab = NULL;
	if (lpoly->pos_list != lpoly->motion_pos_list)
	{
		lmotion_pos_tab = (eiDataTable *)ei_db_access(db, lpoly->motion_pos_list);
	}
	if (lpoly->varying_list != eiNULL_TAG)
	{
		lvarying_tab = (eiDataTable *)ei_db_access(db, lpoly->varying_list);
	}
	if (lpoly->vertex_list != eiNULL_TAG)
	{
		lvertex_tab = (eiDataTable *)ei_db_access(db, lpoly->vertex_list);
	}
	ltriangle_tab = (eiDataTable *)ei_db_access(db, lpoly->triangle_list);

	rpos_tab = (eiDataTable *)ei_db_access(db, rpoly->pos_list);
	rmotion_pos_tab = NULL;
	if (rpoly->pos_list != rpoly->motion_pos_list)
	{
		rmotion_pos_tab = (eiDataTable *)ei_db_access(db, rpoly->motion_pos_list);
	}
	if (rpoly->varying_list != eiNULL_TAG)
	{
		rvarying_tab = (eiDataTable *)ei_db_access(db, rpoly->varying_list);
	}
	if (rpoly->vertex_list != eiNULL_TAG)
	{
		rvertex_tab = (eiDataTable *)ei_db_access(db, rpoly->vertex_list);
	}
	rtriangle_tab = (eiDataTable *)ei_db_access(db, rpoly->triangle_list);

	/* classify all primitives into left set and right set regarding the splitting plane */
	for (i = 0; i < num_half_triangles; ++i)
	{
		eiIndex		tri_index;
		eiIndex		v1, v2, v3;
		eiIndex		e1, e2, e3;
		eiIndex		prim_index;
		eiVector	pos, motion_pos;

		tri_index = tri_sort_elems[i].tri_index;

		/* access 3 vertex indices of triangle */
		v1 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 0));
		v2 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 1));
		v3 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 2));

		/* access 3 edge segments */
		e1 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 3));
		e2 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 4));
		e3 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 5));

		/* access primitive index */
		prim_index = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 6));

		/* map vertex indices to the vertex indices in the vertex list of sub-object */
		if (vtx_map_elems[v1].left_index == eiNULL_TAG)
		{
			vtx_map_elems[v1].left_index = ei_data_table_size(lpos_tab);

			movv(&pos, (eiVector *)ei_data_table_read(&pos_list_iter, v1));
			movv(&motion_pos, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v1));

			add_new_vertex(
				db, 
				poly, 
				&lpos_tab, 
				&lmotion_pos_tab, 
				&lvarying_tab, 
				&varying_list_iter, 
				&lvertex_tab, 
				&vertex_list_iter, 
				&pos, 
				&motion_pos, 
				v1);
		}
		v1 = vtx_map_elems[v1].left_index;

		if (vtx_map_elems[v2].left_index == eiNULL_TAG)
		{
			vtx_map_elems[v2].left_index = ei_data_table_size(lpos_tab);

			movv(&pos, (eiVector *)ei_data_table_read(&pos_list_iter, v2));
			movv(&motion_pos, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v2));

			add_new_vertex(
				db, 
				poly, 
				&lpos_tab, 
				&lmotion_pos_tab, 
				&lvarying_tab, 
				&varying_list_iter, 
				&lvertex_tab, 
				&vertex_list_iter, 
				&pos, 
				&motion_pos, 
				v2);
		}
		v2 = vtx_map_elems[v2].left_index;

		if (vtx_map_elems[v3].left_index == eiNULL_TAG)
		{
			vtx_map_elems[v3].left_index = ei_data_table_size(lpos_tab);

			movv(&pos, (eiVector *)ei_data_table_read(&pos_list_iter, v3));
			movv(&motion_pos, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v3));

			add_new_vertex(
				db, 
				poly, 
				&lpos_tab, 
				&lmotion_pos_tab, 
				&lvarying_tab, 
				&varying_list_iter, 
				&lvertex_tab, 
				&vertex_list_iter, 
				&pos, 
				&motion_pos, 
				v3);
		}
		v3 = vtx_map_elems[v3].left_index;

		/* add to left poly */
		add_new_triangle(
			db, 
			&ltriangle_tab, 
			v1, 
			v2, 
			v3, 
			e1, 
			e2, 
			e3, 
			prim_index);
	}

	for (i = num_half_triangles; i < num_triangles; ++i)
	{
		eiIndex		tri_index;
		eiIndex		v1, v2, v3;
		eiIndex		e1, e2, e3;
		eiIndex		prim_index;
		eiVector	pos, motion_pos;

		tri_index = tri_sort_elems[i].tri_index;

		/* access 3 vertex indices of triangle */
		v1 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 0));
		v2 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 1));
		v3 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 2));

		/* access 3 edge segments */
		e1 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 3));
		e2 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 4));
		e3 = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 5));

		/* access primitive index */
		prim_index = *((eiIndex *)ei_data_table_read(&triangle_list_iter, tri_index * NUM_TRI_CHANS + 6));

		/* map vertex indices to the vertex indices in the vertex list of sub-object */
		if (vtx_map_elems[v1].right_index == eiNULL_TAG)
		{
			vtx_map_elems[v1].right_index = ei_data_table_size(rpos_tab);

			movv(&pos, (eiVector *)ei_data_table_read(&pos_list_iter, v1));
			movv(&motion_pos, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v1));

			add_new_vertex(
				db, 
				poly, 
				&rpos_tab, 
				&rmotion_pos_tab, 
				&rvarying_tab, 
				&varying_list_iter, 
				&rvertex_tab, 
				&vertex_list_iter, 
				&pos, 
				&motion_pos, 
				v1);
		}
		v1 = vtx_map_elems[v1].right_index;

		if (vtx_map_elems[v2].right_index == eiNULL_TAG)
		{
			vtx_map_elems[v2].right_index = ei_data_table_size(rpos_tab);

			movv(&pos, (eiVector *)ei_data_table_read(&pos_list_iter, v2));
			movv(&motion_pos, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v2));

			add_new_vertex(
				db, 
				poly, 
				&rpos_tab, 
				&rmotion_pos_tab, 
				&rvarying_tab, 
				&varying_list_iter, 
				&rvertex_tab, 
				&vertex_list_iter, 
				&pos, 
				&motion_pos, 
				v2);
		}
		v2 = vtx_map_elems[v2].right_index;

		if (vtx_map_elems[v3].right_index == eiNULL_TAG)
		{
			vtx_map_elems[v3].right_index = ei_data_table_size(rpos_tab);

			movv(&pos, (eiVector *)ei_data_table_read(&pos_list_iter, v3));
			movv(&motion_pos, (eiVector *)ei_data_table_read(&motion_pos_list_iter, v3));

			add_new_vertex(
				db, 
				poly, 
				&rpos_tab, 
				&rmotion_pos_tab, 
				&rvarying_tab, 
				&varying_list_iter, 
				&rvertex_tab, 
				&vertex_list_iter, 
				&pos, 
				&motion_pos, 
				v3);
		}
		v3 = vtx_map_elems[v3].right_index;

		/* add to right poly */
		add_new_triangle(
			db, 
			&rtriangle_tab, 
			v1, 
			v2, 
			v3, 
			e1, 
			e2, 
			e3, 
			prim_index);
	}

	ei_db_end(db, rpoly->triangle_list);
	if (rpoly->vertex_list != eiNULL_TAG)
	{
		ei_db_end(db, rpoly->vertex_list);
	}
	if (rpoly->varying_list != eiNULL_TAG)
	{
		ei_db_end(db, rpoly->varying_list);
	}
	if (rmotion_pos_tab != NULL)
	{
		ei_db_end(db, rpoly->motion_pos_list);
	}
	ei_db_end(db, rpoly->pos_list);

	ei_db_end(db, lpoly->triangle_list);
	if (lpoly->vertex_list != eiNULL_TAG)
	{
		ei_db_end(db, lpoly->vertex_list);
	}
	if (lpoly->varying_list != eiNULL_TAG)
	{
		ei_db_end(db, lpoly->varying_list);
	}
	if (lmotion_pos_tab != NULL)
	{
		ei_db_end(db, lpoly->motion_pos_list);
	}
	ei_db_end(db, lpoly->pos_list);

	eiCHECK_FREE(vtx_map_elems);
	eiCHECK_FREE(tri_sort_elems);

	ei_data_table_end(&triangle_list_iter);
	ei_data_table_end(&vertex_list_iter);
	ei_data_table_end(&varying_list_iter);
	ei_data_table_end(&motion_pos_list_iter);
	ei_data_table_end(&pos_list_iter);

	/* finish creating right poly */
	ei_db_end(db, rpoly_tag);

	/* finish creating left poly */
	ei_db_end(db, lpoly_tag);

	/* schedule tessellation jobs for generated sub-objects */
	tessel_job = (eiTesselJob *)ei_db_create(
		db, 
		&left_tessel_job_tag, 
		EI_DATA_TYPE_JOB_TESSEL, 
		sizeof(eiTesselJob), 
		EI_DB_FLUSHABLE);

	tessel_job->cam = job->cam;
	tessel_job->inst = job->inst;
	tessel_job->approx = job->approx;
	tessel_job->motion = job->motion;
	tessel_job->object_desc = job->object_desc;
	tessel_job->tessellable = lpoly_tag;
	tessel_job->raytraceable = job->raytraceable;
	tessel_job->subdiv = job->subdiv + 1;
	tessel_job->displace_list = job->displace_list;
	tessel_job->deferred_dice = eiFALSE;

	ei_db_end(db, left_tessel_job_tag);

	tessel_job = (eiTesselJob *)ei_db_create(
		db, 
		&right_tessel_job_tag, 
		EI_DATA_TYPE_JOB_TESSEL, 
		sizeof(eiTesselJob), 
		EI_DB_FLUSHABLE);

	tessel_job->cam = job->cam;
	tessel_job->inst = job->inst;
	tessel_job->approx = job->approx;
	tessel_job->motion = job->motion;
	tessel_job->object_desc = job->object_desc;
	tessel_job->tessellable = rpoly_tag;
	tessel_job->raytraceable = job->raytraceable;
	tessel_job->subdiv = job->subdiv + 1;
	tessel_job->displace_list = job->displace_list;
	tessel_job->deferred_dice = eiFALSE;

	ei_db_end(db, right_tessel_job_tag);

	if (queue != NULL)
	{
		/* add to thread local job queue if available */
		ei_job_queue_add(queue, left_tessel_job_tag);
		ei_job_queue_add(queue, right_tessel_job_tag);
	}
	else if (master != NULL)
	{
		/* otherwise, add to global job queue */
		ei_master_add_job(master, left_tessel_job_tag);
		ei_master_add_job(master, right_tessel_job_tag);
	}
	else
	{
		ei_error("No valid job queue for geometric approximation.\n");
	}
}

void ei_poly_object_split(
	eiDatabase *db, 
	eiTesselJob *job, 
	eiObject *obj, 
	const eiBound *box, 
	eiJobQueue *queue)
{
	eiPolyTessel	*poly;
	eiDataTable		*triangle_tab;
	eiInt			num_triangles;

	poly = (eiPolyTessel *)obj;

	triangle_tab = (eiDataTable *)ei_db_access(db, poly->triangle_list);
	num_triangles = ei_data_table_size(triangle_tab) / NUM_TRI_CHANS;
	ei_db_end(db, poly->triangle_list);

	if (num_triangles == 1)
	{
		/* only one triangle, cannot do spatial split further, 
		   the triangle is big, do parametric split instead */
		ei_poly_object_parametric_split(
			db, 
			job, 
			poly, 
			box, 
			queue);
	}
	else
	{
		/* do splatial split since there are more than one triangle */
		ei_poly_object_spatial_split(
			db, 
			job, 
			poly, 
			box, 
			queue);
	}
}

void ei_poly_object_intersect(
	eiObject *obj, 
	const eiTag tessel_tag, 
	const eiIndex tessel_instance_index, 
	const eiIndex parent_bsptree, 
	eiState *state, 
	ei_array *hit_info_array, 
	const eiBool sort_by_distance)
{
	ei_warning("Poly does not support intersect function.\n");
}

eiBool ei_poly_object_interp_varying(
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
	eiRayTessel			*tessel;
	eiRayTriangle		*tri;
	eiInt				i;

	if (param == NULL)
	{
		if (strcmp(name, "P") == 0)
		{
			tessel = (eiRayTessel *)ei_db_access(nodesys->m_db, tessel_tag);

			tri = ei_rt_tessel_get_triangle(tessel, tri_index);

			interp_point((eiVector *)x, 
				&ei_rt_tessel_get_vertex(tessel, tri->v1)->pos, 
				&ei_rt_tessel_get_vertex(tessel, tri->v2)->pos, 
				&ei_rt_tessel_get_vertex(tessel, tri->v3)->pos, 
				bary);

			sub((eiVector *)dx1, &ei_rt_tessel_get_vertex(tessel, tri->v2)->pos, &ei_rt_tessel_get_vertex(tessel, tri->v1)->pos);
			sub((eiVector *)dx2, &ei_rt_tessel_get_vertex(tessel, tri->v3)->pos, &ei_rt_tessel_get_vertex(tessel, tri->v1)->pos);

			ei_db_end(nodesys->m_db, tessel_tag);

			return eiTRUE;
		}
		else if (strcmp(name, "Pm") == 0)
		{
			tessel = (eiRayTessel *)ei_db_access(nodesys->m_db, tessel_tag);

			tri = ei_rt_tessel_get_triangle(tessel, tri_index);

			interp_point((eiVector *)x, 
				&ei_rt_tessel_get_vertex(tessel, tri->v1)->m_pos, 
				&ei_rt_tessel_get_vertex(tessel, tri->v2)->m_pos, 
				&ei_rt_tessel_get_vertex(tessel, tri->v3)->m_pos, 
				bary);

			sub((eiVector *)dx1, &ei_rt_tessel_get_vertex(tessel, tri->v2)->m_pos, &ei_rt_tessel_get_vertex(tessel, tri->v1)->m_pos);
			sub((eiVector *)dx2, &ei_rt_tessel_get_vertex(tessel, tri->v3)->m_pos, &ei_rt_tessel_get_vertex(tessel, tri->v1)->m_pos);

			ei_db_end(nodesys->m_db, tessel_tag);

			return eiTRUE;
		}

		return eiFALSE;
	}
	else
	{
		tessel = (eiRayTessel *)ei_db_access(nodesys->m_db, tessel_tag);

		tri = ei_rt_tessel_get_triangle(tessel, tri_index);

		for (i = 0; i < param->channel_dim; ++i)
		{
			eiScalar	*vertex_list;

			vertex_list = ei_rt_tessel_get_vertex_channel(tessel, param->channel_offset + i);

			interp_scalar(x + i, 
				vertex_list[tri->v1], 
				vertex_list[tri->v2], 
				vertex_list[tri->v3], 
				bary);

			dx1[i] = vertex_list[tri->v2] - vertex_list[tri->v1];
			dx2[i] = vertex_list[tri->v3] - vertex_list[tri->v1];
		}

		ei_db_end(nodesys->m_db, tessel_tag);

		return eiTRUE;
	}
}

eiNodeObject *ei_create_poly_object_node_object(void *param)
{
	eiObjectElement	*element;

	element = ei_create_object_element();

	((eiPluginObject *)element)->deletethis = ei_object_element_deletethis;
	((eiNodeObject *)element)->init_node = ei_poly_object_init;
	((eiNodeObject *)element)->exit_node = ei_poly_object_exit;
	((eiNodeObject *)element)->node_changed = NULL;
	((eiElement *)element)->update_instance = ei_object_update_instance;
	element->create_obj = ei_poly_object_create;
	element->delete_obj = ei_poly_object_delete;
	element->bound = ei_poly_object_bound;
	element->diceable = ei_poly_object_diceable;
	element->dice = ei_poly_object_dice;
	element->deferred_dice = ei_poly_object_deferred_dice;
	element->split = ei_poly_object_split;
	element->intersect = ei_poly_object_intersect;
	element->interp_varying = ei_poly_object_interp_varying;
	/* for polygon object, the vertex storage class is the same as varying */
	element->interp_vertex = ei_poly_object_interp_varying;

	return ((eiNodeObject *)element);
}

void ei_install_poly_object_node(eiNodeSystem *nodesys)
{
	eiTag		desc_tag;
	eiNodeDesc	*desc;
	eiTag		default_tag;
	eiInt		default_int;

	desc = ei_nodesys_node_desc(nodesys, &desc_tag, "poly");
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
		EI_DATA_TYPE_TAG, 
		"pos_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"motion_pos_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"triangle_list", 
		&default_tag);

	ei_nodesys_end_node_desc(nodesys, desc, desc_tag);
}
