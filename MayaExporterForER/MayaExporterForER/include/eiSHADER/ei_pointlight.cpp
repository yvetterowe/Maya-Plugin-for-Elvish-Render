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

LIGHT(pointlight)

	PARAM(scalar, intensity);
	PARAM(color, lightcolor);

	void parameters(int pid)
	{
		DECLARE_SCALAR(intensity, 1.0f);
		DECLARE_COLOR(lightcolor, 1.0f, 1.0f, 1.0f);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		while (illuminate(origin()))
		{
			Cl() = trace_shadow(origin(), L(), intensity() * lightcolor());
		}
	}

END(pointlight)
