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

/** \brief Balanced binary search tree implementation, 
           we implement AVL-tree for fast lookups.
 * \file ei_btree.c
 * \author Elvic Liang
 */

#include <eiCORE/ei_btree.h>
#include <eiCORE/ei_assert.h>

/* Maximum balance factor, should be 1 for AVL-tree. */
#define BTREE_MAX_BF		1
/* Balance factor is height of left sub-tree minus height of right sub-tree. */
#define BTREE_BF(self)		( \
	(((self)->left != NULL)  ? (self)->left->height  : 0) \
   -(((self)->right != NULL) ? (self)->right->height : 0) \
   )

void ei_btree_node_init(ei_btree_node *node)
{
	node->left = NULL;
	node->right = NULL;
	node->height = 0;
}

void ei_btree_node_clear(ei_btree_node *node)
{
	node->left = NULL;
	node->right = NULL;
	node->height = 0;
}

void ei_btree_init(ei_btree *tree, ei_btree_compare compare, ei_btree_delete_node delete_node, void *delete_node_param)
{
	eiDBG_ASSERT(tree != NULL && compare != NULL);

	tree->root = NULL;
	tree->compare = compare;
	tree->delete_node = delete_node;
	tree->size = 0;
	tree->delete_node_param = delete_node_param;
}

static void ei_btree_clear_imp(ei_btree *tree, ei_btree_node *node)
{
	if (node == NULL)
	{
		return;
	}

	ei_btree_clear_imp(tree, node->left);
	ei_btree_clear_imp(tree, node->right);

	node->left = NULL;
	node->right = NULL;
	if (tree->delete_node != NULL)
	{
		tree->delete_node(node, tree->delete_node_param);
	}
}

void ei_btree_clear(ei_btree *tree)
{
	eiDBG_ASSERT(tree != NULL && tree->compare != NULL);

	ei_btree_clear_imp(tree, tree->root);

	tree->root = NULL;
	tree->size = 0;
}

ei_btree_node *ei_btree_lookup(ei_btree *tree, ei_btree_node *node, void *param)
{
	ei_btree_node *p = NULL;
	eiIntptr result;

	eiDBG_ASSERT(tree != NULL && tree->compare != NULL);
	eiDBG_ASSERT(node != NULL);

	p = tree->root;

	while (p != NULL)
	{
		result = tree->compare(node, p, param);

		if (result == 0)
		{
			return p;
		}
		else if (result < 0)
		{
			p = p->left;
		}
		else
		{
			p = p->right;
		}
	}

	return NULL;
}

static ei_btree_node *ei_btree_balance(ei_btree_node *self);

static eiFORCEINLINE ei_btree_node *ei_btree_left_rotate(ei_btree_node *self)
{
	ei_btree_node *r = self->right;
	self->right = r->left;
	r->left = ei_btree_balance(self);
	return ei_btree_balance(r);
}

static eiFORCEINLINE ei_btree_node *ei_btree_right_rotate(ei_btree_node *self)
{
	ei_btree_node *l = self->left;
	self->left = l->right;
	l->right = ei_btree_balance(self);
	return ei_btree_balance(l);
}

static ei_btree_node *ei_btree_balance(ei_btree_node *self)
{
	eiInt bf = BTREE_BF(self);

	if (bf < -BTREE_MAX_BF)
	{
		if (BTREE_BF(self->right) > 0)
		{
			self->right = ei_btree_right_rotate(self->right);
		}
		return ei_btree_left_rotate(self);
	}
	else if (bf > BTREE_MAX_BF)
	{
		if (BTREE_BF(self->left) < 0)
		{
			self->left = ei_btree_left_rotate(self->left);
		}
		return ei_btree_right_rotate(self);
	}

	self->height = 0;
	
	if (self->left != NULL && self->left->height > self->height)
	{
		self->height = self->left->height;
	}

	if (self->right != NULL && self->right->height > self->height)
	{
		self->height = self->right->height;
	}

	++ self->height;
	
	return self;
}

static eiFORCEINLINE ei_btree_node *ei_btree_insert_imp(
	ei_btree *tree, ei_btree_node *self, ei_btree_node *elem, void *param)
{
	eiIntptr result;

	if (self == NULL)
	{
		return elem;
	}

	result = tree->compare(elem, self, param);

	if (result == 0)
	{
		return self;
	}
	else if (result < 0)
	{
		self->left = ei_btree_insert_imp(tree, self->left, elem, param);
	}
	else
	{
		self->right = ei_btree_insert_imp(tree, self->right, elem, param);
	}

	return ei_btree_balance(self);
}

void ei_btree_insert(ei_btree *tree, ei_btree_node *node, void *param)
{
	eiDBG_ASSERT(tree != NULL && tree->compare != NULL);
	eiDBG_ASSERT(node != NULL);

	node->left = NULL;
	node->right = NULL;
	node->height = 0;

	tree->root = ei_btree_insert_imp(tree, tree->root, node, param);

	++ tree->size;
}

static eiFORCEINLINE ei_btree_node *ei_btree_move_right(ei_btree_node *self, ei_btree_node *rhs)
{
	if (self == NULL)
	{
		return rhs;
	}

	self->right = ei_btree_move_right(self->right, rhs);

	return ei_btree_balance(self);
}

static eiFORCEINLINE ei_btree_node *ei_btree_delete_imp(
	ei_btree *tree, ei_btree_node *self, ei_btree_node *elem, void *param)
{
	eiIntptr result;

	if (self == NULL)
	{
		return NULL;
	}

	result = tree->compare(elem, self, param);

	if (result == 0)
	{
		ei_btree_node *tmp = ei_btree_move_right(self->left, self->right);
		self->left = NULL;
		self->right = NULL;
		return tmp;
	}
	else if (result < 0)
	{
		self->left = ei_btree_delete_imp(tree, self->left, elem, param);
	}
	else
	{
		self->right = ei_btree_delete_imp(tree, self->right, elem, param);
	}

	return ei_btree_balance(self);
}

void ei_btree_delete(ei_btree *tree, ei_btree_node *node, void *param)
{
	eiDBG_ASSERT(tree != NULL && tree->compare != NULL);
	eiDBG_ASSERT(node != NULL);

	tree->root = ei_btree_delete_imp(tree, tree->root, node, param);

	node->left = NULL;
	node->right = NULL;
	if (tree->delete_node != NULL)
	{
		tree->delete_node(node, tree->delete_node_param);
	}

	-- tree->size;
}

static eiFORCEINLINE eiBool ei_btree_traverse_imp(ei_btree_node *node, ei_btree_proc proc, void *param)
{
	if (node == NULL)
	{
		/* must return eiTRUE to continue traversal */
		return eiTRUE;
	}

	if (!proc(node, param))
	{
		return eiFALSE;
	}

	if (!ei_btree_traverse_imp(node->left, proc, param))
	{
		return eiFALSE;
	}

	if (!ei_btree_traverse_imp(node->right, proc, param))
	{
		return eiFALSE;
	}

	return eiTRUE;
}

void ei_btree_traverse(ei_btree *tree, ei_btree_proc proc, void *param)
{
	eiDBG_ASSERT(tree != NULL);

	ei_btree_traverse_imp(tree->root, proc, param);
}

eiSizet ei_btree_size(ei_btree *tree)
{
	eiDBG_ASSERT(tree != NULL);

	return tree->size;
}
