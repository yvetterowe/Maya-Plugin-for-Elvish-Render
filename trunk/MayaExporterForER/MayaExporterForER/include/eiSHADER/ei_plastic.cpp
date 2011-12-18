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

SURFACE(plastic)

	PARAM(color, Cs);
	PARAM(color, Kd);
	PARAM(scalar, Ks);
	PARAM(scalar, roughness);
	PARAM(color, specularcolor);
	PARAM(eiTag, Cs_tex);

	void parameters(int pid)
	{
		DECLARE_COLOR(Cs, 1.0f, 1.0f, 1.0f);
		DECLARE_COLOR(Kd, 1.0f, 1.0f, 1.0f);
		DECLARE_SCALAR(Ks, 0.5f);
		DECLARE_SCALAR(roughness, 0.1f);
		DECLARE_COLOR(specularcolor, 1.0f, 1.0f, 1.0f);
		DECLARE_TAG(Cs_tex, eiNULL_TAG);
	}

	color specularbrdf(const vector & vL, const normal & vN, const vector & V, scalar roughness)
	{
		vector	H = normalize(vL + V);
		scalar	dotNH = vN % H;
		return pow(max(eiSCALAR_EPS, dotNH), 1.0f / roughness);
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

		eiTag tex = Cs_tex();

		if (tex != eiNULL_TAG)
		{
			Cs() = color_texture(tex, 0, get_state()->bary.x, get_state()->bary.y);
		}
		
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

		Ci() += Kd() * irradiance();
		Oi() = color(1.0f);
	}

END(plastic)

SURFACE(plastic_uv)

	PARAM(color, Cs);
	PARAM(color, Kd);
	PARAM(scalar, Ks);
	PARAM(scalar, roughness);
	PARAM(color, specularcolor);
	PARAM(eiTag, Cs_tex);

	void parameters(int pid)
	{
		DECLARE_COLOR(Cs, 1.0f, 1.0f, 1.0f);
		DECLARE_COLOR(Kd, 1.0f, 1.0f, 1.0f);
		DECLARE_SCALAR(Ks, 0.5f);
		DECLARE_SCALAR(roughness, 0.1f);
		DECLARE_COLOR(specularcolor, 1.0f, 1.0f, 1.0f);
		DECLARE_TAG(Cs_tex, eiNULL_TAG);
	}

	color specularbrdf(const vector & vL, const normal & vN, const vector & V, scalar roughness)
	{
		vector	H = normalize(vL + V);
		scalar	dotNH = vN % H;
		return pow(max(eiSCALAR_EPS, dotNH), 1.0f / roughness);
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

		eiTag tex = Cs_tex();

		if (tex != eiNULL_TAG)
		{
			scalar s = fmodf(u() * 16.0f, 1.0f);
			scalar t = fmodf(v() * 16.0f, 1.0f);
			scalar dsdx, dsdy, dtdx, dtdy;
			Dxy(u, dsdx, dsdy);
			Dxy(v, dtdx, dtdy);
			dsdx = fmodf(dsdx * 16.0f, 1.0f);
			dsdy = fmodf(dsdy * 16.0f, 1.0f);
			dtdx = fmodf(dtdx * 16.0f, 1.0f);
			dtdy = fmodf(dtdy * 16.0f, 1.0f);
			Cs() = color_texture(tex, 0, s, t, s + dsdy, t + dtdy, s + dsdx + dsdy, t + dtdx + dtdy, s + dsdx, t + dtdx);
		}
		
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

END(plastic_uv)

SURFACE(simple_hair)

	PARAM(color, Cs);
	PARAM(color, Kd);
	PARAM(scalar, Ks);
	PARAM(color, specularcolor);
	PARAM(scalar, specularpower);

	void parameters(int pid)
	{
		DECLARE_COLOR(Cs, 1.0f, 1.0f, 1.0f);
		DECLARE_COLOR(Kd, 1.0f, 1.0f, 1.0f);
		DECLARE_SCALAR(Ks, 0.5f);
		DECLARE_COLOR(specularcolor, 1.0f, 1.0f, 1.0f);
		DECLARE_SCALAR(specularpower, 60.0f);
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
		vector tangent = normalize(dPdu());
		
		while (illuminance(P(), Nf, PI))
		{
			color	lastC = 0.0f;
			color	localC = 0.0f;
			int		num_samples = 0;
			
			while (sample_light())
			{
				vector lightVector = normalize(L());
				scalar diff = sin(acos(tangent % lightVector));
				scalar spec = pow((tangent % lightVector) * (tangent % V) 
					+ sin(acos(lightVector % tangent)) * sin(acos(tangent % V)), specularpower());

				localC += Cl() * (Kd() * diff + Ks() * specularcolor() * spec);

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

		Ci() *= Cs();
		Ci() += Kd() * irradiance();
		Oi() = color(1.0f);
	}

END(simple_hair)

DISPLACEMENT(simple_displace)

	PARAM(eiTag, disp_tex);
	PARAM(scalar, disp_dist);

	void parameters(int pid)
	{
		DECLARE_TAG(disp_tex, eiNULL_TAG);
		DECLARE_SCALAR(disp_dist, 1.0f);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		eiTag tex = disp_tex();
		scalar dist = disp_dist();
		//scalar offset = 0.0f;

		//if (tex != eiNULL_TAG)
		//{
			//offset = scalar_texture(tex, 0, get_state()->bary.x, get_state()->bary.y);
		//}

		//P() = P() + dist * offset * N();
		P() = P() + dist * noise(P());
	}

END(simple_displace)
