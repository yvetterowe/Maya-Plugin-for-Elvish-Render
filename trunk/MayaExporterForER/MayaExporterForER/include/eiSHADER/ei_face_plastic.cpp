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

SURFACE(face_plastic)

	PARAM(color, Cs);
	PARAM(color, Kd);
	PARAM(scalar, Ks);
	PARAM(scalar, roughness);
	PARAM(color, specularcolor);

	void parameters(int pid)
	{
		DECLARE_COLOR(Cs, 1.0f, 1.0f, 1.0f);
		DECLARE_COLOR(Kd, 1.0f, 1.0f, 1.0f);
		DECLARE_SCALAR(Ks, 0.5f);
		DECLARE_SCALAR(roughness, 0.1f);
		DECLARE_COLOR(specularcolor, 1.0f, 1.0f, 1.0f);
	}

	color specularbrdf(const vector & vL, const normal & vN, const vector & V, scalar roughness)
	{
		vector	H = normalize(vL + V);
		scalar	dotNH = vN % H;
		return pow(max(0.0f, dotNH), 1.0f / roughness);
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
		
		while (illuminance(P(), Nf, PI / 2.0f))
		{
			color	lastC = 0.0f;
			color	localC = 0.0f;
			int		num_samples = 0;
			
			while (sample_light())
			{
				localC += Cl() * (
					Cs() * Kd() * (normalize(L()) % Nf) 
					+ Ks() * specularcolor() * specularbrdf(normalize(L()), Nf, V, roughness()));

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

		Oi() = color(1.0f);
	}

END(face_plastic)
