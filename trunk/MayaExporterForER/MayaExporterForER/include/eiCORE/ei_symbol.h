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
 
#ifndef EI_SYMBOL_H
#define EI_SYMBOL_H

/** \brief Utilities for handling symbols.
 * \file ei_symbol.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>
#include <eiCORE/ei_btree.h>
#include <eiCORE/ei_platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief A string table for converting all strings 
 * into unique addresses. this class is not thread-safe, 
 * users must ensure this from outside */
typedef struct eiStringTable {
	ei_btree		string_set;
} eiStringTable;

/** \brief Initialize a string table. */
eiCORE_API void ei_string_table_init(eiStringTable *tab);
/** \brief Cleanup a string table. */
eiCORE_API void ei_string_table_exit(eiStringTable *tab);

/** \brief Convert a native string to internal unique string. */
eiCORE_API const char *ei_string_table_convert(eiStringTable *tab, const char *str);
/** \brief Unconvert a native string to internal unique string. */
eiCORE_API void ei_string_table_unconvert(eiStringTable *tab, const char *str);
/** \brief Lookup a native or internal unique string, return NULL if failed. */
eiCORE_API const char *ei_string_table_lookup(eiStringTable *tab, const char *str);

/** \brief A symbol map which maps a string to a data. 
 * this class is not thread-safe, users must ensure this 
 * from outside. */
typedef struct eiSymbolMap {
	ei_btree		symbol_map;
	void			*null;
} eiSymbolMap;

/** \brief Initialize the symbol map.
 * @param null The null value, should be eiNULL_TAG for tags and 
 * NULL for user pointers */
eiCORE_API void ei_symbol_map_init(eiSymbolMap *map, void * const null);
/** \brief Cleanup the symbol map. */
eiCORE_API void ei_symbol_map_exit(eiSymbolMap *map);

/** \brief Insert a symbol into the map.
 * @param name The name of the symbol
 * @param data The data of the symbol, can be either a tag or a user pointer
 */
eiCORE_API void ei_symbol_map_insert(eiSymbolMap *map, const char *name, void * const data);
/** \brief Lookup a symbol by its name and returns its data. */
eiCORE_API void *ei_symbol_map_find(eiSymbolMap *map, const char *name);
/** \brief Delete a symbol by its name. */
eiCORE_API void ei_symbol_map_erase(eiSymbolMap *map, const char *name);

/** \brief A symbol table for mapping unique string to a data. 
 * this class is not thread-safe, users must ensure this from outside. */
typedef struct eiSymbolTable {
	/* the string table for converting all strings to unique strings */
	eiStringTable	string_table;
	eiSymbolMap		symbol_map;
} eiSymbolTable;

/** \brief Initialize the symbol table.
 * @param null The null value, should be eiNULL_TAG for tags and 
 * NULL for user pointers */
eiCORE_API void ei_symbol_table_init(eiSymbolTable *tab, void * const null);
/** \brief Cleanup the symbol table. */
eiCORE_API void ei_symbol_table_exit(eiSymbolTable *tab);

/** \brief Insert a symbol into the table.
 * @param name The name of the symbol
 * @param data The data of the symbol, can be either a tag or a user pointer
 */
eiCORE_API void ei_symbol_table_add(eiSymbolTable *tab, const char *name, void * const data);
/** \brief Lookup a symbol by its name and returns its data. */
eiCORE_API void *ei_symbol_table_find(eiSymbolTable *tab, const char *name);
/** \brief Delete a symbol by its name. */
eiCORE_API void ei_symbol_table_remove(eiSymbolTable *tab, const char *name);

#ifdef __cplusplus
}
#endif

#endif
