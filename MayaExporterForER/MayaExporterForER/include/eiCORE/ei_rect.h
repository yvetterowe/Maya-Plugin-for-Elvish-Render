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

#ifndef EI_RECT_H
#define EI_RECT_H

#include <eiCORE/ei_util.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* integer rectangle */
typedef struct eiRect4i {
	eiInt		left;
	eiInt		right;
	eiInt		top;
	eiInt		bottom;
} eiRect4i;

eiFORCEINLINE void ei_byteswap_rect4i(eiRect4i *rect)
{
	ei_byteswap_int(&rect->left);
	ei_byteswap_int(&rect->right);
	ei_byteswap_int(&rect->top);
	ei_byteswap_int(&rect->bottom);
}

eiFORCEINLINE void ei_rect4i_init(eiRect4i *r)
{
	r->left = 0;
	r->right = 0;
	r->top = 0;
	r->bottom = 0;
}

eiFORCEINLINE void ei_rect4i_set(eiRect4i *r,
								 const eiInt rleft, const eiInt rright,
								 const eiInt rtop, const eiInt rbottom)
{
	r->left = rleft;
	r->right = rright;
	r->top = rtop;
	r->bottom = rbottom;
}

eiFORCEINLINE void ei_rect4i_copy(eiRect4i *a, const eiRect4i *b)
{
	memcpy(a, b, sizeof(eiRect4i));
}

/* scalar rectangle */
typedef struct eiRect {
	eiScalar	left;
	eiScalar	right;
	eiScalar	top;
	eiScalar	bottom;
} eiRect;

eiFORCEINLINE void ei_byteswap_rect(eiRect *rect)
{
	ei_byteswap_scalar(&rect->left);
	ei_byteswap_scalar(&rect->right);
	ei_byteswap_scalar(&rect->top);
	ei_byteswap_scalar(&rect->bottom);
}

eiFORCEINLINE void ei_rect_init(eiRect *r)
{
	r->left = 0.0f;
	r->right = 0.0f;
	r->top = 0.0f;
	r->bottom = 0.0f;
}

eiFORCEINLINE void ei_rect_set(eiRect *r,
							   const eiScalar rleft, const eiScalar rright,
							   const eiScalar rtop, const eiScalar rbottom)
{
	r->left = rleft;
	r->right = rright;
	r->top = rtop;\
	r->bottom = rbottom;
}

eiFORCEINLINE void ei_rect_copy(eiRect *a, const eiRect *b)
{
	memcpy(a, b, sizeof(eiRect));
}

#ifdef __cplusplus
}
#endif

#endif
