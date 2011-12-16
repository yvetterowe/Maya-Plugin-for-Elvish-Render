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

SURFACE(volume_shell)

	void parameters(int pid)
	{
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		Ci() = trace_transparent();
	}

END(volume_shell)

SHADOW(shadowvol_shell)

	void parameters(int pid)
	{
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
	}

END(shadowvol_shell)

VOLUME(simple_volume)

	PARAM(color, vol_color);
	PARAM(scalar, vol_density);
	PARAM(scalar, step_size);
	PARAM(scalar, vol_radius);
	PARAM(vector, vol_offset);
	PARAM(scalar, absorption);
	PARAM(scalar, anisotropic);

	void parameters(int pid)
	{
		DECLARE_COLOR(vol_color, 0.9f, 0.95f, 1.0f);
		DECLARE_SCALAR(vol_density, 5.0f);
		DECLARE_SCALAR(step_size, 0.2f);
		DECLARE_SCALAR(vol_radius, 3.0f);
		DECLARE_VECTOR(vol_offset, 0.0f, 0.0f, 0.0f);
		DECLARE_SCALAR(absorption, 0.3f);
		DECLARE_SCALAR(anisotropic, 0.4f);
	}

	scalar turbulence(const point & p)
	{
		scalar s = 0.0f;
		point t = p;
		scalar c = 1.0f;
		for (eiInt i = 0; i < 5; ++i)
		{
			s += absf(c * noise(t));
			t *= 2.0f;
			c *= 0.5f;
		}
		return s;
	}

	scalar noiseDensity(const point & p, const scalar radius)
	{
		return turbulence(p) + (1.0f - length(p / radius));
	}

	scalar phaseSchlick(const vector & w, const vector & wp, const scalar k)
	{
		scalar kcostheta = k * (w % wp);
		return (1.0f / (4.0f * PI)) * 
			(1.0f - k*k) / MAX(eiSCALAR_EPS, ((1.0f - kcostheta) * (1.0f - kcostheta)));
	}

	color computeLighting(const vector & wp, const scalar k)
	{
		color light_color = color(0.0f);

		while (illuminance(P(), N(), PI))
		{
			color	lastC = 0.0f;
			color	localC = 0.0f;
			int		num_samples = 0;

			while (sample_light())
			{
				localC += Cl() * phaseSchlick(-normalize(L()), wp, k);

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
			light_color += localC;
		}

		return light_color;
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		scalar radius = vol_radius();
		vector offset = vol_offset();
		matrix cameraToLocal = to_object();

		int numsteps = MAX(1, int(ceilf(length(I()) / step_size())));
		scalar ds = length(I()) / (scalar)numsteps;
		vector stepdir = normalize(I()) * ds;

		P() = (P() - I()) + stepdir;

		scalar rhomult = -absorption() * ds;
		scalar T = 1.0f;

		if (get_state()->type == eiRAY_SHADOW)
		{
			for (int step = 0; step < numsteps; ++step)
			{
				scalar rho = noiseDensity(P() * cameraToLocal + offset, radius);
				scalar Ti = expf(rhomult * rho);
				T *= Ti;
				if (T < eiSCALAR_EPS)
				{
					break;
				}

				P() += stepdir;
			}

			Cl() *= T;
		}
		else
		{
			color adjustedColor = vol_color() * vol_density();
			scalar k = anisotropic();

			color C = color(0.0f);
			scalar O = 0.0f;

			for (int step = 0; step < numsteps; ++step)
			{
				scalar rho = noiseDensity(P() * cameraToLocal + offset, radius);
				scalar Ti = expf(rhomult * rho);
				T *= Ti;
				if (T < eiSCALAR_EPS)
				{
					break;
				}

				color ci = T * computeLighting(-normalize(I()), k) * adjustedColor * rho * ds;
				C += ci;
				O += (1 - Ti) * (1 - O);
				P() += stepdir;
			}

			Ci() = C;
			Oi() = O;
		}
	}

END(simple_volume)
