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

SURFACE(mirror)

	PARAM(color, Kr);

	void parameters(int pid)
	{
		DECLARE_COLOR(Kr, 0.9f, 0.9f, 0.9f);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		vector V = -normalize(I());

		Ci() = 0.0f;

		vector wo = to_local(V);

		SpecularReflection R;
		vector reflected_dir;

		if (!is_black(Kr()) && R.sample_bsdf(wo, reflected_dir, 0.0f, 0.0f))
		{
			color bsdf = R.bsdf(wo, reflected_dir);
			bsdf = 1.0f;
			Ci() += Kr() * bsdf * trace_reflection(from_local(reflected_dir)) / R.pdf(wo, reflected_dir);
		}

		Oi() = color(1.0f);
	}

END(mirror)
