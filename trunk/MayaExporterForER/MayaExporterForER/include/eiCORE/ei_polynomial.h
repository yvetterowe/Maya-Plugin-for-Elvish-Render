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

#ifndef EI_POLYNOMIAL_H
#define EI_POLYNOMIAL_H

#include <eiCORE/ei_util.h>

#ifdef __cplusplus
extern "C" {
#endif

#define cbrt(x) \
	((x) > 0.0 ? pow(x, 1.0/3.0) : \
	((x) < 0.0 ? -pow(-x, 1.0/3.0) : 0.0))

eiFORCEINLINE eiInt solve_quadric(eiScalar a, eiScalar b, eiScalar c, eiScalar r[2])
{
	if (a != 0)
	{
		const double delta = b*b - 4*a*c;

		if (delta < 0)
		{
			return 0;
		}
		else if (delta == 0)
		{
			r[0] = (eiScalar)(-b) / (2*a);

			return 1;
		}
		else
		{
			double sqrtDelta = sqrt(delta);

			r[0] = (eiScalar)(-sqrtDelta - b) / (2*a);
			r[1] = (eiScalar)(sqrtDelta - b) / (2*a);

			return 2;
		}
	}
	else if (b != 0)
	{
		r[0] = -c/b;

		return 1;
	}
	else
	{
		return 0;
	}
}

eiFORCEINLINE eiInt solve_cubic(eiScalar c[4], eiScalar s[3])
{
    eiInt	i, num;
    double	sub;
    double	A, B, C;
    double	sq_A, p, q;
    double	cb_p, D;
	double	sd[3];

    A = c[2] / c[3];
    B = c[1] / c[3];
    C = c[0] / c[3];

    sq_A = A * A;
    p = (1.0/3 * (-1.0/3.0 * sq_A + B));
    q = (1.0/2 * (2.0/27.0 * A * sq_A - 1.0/3.0 * A * B + C));

    cb_p = p * p * p;
    D = q * q + cb_p;

    if (D == 0)
	{
		if (q == 0)
		{
			sd[0] = 0;
			num = 1;
		}
		else
		{
			double u = (cbrt(-q));
			sd[0] = 2 * u;
			sd[1] = -u;
			num = 2;
		}
    }
	else if (D < 0)
	{
		double phi = (1.0/3.0 * acos(-q / sqrt(-cb_p)));
		double t = (2.0 * sqrt(-p));

		sd[0] =   t * cos(phi);
		sd[1] = - t * cos(phi + eiPI / 3.0);
		sd[2] = - t * cos(phi - eiPI / 3.0);
		num = 3;
    }
	else
	{
		double sqrt_D = sqrt(D);
		double u = cbrt(sqrt_D - q);
		double v = -cbrt(sqrt_D + q);

		sd[0] = (u + v);
		num = 1;
    }

    sub = (1.0/3.0 * A);

    for (i = 0; i < num; ++i)
	{
		s[i] = (eiScalar)(sd[i] - sub);
	}

    return num;
}

#ifdef __cplusplus
}
#endif

#endif
