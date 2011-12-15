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
 
#ifndef EI_MAP_H
#define EI_MAP_H

/** \brief The generic map for photon maps and irradiance maps.
 * \file ei_map.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiCORE/ei_vector.h>
#include <eiCORE/ei_bound.h>
#include <eiCORE/ei_dataflow.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EI_MAP_MAX_DATA_SIZE	256 /* bytes */

/** \brief The base node in map */
typedef struct eiMapNode {
	/* the position in 3D space */
	eiVector		pos;
	/* required for building spatial 
	   acceleration structure */
	eiInt			plane;
	eiInt			index;
} eiMapNode;

/** \brief Byte-swap a base node in map */
eiAPI void ei_byteswap_map_node(eiMapNode *node);

/** \brief The lookup utility cached in thread 
 * local storage */
typedef struct eiMapLookup {
	eiInt			max;
	eiInt			found;
	eiInt			got_heap;
	eiVector		pos;
	eiScalar		*dist2;
	eiInt			*index;
} eiMapLookup;

/** \brief The generic map for caching 3D points 
 * in space */
typedef struct eiMap {
	eiTag			points;
	eiInt			stored_points;
	eiInt			half_stored_points;
	eiInt			max_points;
	eiBound			box;
} eiMap;

/** \brief The callback will be called when a node 
 * is considered within lookup radius, to determine 
 * whether this node should be added to the set of 
 * found nodes.
 * @param node The candidate node.
 * @param R2 The squared distance from the node to the 
 * lookup center. */
typedef eiBool (*eiMapLookupProc)(
	const eiMapNode *node, 
	const eiScalar R2, 
	void *param);
/** \brief The callback for traversing all nodes in 
 * a map. The traversal will stop immediately when 
 * this function returns false. */
typedef eiBool (eiMapTraverseProc)(
	const eiMapNode *node, 
	void *param);

/** \brief Create a map in database. */
eiAPI eiTag ei_create_map(
	eiDatabase *db, 
	const eiInt type, 
	const eiInt max_points);
/** \brief Delete a map from database. */
eiAPI void ei_delete_map(
	eiDatabase *db, 
	const eiTag tag);

/** \brief Collect nodes which satisfy the lookup 
 * parameters and custom criteria. */
eiAPI void ei_map_locate_points(
	eiDatabase *db, 
	const eiTag tag, 
	eiMapLookup *np, 
	const eiInt index, 
	eiMapLookupProc proc, 
	void *param);
/** \brief Return the number of stored nodes in 
 * this map. */
eiAPI eiInt ei_map_size(
	eiDatabase *db, 
	const eiTag tag);
/** \brief Return whether this map is full, and 
 * no more nodes can be added because the maximum 
 * number of nodes has been reached. */
eiAPI eiBool ei_map_full(
	eiDatabase *db, 
	const eiTag tag);
eiAPI void ei_map_store_points(
	eiDatabase *db, 
	const eiTag tag, 
	eiByte *nodes, 
	const eiInt num_nodes);
/** \brief Build spatial acceleration structure for 
 * fast lookups of nodes. */
eiAPI void ei_map_balance(
	eiDatabase *db, 
	const eiTag tag);
/** \brief Traverse all nodes in a map with custom 
 * callback. */
eiAPI void ei_map_traverse(
	eiDatabase *db, 
	const eiTag tag, 
	eiMapTraverseProc proc, 
	void *param);

/* for internal use only */
void byteswap_map(eiDatabase *db, void *ptr, const eiUint size);

#ifdef __cplusplus
}
#endif

#endif
