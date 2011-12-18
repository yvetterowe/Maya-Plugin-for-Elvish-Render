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

#ifndef EI_BSP_H
#define EI_BSP_H

/** \brief The Binary Space Partitioning(BSP) module for splitting 
 * primitives into optimized kd-trees. Conceptually, we treat the 
 * input primitives as an array, and access the primitive using 
 * indices of this array, finally we sort primitives into leaf nodes 
 * with index array, and user is responsible for creating real leaf 
 * data with the primitive indices.
 * \file ei_bsp.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiCORE/ei_bound.h>
#include <eiCORE/ei_platform.h>
#include <eiCORE/ei_pool.h>
#include <eiCORE/ei_timer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EI_MAX_BSP_DEPTH						50
#define EI_MAX_BSP_SORTS						32
#define EI_BSP_STACK_SIZE						83
#define EI_BSP_FASTSORT_SIZE					32
#define EI_CUT_OFF_EMPTY						0.8f
#define EI_BSP_SMALL_PRIM_SIZE					10000
/* TODO: a lot to improve for these experience values */
#define EI_TRAVERSE_COST						0.7f
#define EI_INTERSECT_COST						0.3f

/* forward declarations */
typedef struct eiBSPBuildParams		eiBSPBuildParams;
typedef struct eiState				eiState;
typedef struct eiDatabase			eiDatabase;

/** \brief Event for sorting end-points of primitive bounds fast. */
typedef struct eiBSPFastSortEvent {
	eiInt		left_count;		/* 4 */
	eiInt		right_count;	/* 4 */
	eiInt		planar_count;	/* 4 */
	eiScalar	pos;			/* 4 */
} eiBSPFastSortEvent;

/** \brief Event for sorting end-points of primitive bounds. */
typedef struct eiBSPSortEvent {
	/* the position of the plane. */
	eiScalar	epos;		/* 4 */
	/* a flag specifying whether triangle starts, ends, or is planar at this plane. */
	eiInt		etype;		/* 4 */
} eiBSPSortEvent;

/** \brief Call this function to update progress in percentage. 
 * This function returns eiTRUE when user is aborting the rendering.
 */
typedef eiBool (*ei_bsp_build_progress)(const eiScalar progress);
/** \brief The callback to create leaf primitives, returns the tag 
 * of the created leaf primitives.
 * @param inputs The building input parameters.
 * @param primNums An array of primitive indices in this leaf.
 * @param totalPrims The total number of primitives in this leaf.
 * @param depth The tree depth of the leaf node.
 * @param box The bounding box of the leaf node.
 */
typedef eiTag (*ei_bsp_create_leaf_primitives)(
	eiBSPBuildParams *inputs, 
	const eiUint *primNums, 
	const eiUint totalPrims, 
	const eiInt depth, 
	const eiBound *box);
/** \brief The callback to spatial split a primitive, output the 
 * splitted bounding boxes for left child and right child.
 * @param inputs The input parameters.
 * @param prim_index The index of the primitive to be splitted.
 * @param left_box The bounding box of the left part of the 
 * splitted primitive.
 * @param right_box The bounding box of the right part of the 
 * splitted primitive.
 * @param box The bounding box of the original primitive.
 * @param axis The chosen splitting axis.
 * @param plane The chosen splitting plane along the axis.
 */
typedef void (*ei_bsp_spatial_split_primitive)(
	eiBSPBuildParams *inputs, 
	const eiUint prim_index, 
	eiBound *left_box, 
	eiBound *right_box, 
	const eiBound *box, 
	const eiInt axis, 
	const eiScalar plane);
/** \brief The callback to get the bounding box for a primitive by index.
 * @param inputs The input parameters.
 * @param prim_index The index of the primitive.
 * @param box The bounding box to be output.
 */
typedef void (*ei_bsp_get_primitive_bound)(
	eiBSPBuildParams *inputs, 
	const eiUint prim_index, 
	eiBound *box);

/** \brief The callback to get the intersection test cost of a 
 * primitive by index.
 * @param inputs The input parameters.
 * @param prim_index The index of the primitive.
 */
typedef eiScalar (*ei_bsp_get_primitive_icost)(
	eiBSPBuildParams *inputs, 
	const eiUint prim_index);

/** \brief The input parameters of BSP builder, users 
 * can derive their custom parameters from this. */
typedef struct eiBSPBuildParams {
	/* number of primitives */
	eiUint							num_prims;
	/* maximum number of leaf primitives */
	eiUint							bsp_size;
	/* maximum tree depth */
	eiInt							bsp_depth;
	/* initial tree depth for sub-trees */
	eiInt							initial_bsp_depth;
	/* progress updating callback */
	ei_bsp_build_progress			progress;
	/* create leaf primitives callback */
	ei_bsp_create_leaf_primitives	create_leaf_primitives;
	/* spatial split primitive callback */
	ei_bsp_spatial_split_primitive	spatial_split_primitive;
	/* get primitive bound callback */
	ei_bsp_get_primitive_bound		get_primitive_bound;
	/* get primitive intersection test cost callback */
	ei_bsp_get_primitive_icost		get_primitive_icost;

} eiBSPBuildParams;

/** \brief The node of BSP tree for accelerating 
 * ray-tracing, kept as small as possible. */
typedef struct eiBSPNode {
	union {
		eiScalar	splitter;	/* interior: division plane position */
		eiTag		prims;		/* sub-leaf: tag of the list of primitives */
	};
	eiInt			flags;		/*  2 bits: is_leaf, axis; 
								   30 bits: index of right child */
} eiBSPNode;

/* for internal use only */
void byteswap_bsp_node(eiDatabase *db, void *data, const eiUint size);

eiFORCEINLINE void ei_bsp_node_init(eiBSPNode *node)
{
	node->prims = eiNULL_TAG;
	node->flags = 0;
}

eiFORCEINLINE void ei_bsp_node_create_leaf(eiBSPNode *node, const eiTag prims)
{
	node->prims = prims;
	node->flags |= 3;
}
	
eiFORCEINLINE void ei_bsp_node_create_interior(eiBSPNode *node, const eiInt axis, const eiScalar splitter)
{
	node->splitter = splitter;
	node->flags = ((node->flags & ~3) | axis);
}

eiFORCEINLINE eiScalar ei_bsp_node_get_splitter(const eiBSPNode *node)
{
	return node->splitter;
}

eiFORCEINLINE eiInt ei_bsp_node_get_axis(const eiBSPNode *node)
{
	return (node->flags & 3);
}

eiFORCEINLINE eiBool ei_bsp_node_is_leaf(const eiBSPNode *node)
{
	return ((node->flags & 3) == 3);
}

eiFORCEINLINE eiInt ei_bsp_node_get_right_child_index(const eiBSPNode *node)
{
	return (node->flags >> 2);
}

eiFORCEINLINE void ei_bsp_node_set_right_child_index(eiBSPNode *node, const eiInt i)
{
	node->flags = ((node->flags & 3) | (i << 2));
}

eiFORCEINLINE eiBSPNode *ei_bsp_node_get_left_child(eiBSPNode *node, eiBSPNode *nodes)
{
	return (node + 1);
}

eiFORCEINLINE eiBSPNode *ei_bsp_node_get_right_child(eiBSPNode *node, eiBSPNode *nodes)
{
	return (nodes + ei_bsp_node_get_right_child_index(node));
}

/** \brief The bsp stack element */
typedef struct eiBSPStackElem {
	eiBSPNode			*node;
	eiScalar			t_range[2];
} eiBSPStackElem;

/** \brief The node stack for traversing kd-tree. */
typedef struct eiBSPStack {
	eiBSPStackElem		sbuf[ EI_BSP_STACK_SIZE + 1 ];
	eiBSPStackElem		*buf;
	eiInt				size;
	eiBool				to_delete;
} eiBSPStack;

/** \brief Initialize a BSP stack.
 * @param size The estimated stack size according to maximum traversal depth.
 */
eiAPI void ei_bsp_stack_init(eiBSPStack *stack, const eiInt size);
/** \brief Cleanup the BSP stack.
 */
eiAPI void ei_bsp_stack_exit(eiBSPStack *stack);

/** \brief The outputs generated by BSP builder. */
typedef struct eiBSPBuildOutputs {
	/* user should call ei_bsp_build_outputs_exit 
	   to free this memory */
	eiBSPNode		*nodes;
	/* the number of allocated nodes */
	eiUint			allocatedNodes;
	/* index of next free node, as well as current node count */
	eiUint			nextFreeNode;
	/* bounding box of the entire scene */
	eiBound			scene_box;
} eiBSPBuildOutputs;

/** \brief Initialize BSP build outputs */
eiAPI void ei_bsp_build_outputs_init(eiBSPBuildOutputs *outputs);

/** \brief Cleanup BSP build outputs. */
eiAPI void ei_bsp_build_outputs_exit(eiBSPBuildOutputs *outputs);

/** \brief The working memory of BSP builder. No initialization 
 * is required because ei_bsp_build function will initialize 
 * and cleanup the builder automatically. */
typedef struct eiBSPBuilder {
	ei_pool					memMan;
	eiBound					*boxlist;
	eiBound					*workingMainBounds;
	eiBound					*workingLeftBounds;
	eiBound					*workingRightBounds;
	eiBSPFastSortEvent		plist[ EI_BSP_FASTSORT_SIZE ];
	eiBSPSortEvent			eventList[ EI_MAX_BSP_SORTS * 2 ];
} eiBSPBuilder;

/** \brief The statistics of BSP builder */
typedef struct eiBSPBuildStats {
	eiInt					num_created_nodes;
	eiScalar				last_progress;
	eiScalar				inv_total_jobs;
	eiInt					num_bsp_leaves;
	eiInt					num_bsp_nodes;
	eiInt					num_bsp_empty_leaves;
	eiInt					num_bsp_bad_splits;
	eiInt					average_bsp_size;
	eiInt					average_bsp_depth;
	eiInt					num_bsp_averages;
	eiInt					max_bsp_size;
	eiInt					max_bsp_depth;
	eiInt					num_bsp_allocations;
	eiInt					num_temp_allocations;
	eiInt					num_temp_huge_allocations;
	eiInt					num_tessellated_primitives;
	eiTimer					timer;
} eiBSPBuildStats;

/** \brief Initialize BSP build statistics. */
eiAPI void ei_bsp_build_stats_init(eiBSPBuildStats *stats);

/** \brief Build the BSP, fill the output with created nodes.
 * @param builder The BSP builder.
 * @param inputs The custom input parameters.
 * @param outputs The outputs.
 * @param stats The statistics.
 */
eiAPI void ei_bsp_build(
	eiBSPBuilder *builder, 
	eiBSPBuildParams *inputs, 
	eiBSPBuildOutputs *outputs, 
	eiBSPBuildStats *stats);

/** \brief The callback to do intersection tests with a list of leaf primitives.
 * @param state The state variables.
 * @param prims The primitive index list retrieved from current leaf node.
 * @param params The custom parameters.
 */
typedef eiBool (*ei_bsp_intersect)(eiState *state, const eiTag prims, void *params);

/** \brief Traverse BSP tree and do intersection tests.
 * @param state The state variables.
 * @param nodes The node array to be traversed.
 * @param bspStack The BSP stack associated with current thread.
 * @param intersect The callback when intersection tests are needed.
 */
eiAPI eiBool ei_bsp_traverse(
	eiState *state, 
	eiBSPNode *nodes, 
	eiBSPStack *bspStack, 
	ei_bsp_intersect intersect, 
	void *params);

#ifdef __cplusplus
}
#endif

#endif
