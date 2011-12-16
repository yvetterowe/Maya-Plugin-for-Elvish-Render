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

ENVIRONMENT(simple_env)

	PARAM(color, env_color);

	void parameters(int pid)
	{
		DECLARE_COLOR(env_color, 0.5f, 0.5f, 0.5f);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		Ci() = env_color();
		Oi() = color(0.0f);
	}

END(simple_env)
