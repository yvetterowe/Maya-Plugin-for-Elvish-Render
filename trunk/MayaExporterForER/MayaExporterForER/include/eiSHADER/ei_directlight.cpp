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

LIGHT(directlight)

	PARAM(scalar, intensity);
	PARAM(color, lightcolor);
	PARAM(scalar, deltacone);
	PARAM(vector, direction);
	PARAM(scalar, spread);

	void parameters(int pid)
	{
		DECLARE_SCALAR(intensity, 1.0f);
		DECLARE_COLOR(lightcolor, 1.0f, 1.0f, 1.0f);
		DECLARE_SCALAR(deltacone, 1.0f);
		DECLARE_VECTOR(direction, 0.0f, 0.0f, 1.0f);
		DECLARE_SCALAR(spread, 0.0f);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		vector dir = normalize(direction() * from_light());

		while (solar(dir, 0.0f))
		{
			point Lp;
			point_on_plane((eiVector *)(&Lp), (eiVector *)(&Ps()), (eiVector *)(&dir), (eiVector *)(&origin()));
			scalar Ld = point_plane_dist((eiVector *)(&Ps()), (eiVector *)(&dir), (eiVector *)(&origin()));

			if (Ld < 0.0f)
			{
				Cl() = color(0.0f);
			}
			else
			{
				scalar atten = 1.0f;
				if (spread() > eiSCALAR_EPS)
				{
					atten = 1.0f - smoothstep(spread() - deltacone(), spread(), distance(Lp, origin()));
				}
				Cl() = trace_shadow(Lp, L() * Ld, atten * intensity() * lightcolor());
			}
		}
	}

END(directlight)
