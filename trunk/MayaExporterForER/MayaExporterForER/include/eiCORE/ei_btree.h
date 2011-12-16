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

#ifndef EI_BTREE_H
#define EI_BTREE_H

/** \brief Balanced binary search tree.
 * \file ei_btree.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Declaration of node of balanced binary search tree.
 */
typedef struct ei_btree_node ei_btree_node;

/** \brief Declaration of balanced binary search tree.
 */
typedef struct ei_btree ei_btree;

/** \brief Comparing callback for sorting nodes in ascending order, 
    this function should return < 0 if lhs < rhs, 
	return == 0 if lhs == rhs, 
	return > 0 if lhs > rhs.
 */
typedef eiIntptr (*ei_btree_compare)(ei_btree_node *lhs, ei_btree_node *rhs, void *param);

/** \brief Node definition of balanced binary search tree
 */
struct ei_btree_node {
	struct ei_btree_node	*left;		/** Left child */
	struct ei_btree_node	*right;		/** Right child */
	eiInt					height;		/** Height of the subtree, do not modify this field. */
};

/** \brief The function callback for deleting a node.
 */
typedef void (*ei_btree_delete_node)(ei_btree_node *node, void *param);

/** \brief The procedure used to traverse the tree, if zero is 
 * returned while traversing nodes, the traversal will be terminated.
 */
typedef eiInt (*ei_btree_proc)(ei_btree_node *node, void *param);

/** \brief Balanced binary search tree definition
 */
struct ei_btree {
	ei_btree_node			*root;			/** Root of the tree */
	ei_btree_compare		compare;		/** compare callback */
	ei_btree_delete_node	delete_node;	/** delete node callback */
	eiSizet					size;			/** Number of nodes */
	void					*delete_node_param;
};

/** \brief Initialize the node.
 */
eiCORE_API void ei_btree_node_init(ei_btree_node *node);

/** \brief Clear the node.
 */
eiCORE_API void ei_btree_node_clear(ei_btree_node *node);

/** \brief Initialize a tree.
 */
eiCORE_API void ei_btree_init(ei_btree *tree, ei_btree_compare compare, ei_btree_delete_node delete_node, void *delete_node_param);

/** \brief Clear a tree.
 */
eiCORE_API void ei_btree_clear(ei_btree *tree);

/** \brief Lookup a node by key, and return the actual node in the tree. 
    You can fill up the input node with only key field, leaving the 
    data field empty.
 */
eiCORE_API ei_btree_node *ei_btree_lookup(ei_btree *tree, ei_btree_node *node, void *param);

/** \brief Insert a node into a tree, if the key already exists in the 
    tree, insertion will not be performed.
 */
eiCORE_API void ei_btree_insert(ei_btree *tree, ei_btree_node *node, void *param);

/** \brief Delete a node from a tree.
 */
eiCORE_API void ei_btree_delete(ei_btree *tree, ei_btree_node *node, void *param);

/** \brief Traverse all nodes in the tree. The traversal order is not 
 * guaranteed to be increasing regarding the key.
 */
eiCORE_API void ei_btree_traverse(ei_btree *tree, ei_btree_proc proc, void *param);

/** \brief Return the number of nodes in this tree.
 */
eiCORE_API eiSizet ei_btree_size(ei_btree *tree);

#ifdef __cplusplus
}
#endif

#endif
