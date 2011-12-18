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
 
#ifndef EI_ALGORITHM_H
#define EI_ALGORITHM_H

/**
 * \file ei_algorithm.h
 * \brief This file contains routines for common algorithms.
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Generic comparing callback for sorting items in ascending order.
    \return Return value == 0 if lhs == rhs, 
	return value < 0 if lhs < rhs, 
	return value > 0 if lhs > rhs.
 */
typedef eiInt (*ei_compare)(void *lhs, void *rhs);

/** \brief In-place heap sort routine for array.
 */
eiCORE_API void ei_heapsort(void *a, const eiIntptr n, const eiIntptr item_size, ei_compare compare);

/** \brief Binary search an item in sorted array.
    \return The index of the item in sorted array, 
	-1 indicates that the item does not exist.
 */
eiCORE_API eiIntptr ei_binsearch(void *a, const eiIntptr n, void *x, const eiIntptr item_size, ei_compare compare);

#ifdef __cplusplus
}
#endif

#endif
