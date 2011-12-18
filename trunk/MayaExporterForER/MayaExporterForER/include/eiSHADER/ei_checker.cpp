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

SURFACE(checker)

	PARAM(color, color1);
	PARAM(color, color2);
	PARAM(scalar, xscale);
	PARAM(scalar, yscale);
	PARAM(color, result);

	void parameters(int pid)
	{
		DECLARE_COLOR(color1, 0.0f, 0.0f, 0.0f);
		DECLARE_COLOR(color2, 1.0f, 1.0f, 1.0f);
		DECLARE_SCALAR(xscale, 2.0f);
		DECLARE_SCALAR(yscale, 2.0f);
		DECLARE_COLOR(result, 0.0f, 0.0f, 0.0f);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		scalar	x, y;

		x = P().x * xscale();
		y = P().y * yscale();

		if ((((eiInt)x + (eiInt)y) % 2) == 0)
		{
			result() = color1();
		}
		else
		{
			result() = color2();
		}
	}

END(checker)

SURFACE(checker_uv)

	PARAM(color, color1);
	PARAM(color, color2);
	PARAM(scalar, uscale);
	PARAM(scalar, vscale);
	PARAM(color, result);

	void parameters(int pid)
	{
		DECLARE_COLOR(color1, 0.0f, 0.0f, 0.0f);
		DECLARE_COLOR(color2, 1.0f, 1.0f, 1.0f);
		DECLARE_SCALAR(uscale, 2.0f);
		DECLARE_SCALAR(vscale, 2.0f);
		DECLARE_COLOR(result, 0.0f, 0.0f, 0.0f);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		scalar	x, y;

		x = u() * uscale();
		y = v() * vscale();

		if ((((eiInt)x + (eiInt)y) % 2) == 0)
		{
			result() = color1();
		}
		else
		{
			result() = color2();
		}
	}

END(checker_uv)

SURFACE(simple_texture)

	PARAM(eiTag, texture);
	PARAM(color, result);

	void parameters(int pid)
	{
		DECLARE_TAG(texture, eiNULL_TAG);
		DECLARE_COLOR(result, 0.0f, 0.0f, 0.0f);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		result() = color(0.0f, 0.0f, 0.0f);

		eiTag tex = texture();

		if (tex != eiNULL_TAG)
		{
			result() = color_texture(tex, 0, get_state()->bary.x, get_state()->bary.y);
		}
	}

END(simple_texture)
