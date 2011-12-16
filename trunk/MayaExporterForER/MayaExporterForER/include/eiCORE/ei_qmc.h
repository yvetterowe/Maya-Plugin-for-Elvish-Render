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

#ifndef EI_QMC_H
#define EI_QMC_H

#include <eiCORE/ei_util.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QMC_MAX_DIM					128
#define QMC_SAMPLING_PERIOD			(1 << 10)
#define INV_QMC_SAMPLING_PERIOD		((eiGeoScalar)1.0 / (eiGeoScalar)QMC_SAMPLING_PERIOD)

/** \brief This class represents a prime base for generating 
 * low-discrepancy sequence. */
typedef struct eiLDSBase {
	int				prime;
	double			inv_prime;
	unsigned short	*permutation;
} eiLDSBase;

/* these member functions are only used for precomputation */
eiFORCEINLINE void ei_lds_base_init(eiLDSBase *base, int p)
{
	base->prime = p;
	base->inv_prime = 1.0 / (double)base->prime;
	base->permutation = (unsigned short *)malloc(sizeof(unsigned short) * base->prime);
}

eiFORCEINLINE void ei_lds_base_exit(eiLDSBase *base)
{
	free(base->permutation);
}
	
eiFORCEINLINE void ei_lds_base_get_permutation(eiLDSBase *base, int **permut)
{
	int i;
	for (i = 0; i < base->prime; ++i) {
		base->permutation[i] = permut[base->prime][i];
	}
}

/* NOTICE: run the precomputing program first before including this file! */
#include "ei_qmc_data.h"

/** \brief Compute the instance number and QMC offsets from sub-pixel coordinates. */
eiFORCEINLINE void ei_sample_subpixel(eiUint *i, eiGeoScalar *x, eiGeoScalar *y, eiInt sx, eiInt sy)
{
	int	j = sx % (QMC_SAMPLING_PERIOD - 1);
	int	k = sy % (QMC_SAMPLING_PERIOD - 1);

	if (j < 0) {
		j = 0;
	}
	if( k < 0 ) {
		k = 0;
	}

	*i = j * QMC_SAMPLING_PERIOD + base_2_table[k];
	*x = (eiGeoScalar)base_2_table[k] * INV_QMC_SAMPLING_PERIOD;
	*y = (eiGeoScalar)base_2_table[j] * INV_QMC_SAMPLING_PERIOD;
}

/** \brief The sigma function. bi is the dimension, i is the instance number. */
eiFORCEINLINE eiGeoScalar ei_sigma(eiInt bi, eiInt i)
{
	int			b = lds_bases[bi].prime;
	eiGeoScalar	inv_b = (eiGeoScalar)lds_bases[bi].inv_prime;
	eiGeoScalar	x = 0.0, f = inv_b;

	while (i != 0) {
		/* TODO: this division is very expensive, fix it. */
		register int i_div_b = i / b;
		register int i_mod_b = i - i_div_b * b;
		x += f * (eiGeoScalar)lds_bases[bi].permutation[i_mod_b];
		i = i_div_b;
		f *= inv_b;
	}

	return x;
}

#ifdef __cplusplus
}
#endif

#endif
