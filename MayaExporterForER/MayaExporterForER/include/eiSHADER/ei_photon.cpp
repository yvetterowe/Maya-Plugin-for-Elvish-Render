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

#define EI_EPS		0.0001f

PHOTON(simple_photon)

	PARAM(color, diffuse);
	PARAM(color, specular);
	PARAM(scalar, transp);
	PARAM(scalar, ior);

	void parameters(int pid)
	{
		DECLARE_COLOR(diffuse, 0.5f, 0.5f, 0.5f);
		DECLARE_COLOR(specular, 0.3f, 0.3f, 0.3f);
		DECLARE_SCALAR(transp, 0.0f);
		DECLARE_SCALAR(ior, 1.5f);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		color	m_diffuse, m_specular;
		scalar	m_transp, m_ior;

		m_diffuse = diffuse();

		// store this photon if it's a diffuse surface
		if (m_diffuse.r > EI_EPS || m_diffuse.g > EI_EPS || m_diffuse.b > EI_EPS)
		{
			store_photon();
		}

		m_specular = specular();
		m_transp = transp();

		switch (choose_simple_scatter_type(m_transp, m_diffuse, m_specular))
		{
		case eiPHOTON_REFLECT_SPECULAR:
			{
				photon_reflect_specular(Ci() * m_specular, reflect_specular());
			}
			break;

		case eiPHOTON_REFLECT_DIFFUSE:
			{
				geoscalar	s[2];

				sample(s, NULL, 2, NULL);

				photon_reflect_diffuse(Ci() * m_diffuse, reflect_diffuse((scalar)s[0], (scalar)s[1]));
			}
			break;

		case eiPHOTON_TRANSMIT_SPECULAR:
			{
				m_ior = ior();

				if (m_ior == 1.0f)
				{
					photon_transparent(Ci() * m_specular);
				}
				else
				{
					photon_transmit_specular(Ci() * m_specular, transmit_specular(m_ior));
				}
			}
			break;

		default:
			break;
		}
	}

END(simple_photon)

EMITTER(point_emitter)

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
		geoscalar	samples[2];

		sample(samples, NULL, 2, NULL);

		E() = origin();
		I() = sample_sphere(E(), 1.0f, (scalar)samples[0], (scalar)samples[1]) - E();
	}

END(point_emitter)

EMITTER(sphere_emitter)

	PARAM(scalar, radius);

	void parameters(int pid)
	{
		DECLARE_SCALAR(radius, 1.0f);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		geoscalar	samples[4];

		sample(samples, NULL, 4, NULL);

		E() = sample_sphere(origin(), radius(), (scalar)samples[0], (scalar)samples[1]);
		I() = sample_hemisphere(E(), normalize(E() - origin()), 1.0f, (scalar)samples[2], (scalar)samples[3]) - E();
	}

END(sphere_emitter)
