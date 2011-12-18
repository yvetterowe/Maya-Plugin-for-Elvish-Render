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

#ifndef EI_RGBE_H
#define EI_RGBE_H

/** \brief Definition of an rgbe class. Used to store irradiance values 
 * with high dynamic range in only 32 bits.
 * \file ei_rgbe.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_util.h>
#include <eiCORE/ei_vector.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The RGBE color class */
typedef struct eiRGBE {
	eiByte		rgbe[4];
} eiRGBE;

eiFORCEINLINE void setRGBE(eiRGBE *rgbe, const eiVector *rgb)
{
	eiScalar v;
	eiInt e;

	v = rgb->r;
	if (rgb->g > v)
	{
		v = rgb->g;
	}
	if (rgb->b > v)
	{
		v = rgb->b;
	}
	if (v < 1e-32f)
	{
		rgbe->rgbe[0] = rgbe->rgbe[1] = rgbe->rgbe[2] = rgbe->rgbe[3] = 0;
	}
	else
	{
		v = frexpf(v, &e) * 256.0f / v;
		rgbe->rgbe[0] = (eiByte)(rgb->r * v);
		rgbe->rgbe[1] = (eiByte)(rgb->g * v);
		rgbe->rgbe[2] = (eiByte)(rgb->b * v);
		rgbe->rgbe[3] = (eiByte)(e + 128);
	}
}

/* Note: Ward uses ldexp(col+0.5,exp-(128+8)). However we wanted pixels 
   in the range [0,1] to map back into the range [0,1]. */
eiFORCEINLINE void getRGBE(eiVector *rgb, const eiRGBE *rgbe)
{
	eiScalar f;

	if (rgbe->rgbe[3])
	{
		/* nonzero pixel */
		f = ldexpf(1.0f, rgbe->rgbe[3] - (eiInt)(128 + 8));
		rgb->r = rgbe->rgbe[0] * f;
		rgb->g = rgbe->rgbe[1] * f;
		rgb->b = rgbe->rgbe[2] * f;
	}
	else
	{
		rgb->r = rgb->g = rgb->b = 0.0f;
	}
}

#ifdef __cplusplus
}
#endif

#endif
