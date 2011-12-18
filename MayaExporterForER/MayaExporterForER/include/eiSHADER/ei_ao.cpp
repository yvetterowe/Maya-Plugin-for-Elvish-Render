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

#include <eiAPI/ei_shaderx.h>

SURFACE(ao)

	PARAM(int, rays);
	PARAM(scalar, intensity);
	PARAM(scalar, maxdist);

	void parameters(int pid)
	{
		DECLARE_INT(rays, 16);
		DECLARE_SCALAR(intensity, 1.0f);
		DECLARE_SCALAR(maxdist, eiMAX_SCALAR);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		normal Nf = faceforward(normalize(N()), I());

		uint num_rays = max(rays(), 1);
		int num_misses = 0;
		geoscalar u[2];
		uint count = 0;

		while (sample(u, &count, 2, &num_rays))
		{
			vector dir = sample_cosine_hemisphere(point(0.0f), Nf, 1.0f, (scalar)u[0], (scalar)u[1]);
			if (!trace_probe(P(), dir, maxdist()))
			{
				++ num_misses;
			}
		}

		Ci() = color(intensity() * (scalar)num_misses / (scalar)num_rays);
		Oi() = color(1.0f);
	}

END(ao)
