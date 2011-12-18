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

#include <eiAPI/ei_map.h>
#include <eiCORE/ei_data_table.h>
#include <eiCORE/ei_assert.h>

#define EI_MAP_POINTS_PER_BLOCK		100000	/* 2 MB */

void ei_byteswap_map_node(eiMapNode *node)
{
	ei_byteswap_vector(&node->pos);
	ei_byteswap_int(&node->plane);
	ei_byteswap_int(&node->index);
}

eiTag ei_create_map(
	eiDatabase *db, 
	const eiInt type, 
	const eiInt max_points)
{
	eiMap			*map;
	eiTag			tag;
	eiDataTable		*tab;
	eiByte			dummy_data[ EI_MAP_MAX_DATA_SIZE ];
	eiMapNode		*dummy = (eiMapNode *)dummy_data;

	map = (eiMap *)ei_db_create(
		db, 
		&tag, 
		EI_DATA_TYPE_MAP, 
		sizeof(eiMap), 
		EI_DB_FLUSHABLE);

	map->points = ei_create_data_table(
		db, 
		type, 
		EI_MAP_POINTS_PER_BLOCK);
	/* add a dummy point */
	tab = (eiDataTable *)ei_db_access(db, map->points);
	ei_data_table_push_back(
		db, 
		&tab, 
		&dummy);
	ei_db_end(db, map->points);
	
	map->stored_points = 0;
	map->half_stored_points = 0;
	map->max_points = max_points;

	initb(&map->box);

	ei_db_end(db, tag);

	return tag;
}

void ei_delete_map(
	eiDatabase *db, 
	const eiTag tag)
{
	eiMap		*map;

	map = (eiMap *)ei_db_access(db, tag);

	ei_delete_data_table(db, map->points);

	ei_db_end(db, tag);

	ei_db_delete(db, tag);
}

static eiFORCEINLINE void locate_points(
	eiMap *map, 
	eiDataTableIterator *iter, 
	const eiSizet item_size, 
	eiMapLookup *np, 
	const eiInt index, 
	eiMapLookupProc proc, 
	void *param)
{
	eiByte		p_data[ EI_MAP_MAX_DATA_SIZE ];
	eiMapNode	*p;
	eiScalar	dist1;
	eiScalar	dist_2;

	p = (eiMapNode *)p_data;

	memcpy(p, ei_data_table_read(iter, index), item_size);

	if (index < map->half_stored_points)
	{
		/* compute the signed distance to the splitting plane */
		dist1 = np->pos.comp[ p->plane ] - p->pos.comp[ p->plane ];

		if (dist1 > 0.0f)
		{
			/* we are right of the plane, search right child first */
			locate_points(map, iter, item_size, np, 2 * index + 1, proc, param);

			if (dist1 * dist1 < np->dist2[0])
			{
				locate_points(map, iter, item_size, np, 2 * index, proc, param);
			}
		}
		else
		{
			/* we are left of the plane, search left child first */
			locate_points(map, iter, item_size, np, 2 * index, proc, param);

			if (dist1 * dist1 < np->dist2[0])
			{
				locate_points(map, iter, item_size, np, 2 * index + 1, proc, param);
			}
		}
	}

	/* compute true squared distance to node */
	dist_2 = distsq(&p->pos, &np->pos);

	/* check if the photon is close enough */
	/* meawhile, check if the node satisfies some custom criteria */
	if (dist_2 < np->dist2[0] && 
		proc(p, dist_2, param))
	{
		if (np->found < np->max)
		{
			/* if the heap is not full, insert the node */
			++ np->found;
			np->dist2[ np->found ] = dist_2;
			np->index[ np->found ] = index;
		}
		else
		{
			/* if the heap is full, clear more space */
			eiInt		j, parent;

			if (np->got_heap == 0)
			{
				eiScalar	dst2;
				eiInt		phot;
				eiInt		half_found;
				eiInt		k;

				half_found = np->found / 2;

				for (k = half_found; k >= 1; --k)
				{
					parent = k;
					phot = np->index[k];
					dst2 = np->dist2[k];

					while (parent <= half_found)
					{
						j = parent + parent;
						if (j < np->found && np->dist2[j] < np->dist2[j + 1])
						{
							++j;
						}
						if (dst2 >= np->dist2[j])
						{
							break;
						}
						np->dist2[parent] = np->dist2[j];
						np->index[parent] = np->index[j];
						parent = j;
					}

					np->dist2[parent] = dst2;
					np->index[parent] = phot;
				}

				np->got_heap = 1;
			}

			parent = 1;
			j = 2;

			while (j <= np->found)
			{
				if (j < np->found && np->dist2[j] < np->dist2[j + 1])
				{
					++j;
				}
				if (dist_2 > np->dist2[j])
				{
					break;
				}
				np->dist2[parent] = np->dist2[j];
				np->index[parent] = np->index[j];
				parent = j;
				j += j;
			}

			np->index[parent] = index;
			np->dist2[parent] = dist_2;

			/* the heap is full, adjust maximum distance to prune the search */
			np->dist2[0] = np->dist2[1];
		}
	}
}

void ei_map_locate_points(
	eiDatabase *db, 
	const eiTag tag, 
	eiMapLookup *np, 
	const eiInt index, 
	eiMapLookupProc proc, 
	void *param)
{
	eiMap					*map;
	eiDataTableIterator		iter;
	eiSizet					item_size;

	map = (eiMap *)ei_db_access(db, tag);

	if (map->stored_points == 0)
	{
		ei_db_end(db, tag);
		return;
	}

	ei_data_table_begin(db, map->points, &iter);

	item_size = ei_db_type_size(db, iter.tab->item_type);

	locate_points(map, &iter, item_size, np, index, proc, param);

	ei_data_table_end(&iter);
	ei_db_end(db, tag);
}

eiInt ei_map_size(
	eiDatabase *db, 
	const eiTag tag)
{
	eiMap		*map;
	eiInt		size;

	map = (eiMap *)ei_db_access(db, tag);

	size = map->stored_points;

	ei_db_end(db, tag);

	return size;
}

eiBool ei_map_full(
	eiDatabase *db, 
	const eiTag tag)
{
	eiMap		*map;
	eiBool		full;

	map = (eiMap *)ei_db_access(db, tag);

	full = (map->stored_points >= map->max_points);

	ei_db_end(db, tag);

	return full;
}

void ei_map_store_points(
	eiDatabase *db, 
	const eiTag tag, 
	eiByte *nodes, 
	const eiInt num_nodes)
{
	eiMap			*map;
	eiDataTable		*tab;
	eiByte			*item;
	eiSizet			item_size;
	eiInt			nodes_to_store;
	eiInt			i;

	map = (eiMap *)ei_db_access(db, tag);
	tab = (eiDataTable *)ei_db_access(db, map->points);

	/* limit the number of points to be stored */
	nodes_to_store = MIN(num_nodes, map->max_points - map->stored_points);

	item = nodes;
	item_size = ei_db_type_size(db, tab->item_type);

	for (i = 0; i < nodes_to_store; ++i)
	{
		eiMapNode	*node;

		node = (eiMapNode *)item;

		++ map->stored_points;

		addbv(&map->box, &node->pos);

		ei_data_table_push_back(db, &tab, node);

		item += item_size;
	}

	ei_db_end(db, map->points);
	ei_db_end(db, tag);

	/* dirt the map automatically */
	ei_db_dirt(db, tag);
}

#define PMOV(a, b) \
	memcpy((a), (b), (item_size));

#define PGET(tab, dst, i) \
	memcpy((dst), ei_data_table_read((tab), (i)), (item_size));

#define PSET(tab, i, src) \
	memcpy(ei_data_table_write((tab), (i)), (src), (item_size));

#define PSWAP(p, a, b) { \
	eiByte		p1_data[ EI_MAP_MAX_DATA_SIZE ]; \
	eiByte		p2_data[ EI_MAP_MAX_DATA_SIZE ]; \
	eiMapNode	*p1 = (eiMapNode *)p1_data; \
	eiMapNode	*p2 = (eiMapNode *)p2_data; \
	PGET(p, p1, a); \
	PGET(p, p2, b); \
	PSET(p, a, p2); \
	PSET(p, b, p1); \
}

static eiFORCEINLINE void median_split(
	eiDataTableIterator *iter, 
	const eiSizet item_size, 
	const eiInt start, 
	const eiInt end, 
	const eiInt median, 
	const eiInt axis)
{
	eiInt	left = start;
	eiInt	right = end;

	while (right > left)
	{
		eiByte		pr_data[ EI_MAP_MAX_DATA_SIZE ];
		eiMapNode	*pr = (eiMapNode *)pr_data;
		eiScalar	v;
		eiInt		i, j;

		PGET(iter, pr, right);

		v = pr->pos.comp[axis];

		i = left - 1;
		j = right;

		for (;;)
		{
			eiByte		pi_data[ EI_MAP_MAX_DATA_SIZE ];
			eiByte		pj_data[ EI_MAP_MAX_DATA_SIZE ];
			eiMapNode	*pi = (eiMapNode *)pi_data;
			eiMapNode	*pj = (eiMapNode *)pj_data;

			do
			{
				PGET(iter, pi, ++i);
			}
			while (pi->pos.comp[axis] < v);

			do
			{
				PGET(iter, pj, --j);
			}
			while (pj->pos.comp[axis] > v && j > left);

			if (i >= j)
			{
				break;
			}

			PSWAP(iter, i, j);
		}

		PSWAP(iter, i, right);
		
		if (i >= median)
		{
			right = i - 1;
		}
		if (i <= median)
		{
			left = i + 1;
		}
	}
}

static eiFORCEINLINE void balance_segment(
	eiMap *map, 
	eiDataTableIterator *iter, 
	const eiSizet item_size, 
	const eiInt index, 
	const eiInt start, 
	const eiInt end)
{
	eiInt		median = 1;
	eiInt		axis;
	eiByte		median_point_data[ EI_MAP_MAX_DATA_SIZE ];
	eiMapNode	*median_point = (eiMapNode *)median_point_data;

	while ((4 * median) <= (end - start + 1))
	{
		median += median;
	}

	if ((3 * median) <= (end - start + 1))
	{
		median += median;
		median += start - 1;
	}
	else
	{
		median = end - median + 1;
	}

	axis = bmax_axis(&map->box);

	median_split(iter, item_size, start, end, median, axis);

	/* no body is going to swap the median element, 
	   so it's ok to flag the target index here */
	PGET(iter, median_point, median);
	
	median_point->index = index;
	median_point->plane = axis;

	PSET(iter, median, median_point);

	if (median > start)
	{
		if (start < (median - 1))
		{
			const eiScalar tmp = map->box.max.comp[axis];

			map->box.max.comp[axis] = median_point->pos.comp[axis];
			
			balance_segment(map, iter, item_size, 2 * index, start, median - 1);
			
			map->box.max.comp[axis] = tmp;
		}
		else
		{
			/* no to recuse, set the target index directly */
			eiByte		pl_data[ EI_MAP_MAX_DATA_SIZE ];
			eiMapNode	*pl = (eiMapNode *)pl_data;

			PGET(iter, pl, start);

			pl->index = 2 * index;

			PSET(iter, start, pl);
		}
	}

	if (median < end)
	{
		if ((median + 1) < end)
		{
			const eiScalar tmp = map->box.min.comp[axis];
			
			map->box.min.comp[axis] = median_point->pos.comp[axis];

			balance_segment(map, iter, item_size, 2 * index + 1, median + 1, end);
			
			map->box.min.comp[axis] = tmp;
		}
		else
		{
			/* no to recuse, set the target index directly */
			eiByte		pr_data[ EI_MAP_MAX_DATA_SIZE ];
			eiMapNode	*pr = (eiMapNode *)pr_data;

			PGET(iter, pr, end);

			pr->index = 2 * index + 1;

			PSET(iter, end, pr);
		}
	}
}

void ei_map_balance(
	eiDatabase *db, 
	const eiTag tag)
{
	eiMap					*map;
	eiDataTableIterator		iter;
	eiSizet					item_size;

	map = (eiMap *)ei_db_access(db, tag);
	ei_data_table_begin(db, map->points, &iter);

	item_size = ei_db_type_size(db, iter.tab->item_type);

	if (map->stored_points > 1)
	{
		eiByte		src_data[ EI_MAP_MAX_DATA_SIZE ];
		eiMapNode	*src = (eiMapNode *)src_data;
		eiInt		src_id = 1;
		eiInt		dst_id;
		eiInt		foo = 1;	/* the starting point of search circle */
		eiInt		i;

		balance_segment(map, &iter, item_size, 1, 1, map->stored_points);

		/* make the kd-tree into a heap */
		PGET(&iter, src, 1);

		dst_id = src->index;

		for (i = 1; i <= map->stored_points; ++i)
		{
			eiByte		prev_point_data[ EI_MAP_MAX_DATA_SIZE ];
			eiMapNode	*prev_point = (eiMapNode *)prev_point_data;

			/* flag previous moved */
			PGET(&iter, prev_point, src_id);
			prev_point->index = -1;
			PSET(&iter, src_id, prev_point);

			if (dst_id == foo)	/* we are going to complete the circle */
			{
				/* move what we should move first if needed */
				if (src_id != dst_id)
				{
					PSET(&iter, dst_id, src);
				}

				/* search new starting point */
				for (foo = foo + 1; foo <= map->stored_points; ++foo)
				{
					eiByte		foo_point_data[ EI_MAP_MAX_DATA_SIZE ];
					eiMapNode	*foo_point = (eiMapNode *)foo_point_data;

					PGET(&iter, foo_point, foo);

					if (foo_point->index != -1)
					{
						PMOV(src, foo_point);
						src_id = foo;
						dst_id = src->index;

						break;
					}
				}
			}
			else
			{
				if (src_id != dst_id)
				{
					eiByte		new_src_data[ EI_MAP_MAX_DATA_SIZE ];
					eiMapNode	*new_src = (eiMapNode *)new_src_data;

					PGET(&iter, new_src, dst_id);
					PSET(&iter, dst_id, src);

					PMOV(src, new_src);
					src_id = dst_id;
					dst_id = src->index;
				}
				else
				{
					for (foo = foo + 1; foo <= map->stored_points; ++foo)
					{
						eiByte		foo_point_data[ EI_MAP_MAX_DATA_SIZE ];
						eiMapNode	*foo_point = (eiMapNode *)foo_point_data;

						PGET(&iter, foo_point, foo);

						if (foo_point->index != -1)
						{
							PMOV(src, foo_point);
							src_id = foo;
							dst_id = src->index;

							break;
						}
					}
				}
			}
		}
	}

	map->half_stored_points = map->stored_points / 2 - 1;

	ei_data_table_end(&iter);
	ei_db_end(db, tag);

	/* dirt the map automatically */
	ei_db_dirt(db, tag);
}

void ei_map_traverse(
	eiDatabase *db, 
	const eiTag tag, 
	eiMapTraverseProc proc, 
	void *param)
{
	eiMap					*map;
	eiDataTableIterator		iter;
	eiInt					i;

	map = (eiMap *)ei_db_access(db, tag);
	ei_data_table_begin(db, map->points, &iter);

	for(i = 1; i <= map->stored_points; ++i)
	{
		eiMapNode	*p;

		p = (eiMapNode *)ei_data_table_read(&iter, i);

		if (!proc(p, param))
		{
			break;
		}
	}

	ei_data_table_end(&iter);
	ei_db_end(db, tag);
}

void byteswap_map(eiDatabase *db, void *ptr, const eiUint size)
{
	eiMap		*map;

	map = (eiMap *)ptr;

	ei_byteswap_int(&map->points);
	ei_byteswap_int(&map->stored_points);
	ei_byteswap_int(&map->half_stored_points);
	ei_byteswap_int(&map->max_points);
	ei_byteswap_bound(&map->box);
}
