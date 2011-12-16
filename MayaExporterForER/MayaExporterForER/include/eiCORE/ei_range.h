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

#ifndef EI_RANGE_H
#define EI_RANGE_H

#include <eiCORE/ei_util.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eiRange {
	eiScalar	min;
	eiScalar	max;
} eiRange;

eiFORCEINLINE void ei_byteswap_range(eiRange *range)
{
	ei_byteswap_scalar(&range->min);
	ei_byteswap_scalar(&range->max);
}

eiFORCEINLINE void ei_range_init(eiRange *r)
{
	r->min = 0.0f;
	r->max = 0.0f;
}

eiFORCEINLINE void ei_range_set(eiRange *r, const eiScalar rmin, const eiScalar rmax)
{
	r->min = rmin;
	r->max = rmax;
}

eiFORCEINLINE void ei_range_copy(eiRange *a, const eiRange *b)
{
	memcpy(a, b, sizeof(eiRange));
}

eiFORCEINLINE eiBool ei_range_cover(const eiRange *r, const eiScalar x)
{
	return (x >= r->min && x <= r->max);
}

#ifdef __cplusplus
}
#endif

#endif