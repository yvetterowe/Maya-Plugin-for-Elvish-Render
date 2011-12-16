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

SURFACE(uber)

	PARAM(color, Kd);
	PARAM(color, Ks);
	PARAM(scalar, roughness);
	PARAM(scalar, ior);
	PARAM(int, reflection_samples);

	void parameters(int pid)
	{
		DECLARE_COLOR(Kd, 0.05f, 0.05f, 0.05f);
		DECLARE_COLOR(Ks, 0.05f, 0.05f, 0.05f);
		DECLARE_SCALAR(roughness, 0.02f);
		DECLARE_SCALAR(ior, 1.5f);
		DECLARE_INT(reflection_samples, 4);
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
		vector V = -normalize(I());

		Ci() = 0.0f;

		vector wo = to_local(V);
		Lambert Rd;
		Blinn Rs(1.0f / MAX(0.001f, roughness()));
		
		while (illuminance(P(), Nf, PI / 2.0f))
		{
			color	lastC = 0.0f;
			color	localC = 0.0f;
			int		num_samples = 0;
			
			while (sample_light())
			{
				vector wi = to_local(normalize(L()));

				localC += Cl() * (
					Kd() * Rd.bsdf(wo, wi) 
					+ Ks() * fresnel_dielectric(halfangle_cosine(wo, wi), ior(), 1.0f) * 
					microfacet_term(wo, wi) * 
					Rs.bsdf(wo, wi)
					);

				++ num_samples;
				
				if ((num_samples % 4) == 0)
				{
					color	thisC = localC * (1.0f / (scalar)num_samples);
					
					if (converged(thisC, lastC))
					{
						break;
					}

					lastC = thisC;
				}
			}

			localC *= (1.0f / (scalar)num_samples);
			Ci() += localC;
		}

		if (!is_black(Ks()))
		{
			uint numReflSamples = reflection_samples();
			geoscalar u[2];
			uint count = 0;
			color Cr = 0.0f;
			uint sampledRays = 0;

			while (sample(u, &count, 2, &numReflSamples))
			{
				vector refl_dir;
				if (Rs.sample_bsdf(wo, refl_dir, (scalar)u[0], (scalar)u[1]))
				{
					Cr += Ks() * 
						fresnel_dielectric(halfangle_cosine(wo, refl_dir), ior(), 1.0f) * 
						microfacet_term(wo, refl_dir) * 
						Rs.bsdf(wo, refl_dir) * 
						trace_reflection(from_local(refl_dir)) / Rs.pdf(wo, refl_dir);
					++ sampledRays;
				}
			}
			Ci() += Cr * (1.0f / (scalar)MAX(1, sampledRays));
		}

		Ci() += Kd() * irradiance();
		Oi() = color(1.0f);
	}

END(uber)
