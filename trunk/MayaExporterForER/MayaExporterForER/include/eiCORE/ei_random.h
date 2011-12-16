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

#ifndef EI_RANDOM_H
#define EI_RANDOM_H

#include <eiCORE/ei_util.h>
#include <eiCORE/ei_vector.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EI_DEFAULT_RANDOM_SEED	5489
#if (1)
/* constants for MT11213A */
#define RANDOM_N				351
#define RANDOM_M				175
#define RANDOM_R				19
#define RANDOM_MATRIX_A			0xEABD75F5
#define RANDOM_TEMU				11
#define RANDOM_TEMS				7
#define RANDOM_TEMT				15
#define RANDOM_TEML				17
#define RANDOM_TEMB				0x655E5280
#define RANDOM_TEMC				0xFFD58000
#else
/* constants for MT19937 */
#define RANDOM_N				624
#define RANDOM_M				397
#define RANDOM_R				31
#define RANDOM_MATRIX_A			0x9908B0DF
#define RANDOM_TEMU				11
#define RANDOM_TEMS				7
#define RANDOM_TEMT				15
#define RANDOM_TEML				18
#define RANDOM_TEMB				0x9D2C5680
#define RANDOM_TEMC				0xEFC60000
#endif

/** \brief A class encapsulates Mersenne random number generator, 
 * there should be one random number generator per thread. */
typedef struct eiRandomGen {
	eiUint		mt[RANDOM_N];	/* state vector */
	eiInt		mti;			/* index into mt */
} eiRandomGen;

/** \brief Re-seed the random generator. */
eiFORCEINLINE void ei_random_reset(eiRandomGen *gen, const eiInt seed)
{
	eiUint	s = (eiUint)seed;

	for (gen->mti = 0; gen->mti < RANDOM_N; ++gen->mti)
	{
		s = s * 29943829 - 1;
		gen->mt[gen->mti] = s;
	}
}

/** \brief Output 32 random bits */
eiFORCEINLINE eiUint ei_brandom(eiRandomGen *gen)
{
	eiUint	y;

	if (gen->mti >= RANDOM_N)
	{
		/* generate N words at one time */
		const eiUint	LOWER_MASK = (1u << RANDOM_R) - 1;	/* lower R bits */
		const eiUint	UPPER_MASK = -1 << RANDOM_R;		/* upper 32-R bits */
		eiInt			kk, km;

		for (kk = 0, km = RANDOM_M; kk < RANDOM_N - 1; ++kk)
		{
			y = (gen->mt[kk] & UPPER_MASK) | (gen->mt[kk+1] & LOWER_MASK);
			gen->mt[kk] = gen->mt[km] ^ (y >> 1) ^ (-(eiInt)(y & 1) & RANDOM_MATRIX_A);
			if (++km >= RANDOM_N)
			{
				km = 0;
			}
		}

		y = (gen->mt[RANDOM_N-1] & UPPER_MASK) | (gen->mt[0] & LOWER_MASK);
		gen->mt[RANDOM_N-1] = gen->mt[RANDOM_M-1] ^ (y >> 1) ^ (-(eiInt)(y & 1) & RANDOM_MATRIX_A);
		gen->mti = 0;
	}
	
	y = gen->mt[gen->mti++];
	
	/* tempering (may be omitted) */
	y ^=  y >> RANDOM_TEMU;
	y ^= (y << RANDOM_TEMS) & RANDOM_TEMB;
	y ^= (y << RANDOM_TEMT) & RANDOM_TEMC;
	y ^=  y >> RANDOM_TEML;

	return y;
}

/** \brief Output random float number in the interval 0 <= x < 1 */
eiFORCEINLINE eiGeoScalar ei_random(eiRandomGen *gen)
{
	eiUint	r;

	union {
		eiGeoScalar	f;
		eiInt		i[2];
	} convert;

	/* get 32 random bits and convert to float */
	r = ei_brandom(gen);
	
	convert.i[0] = r << 20;
	convert.i[1] = (r >> 12) | 0x3FF00000;
	
	return convert.f - 1.0;
}

/** \brief Output random integer in the interval min <= x <= max */
eiFORCEINLINE eiInt ei_irandom(eiRandomGen *gen, const eiInt min, const eiInt max)
{
	eiInt	r;
	
	/* multiply interval with random and truncate */
	r = (eiInt)((eiGeoScalar)(max - min + 1) * ei_random(gen)) + min;
	if (r > max)
	{
		r = max;
	}
	if (max < min)
	{
		return 0x80000000;
	}
	return r;
}

/* mapping uniformly distributed points in a square into misc shapes */
eiFORCEINLINE void ei_uniform_sample_hemisphere(eiVector *vec, eiScalar u1, eiScalar u2)
{
	eiScalar z = u1;
	eiScalar r = sqrtf(MAX(0.0f, 1.0f - z*z));
	eiScalar phi = 2.0f * (eiScalar)eiPI * u2;
	eiScalar x = r * cosf(phi);
	eiScalar y = r * sinf(phi);
	vec->x = x;
	vec->y = y;
	vec->z = z;
}

eiFORCEINLINE void ei_uniform_sample_sphere(eiVector *vec, eiScalar u1, eiScalar u2)
{
	eiScalar z = 1.0f - 2.0f * u1;
	eiScalar r = sqrtf(MAX(0.0f, 1.0f - z*z));
	eiScalar phi = 2.0f * (eiScalar)eiPI * u2;
	eiScalar x = r * cosf(phi);
	eiScalar y = r * sinf(phi);
	vec->x = x;
	vec->y = y;
	vec->z = z;
}

eiFORCEINLINE void ei_uniform_sample_disk(eiScalar *x, eiScalar *y, eiScalar u1, eiScalar u2)
{
	eiScalar r = sqrtf(u1);
	eiScalar theta = 2.0f * (eiScalar)eiPI * u2;
	*x = r * cosf(theta);
	*y = r * sinf(theta);
}

eiFORCEINLINE void ei_concentric_sample_disk(eiScalar *dx, eiScalar *dy, eiScalar u1, eiScalar u2)
{
	eiScalar r, theta;
	/* map uniform random numbers to [-1,1]^2 */
	eiScalar sx = 2.0f * u1 - 1.0f;
	eiScalar sy = 2.0f * u2 - 1.0f;
	/* map square to (r,theta) */
	/* handle degeneracy at the origin */
	if (sx == 0.0f && sy == 0.0f) {
		*dx = 0.0f;
		*dy = 0.0f;
		return;
	}
	if (sx >= -sy) {
		if (sx > sy) {
			/* handle first region of disk */
			r = sx;
			if (sy > 0.0f)
				theta = sy/r;
			else
				theta = 8.0f + sy/r;
		}
		else {
			/* handle second region of disk */
			r = sy;
			theta = 2.0f - sx/r;
		}
	}
	else {
		if (sx <= sy) {
			/* handle third region of disk */
			r = -sx;
			theta = 4.0f - sy/r;
		}
		else {
			/* handle fourth region of disk */
			r = -sy;
			theta = 6.0f + sx/r;
		}
	}
	theta *= (eiScalar)eiPI / 4.0f;
	*dx = r*cosf(theta);
	*dy = r*sinf(theta);
}

eiFORCEINLINE void ei_cosine_sample_hemisphere(eiVector *vec, eiScalar u1, eiScalar u2)
{
	ei_concentric_sample_disk(&vec->x, &vec->y, u1, u2);
	vec->z = sqrtf(MAX(0.0f, 1.0f - vec->x * vec->x - vec->y * vec->y));
}

eiFORCEINLINE void ei_uniform_sample_triangle(eiVector *bary, eiScalar u1, eiScalar u2)
{
	bary->x = 1.0f - sqrtf(u1);
	bary->y = u2  * sqrtf(u1);
	bary->z = 1.0f - bary->x - bary->y;
}

eiFORCEINLINE void ei_uniform_sample_cone(eiVector *vec, eiScalar u1, eiScalar u2, eiScalar costhetamax)
{
	eiScalar	costheta, sintheta, phi;
	lerp(&costheta, costhetamax, 1.0f, u1);
	sintheta = sqrtf(1.0f - costheta * costheta);
	phi = u2 * 2.0f * (eiScalar)eiPI;
	vec->x = cosf(phi) * sintheta;
	vec->y = sinf(phi) * sintheta;
	vec->z = costheta;
}

#ifdef __cplusplus
}
#endif

#endif
