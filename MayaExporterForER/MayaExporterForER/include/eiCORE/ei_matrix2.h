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

#ifndef EI_MATRIX2_H
#define EI_MATRIX2_H

#include <eiCORE/ei_util.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eiMatrix2 {
	union {
		struct {
			eiScalar	m1, m2;
			eiScalar	m3, m4;
		};
		eiScalar		comp[4];
		eiScalar		m[2][2];
	};
} eiMatrix2;

eiFORCEINLINE void setm2(eiMatrix2 *r, 
						const eiScalar _m1, const eiScalar _m2, 
						const eiScalar _m3, const eiScalar _m4)
{
	r->m1 = _m1; r->m2 = _m2;
	r->m3 = _m3; r->m4 = _m4;
}

eiFORCEINLINE void movm2(eiMatrix2 *a, const eiMatrix2 *b)
{
	memcpy(a, b, sizeof(eiMatrix2));
}

eiFORCEINLINE void inv2(eiMatrix2 *r, const eiMatrix2 *a)
{
	eiScalar	det;

	det = (a->m1 * a->m4 - a->m2 * a->m3);

	if (almost_zero(det, eiSCALAR_EPS))
	{
		r->m1 = 1.0f; r->m2 = 0.0f;
		r->m3 = 0.0f; r->m4 = 1.0f;
		return;
	}

	det = 1.0f / det;

	r->m1 = a->m4 * det;
	r->m2 = -a->m2 * det;
	r->m3 = -a->m3 * det;
	r->m4 = a->m1 * det;
}

eiFORCEINLINE void inv2i(eiMatrix2 *a)
{
	eiMatrix2 new_a;
	inv2(&new_a, a);
	movm2(a, &new_a);
}

#ifdef __cplusplus
}
#endif

#endif
