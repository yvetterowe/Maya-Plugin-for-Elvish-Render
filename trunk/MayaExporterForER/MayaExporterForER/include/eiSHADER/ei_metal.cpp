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

SURFACE(metal)

	PARAM(color, Cs);
	PARAM(color, eta);
	PARAM(color, k);
	PARAM(scalar, roughness);
	PARAM(int, reflection_samples);

	void parameters(int pid)
	{
		DECLARE_COLOR(Cs, 0.15f, 0.15f, 0.1f);
		DECLARE_COLOR(eta, 1.2f, 1.2f, 1.2f);
		DECLARE_COLOR(k, 2.0f, 2.0f, 2.0f);
		DECLARE_SCALAR(roughness, 0.02f);
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
		Blinn R(1.0f / MAX(0.001f, roughness()));
		
		while (illuminance(P(), Nf, PI / 2.0f))
		{
			color	lastC = 0.0f;
			color	localC = 0.0f;
			int		num_samples = 0;
			
			while (sample_light())
			{
				vector wi = to_local(normalize(L()));

				localC += Cs() * Cl() * 
					fresnel_conductor(halfangle_cosine(wo, wi), eta(), k()) * 
					microfacet_term(wo, wi) * 
					R.bsdf(wo, wi);

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

		uint numReflSamples = reflection_samples();
		geoscalar u[2];
		uint count = 0;
		color Cr = 0.0f;
		uint sampledRays = 0;

		while (sample(u, &count, 2, &numReflSamples))
		{
			vector refl_dir;
			if (R.sample_bsdf(wo, refl_dir, (scalar)u[0], (scalar)u[1]))
			{
				Cr += Cs() * 
					fresnel_conductor(halfangle_cosine(wo, refl_dir), eta(), k()) * 
					microfacet_term(wo, refl_dir) * 
					R.bsdf(wo, refl_dir) * 
					trace_reflection(from_local(refl_dir)) / R.pdf(wo, refl_dir);
				++ sampledRays;
			}
		}
		Ci() += Cr * (1.0f / (scalar)MAX(1, sampledRays));

		Oi() = color(1.0f);
	}

END(metal)
