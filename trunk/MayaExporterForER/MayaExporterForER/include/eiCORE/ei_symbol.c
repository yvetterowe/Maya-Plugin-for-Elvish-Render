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

#include <eiCORE/ei_symbol.h>
#include <eiCORE/ei_assert.h>

typedef struct eiStringNode {
	ei_btree_node	node;
	char			*string;
} eiStringNode;

static eiIntptr ei_string_node_compare(void *lhs, void *rhs, void *param)
{
	return strcmp(((eiStringNode *)lhs)->string, ((eiStringNode *)rhs)->string);
}

static eiStringNode *ei_create_string_node(const char *string)
{
	eiStringNode	*node;

	node = (eiStringNode *)ei_allocate(sizeof(eiStringNode));

	ei_btree_node_init(&node->node);
	node->string = strdup(string);

	return node;
}

static void ei_string_node_delete(ei_btree_node *n, void *param)
{
	eiStringNode	*node;

	node = (eiStringNode *)n;
	if (node == NULL)
	{
		eiASSERT(0);
		return;
	}

	eiCHECK_FREE(node->string);
	ei_btree_node_clear(&node->node);

	eiCHECK_FREE(node);
}

void ei_string_table_init(eiStringTable *tab)
{
	ei_btree_init(&tab->string_set, ei_string_node_compare, ei_string_node_delete, NULL);
}

void ei_string_table_exit(eiStringTable *tab)
{
	ei_btree_clear(&tab->string_set);
}

const char *ei_string_table_convert(eiStringTable *tab, const char *str)
{
	eiStringNode	key;
	eiStringNode	*node;

	/* we won't modify the string during lookup */
	key.string = (char *)str;
	node = (eiStringNode *)ei_btree_lookup(&tab->string_set, &key.node, NULL);

	if (node == NULL)
	{
		node = ei_create_string_node(str);
		ei_btree_insert(&tab->string_set, &node->node, NULL);
	}

	return node->string;
}

void ei_string_table_unconvert(eiStringTable *tab, const char *str)
{
	eiStringNode	key;
	eiStringNode	*node;

	/* we won't modify the string during lookup */
	key.string = (char *)str;
	node = (eiStringNode *)ei_btree_lookup(&tab->string_set, &key.node, NULL);

	if (node != NULL)
	{
		ei_btree_delete(&tab->string_set, &node->node, NULL);
	}
}

const char *ei_string_table_lookup(eiStringTable *tab, const char *str)
{
	eiStringNode	key;
	eiStringNode	*node;
	const char		*ustr;

	ustr = NULL;

	/* we won't modify the string during lookup */
	key.string = (char *)str;
	node = (eiStringNode *)ei_btree_lookup(&tab->string_set, &key.node, NULL);

	if (node != NULL)
	{
		ustr = node->string;
	}

	return ustr;
}

typedef struct eiSymbolNode {
	ei_btree_node	node;
	const char		*name;
	/* this can be a tag or a user pointer */
	void			*data;
} eiSymbolNode;

static eiIntptr ei_symbol_node_compare(void *lhs, void *rhs, void *param)
{
	/* we can compare the string pointer directly, because 
	   we assume all strings have been converted to unique 
	   strings using string table. */
	return (((eiIntptr)((eiSymbolNode *)lhs)->name) - ((eiIntptr)((eiSymbolNode *)rhs)->name));
}

static eiSymbolNode *ei_create_symbol_node(const char *name, void * const data)
{
	eiSymbolNode	*node;

	node = (eiSymbolNode *)ei_allocate(sizeof(eiSymbolNode));

	ei_btree_node_init(&node->node);
	/* NOTICE: we keep a reference to the string, the string must 
	   be a unique string managed by string table. */
	node->name = name;
	node->data = data;

	return node;
}

static void ei_symbol_node_delete(ei_btree_node *n, void *param)
{
	eiSymbolNode	*node;

	node = (eiSymbolNode *)n;
	if (node == NULL)
	{
		eiASSERT(0);
		return;
	}

	/* NOTICE: we don't need to delete the string because we just hold 
	   a reference to it. */
	ei_btree_node_clear(&node->node);

	eiCHECK_FREE(node);
}

void ei_symbol_map_init(eiSymbolMap *map, void * const null)
{
	ei_btree_init(&map->symbol_map, ei_symbol_node_compare, ei_symbol_node_delete, NULL);
	map->null = null;
}

void ei_symbol_map_exit(eiSymbolMap *map)
{
	ei_btree_clear(&map->symbol_map);
}

void ei_symbol_map_insert(eiSymbolMap *map, const char *name, void * const data)
{
	eiSymbolNode	key;
	eiSymbolNode	*node;

	key.name = name;
	node = (eiSymbolNode *)ei_btree_lookup(&map->symbol_map, &key.node, NULL);

	if (node == NULL)
	{
		node = ei_create_symbol_node(name, data);
		ei_btree_insert(&map->symbol_map, &node->node, NULL);
	}
}

void *ei_symbol_map_find(eiSymbolMap *map, const char *name)
{
	eiSymbolNode	key;
	eiSymbolNode	*node;
	void			*data;

	data = map->null;

	key.name = name;
	node = (eiSymbolNode *)ei_btree_lookup(&map->symbol_map, &key.node, NULL);

	if (node != NULL)
	{
		data = node->data;
	}

	return data;
}

void ei_symbol_map_erase(eiSymbolMap *map, const char *name)
{
	eiSymbolNode	key;
	eiSymbolNode	*node;

	key.name = name;
	node = (eiSymbolNode *)ei_btree_lookup(&map->symbol_map, &key.node, NULL);

	if (node != NULL)
	{
		ei_btree_delete(&map->symbol_map, &node->node, NULL);
	}
}

void ei_symbol_table_init(eiSymbolTable *tab, void * const null)
{
	ei_string_table_init(&tab->string_table);
	ei_symbol_map_init(&tab->symbol_map, null);
}

void ei_symbol_table_exit(eiSymbolTable *tab)
{
	ei_symbol_map_exit(&tab->symbol_map);
	ei_string_table_exit(&tab->string_table);
}

void ei_symbol_table_add(eiSymbolTable *tab, const char *name, void * const data)
{
	const char	*unique_name;

	/* convert to unique string */
	unique_name = ei_string_table_convert(&tab->string_table, name);

	/* add to symbol map */
	ei_symbol_map_insert(&tab->symbol_map, unique_name, data);
}

void *ei_symbol_table_find(eiSymbolTable *tab, const char *name)
{
	const char	*unique_name;

	/* lookup and convert to unique string */
	unique_name = ei_string_table_lookup(&tab->string_table, name);

	if (unique_name == NULL)
	{
		return tab->symbol_map.null;
	}

	/* lookup the data by unique string in symbol map */
	return ei_symbol_map_find(&tab->symbol_map, unique_name);
}

void ei_symbol_table_remove(eiSymbolTable *tab, const char *name)
{
	const char	*unique_name;

	/* lookup and convert to unique string */
	unique_name = ei_string_table_lookup(&tab->string_table, name);

	if (unique_name == NULL)
	{
		return;
	}

	/* remove from symbol map */
	ei_symbol_map_erase(&tab->symbol_map, unique_name);

	/* remove from string table */
	ei_string_table_unconvert(&tab->string_table, name);
}
